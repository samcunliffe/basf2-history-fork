//
// Created by abaur on 02.05.18.
//

#include <framework/pcore/ProcHelper.h>
#include <framework/pcore/zmq/processModules/ZMQDefinitions.h>
#include <framework/pcore/zmq/processModules/ZMQHelper.h>
#include <framework/pcore/PathUtils.h>

#include <framework/pcore/pEventProcessor.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/RxModule.h>
#include <framework/pcore/TxModule.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/RbTuple.h>

#include <framework/core/ModuleManager.h>
#include <framework/core/Environment.h>
#include <framework/logging/LogSystem.h>

#include <TROOT.h>

#include <chrono>
#include <thread>

#include <signal.h>
#include <fstream>

using namespace std;
using namespace Belle2;

namespace {
  /**
   * Signal handlers do not work with member functions. So we need to "wrap" them in this top level function.
   * Unfortunately, we will then loose the object information, so we store it in this global variable (eval!)
   * We also store the signal we have received here
   */

  /// Received signal via a signal handler
  static int g_signalReceived = 0;

  /// Memory for the current (single!) instance for the signal handler
  static pEventProcessor* g_eventProcessorForSignalHandling = nullptr;

  static void cleanupAndRaiseSignal(int signalNumber)
  {
    if (g_eventProcessorForSignalHandling) {
      g_eventProcessorForSignalHandling->cleanup();
    }
    // uninstall current handler and call default one.
    signal(signalNumber, SIG_DFL);
    raise(signalNumber);
  }

  static void cleanupAndStoreSignal(int signalNumber)
  {
    if (signalNumber == SIGINT) {
      EventProcessor::writeToStdErr("\nStopping basf2 gracefully...\n");
      if (g_eventProcessorForSignalHandling) {
        g_eventProcessorForSignalHandling->cleanup();
      }
    }

    // Well, what do we do in the other cases? We probably just die...

    // We do not want to remove the first signal
    if (g_signalReceived == 0) {
      g_signalReceived = signalNumber;
    }
  }
} // namespace

pEventProcessor::pEventProcessor(const std::string& socketAddress) : EventProcessor(),
  m_socketAddress(socketAddress)
{
  B2ASSERT("You are having two instances of the pEventProcessor running! This is not possible",
           not g_eventProcessorForSignalHandling);
  g_eventProcessorForSignalHandling = this;
}

pEventProcessor::~pEventProcessor()
{
  cleanup();
  g_eventProcessorForSignalHandling = nullptr;
}

void pEventProcessor::process(PathPtr path, long maxEvent)
{
  if (path->isEmpty()) {
    return;
  }

  const int numProcesses = Environment::Instance().getNumberProcesses();
  if (numProcesses == 0) {
    B2FATAL("pEventProcessor::process() called for serial processing! Most likely a bug in Framework.");
  }

  // Split the path into input, main and output. A nullptr means, the path should not be used
  PathPtr inputPath, mainPath, outputPath;
  std::tie(inputPath, mainPath, outputPath) = PathUtils::splitPath(path);
  const ModulePtr& histogramManager = PathUtils::getHistogramManager(inputPath, mainPath, outputPath);

  if (not mainPath or mainPath->isEmpty()) {
    B2WARNING("Cannot run any modules in parallel (no c_ParallelProcessingCertified flag), falling back to single-core mode.");
    EventProcessor::process(path, maxEvent);
    return;
  }

  // inserts Rx/Tx modules into path (sets up IPC structures)
  const ModulePtrList& moduleList = PathUtils::preparePaths(inputPath, mainPath, outputPath, m_socketAddress);

  // Run the initialization of the modules and the histogram manager
  initialize(moduleList, histogramManager);

  // The main part: fork into the different processes and run!
  const ModulePtrList& terminateGlobally = PathUtils::getTerminateGloballyModules(moduleList);
  forkAndRun(maxEvent, inputPath, mainPath, outputPath, terminateGlobally);

  // Run the final termination and cleanup with error check
  terminateAndCleanup(histogramManager);
}

