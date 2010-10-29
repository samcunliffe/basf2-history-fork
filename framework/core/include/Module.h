/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MODULE_H
#define MODULE_H

#include <boost/python.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/list.hpp>
#include <boost/python/dict.hpp>

#include <framework/core/CondParser.h>
#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>
#include <framework/core/ModuleManager.h>

#include <vector>
#include <list>
#include <string>
#include <set>
#include <map>

namespace Belle2 {

  class Path;

  /**
   * Base Class for Modules.
   *
   * A module is the smallest building block of the framework.
   * A typical event processing chain consists of a linear arrangement
   * of modules. By inheriting from this base class, various types of
   * modules can be created. Each module is identified by its unique name.
   */
  class Module {

  public:

    /** The process record types. */
    enum EProcessRecordType {
      prt_Event,       /**< The default value: Marks event data */
      prt_BeginRun,    /**< Marks the beginning of a new run (only used for the first method in the chain) */
      prt_EndRun,      /**< Marks the end of a new run (only used for the first method in the chain) */
      prt_EndOfData    /**< Marks the end of the data. This will stop the
                            processing of the events. This is especially useful
                            for input modules which run out of events before the
                            maximum number of events is reached. The processing of
                            the events is stopped immediately after the module
                            setting this EProcessRecordType has finished.
                          */
    };

    /** Each module can be tagged with property flags, which indicate certain features of the module. */
    enum EModulePropFlags {
      c_TriggersNewRun          = 1,   /**< This module is able to trigger new runs. */
      c_TriggersEndOfData       = 2,   /**< This module is able to send the message that there is no more data available. */
      c_ReadsDataSingleProcess  = 4,   /**< This module is able to read data from a single data stream (disk/server). */
      c_ReadsDataMultiProcess   = 8,   /**< This module is able to read data from an event streaming server. */
      c_WritesDataSingleProcess = 16,  /**< This module is able to write data into a single data stream (disk/server). */
      c_WritesDataMultiProcess  = 32,  /**< This module is able to write data to an event streaming server. */
      c_RequiresSingleProcess   = 64,  /**< This module requires the framework to run in single processing mode. */
      c_RequiresGUISupport      = 128  /**< This module requires the framework to have GUI support built-in. */
    };

    /**
     * Constructor.
     *
     * Create and allocate memory for variables here. Add the module parameters in this method.
     */
    Module();

    /**
     * Destructor.
     *
     * Use the destructor to release the memory you allocated in the constructor.
     */
    virtual ~Module();

    /**
     * Initialize the Module.
     *
     * This method is called only once before the actual event processing starts.
     * Use this method to initialize variables, open files etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void initialize() {};

    /**
     * Called when entering a new run.
     *
     * Called at the beginning of each run, the method gives you the chance to
     * change run dependent constants like alignment parameters, etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void beginRun() {};

    /**
     * This method is the core of the module.
     *
     * This method is called for each event. All processing of the event has to
     * take place in this method.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void event() {};

    /**
     * This method is called if the current run ends.
     *
     * Use this method to store information, which should be aggregated over one run.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void endRun() {};

    /**
     * This method is called at the end of the event processing.
     *
     * This method is called only once after the event processing finished.
     * Use this method for cleaning up, closing files, etc.
     *
     * This method has to be implemented by subclasses.
     */
    virtual void terminate() {};

    /**
     * Returns the name of the module.
     *
     * @return The name of the module as string.
     */
    const std::string& getName() const {return m_name;}

    /**
     * Returns the description of the module.
     *
     * @return The description of the module as string.
     */
    const std::string& getDescription() const {return m_description;}

    /**
     * Returns the log level used for this module.
     *
     * @return Returns the log level of the module.
     */
    LogCommon::ELogLevel getLogLevel() {return static_cast<LogCommon::ELogLevel>(m_logLevel); };

    /**
     * Returns the debug messaging level used for this module.
     *
     * @return Returns the debug messaging level of the module.
     */
    int getDebugLevel() {return m_debugLevel; };

    /**
     * Sets the condition of the module.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * @param expression The expression of the condition.
     * @return Shared pointer to the Path, which will be executed if the condition is evaluated to true.
     */
    void setCondition(const std::string& expression, boost::shared_ptr<Path> path);

    /**
     * A simplified version to set the condition of the module.
     *
     * Please be careful: Avoid creating cyclic paths, e.g. by linking a condition
     * to a path which is processed before the path where this module is
     * located in.
     *
     * It is equivalent to the setCondition() method, using the expression "<1".
     * This method is meant to be used together with the setReturnValue(bool value) method.
     * @return Shared pointer to the Path, which will be executed if the condition is evaluated to true.
     */
    void setCondition(boost::shared_ptr<Path> path);

    /**
     * Returns true if a condition and a return value was set for the module.
     *
     * @return True if a condition and a return value was set for the module.
     */
    bool hasCondition() const { return m_hasCondition; };

