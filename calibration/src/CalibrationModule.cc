/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka (tadeas.bilka@gmail.com)                    *
 *               Sergey Yashchenko (sergey.yaschenko@desy.de)             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <calibration/CalibrationModule.h>
#include <calibration/CalibrationManager.h>
#include <framework/core/Environment.h>
#include <framework/pcore/ProcHandler.h>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace Belle2;
using namespace calibration;

//-----------------------------------------------------------------
//     DO NOT            Register the Module
//-----------------------------------------------------------------
//REG_MODULE(Calibration)

CalibrationModule::CalibrationModule() : Module(),
  m_mode("offline"),
  m_state(CalibrationModule::c_Waiting),
  m_iterationNumber(0),
  m_datasetCategory(""),
  m_granularityOfCalibration("run"),
  m_numberOfIterations(0),
  m_calibrationFileName(""),
  //m_calibrationModuleInitialized(false),
  //m_calibrationManagerInitialized(false),
  m_dependencyList("")
{

  // This describtion has to overriden by implementing module
  setDescription("A standard calibration module");

  // Default parameters of the base module
  addParam("datasetCategory", m_datasetCategory,
           "Used dataset category", string("data"));
  addParam("granularityOfCalibration", m_granularityOfCalibration,
           "Granularity of calibration: run, experiment, or other", string("run"));
  addParam("numberOfIterations", m_numberOfIterations,
           "Maximal number of iterations", int(1));
  addParam("calibrationFileName", m_calibrationFileName, "Name of the calibration output file", string("CalibrationFile.root"));

  m_dependencies.clear();
}

CalibrationModule::~CalibrationModule()
{
  if (ProcHandler::EvtProcID() == -1) {   // should be called from main proc.
    if (Environment::Instance().getNumberProcesses() > 0 && ProcHandler::EvtProcID() == -1) {
      // Adding histogram files
      // Add an option later in case the calibration file is not a root tree/histogram
      //calibration::CalibrationManager::Instance().hadd();
    }
  }
}

void CalibrationModule::initialize()
{
  // First parse the dependency list
  std::vector<std::string> dependencies;
  // Split by ","

  if (m_dependencyList != "") {
    boost::algorithm::split(dependencies, m_dependencyList, boost::is_any_of(","));
    for (auto dependency : dependencies) {
      boost::trim(dependency);
      std::vector<std::string> moduleAndState;
      // split by ":"
      boost::split(moduleAndState, dependency, boost::is_any_of(":"));
      if (moduleAndState.size() == 0 || moduleAndState.size() > 2)
        B2FATAL("Error in parsing module dependencies.");

      std::string module("");
      std::string state("");

      // We allow to ommit ":state" - in such case we assume ":done"
      if (moduleAndState.size() == 1) {
        module = moduleAndState[0];
        state = "done";
      } else {
        module = moduleAndState[0];
        state = moduleAndState[1];
      }
      boost::trim(module);
      boost::trim(state);
      ModuleDependency moduleDependency = std::make_pair(module, CalibrationModule::stringToState(state));
      m_dependencies.push_back(moduleDependency);
    }
  }

  CalibrationManager::getInstance().register_module(this);
  //calibration::CalibrationManager::Instance().init(Environment::Instance().getNumberProcesses(), m_CalibrationFileName.c_str());
  // calibManager.setNumberOfIterations(m_numberOfIterations);
  Prepare();
  m_iterationNumber = 0;
}

void CalibrationModule::beginRun()
{
  //if (!CalibrationManager::getInstance().checkDependencies(this))
  //  B2FATAL("Dependencies not fullfiled");
}

void CalibrationModule::endRun()
{
  if (m_granularityOfCalibration == "run") {
    ECalibrationModuleResult calibrationResult;
    calibrationResult = Calibrate();
    if (calibrationResult == c_Success) {
      StoreInDataBase();
      setState(CalibrationModule::c_Monitoring);
      ECalibrationModuleMonitoringResult result = Monitor();

      if (result == c_MonitoringSuccess)
        setState(CalibrationModule::c_Done);
      else if (result == c_MonitoringIterateCalibration)
        setState(CalibrationModule::c_Running);
      else if (result == c_MonitoringIterateMonitor)
        setState(CalibrationModule::c_Running);
      else
        setState(CalibrationModule::c_Failed);

    } else {
      if (calibrationResult == calibration::CalibrationModule::c_Failure)
        B2ERROR("Calibration failed.");
      if (calibrationResult == calibration::CalibrationModule::c_NotEnoughData)
        B2ERROR("Insufficient data for calibration.");
      if (calibrationResult == calibration::CalibrationModule::c_NoChange)
        B2INFO("No change in calibration.");
    }
    m_iterationNumber++;
  }
}

void CalibrationModule::event()
{
  if (getState() == CalibrationModule::c_Running)
    CollectData();

}

void CalibrationModule::terminate()
{
  //calibration::CalibrationManager::Instance().terminate();

}

void CalibrationModule::Prepare() {}

void CalibrationModule::CollectData() {}

CalibrationModule::ECalibrationModuleResult CalibrationModule::Calibrate() { return calibration::CalibrationModule::c_Success;}

CalibrationModule::ECalibrationModuleMonitoringResult CalibrationModule::Monitor() { return calibration::CalibrationModule::c_MonitoringSuccess;}

bool CalibrationModule::StoreInDataBase() { return true;}

CalibrationModule::ECalibrationModuleState CalibrationModule::stringToState(string state)
{
  std::map<ECalibrationModuleState, std::string> stateNames;
  stateNames.insert(std::make_pair(c_Waiting, "waiting"));
  stateNames.insert(std::make_pair(c_Running, "running"));
  stateNames.insert(std::make_pair(c_Monitoring, "monitoring"));
  stateNames.insert(std::make_pair(c_Done, "done"));
  stateNames.insert(std::make_pair(c_Failed, "failed"));
  for (auto stateName : stateNames) {
    if (stateName.second == state)
      return stateName.first;
  }

  B2FATAL("stringToState: Unknown CalibrationModule state!");
}

void CalibrationModule::addDefaultDependencyList(string list)
{
  addParam("Dependencies", m_dependencyList,
           "Sets dependencies on other calibration modules. Separate module names by comma. If the state of the module has to differ from 'done', use "
           "ModuleName:state where state=done|waiting|monitoring|failed|running", std::string(list));
}