void pEventProcessor::initialize(const ModulePtrList& moduleList, const ModulePtr& histogramManager)
{
  // signal handler: cleanup everything and raise the signal afterwards
  installMainSignalHandlers(cleanupAndRaiseSignal);

  if (histogramManager) {
    histogramManager->initialize();
  }

  // init statistics
  m_processStatisticsPtr.registerInDataStore();
  if (!m_processStatisticsPtr)
    m_processStatisticsPtr.create();

  //add modules to statistics
  for (ModulePtr module : moduleList) {
    m_processStatisticsPtr->initModule(module.get());
  }

  // from now datastore available
  processInitialize(moduleList, true);

  //Don't start processing in case of no master module
  if (!m_master) {
    B2ERROR("There is no module that provides event and run numbers. You must either add the EventInfoSetter module to your path, or, if using an input module, read EventMetaData objects from file.");
  }

  // Check if errors appeared. If yes, don't start the event processing.
  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != 0) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  // TODO: I do not really understand what is going on here...
  /** TFiles are stored in a global list and cleaned up by root
   * since this will happen in all forked processes, these will be corrupted if we don't clean the list!
   *
   * needs to be called at the end of every process.
   */
  // disable ROOT's management of TFiles
  // clear list, but don't actually delete the objects
  gROOT->GetListOfFiles()->Clear("nodelete");
}

void pEventProcessor::terminateAndCleanup(const ModulePtr& histogramManager)
{
  // No matter what the user does, we want to do this cleanup...
  installSignalHandler(SIGINT, SIG_IGN);

  cleanup();

  if (histogramManager) {
    B2INFO("HistoManager:: adding histogram files");
    RbTupleManager::Instance().hadd();
  }

  // did anything bad happen?
  if (g_signalReceived) {
    if (g_signalReceived == SIGINT) {
      B2RESULT("Processing aborted via signal " << g_signalReceived <<
               ", terminating. Output files have been closed safely and should be readable.");
    } else {
      B2ERROR("Processing aborted via signal " << g_signalReceived <<
              ", terminating. Output files have been closed safely and should be readable.");
    }
    // re-raise the signal
    installSignalHandler(g_signalReceived, SIG_DFL);
    raise(g_signalReceived);
  }
}

