/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/eventinfo/EventInfoSetterModule.h>

#include <framework/core/Environment.h>
#include <framework/core/InputController.h>
#include <framework/dataobjects/FileMetaData.h>

#include <chrono>
#include <set>
#include <algorithm>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(EventInfoSetter)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

EventInfoSetterModule::EventInfoSetterModule() : Module()
{
  m_evtNumber = 0;
  m_eventsToSkip = 0;
  m_colIndex = 0;
  m_production = 0;
  if (getenv("BELLE2_PRODUCTION")) m_production = stoi(getenv("BELLE2_PRODUCTION"));

  //Set module properties
  setDescription("Sets the event meta data information (exp, run, evt). You must use this module to tell basf2 about the number of events you want to generate, unless you have an input module that already does so. Note that all experiment/run combinations specified must be unique");

  //Parameter definition
  std::vector<int> defaultExpRunList;
  defaultExpRunList.push_back(0);
  std::vector<int> defaultEvtNum;
  defaultEvtNum.push_back(1);

  addParam("expList",      m_expList,      "List of experiment numbers.", defaultExpRunList);
  addParam("runList",      m_runList,      "List of run numbers.",        defaultExpRunList);
  addParam("evtNumList",   m_evtNumList,
           "List of the number of events which should be processed. Can be overridden via -n argument to basf2.", defaultEvtNum);

  addParam("skipNEvents", m_skipNEvents,
           "Skip this number of events before starting. Equivalent to running over this many events without performing any action, to allow starting at higher event numbers.",
           0ul);
}


EventInfoSetterModule::~EventInfoSetterModule()
{

}


void EventInfoSetterModule::initialize()
{
  //Register the EventMetaData in the data store
  m_eventMetaDataPtr.registerInDataStore(DataStore::c_ErrorIfAlreadyRegistered);

  //steering file content overwritten via command line arguments?
  int numEventsArgument = Environment::Instance().getNumberEventsOverride();
  if (numEventsArgument > 0) {
    if (m_evtNumList.size() > 1) {
      B2ERROR("The -n/--events option cannot be used when multiple runs are specified for EventInfoSetter!");
    }
    m_evtNumList[0] = numEventsArgument;
  }

  //Make sure all lists have the same size
  unsigned int defListSize = m_expList.size();
  if ((m_runList.size() != defListSize) || (m_evtNumList.size() != defListSize)) {
    B2ERROR("Parameters are inconsistent. The exp, run and evt lists must have the same number of entries.");
  } else if (defListSize == 0) {
    B2ERROR("There are no events to be processed!");
  } else {
    set<pair<int, int>> expRunSet;
    for (unsigned int i = 0; i < defListSize; i++) {
      auto ret = expRunSet.insert(make_pair(m_expList[i], m_runList[i]));
      if (!ret.second) {
        B2ERROR("Exp " << ret.first->first << ", run " << ret.first->second <<
                " used more than once! Please make sure all experiment/run combinations are unique.");
      }
    }
  }

  m_evtNumber = 0;
  m_eventsToSkip = m_skipNEvents;
  m_colIndex = 0; //adjusted in event() if mismatched
}


void EventInfoSetterModule::event()
{
  while (true) {
    if (m_evtNumber >= static_cast<unsigned long>(m_evtNumList[m_colIndex])) {

      //Search for a column where the event number is greater than 0.
      do {
        m_colIndex++;
      } while ((m_colIndex < static_cast<int>(m_expList.size())) &&
               (m_evtNumList[m_colIndex] <= 0));

      if (m_colIndex < static_cast<int>(m_expList.size())) {
        m_evtNumber = 0;
      } else { //no experiment/run with non-zero number of events found
        return;
      }
    }

    if (m_eventsToSkip != 0) { //are we still skipping?
      unsigned long nskip = 1;
      const unsigned long eventsInList = m_evtNumList[m_colIndex];
      if (m_evtNumber < eventsInList) //skip to end of current run?
        nskip = eventsInList - m_evtNumber;
      if (nskip > m_eventsToSkip)
        nskip = m_eventsToSkip;

      m_eventsToSkip -= nskip;
      m_evtNumber += nskip;
    } else { //no? then stop.
      break;
    }
  }

  m_eventMetaDataPtr.create();
  m_eventMetaDataPtr->setProduction(m_production);
  m_eventMetaDataPtr->setExperiment(m_expList[m_colIndex]);
  m_eventMetaDataPtr->setRun(m_runList[m_colIndex]);
  m_eventMetaDataPtr->setEvent(m_evtNumber);
  auto time = std::chrono::high_resolution_clock::now().time_since_epoch();
  m_eventMetaDataPtr->setTime(std::chrono::duration_cast<std::chrono::nanoseconds>(time).count());

  InputController::mcEvents()++;
  m_evtNumber++;
}
