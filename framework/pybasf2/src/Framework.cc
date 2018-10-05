/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Thomas Kuhr                                *
 *               R.Itoh, addition of parallel processing function         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/pybasf2/Framework.h>

#include <framework/core/PyObjConvUtils.h>
#include <framework/core/Environment.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/EventProcessor.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/DataStore.h>
#include <framework/database/DBStore.h>
#include <framework/pcore/pEventProcessor.h>

#include <framework/logging/Logger.h>
#include <framework/logging/LogSystem.h>

#include <boost/python.hpp>

#include <set>

using namespace boost::python;
using namespace Belle2;


Framework::Framework()
{
  DataStore::s_DoCleanup = true;
  LogSystem::Instance().enableErrorSummary(true);

  RandomNumbers::initialize();
  Environment::Instance();
}


Framework::~Framework()
{
  //empty module manager of modules
  //since modules may contain shared pointers of Path objects created in Python,
  //these shared pointers have special cleanup hooks that can cause crashes if run
  //after Py_Finalize(). The framework object is cleaned up before, so this is a good place.
  ModuleManager::Instance().reset();
  DataStore::s_DoCleanup = false;
}


void Framework::addModuleSearchPath(const std::string& path)
{
  ModuleManager::Instance().addModuleSearchPath(path);
}


void Framework::setExternalsPath(const std::string& path)
{
  Environment::Instance().setExternalsPath(path);
}


ModulePtr Framework::registerModule(const std::string& moduleName)
{
  return ModuleManager::Instance().registerModule(moduleName);
}


ModulePtr Framework::registerModule(const std::string& moduleName, const std::string& sharedLibPath)
{
  return ModuleManager::Instance().registerModule(moduleName, sharedLibPath);
}


void Framework::process(PathPtr startPath, long maxEvent)
{
  if (Environment::Instance().getDryRun()) {
    Environment::Instance().setJobInformation(startPath);
    return; //processing disabled!
  }

  static bool already_executed = false;
  static std::set<const Module*> previously_run_modules; //not a shared pointer to not screw up ownership
  static int errors_from_previous_run = 0;
  const auto moduleListUnique = startPath->buildModulePathList(true);
  if (already_executed) {
    B2WARNING("Calling process() more than once per steering file is still experimental, please check results carefully! Python modules especially should reinitialise their state in initialise() to avoid problems");
    if (startPath->buildModulePathList(true) != startPath->buildModulePathList(false)) {
      B2FATAL("Your path contains the same module instance in multiple places. Calling process() multiple times is not implemented for this case.");
    }

    //were any modules in moduleListUnique already run?
    for (auto m : moduleListUnique) {
      if (previously_run_modules.count(m.get()) > 0) {
        //only clone if modules have been run before
        startPath = std::static_pointer_cast<Path>(startPath->clone());
        break;
      }
    }
  }
  for (auto m : moduleListUnique) {
    previously_run_modules.insert(m.get());
  }

  int numLogError = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);
  if (numLogError != errors_from_previous_run) {
    B2FATAL(numLogError << " ERROR(S) occurred! The processing of events will not be started.");
  }

  try {
    LogSystem::Instance().resetMessageCounter();
    DataStore::Instance().reset();
    DataStore::Instance().setInitializeActive(true);

    already_executed = true;
    if (Environment::Instance().getNumberProcesses() == 0) {
      EventProcessor processor;
      processor.setProfileModuleName(Environment::Instance().getProfileModuleName());
      processor.process(startPath, maxEvent);
    } else {
      pEventProcessor processor;
      processor.process(startPath, maxEvent);
    }
    errors_from_previous_run = LogSystem::Instance().getMessageCounter(LogConfig::c_Error);

    DBStore::Instance().reset();
  } catch (std::exception& e) {
    B2ERROR("Uncaught exception encountered: " << e.what()); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
  } catch (...) {
    B2ERROR("Uncaught exception encountered!"); //should show module name
    DataStore::Instance().reset(); // ensure we are executed before ROOT's exit handlers
    throw; //and let python's global handler do the rest
    //TODO: having a stack trace would be nicer, but somehow a handler I set using std::set_terminate() never gets called
  }
}


void Framework::setNumberProcesses(int numProcesses)
{
  Environment::Instance().setNumberProcesses(numProcesses);
}


int Framework::getNumberProcesses()
{
  return Environment::Instance().getNumberProcesses();
}


void Framework::setPicklePath(std::string path)
{
  Environment::Instance().setPicklePath(path);
}


std::string Framework::getPicklePath()
{
  return Environment::Instance().getPicklePath();
}

void Framework::setStreamingObjects(boost::python::list streamingObjects)
{
  auto vec = PyObjConvUtils::convertPythonObject(streamingObjects, std::vector<std::string>());
  Environment::Instance().setStreamingObjects(vec);
}

//=====================================================================
//                          Python API
//=====================================================================

boost::python::list Framework::getModuleSearchPathsPython()
{
  boost::python::list returnList;

  for (std::string path : ModuleManager::Instance().getModuleSearchPaths())
    returnList.append(boost::python::object(path));
  return returnList;
}


boost::python::dict Framework::getAvailableModulesPython()
{
  boost::python::dict returnDict;
  for (auto modulePair : ModuleManager::Instance().getAvailableModules())
    returnDict[boost::python::object(modulePair.first)] = boost::python::object(modulePair.second);
  return returnDict;
}


boost::python::list Framework::getRegisteredModulesPython()
{
  boost::python::list returnList;

  for (ModulePtr mod : ModuleManager::Instance().getCreatedModules())
    returnList.append(boost::python::object(mod));
  return returnList;
}