    /**
     * If a condition was set, it is evaluated and the result is returned.
     *
     * If no condition or result value was defined, the method returns false.
     * Otherwise, the condition is evaluated and the result of the evaluation returned.
     * To speed up the evaluation, the condition string was already parsed in the method setCondition().
     *
     * @return True if a condition and return value exists and the condition expression was evaluated to true.
     */
    bool evalCondition();

    /**
     * Returns the path of the condition.
     *
     * @return The path of the condition.
     */
    boost::shared_ptr<Path> getConditionPath() const {return m_conditionPath; };

    /**
     * Returns true if all specified property flags are available in this module.
     *
     * @param propertyFlags The flags which should be compared with the module flags.
     * @return True if all specified property flags are available in this module.
     */
    bool hasProperties(unsigned int propertyFlags);

    /**
     * Returns the current process record type.
     *
     * @return The current process record type.
     */
    EProcessRecordType getProcessRecordType() const {return m_processRecordType; };

    /**
     * Returns true if the module has still unset parameters which the user has to set in the steering file.
     *
     * @return True if the module has still unset parameters which the user has to set in the steering file.
     */
    bool hasUnsetForcedParams() const;


    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                   Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Returns a python list of all parameters.
     *
     * Each item in the list consists of the name of the parameter, a string describing its type,
     * a python list of all default values and the description of the parameter.
     * @return A python list containing the parameters of this parameter list.
     */
    boost::python::list getParamInfoListPython() const;

    /**
     * Exposes methods of the Module class to Python.
     */
    static void exposePythonAPI();

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  protected:

    /**
     * Sets the flags for the module properties.
     *
     * @param propertyFlags The flags for the module properties.
     */
    void setPropertyFlags(unsigned int propertyFlags);

    /**
     * Sets the description of the module.
     *
     * @param description A short description of the module.
     */
    void setDescription(const std::string description);

    /**
     * Adds a new parameter to the module. This method has to be called in the constructor of the module.
     *
     * @param name The unique name of the parameter.
     * @param paramVariable The local member variable of the module to which the value from the steering file is written.
     * @param defaultValue The default value of the parameter which is used if there was no value given in the steering file.
     * @param description Optional: a short description of the parameter.
     * @param force Optional: If set to true the parameter has to be set in the steering file by the user.
     */
    template<typename T>
    void addParam(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description = "", bool force = false);

    /**
     * Returns a reference to a parameter. The returned parameter has already the correct type.
     *
     * Throws an exception of type ModuleParameterNotFoundError if a parameter with the given name does not exist.
     * Throws an exception of type ModuleParameterTypeError if the parameter type of does not match to the template parameter.
     *
     * @param name The unique name of the parameter.
     * @return A reference to a module parameter having the correct type.
     */
    template<typename T>
    ModuleParam<T>& getParam(const std::string& name) const throw(ModuleParameterNotFoundError, ModuleParameterTypeError);

    /**
     * Sets the return value for this module as integer.
     * The value can be used in the steering file to divide the analysis chain
     * into several paths.
     *
     * @param value The value of the return value.
     */
    void setReturnValue(int value);

    /**
     * Sets the return value for this module as bool.
     * The bool value is saved as an integer with the convention 1 meaning true and 0 meaning false.
     * The value can be used in the steering file to divide the analysis chain
     * into several paths.
     *
     * @param value The value of the return value.
     */
    void setReturnValue(bool value);

    /**
     * Sets the process record type.
     * Usually this method is used by data generating or data reading modules.
     * Please note: Only for the first module in the chain the BEGIN_RUN and
     * END_RUN process record types are checked.
     *
     * @param processRecordType The process record type to be returned.
     */
    void setProcessRecordType(EProcessRecordType processRecordType);


  private:

    int m_debugFlag; /**< This variable controls the messaging level. */

    std::string m_name;           /**< The name of the module, saved as a string. */
    std::string m_description;    /**< The description of the module. */
    unsigned int m_propertyFlags; /**< The properties of the module (Master, multi processing etc.) saved as bitwise flags. */

    int m_logLevel;            /**< The log messaging level of the module. Defined as int for the parameter handling. */
    int m_debugLevel;          /**< The debug messaging level of the module. */

    ModuleParamList m_moduleParamList; /**< List storing and managing all parameter of the module. */

    bool m_hasReturnValue;     /**< True, if the default return value is set. */
    int  m_returnValue;        /**< The default return value. */
    EProcessRecordType m_processRecordType;  /**< The current process record type. */

