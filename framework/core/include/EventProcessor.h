/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Pulvermacher, Martin Ritter      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTPROCESSOR_H_
#define EVENTPROCESSOR_H_

#include <framework/core/Path.h>
#include <framework/core/Module.h>
#include <framework/core/PathIterator.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <stdexcept>

class TRandom;

namespace Belle2 {

  /**
   * The EventProcessor Class.
   *
   * This class provides the core event processing loop.
   */
  class EventProcessor {

  public:

    /**
     * Constructor.
     */
    EventProcessor();

    /**
     * Destructor.
     */
    virtual ~EventProcessor();

    /**
     * Processes the full module chain, starting with the first module in the given path.
     *
     * Processes all events for the given run number and for events from 0 to maxEvent.
     * If maxEvent is smaller or equal 0 the maximum number check is disabled and all events are processed.
     * If runNumber is smaller than 0, the run number has to be set externally by a module and not the given number is used.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param maxEvent Optional: The maximum number of events that will be processed. If the number is smaller or equal 0, all events will be processed.
     */
    void process(PathPtr startPath, long maxEvent = 0);

    /** Set the name of the module we want to profile
     * @param name Name of the module as returned by getName()
     */
    void setProfileModuleName(const std::string& name) { m_profileModuleName = name; }

    /** async-safe method to write something to STDERR. */
    static void writeToStdErr(const char msg[]);

  protected:
    /** Exception thrown when execution is stopped by a signal. */
    class StoppedBySignalException : public std::runtime_error {
    public:
      /** Constructor. */
      StoppedBySignalException(int signal);
      int signal; /**< see 'man 7 signal'. */
    };


    /**
     * Initializes the modules.
     *
     * Loops over all module instances specified in a list and calls their initialize() method.
     *
     * @param modulePathList A list of all modules which could be executed during the data processing.
     */
    void processInitialize(const ModulePtrList& modulePathList);

    /**
     * Processes the full module chain consisting of an arbitrary number of connected paths, starting with the first module in the specified path.
     *
     * @param startPath The processing starts with the first module of this path.
     * @param modulePathList A list of all modules which could be executed during the data processing (used for calling the beginRun() and endRun() method).
     * @param maxEvent The maximum number of events that will be processed. If the number is smaller or equal 0, all events are processed.
     */
    void processCore(PathPtr startPath, const ModulePtrList& modulePathList, long maxEvent = 0);

    /** Calls event() functions on all modules for the current event. Used by processCore.
     *
     * @return true if execution should stop.
     */
    bool processEvent(PathIterator moduleIter);

    /**
     * Terminates the modules.
     *
     * Loops over all module instances in reverse order specified in a list and calls their terminate() method.
     * @param modulePathList A list of all modules which could be executed during the data processing.
     */
    void processTerminate(const ModulePtrList& modulePathList);

    /**
     * Calls the begin run methods of all modules.
     *
     * Loops over all module instances specified in a list
     * and calls their beginRun() method. Please note: the
     * beginRun() method of the module which triggered
     * the beginRun() loop will also be called.
     */
    void processBeginRun();

    /**
     * Calls the end run methods of all modules.
     *
     * Loops over all module instances specified in a list
     * and calls their endRun() method. Please note: the
     * endRun() method of the module which triggered
     * the endRun() loop will also be called.
     */
    void processEndRun();

    /** Install signal handlers. */
    void setupSignalHandler();

    const Module* m_master;  /**< The master module that determines the experiment/run/event number **/
    ModulePtrList m_moduleList; /**< List of all modules in order initialized. */

    /** Name of the module which should be profiled, empty if no profiling is requested */
    std::string m_profileModuleName;

    /** Adress of the module which we want to profile, nullptr if no profiling is requested */
    Module* m_profileModule = nullptr;

    /** EventMetaData is used by processEvent()/processCore(). */
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    /** Stores state of EventMetaData before it was last changed. Useful since processEndRun() needs info about which run it needs to end. */
    EventMetaData m_previousEventMetaData;

    /** Also used in a number of places. */
    StoreObjPtr<ProcessStatistics> m_processStatisticsPtr;

    /** Are we currently in a run? If yes, processEndRun() needs to do something. */
    bool m_inRun;
  };

}

#endif /* EVENTPROCESSOR_H_ */