#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#endif
BOOST_PYTHON_FUNCTION_OVERLOADS(process_overloads, Framework::process, 1, 2)
#if !defined(__GNUG__) || defined(__ICC)
#else
#pragma GCC diagnostic pop
#endif

namespace {
  PyObject* PyExc_ModuleNotCreatedError{nullptr};
  /** Translate the ModuleNotCreatedError to a special python exception so that
   * we can differentiate on the python side */
  void moduleNotCreatedTranslator(const ModuleManager::ModuleNotCreatedError& e)
  {
    PyErr_SetString(PyExc_ModuleNotCreatedError, e.what());
  }
}

void Framework::exposePythonAPI()
{
  PyExc_ModuleNotCreatedError = PyErr_NewExceptionWithDoc("basf2.ModuleNotCreatedError",
                                                          "This exception is raised when a basf2 module could not be created for any reason",
                                                          PyExc_RuntimeError, NULL);
  scope().attr("ModuleNotCreatedError") = handle<>(borrowed(PyExc_ModuleNotCreatedError));
  register_exception_translator<ModuleManager::ModuleNotCreatedError>(moduleNotCreatedTranslator);
  //Overloaded methods
  ModulePtr(*registerModule1)(const std::string&) = &Framework::registerModule;
  ModulePtr(*registerModule2)(const std::string&, const std::string&) = &Framework::registerModule;

  //don't show c++ signature in python doc to keep it simple
  docstring_options options(true, true, false);

  //Expose framework class
  class_<Framework, std::shared_ptr<Framework>, boost::noncopyable>("Framework", "Initialize and Cleanup functions", no_init);
  std::shared_ptr<Framework> initguard{new Framework()};
  scope().attr("__framework") = initguard;

  def("add_module_search_path", &Framework::addModuleSearchPath, R"DOCSTRING(
Add a directory in which to search for compiled basf2 C++ `Modules <Module>`.

This directory needs to contain the shared libraries containing the compiled
modules as well as companion files ending in ``.b2modmap`` which contain a list
of the module names contained in each library.

Note:
  The newly added path will not override existing modules

Parameters:
  path (str): directory containing the modules.
)DOCSTRING", args("path"));
  def("set_externals_path", &Framework::setExternalsPath, R"DOCSTRING(
Set the path to the externals to be used.

Warning:
  This will not change the library and executable paths but will just change
  the directory where to look for certain data files like the Evtgen particle
  definition file. Don't use this unless you really know what you are doing.

Parameters:
  path (str): new top level directory for the externals
)DOCSTRING", args("path"));
  def("list_module_search_paths", &Framework::getModuleSearchPathsPython, R"DOCSTRING(
Return a python list containing all the directories included in the module
search Path.

See:
  `add_module_search_path`
)DOCSTRING");
  def("list_available_modules", &Framework::getAvailableModulesPython, R"DOCSTRING(
Return a dictionary containing the names of all known modules
as keys and the name of the shared library containing these modules as values.
)DOCSTRING");
  def("list_registered_modules", &Framework::getRegisteredModulesPython, R"DOCSTRING(
Return a list with pointers to all previously created module instances by calling `register_module()`
)DOCSTRING");
  def("get_pickle_path", &Framework::getPicklePath, R"DOCSTRING(
Return the filename where the pickled path is or should be stored
)DOCSTRING");
  def("set_pickle_path", &Framework::setPicklePath, R"DOCSTRING(
Set the filename where the pickled path should be stored or retrieved from
)DOCSTRING", args("path"));
  def("set_nprocesses", &Framework::setNumberProcesses, R"DOCSTRING(
Sets number of worker processes for parallel processing.

Can be overridden using the ``-p`` argument to basf2.

Note:
  Setting this to 1 will have one parallel worker job which is almost always
  slower than just running without parallel processing but is still provided to
  allow debugging of parallel execution.

Parameters:
  nproc (int): number of worker processes. 0 to disable parallel processing.
)DOCSTRING");
  def("get_nprocesses", &Framework::getNumberProcesses, R"DOCSTRING(
Gets number of worker processes for parallel processing. 0 disables parallel processing
)DOCSTRING");
  def("set_streamobjs", &Framework::setStreamingObjects, R"DOCSTRING(
Set the names of all DataStore objects which should be sent between the
parallel processes. This can be used to improve parallel processing performance
by removing objects not required.
)DOCSTRING");
  {
    // The register_module function is overloaded with different signatures which makes
    // the boost docstring very useless so we handcraft a docstring
    docstring_options param_options(true, false, false);
    def("_register_module", registerModule1);
    def("_register_module", registerModule2, R"DOCSTRING(register_module(name, library=None)
Register a new Module.

This function will try to create a new instance of a module with the given name. If no library is given it will try to find the module by itself from the module search path. Optionally one can specify the name of a shared library containing the module code then this library will be loaded

See:
  `list_module_search_paths()`, `add_module_search_path()`

Parameters:
  name (str): Type of the module to create
  library (str): Optional, name of a shared library containing the module

Returns:
  An instance of the module if successful.

Raises:
  will raise a `ModuleNotCreatedError` if there is any problem creating the module.
)DOCSTRING");
    def("_process", &Framework::process, process_overloads(R"DOCSTRING(process(path, num_events=0)
Processes up to max_events events by starting with the first module in the specified path.

 This method starts processing events only if there is a module in the path
 which is capable of specifying the end of the data flow.

 Parameters:
   path (Path): The processing starts with the first module of this path.
   max_events (int): The maximum number of events that will be processed.
       If the number is smaller than 1, all events will be processed (default).
)DOCSTRING"));
    ;
  }
}