void pEventProcessor::forkAndRun(long maxEvent, const PathPtr& inputPath, const PathPtr& mainPath, const PathPtr& outputPath,
                                 const ModulePtrList& terminateGlobally)
{
  const int numProcesses = Environment::Instance().getNumberProcesses();
  // the handler for forking and handling all processes
  m_procHandler.reset(new ProcHandler(numProcesses));

  // install new signal handlers before forking: do not raise signal but just store it
  installMainSignalHandlers(cleanupAndStoreSignal);
  //Path for current process
  PathPtr localPath;

  // =====================
  // 3. Fork proxy process
  // =====================
  const auto pubSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_pub));
  const auto subSocketAddress(ZMQHelper::getSocketAddress(m_socketAddress, ZMQAddressType::c_sub));

  m_procHandler->startProxyProcess(pubSocketAddress, subSocketAddress);
  if (m_procHandler->isProcess(ProcType::c_Proxy)) {
    // proxy is blocking
  } else {

    m_procHandler->initPCBMulticast(pubSocketAddress, subSocketAddress); // the multicast for the monitoring process
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_helloMessage);
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_deathMessage);     // worker run in process timeout
    m_procHandler->subscribePCBMulticast(c_MessageTypes::c_terminateMessage); // input is in terminate mode -> no restart of worker

    B2DEBUG(100, "Multicast for Init Process was set up");

    // =====================
    // 4. Fork input path
    // =====================
    m_procHandler->startInputProcess();
    if (m_procHandler->isProcess(ProcType::c_Input)) {
      // input process gets the path of input modules
      if (inputPath and not inputPath->isEmpty()) {
        localPath = inputPath;
      }
    } else {
      // This is not the input process, clean up datastore to not contain the first event
      DataStore::Instance().invalidateData(DataStore::c_Event);

      // =================================
      // 5. Fork out output path
      // =================================
      m_procHandler->startOutputProcess();
      if (m_procHandler->isProcess(ProcType::c_Output)) {
        if (outputPath and not outputPath->isEmpty()) {
          localPath = outputPath;
          m_master = localPath->getModules().begin()->get(); //set Rx as master
        }
      } else {
        if (m_procHandler->waitForStartEvtProc()) {
          B2INFO("Input and Output online, start event processing...");

          // ===========================================
          // 6. Fork out worker path (parallel section)
          // ===========================================
          m_procHandler->startWorkerProcesses();
          if (m_procHandler->isProcess(ProcType::c_Worker)) {
            localPath = mainPath;
            if (inputPath and not inputPath->isEmpty()) {
              m_master = localPath->getModules().begin()->get(); //set Rx as master
            }
          } else {
            // still in parent process: the init process becomes now the monitor process
            m_procHandler->setAsMonitoringProcess();
          }
        } else {
          // TODO : on fail need to kill all child processes ?
          //m_procHandler->killAllChildProc();
          B2FATAL("Not able to start event processing... aborting");
        }
      }
    }
  }

  if (m_procHandler->isProcess(ProcType::c_Monitor)) {
    B2RESULT("Running as " << m_procHandler->getProcessName());

    // self healing mode will restart died workers
    bool selfHealing = true;
    // Monitoring process ignores SIGINT, SIGTERM, SIGQUIT
    B2INFO("Waiting for all processes to finish.");
    if (selfHealing) {
      while (m_procHandler->checkProcessStatus() && g_signalReceived == 0) {
        bool workerDied = m_procHandler->proceedPCBMulticast();
        // ===========================================
        // 7. Restart died workers
        // ===========================================
        if (workerDied) {
          B2RESULT("Restart worker");
          m_procHandler->restartWorkerProcess();
          if (m_procHandler->isProcess(ProcType::c_Worker)) {
            localPath = mainPath;
            if (inputPath and not inputPath->isEmpty()) {
              m_master = localPath->getModules().begin()->get(); //set Rx as master
            }
            break;
          }
        }
      }
    } else {
      m_procHandler->waitForAllProcesses();
    }
  } else
    installMainSignalHandlers(); // Main signals have no effect, still do this in prochandler::startproc

  // This is very all processes and up:
  if (not m_procHandler->isProcess(ProcType::c_Output) and not m_procHandler->isProcess(ProcType::c_Monitor)) {
    DataStoreStreamer::removeSideEffects();
  }

  bool gotSigINT = false;

  if (localPath != nullptr) {
    // if not monitoring process then process the module paths
    B2RESULT("Running as " << m_procHandler->getProcessName());
    ModulePtrList localModules = localPath->buildModulePathList();
    try {
      // ======================================
      // 8. here all the modules are processed
      // ======================================

      maxEvent = getMaximumEventNumber(maxEvent);
      processCore(localPath, localModules, maxEvent, m_procHandler->isProcess(ProcType::c_Input));

      B2INFO("After process core");
    } catch (StoppedBySignalException& e) {
      if (e.signal != SIGINT) {
        B2FATAL(e.what());
      }
      B2INFO("Stopped by exception");
      //in case of SIGINT, we move on to processTerminate() to shut down safely
      gotSigINT = true;
    }

    B2DEBUG(100, "terminate process...");
    PathUtils::prependModulesIfNotPresent(&localModules, terminateGlobally);
    // process the safe shutdown
    processTerminate(localModules);
  }

  // ============================================
  // 9. all processes stop here except monitor
  // ============================================
  if (not m_procHandler->isProcess(ProcType::c_Monitor)) {
    B2INFO(m_procHandler->getProcessName() << " process finished.");
    exit(0);
  }
}

void pEventProcessor::cleanup()
{
  std::cerr << "Running cleanup" << std::endl;
  // TODO: what to do if no process type?
  if (m_procHandler and (!m_procHandler->parallelProcessingUsed() or m_procHandler->isProcess(ProcType::c_Monitor))) {
    std::cerr << "Trying to kill everyone" << std::endl;
    //TODO: enter here the PCB stuff
    if (m_multicastOnline)
      m_procHandler->stopEvtProc();
    // interrupts the proxy and kills all left processes, timeout to hard kill
    m_procHandler->killAllChildProc(5);
  }
}