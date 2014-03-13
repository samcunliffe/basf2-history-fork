/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/datastore/DataStore.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/RelationEntry.h>
#include <framework/dataobjects/RelationContainer.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationIndexManager.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreAccessorBase.h>

#include <TClonesArray.h>
#include <TClass.h>

using namespace std;
using namespace Belle2;


DataStore& DataStore::Instance()
{
  static DataStore instance;
  return instance;
}


DataStore::DataStore() : m_initializeActive(false)
{
}

DataStore::~DataStore()
{
  //release all memory in data store
  reset();
}

void DataStore::reset()
{
  m_initializeActive = false;
  m_currentModule = "";
  m_moduleInfo.clear();

  for (int i = 0; i < c_NDurabilityTypes; i++)
    reset((EDurability)i);
}


bool DataStore::checkType(const StoreEntry& entry, const StoreAccessorBase& accessor) const
{
  // Check whether the existing entry and the requested object are both arrays or both single objects
  const char* entryType = (entry.isArray) ? "array" : "object";
  if (entry.isArray != accessor.isArray()) {
    B2FATAL("Existing entry '" << entry.name << "' is an " << entryType << " and the requested one an " << ((accessor.isArray()) ? "array" : "object"));
    return false;
  }

  // Check whether the existing entry has the same type
  TClass* entryClass = entry.object->IsA();
  if (entry.isArray) {
    entryClass = static_cast<TClonesArray*>(entry.object)->GetClass();
  }
  if (!entryClass->InheritsFrom(accessor.getClass())) {
    B2FATAL("Existing " << accessor.readableName() << " of type " << entryClass->GetName() << " doesn't match requested type " << accessor.getClass()->GetName());
    return false;
  }

  return true;
}


bool DataStore::registerEntry(const std::string& name, EDurability durability,
                              const TClass* objClass, bool array, bool transient, bool errorIfExisting)
{
  const StoreAccessorBase accessor(name, durability, objClass, array);
  // Check whether this method is called in the initialization phase
  if (!m_initializeActive) {
    B2ERROR("Attempt to register " << accessor.readableName() << " outside of the initialization phase. Please move calls to registerPersistent() etc. into your Module's initialize() function.");
    return false;
  }

  //add to current module's outputs
  ModuleInfo& info = m_moduleInfo[m_currentModule];
  info.addEntry(name, ModuleInfo::c_Output, (objClass == RelationContainer::Class()));

  // Check whether the map entry already exists
  const StoreObjConstIter& it = m_storeObjMap[durability].find(name);
  if (it != m_storeObjMap[durability].end()) {
    StoreEntry* entry = it->second;

    // Complain about existing entry
    if (errorIfExisting) {
      B2ERROR("An " << accessor.readableName() << " of type " << entry->object->ClassName() << " was already registered before. (Multiple calls to registerPersistent() etc. are fine if the errorIfExisting parameter is set to false. For objects you will want to make sure that you don't discard existing data from other modules in that case.");
      return false;
    }

    // Check whether the types match
    if (!checkType(*entry, accessor)) return false;

    // Check whether the persistency type matches
    if (entry->isTransient != transient) {
      B2WARNING("Existing " << accessor.readableName() << " has different persistency type than requested. Changing persistency to " << (transient ? "transient" : "persistent") << ".");
      entry->isTransient = transient;
    }

    B2DEBUG(100, "An " << accessor.readableName() << " was registered once more (with errorIfExisting=false).");
    return true;
  }

  // check reserved names
  if (array and name == "ALL") {
    B2ERROR("Creating an array with the reserved name 'ALL' is not allowed!");
    return false;
  }

  // Add the DataStore entry
  StoreEntry* entry = new StoreEntry;
  m_storeObjMap[durability][name] = entry;
  entry->isArray = array;
  entry->isTransient = transient;
  if (array) {
    entry->object = new TClonesArray(objClass);
  } else {
    entry->object = static_cast<TObject*>(objClass->New());
  }
  entry->name = name;

  B2DEBUG(100, "Successfully registered " << accessor.readableName());
  return true;
}


DataStore::StoreEntry* DataStore::getEntry(const StoreAccessorBase& accessor) const
{
  const StoreObjConstIter& it = m_storeObjMap[accessor.getDurability()].find(accessor.getName());

  if (it != m_storeObjMap[accessor.getDurability()].end() and checkType(*(it->second), accessor)) {
    return it->second;
  } else {
    return nullptr;
  }
}


TObject** DataStore::getObject(const StoreAccessorBase& accessor) const
{
  StoreEntry* entry = getEntry(accessor);
  if (!entry) {
    return nullptr;
  }
  return &(entry->ptr);
}


