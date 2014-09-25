/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/core/SubEventModule.h>

#include <framework/core/Environment.h>
#include <framework/core/ProcessStatistics.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>


using namespace Belle2;


//Note: should not appear in module list since we're not in the right directory
REG_MODULE(SubEvent)


SubEventModule::SubEventModule():
  Module(),
  EventProcessor(),
  m_objectName(""),
  m_loopOver(),
  m_path()
{
  //since we might be created via 'new'...
  setDescription("Internal module, please use the for_each() function in Path instead.");

  addParam("loopOver", m_loopOverName, "Name of array to iterate over.", std::string(""));
  addParam("objectName", m_objectName, "Name of the object holding the current iteration's item.", std::string(""));
  addParam("path", m_path, "Path to execute for each iteration.", PathPtr(nullptr));

}

SubEventModule::~SubEventModule()
{
}

void SubEventModule::initSubEvent(const std::string& objectName, const std::string& loopOver, boost::shared_ptr<Path> path)
{
  m_objectName = objectName;
  m_loopOverName = loopOver;
  m_path = path;
  setModuleName("for_each(" + m_objectName + " : " + m_loopOverName + ")");

  m_moduleList = m_path->buildModulePathList();
  //set c_ParallelProcessingCertified flag if _all_ modules have it set
  bool allCertified = true;
  for (const auto & mod : m_moduleList) {
    if (!mod->hasProperties(c_ParallelProcessingCertified)) {
      allCertified = false;
      break;
    }
  }
  if (allCertified)
    setPropertyFlags(c_ParallelProcessingCertified);
}

void restoreContents(const DataStore::StoreEntryMap& orig, DataStore::StoreEntryMap& dest)
{
  for (auto entry : orig) {
    auto& destEntry = dest[entry.first];
    auto& srcEntry = entry.second;
    if (srcEntry.ptr == nullptr)
      destEntry.ptr = nullptr;
  }
}


void SubEventModule::initialize()
{
  m_loopOver.isRequired(m_loopOverName);

  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();

  //register loop object
  const DataStore::StoreEntry& arrayEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_loopOver.getName());
  TClass* arrayClass = static_cast<TClonesArray*>(arrayEntry.object)->GetClass();
  DataStore::Instance().registerEntry(m_objectName, DataStore::c_Event, arrayClass, false, DataStore::c_ErrorIfAlreadyRegistered);

  m_moduleList = m_path->buildModulePathList();
  processInitialize(m_moduleList);

  //set c_DontWriteOut to disable writing the object
  DataStore::StoreEntry& objectEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_objectName);
  objectEntry.dontWriteOut = true;

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::terminate()
{
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();

  //get event map and make a deep copy of the StoreEntry objects
  //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
  DataStore::StoreEntryMap& persistentMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Persistent);
  DataStore::StoreEntryMap persistentMapCopy = persistentMap;

  processTerminate(m_moduleList);

  restoreContents(persistentMapCopy, persistentMap);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::beginRun()
{
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();
  processBeginRun();

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}
void SubEventModule::endRun()
{
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  processStatistics->suspendGlobal();
  processEndRun();

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}

void SubEventModule::event()
{
  //disable statistics for subevent
  const bool noStats = Environment::Instance().getNoStats();
  StoreObjPtr<ProcessStatistics> processStatistics("", DataStore::c_Persistent);
  //Environment::Instance().setNoStats(true);
  processStatistics->suspendGlobal();

  const int numEntries = m_loopOver.getEntries();

  //get event map and make a deep copy of the StoreEntry objects
  //(we want to revert changes to the StoreEntry objects, but not to the arrays/objects)
  DataStore::StoreEntryMap& eventMap = DataStore::Instance().getStoreEntryMap(DataStore::c_Event);
  DataStore::StoreEntryMap eventMapCopy = eventMap;

  DataStore::StoreEntry& objectEntry = DataStore::Instance().getStoreEntryMap(DataStore::c_Event).at(m_objectName);

  //don't call processBeginRun/EndRun() again (we do that in our implementations)
  m_previousEventMetaData = *(StoreObjPtr<EventMetaData>());

  for (int i = 0; i < numEntries; i++) {
    //set loopObject
    objectEntry.object = m_loopOver[i];
    objectEntry.ptr = m_loopOver[i];

    //stuff usually done in processCore()
    PathIterator moduleIter(m_path);
    processEvent(moduleIter);

    //restore datastore
    restoreContents(eventMapCopy, eventMap);
  }

  objectEntry.object = nullptr;
  objectEntry.ptr = nullptr;

  Environment::Instance().setNoStats(noStats);

  //don't screw up statistics for this module
  processStatistics->startModule();
  processStatistics->resumeGlobal();
}