    bool m_hasCondition;     /**< True, if a condition was set for the module. */
    boost::shared_ptr<Path> m_conditionPath; /**< The path which which will be executed if the condition is evaluated to true. */
    Belle2::CondParser::EConditionOperators m_conditionOperator;  /**< The operator of the condition (set by parsing the condition expression). */
    int m_conditionValue;                    /**< The value of the condition (set by parsing the condition expression). */

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //                    Python API
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Implements a method for setting boost::python objects.
     * The method supports the following types: int, double, string, bool
     * The conversion of the python object to the C++ type and the final storage of the
     * parameter value is done by specializing the template method setParamObjectTemplate().
     *
     * @param name The unique name of the parameter.
     * @param pyObj The object which should be converted and stored as the parameter value.
     */
    void setParamObject(const std::string& name, const boost::python::object& pyObj);

    /**
     * Implements a method for setting boost::python lists.
     * The method supports lists of the following types: int, double, string, bool
     * The conversion of the python list to the std::vector and the final storage of the
     * parameter value is done by specializing the template method setParamListTemplate().
     *
     * @param name The unique name of the parameter.
     * @param pyList The list which should be converted to a std::vector and stored as the parameter value.
     */
    void setParamList(const std::string& name, const boost::python::list& pyList);

    /**
     * Implements a method for reading the parameter values from a boost::python dictionary.
     * The key of the dictionary has to be the name of the parameter and the value has to
     * be of one of the supported parameter types (both, single parameters and lists are allowed).
     *
     * @param dictionary The python dictionary from which the parameter values are read.
     */
    void setParamDict(const boost::python::dict& dictionary);

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    template<class U> friend class ModuleProxy;
  };


  //------------------------------------------------------
  //       Implementation of template based methods
  //------------------------------------------------------
  template<typename T>
  void Module::addParam(const std::string& name, T& paramVariable, const T& defaultValue, const std::string& description, bool force)
  {
    m_moduleParamList.addParameter(name, paramVariable, defaultValue, description, force);
  }


  template<typename T>
  ModuleParam<T>& Module::getParam(const std::string& name) const throw(ModuleParameterNotFoundError, ModuleParameterTypeError)
  {
    return m_moduleParamList.getParameter<T>(name);
  }


  //------------------------------------------------------
  //             Define convenient typdefs
  //------------------------------------------------------

  typedef boost::shared_ptr<Module> ModulePtr;

  /**
   * Class that defines the < comparison operator ModulePtrs. Used to declare a set of ModulePtrs.
   */
  struct ModulePtrOperators {
    //! Comparison operator for two ModulePtr.
    bool operator()(const ModulePtr& a, const ModulePtr& b) {
      return a.get() < b.get();
    }
  };

  /**
   * Class that defines the equality operator for ModulePtrs.
   */
  struct ModulePtrOperatorsEq: public std::binary_function<ModulePtr, ModulePtr, bool> {
    //! Equality operator for two ModulePtr.
    bool operator()(const ModulePtr& a, const ModulePtr& b) const {
      return a.get() == b.get();
    }
  };

  typedef std::set<ModulePtr, ModulePtrOperators> ModulePtrSet;
  typedef std::list<ModulePtr> ModulePtrList;


  //------------------------------------------------------
  //  Proxy class for creating an instance of the module
  //------------------------------------------------------
  /**
   * The base module proxy class is used to create new instances of a module.
   * By loading a shared library, which contains modules, the proxy class of each
   * module is automatically registered to the global ModuleManager. If an instance of
   * a module is required, its proxy class is responsible to create an instance of the
   * module.
   */
  class ModuleProxyBase {

  public:
    /**
     * The constructor of the ModuleProxyBase class.
     * The constructor registers the proxy to the ModuleManager.
     * @param moduleName The type name of the module.
     */
    ModuleProxyBase(const std::string& moduleName);

    /**
     * Abstract method which creates a new module and returns a shared pointer to it.
     * Instances of modules can only be created by this method.
     * @return A shared pointer to the created module instance.
     */
    virtual ModulePtr createModule() const = 0;

    /**
     * Returns the module name of the module associated to this proxy.
     * @return The module name of the module associated to this proxy.
     */
    const std::string& getModuleName() const {return m_moduleName; }


  protected:

    std::string m_moduleName; /**< The type name of the module. */
  };


  /**
   * The templated proxy class.
   * Be defining a global variable of this class, any module can be registered to the ModuleManager.
   * This definition has to be added to each module definition. A macro is available
   */
  template <class T>
  class ModuleProxy : ModuleProxyBase {

  public:

    ModuleProxy(const std::string& moduleName) : ModuleProxyBase(moduleName) {};

    /**
     * Creates a new module and returns a shared pointer to it.
     * Instances of modules can only be created by this method.
     * @return A shared pointer to the created module instance.
     */
    ModulePtr createModule() const {
      ModulePtr nm(new T());
      nm->m_name = m_moduleName;
      return nm;
    }
  };


  //------------------------------------------------------
  //             Define convenient macros
  //------------------------------------------------------
#define REG_MODULE(className, moduleName) ModuleProxy<className> regProxy##className(moduleName);

  //-------------------------------

} // end namespace Belle2

#endif // MODULE_H