bool DataStore::createObject(TObject* object, bool replace, const StoreAccessorBase& accessor)
{
  StoreEntry* entry = getEntry(accessor);
  if (!entry) {
    B2ERROR("No " << accessor.readableName() << " exists in the DataStore, did you forget to register it in your Module's initialize() function? Note: direct accesses to it will crash!");
    return false;
  }

  if (entry->ptr && !replace) {
    B2ERROR("An " << accessor.readableName() << " was already created in the DataStore.");
    return false;
  }

  if (object) {
    delete entry->object;
    entry->object = object;
  } else {
    if (accessor.isArray()) {
      static_cast<TClonesArray*>(entry->object)->Delete();
    } else {
      delete entry->object;
      entry->object = static_cast<TObject*>(accessor.getClass()->New());
    }
  }

  entry->ptr = entry->object;
  return true;
}


bool DataStore::findStoreEntry(const TObject* object, DataStore::StoreEntry*& entry, int& index) const
{
  // check whether the cached information is (still) valid
  if (entry && entry->ptr && (index >= 0)) {
    if (static_cast<TClonesArray*>(entry->ptr)->At(index) == object) return true;
    index = static_cast<TClonesArray*>(entry->ptr)->IndexOf(object);
    if (index >= 0) return true;
  }
  entry = nullptr;
  index = -1;

  //searching for nullptr should be safe
  if (!object)
    return false;

  // search for the object and set the entry and index
  const TClass* objectClass = object->IsA();
  for (auto & mapEntry : m_storeObjMap[c_Event]) {
    if (mapEntry.second->ptr && mapEntry.second->isArray) {
      TClonesArray* array = static_cast<TClonesArray*>(mapEntry.second->ptr);
      const TClass* arrayClass = array->GetClass();
      if (arrayClass == objectClass) {
        if (object == array->Last()) {
          //quickly find entry if it's at the end of the array
          index = array->GetLast();
        } else {
          if (arrayClass->InheritsFrom(RelationsObject::Class())) {
            //update cache for entire array
            int nEntries = array->GetEntriesFast();
            for (int i = 0; i < nEntries; i++) {
              RelationsObject* relobj = static_cast<RelationsObject*>((*array)[i]);
              relobj->m_cacheArrayIndex = i;
              relobj->m_cacheDataStoreEntry = mapEntry.second;
            }
          }
          index = array->IndexOf(object);
        }

        if (index >= 0) {
          entry = mapEntry.second;
          return true;
        }
      }
    }
  }
  return false;
}

void DataStore::getArrayNames(std::vector<std::string>& names, const std::string& arrayName, const TClass* arrayClass) const
{
  if (arrayName.empty()) {
    names.push_back(defaultArrayName(arrayClass->GetName()));
  } else if (arrayName == "ALL") {
    for (auto & mapEntry : m_storeObjMap[c_Event]) {
      if (mapEntry.second->ptr && mapEntry.second->isArray) {
        TClonesArray* array = static_cast<TClonesArray*>(mapEntry.second->ptr);
        if (array->GetClass()->InheritsFrom(arrayClass)) {
          names.push_back(mapEntry.second->name);
        }
      }
    }
  } else {
    names.push_back(arrayName);
  }
}

bool DataStore::addRelation(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject, StoreEntry*& toEntry, int& toIndex, double weight)
{
  // get entry from which the relation points
  if (!findStoreEntry(fromObject, fromEntry, fromIndex)) return false;

  // get entry to which the relation points
  if (!findStoreEntry(toObject, toEntry, toIndex)) return false;

  // get the relations from -> to
  const string& relationsName = relationName(fromEntry->name, toEntry->name);
  const StoreObjConstIter& it = m_storeObjMap[c_Event].find(relationsName);
  if (it == m_storeObjMap[c_Event].end()) {
    B2ERROR("No relation '" << relationsName << "' found. Please register it (using RelationArray::registerPersistent()) before trying to add relations.");
    return false;
  }
  StoreEntry* entry = it->second;

  // auto create relations if needed
  if (!entry->ptr) {
    delete entry->object;
    RelationContainer* relations = new RelationContainer;
    relations->setFromName(fromEntry->name);
    relations->setFromDurability(c_Event);
    relations->setToName(toEntry->name);
    relations->setToDurability(c_Event);
    entry->object = relations;
    entry->ptr = entry->object;
  }

  // add relation
  RelationContainer* relContainer = static_cast<RelationContainer*>(entry->ptr);
  TClonesArray& relations = relContainer->elements();
  new(relations.AddrAt(relations.GetLast() + 1)) RelationElement(fromIndex, toIndex, weight);

  RelationIndexContainer<TObject, TObject>* relIndex = RelationIndexManager::Instance().getIndexIfExists<TObject, TObject>(relationsName, c_Event);
  if (relIndex) {
    // add it to index (so we avoid expensive rebuilding later)
    relIndex->index().insert(RelationIndex<TObject, TObject>::Element(fromIndex, toIndex, fromObject, toObject, weight));
  } else {
    //mark for rebuilding later on
    relContainer->setModified(true);
  }

  return true;
}

std::vector<RelationEntry> DataStore::getRelationsWith(ESearchSide searchSide, const TObject* object, DataStore::StoreEntry*& entry, int& index, const TClass* withClass, const std::string& withName)
{
  if (searchSide == c_BothSides) {
    std::vector<RelationEntry> result = getRelationsWith(c_ToSide, object, entry, index, withClass, withName);
    const std::vector<RelationEntry>& fromResult = getRelationsWith(c_FromSide, object, entry, index, withClass, withName);
    result.insert(result.end(), fromResult.begin(), fromResult.end());
    return result;
  }

  std::vector<RelationEntry> result;

  // get StoreEntry for 'object'
  if (!findStoreEntry(object, entry, index)) return result;

  // get names of store arrays to search
  std::vector<string> names;
  getArrayNames(names, withName, withClass);

  // loop over found store arrays
  for (const std::string & name : names) {
    const StoreObjConstIter& arrayIter = m_storeObjMap[c_Event].find(name);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == nullptr) continue;

    // get the relations from -> to
    const string& relationsName = (searchSide == c_ToSide) ? relationName(entry->name, name) : relationName(name, entry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    //get relations with object
    if (searchSide == c_ToSide) {
      for (const auto & rel : relIndex.getElementsFrom(object)) {
        TObject* const toObject = const_cast<TObject*>(rel.to);
        if (toObject)
          result.emplace_back(toObject, rel.weight);
      }
    } else {
      for (const auto & rel : relIndex.getElementsTo(object)) {
        TObject* const fromObject = const_cast<TObject*>(rel.from);
        if (fromObject)
          result.emplace_back(fromObject, rel.weight);
      }
    }
  }

  return result;
}

RelationEntry DataStore::getRelationWith(ESearchSide searchSide, const TObject* object, DataStore::StoreEntry*& entry, int& index, const TClass* withClass, const std::string& withName)
{
  if (searchSide == c_BothSides) {
    RelationEntry result = getRelationWith(c_ToSide, object, entry, index, withClass, withName);
    if (!result.object) {
      result = getRelationWith(c_FromSide, object, entry, index, withClass, withName);
    }
    return result;
  }

  // get StoreEntry for 'object'
  if (!findStoreEntry(object, entry, index)) return RelationEntry(nullptr);

  // get names of store arrays to search
  std::vector<string> names;
  getArrayNames(names, withName, withClass);

  // loop over found store arrays
  for (const std::string & name : names) {
    const StoreObjConstIter& arrayIter = m_storeObjMap[c_Event].find(name);
    if (arrayIter == m_storeObjMap[c_Event].end() or arrayIter->second->ptr == nullptr) continue;

    // get the relations from -> to
    const string& relationsName = (searchSide == c_ToSide) ? relationName(entry->name, name) : relationName(name, entry->name);
    RelationIndex<TObject, TObject> relIndex(relationsName, c_Event);
    if (!relIndex)
      continue;

    // get first element
    if (searchSide == c_ToSide) {
      const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementFrom(object);
      if (element && element->to) {
        return RelationEntry(const_cast<TObject*>(element->to), element->weight);
      }
    } else {
      const RelationIndex<TObject, TObject>::Element* element = relIndex.getFirstElementTo(object);
      if (element && element->from) {
        return RelationEntry(const_cast<TObject*>(element->from), element->weight);
      }
    }
  }

  return RelationEntry(nullptr);
}


void DataStore::clearMaps(EDurability durability)
{
  for (auto & mapEntry : m_storeObjMap[durability]) {
    mapEntry.second->ptr = nullptr;
  }
}

void DataStore::reset(EDurability durability)
{
  for (auto & mapEntry : m_storeObjMap[durability]) {
    //delete stored object/array
    delete mapEntry.second->object;
    //delete StoreEntry
    delete mapEntry.second;
  }
  m_storeObjMap[durability].clear();
}

bool DataStore::require(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    ModuleInfo& info = m_moduleInfo[m_currentModule];
    info.addEntry(accessor.getName(), ModuleInfo::c_Input, (accessor.getClass() == RelationContainer::Class()));
  }

  if (!getEntry(accessor)) {
    B2ERROR("The required " << accessor.readableName() << " does not exist. Maybe you forgot the module that registers it?");
    return false;
  }
  return true;
}

bool DataStore::optionalInput(const StoreAccessorBase& accessor)
{
  if (m_initializeActive) {
    ModuleInfo& info = m_moduleInfo[m_currentModule];
    info.addEntry(accessor.getName(), ModuleInfo::c_OptionalInput, (accessor.getClass() == RelationContainer::Class()));
  }

  return (getEntry(accessor) != nullptr);
}
