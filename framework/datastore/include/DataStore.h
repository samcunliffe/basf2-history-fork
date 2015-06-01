/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr, Martin Ritter,                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/datastore/StoreEntry.h>
#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)
//CINT doesn't like forward declarations
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/RelationEntry.h>
#endif

#include <vector>
#include <string>
#include <map>

class TObject;
class TClass;

namespace Belle2 {
  class StoreAccessorBase;
  class DependencyMap;
  template <class T> class RelationVector;
  struct RelationEntry;

  /** In the store you can park objects that have to be accessed by various modules.
   *
   *  Normal users should to access the store via StoreAccessorBase-derived classes like
   *  StoreObjPtr or StoreArray.
   *
   *  Nomenclature:
   *  - Entry: unique (name, durability) key that can store an array/object, see StoreEntry. Entries are created using registerEntry().
   *  - Object: In this context, can mean an array or object stored in an entry, see e.g. createObject(), getObject()
   *  - Durability: defines duration that objects are valid, see DataStore::EDurability.
   *
   *  @sa EDurability StoreObjPtr StoreArray RelationsObject
   *  @author <a href="mailto:software@belle2.kek.jp?subject=DataStore">The basf2 developers</a>
   */
  class DataStore {
  public:
    //----------------------------------- enums and typedefs ---------------------------------------------------
    /** Durability types.
     *
     * Defines how long an object should be valid.
     */
    enum EDurability {
      c_Event,     /**< Different object in each event, all objects/arrays are invalidated after event() function has been called on last module in path.  */
      c_Persistent /**< Object is available during entire execution time. Objects are never invalidated and are accessible even after process(Path) returns. */
    };

    /** Flags describing behaviours of objects etc.
     */
    enum EStoreFlag {
      c_WriteOut = 0,                /**< Object/array should be saved by output modules. (default) */
      c_DontWriteOut = 1,            /**< Object/array should be NOT saved by output modules. Can be overridden using the 'branchNames' parameter of RootOutput. */
      c_ErrorIfAlreadyRegistered = 2,/**< If the object/array was already registered, produce an error (aborting initialisation). */
    };
    /** Combination of DataStore::EStoreFlag flags. */
    typedef int EStoreFlags;

    /** Number of Durability Types.
     *
     *  Probably useless, but in principle additional maps are easily created this way.
     */
    enum ENDurabilityTypes {
      c_NDurabilityTypes = 2 /**< Total number of durability types. */
    };

    /** Which side of relations should be returned? */
    enum ESearchSide {
      c_FromSide, /**< Return relations/objects pointed from (to a given object). */
      c_ToSide,   /**< Return relations/objects pointed to (from a given object). */
      c_BothSides /**< Combination of c_FromSide and c_ToSide. */
    };


    /** Wraps a stored array/object, stored under unique (name, durability) key. */
    typedef Belle2::StoreEntry StoreEntry;

    // Convenient typedefs.
    typedef std::map<std::string, StoreEntry> StoreEntryMap;  /**< Map for StoreEntries. */
    typedef StoreEntryMap::iterator StoreEntryIter;              /**< Iterator for a StoreEntry map. */
    typedef StoreEntryMap::const_iterator StoreEntryConstIter;   /**< const_iterator for a StoreEntry map. */

    /** Global flag to to decide if we can do normal cleanup.
     *
     * If the normal basf2 execution is bypassed, e.g. in tests or separate binaries,
     * we cannot guarantee that exit handlers are called in the correct order. Since
     * this may lead to double deletion of objects through ROOT and misleading error
     * stack traces, we will skip cleanup in these exceptional cases.
     */
    static bool s_DoCleanup;


    //--------------------------------- Instance ---------------------------------------------------------------
    /** Instance of singleton Store.
     *
     *  This method is used to access the DataStore directly. It is used internally in the Store accessor classes.
     */
    static DataStore& Instance();

    //--------------------------------- default name stuff -----------------------------------------------------
    /** Return the default storage name for given class name. */
    static std::string defaultObjectName(const std::string& classname);

    /** Return the default storage name for an object of the given type. */
    template<class T> static std::string defaultObjectName()
    {
      const static std::string s = defaultObjectName(T::Class_Name());
      return s;
    }

    /** Return the storage name for an object of the given type and name. */
    template<class T> static std::string objectName(const std::string& name)
    {
      return ((name.empty()) ? defaultObjectName<T>() : name);
    }

    /** Return the default storage name for an given class name. */
    static std::string defaultArrayName(const std::string& classname)
    {
      const std::string& objName = defaultObjectName(classname);
      std::string s;
      s.reserve(objName.length() + 1);
      s += objName;
      s += 's';
      return s;
    }

    /** Return the default storage name for an array of the given type. */
    template<class T> static std::string defaultArrayName()
    {
      const static std::string s = defaultArrayName(defaultObjectName<T>());
      return s;
    }

    /** Return the storage name for an object of the given type and name. */
    template<class T> static std::string arrayName(const std::string& name)
    {
      return ((name.empty()) ? defaultArrayName<T>() : name);
    }

    /** Return the default storage name for a relation between the given types. */
    template<class FROM, class TO> static std::string defaultRelationName()
    {
      const static std::string s = relationName(defaultArrayName<FROM>(), defaultArrayName<TO>());
      return s;
    }

    /** Return the storage name for a relation with given name between the given types. */
    template<class FROM, class TO> static std::string relationName(const std::string& name)
    {
      return ((name.empty()) ? defaultArrayName<FROM, TO>() : name);
    }

    /** Return storage name for a relation between two arrays of the given names. */
    static std::string relationName(const std::string& fromName, const std::string& toName)
    {
      std::string s;
      s.reserve(fromName.length() + toName.length() + 2);
      s += fromName;
      s += "To";
      s += toName;
      return s;
    }

    //------------------------------ Accessing objects and arrays ----------------------------------------------
    /** Register an entry in the DataStore map.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type, an error will be reported. <br>
     *  Otherwise a new map slot is created.
     *  This must be called in the initialization phase. Otherwise an error is returned.
     *  @param name       Name under which you want to save the object in the DataStore.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param objClass   The class of the object.
     *  @param array      Whether it is a TClonesArray or not.
     *  @param storeFlags ORed combination of DataStore::EStoreFlag flags.
     *  @return           True if the registration succeeded.
     */
    bool registerEntry(const std::string& name, EDurability durability,
                       const TClass* objClass, bool array, EStoreFlags storeFlags);

    /** Register a relation in the DataStore map.
     *
     *  This must be called in the initialization phase. Otherwise an error is returned.
     *  @param durability Decide with which durability map you want to perform the requested action.
     *  @param storeFlags ORed combination of DataStore::EStoreFlag flags.
     *  @return           True if the registration succeeded.
     */
    bool registerRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability,
                          EStoreFlags storeFlags);

    /** Produce ERROR message if no entry of the given type is registered in the DataStore.
     *
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           True if the requested object exists.
     */
    bool requireInput(const StoreAccessorBase& accessor);

    /** Produce ERROR message if no relation of given durability exists between fromArray and toArray (in that direction).
     *
     *  @return           True if the requested object exists.
     */
    bool requireRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability);

    /** Register the given object/array as an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           True if the requested object exists.
     */
    bool optionalInput(const StoreAccessorBase& accessor);

    /** Register the given relation as an optional input.
     *
     *  Mainly useful for creating diagrams of module inputs and outputs.
     *
     *  @return           True if the requested object exists.
     */
    bool optionalRelation(const StoreAccessorBase& fromArray, const StoreAccessorBase& toArray, EDurability durability);

    /** Check whether an entry with the correct type is registered in the DataStore map and return it.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type one, an error will be reported. <br>
     *
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           StoreEntry, or NULL if not found
     */
    StoreEntry* getEntry(const StoreAccessorBase& accessor);

    /** Get a pointer to a pointer of an object in the DataStore.
     *
     *  If the map of requested durability already contains an object under the key name with a DIFFERENT type
     *  than the given type one, an error will be reported. <br>
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           Pointer to pointer to object, NULL if none exists
     */
    TObject** getObject(const StoreAccessorBase& accessor);

    /** Create a new object/array in the DataStore or add an existing one.
     *
     *  A matching map entry must already exist. Otherwise an error will be generated.
     *  @param object     Pointer to the object that should be stored. If 0, a new default object is created.
     *  @param replace    If an object already exists, it will be replaced if this is true. If false, an error will be printed.
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           Wether the object was successfully inserted/created
     */
    bool createObject(TObject* object, bool replace, const StoreAccessorBase& accessor);

    /** For two StoreAccessors of same type, move all data in 'from' into 'to', discarding previous contents of 'to' and leaving 'from' empty.
     *
     * Meta-data like c_DontWriteOut flags or info about associated arrays for RelationContainers is not replaced.
     */
    void replaceData(const StoreAccessorBase& from, const StoreAccessorBase& to);

    /** Get a reference to the object/array map.
     *
     * This is intended to be used for input/output or other framework-internal modules.
     */
    StoreEntryMap& getStoreEntryMap(EDurability durability) { return m_storeEntryMap[durability]; }


    /** Add a relation from an object in a store array to another object in a store array.
     *
     *  @param fromObject     Pointer to the object from which the relation points.
     *  @param fromEntry      Data store entry that contains the fromObject. Used for caching. Will be set if NULL.
     *  @param fromIndex      Index in TClonesArray that contains the fromObject. Used for caching. Will be set if < 0.
     *  @param toObject       Pointer to the object to which the relation points.
     *  @param toEntry        Data store entry that contains the toObject. Used for caching. Will be set if NULL.
     *  @param toIndex        Index in TClonesArray that contains the toObject. Used for caching. Will be set if < 0.
     *  @param weight         Weight of the relation.
     */
    void addRelation(const TObject* fromObject, StoreEntry*& fromEntry, int& fromIndex, const TObject* toObject, StoreEntry*& toEntry,
                     int& toIndex, double weight);


    /** Get the relations between an object and other objects in a store array.
     *
     *
     *  @param searchSide     Search objects of type withClass on this side of relations (with 'object' on the other side).
     *  @param object         Pointer to the object from or to which the relations point.
     *  @param entry          Data store entry that contains the object. Used for caching. Will be set if 0.
     *  @param index          Index in TClonesArray that contains the object. Used for caching. Will be set if < 0.
     *  @param withClass      Class of the objects to or from which the relations point.
     *  @param withName        The name of the store array to or from which the relations point.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objectt of type withClass are considered.
     *  @return               Vector of relation entry objects.
     */
    std::vector<Belle2::RelationEntry> getRelationsWith(ESearchSide searchSide, const TObject* object, StoreEntry*& entry, int& index,
                                                        const TClass* withClass, const std::string& withName);

    /** Get the first relation between an object and another object in a store array.
     *
     *  @param searchSide     Search objects of type withClass on this side of relations (with 'object' on the other side).
     *  @param object         Pointer to the object from or to which the relation points.
     *  @param entry          Data store entry that contains the object. Used for caching. Will be set if 0.
     *  @param index          Index in TClonesArray that contains the object. Used for caching. Will be set if < 0.
     *  @param withClass      Class of the objects to or from which the relation points.
     *  @param withName       The name of the store array to or from which the relation points.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objectt of type withClass are considered.
     *  @return               The entry of the first related object.
     */
    Belle2::RelationEntry getRelationWith(ESearchSide searchSide, const TObject* object, StoreEntry*& entry, int& index,
                                          const TClass* withClass, const std::string& withName);

    /** Add a relation from an object in a store array to another object in a store array.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching. Currently this should only be necessary for genfit objects.
     *
     *  @sa RelationsInterface::addRelationTo
     *  @param fromObject     Pointer to the object from which the relation points.
     *  @param toObject       Pointer to the object to which the relation points.
     *  @param weight         Weight of the relation.
     */
    static void addRelationFromTo(const TObject* fromObject, const TObject* toObject, double weight = 1.0)
    {
      DataStore::StoreEntry* fromEntry = nullptr;
      int fromIndex = -1;
      StoreEntry* toEntry = nullptr;
      int toIndex = -1;
      Instance().addRelation(fromObject, fromEntry, fromIndex, toObject, toEntry, toIndex, weight);
    }

    /** Get the relations from an object to other objects in a store array.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching.
     *
     *  @sa RelationsInterface::getRelationsTo
     *  @param fromObject     Pointer to the object from which the relations point.
     *  @tparam TO            Class of the objects to which the relations point.
     *  @param toName         The name of the store array to which the relations point.
     *                        If empty the default store array name for toClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class TO> static RelationVector<TO> getRelationsFromObj(const TObject* fromObject, const std::string& toName = "")
    {
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return RelationVector<TO>(Instance().getRelationsWith(c_ToSide, fromObject, storeEntry, index, TO::Class(), toName));
    }

    /** Get the relations to an object from other objects in a store array.
     *
     *  @note If at all possible, use RelationsObject members instead, as they allow more efficent caching.
     *
     *  @sa RelationsInterface::getRelationsFrom
     *  @param toObject       Pointer to the object to which the relations point.
     *  @tparam FROM          Class of the objects from which the relations point.
     *  @param fromName       The name of the store array from which the relations point.
     *                        If empty the default store array name for fromClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class FROM> static RelationVector<FROM> getRelationsToObj(const TObject* toObject, const std::string& fromName = "")
    {
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return RelationVector<FROM>(Instance().getRelationsWith(c_FromSide, toObject, storeEntry, index, FROM::Class(), fromName));
    }

    /** Get the relations between an object and other objects in a store array.
     *
     *  Relations in both directions are returned.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching. Currently this should only be necessary for genfit objects.
     *
     *  @sa RelationsInterface::getRelationsWith
     *  @param object         Pointer to the object from or to which the relations point.
     *  @tparam T             Class of the objects to or from which the relations point.
     *  @param name           The name of the store array to or from which the relations point.
     *                        If empty the default store array name for withClass will be used.
     *                        If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return               Vector of relation entry objects.
     */
    template <class T> static RelationVector<T> getRelationsWithObj(const TObject* object, const std::string& name = "")
    {
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return RelationVector<T>(Instance().getRelationsWith(c_BothSides, object, storeEntry, index, T::Class(), name));
    }

    /** Get the object to which another object has a relation.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching. Currently this should only be necessary for genfit objects.
     *  @warning Note that the naming is not consistent with similar member functions of RelationsInterface (exactly switched around). Method will be removed at some point.
     *
     *  @param fromObject  Pointer to the object from which the relation points.
     *  @tparam TO     The class of objects to which the relation points.
     *  @param toName  The name of the store array to which the relation points.
     *                 If empty the default store array name for class TO will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type TO are considered.
     *  @return        The related object or a null pointer.
     */
    template <class TO> static TO* getRelatedFromObj(const TObject* fromObject, const std::string& toName = "")
    {
      if (!fromObject) return nullptr;
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return static_cast<TO*>(DataStore::Instance().getRelationWith(c_ToSide, fromObject, storeEntry, index, TO::Class(), toName).object);
    }

    /** Get the object from which another object has a relation.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching. Currently this should only be necessary for genfit objects.
     *  @warning Note that the naming is not consistent with similar member functions of RelationsInterface (exactly switched around). Method will be removed at some point.
     *
     *  @param toObject Pointer to the object to which the relation points.
     *  @tparam FROM    The class of objects from which the relation points.
     *  @param fromName The name of the store array from which the relation points.
     *                  If empty the default store array name for class FROM will be used.
     *                  If the special name "ALL" is given all store arrays containing objects of type FROM are considered.
     *  @return         The related object or a null pointer.
     */
    template <class FROM> static FROM* getRelatedToObj(const TObject* toObject, const std::string& fromName = "")
    {
      if (!toObject) return nullptr;
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return static_cast<FROM*>(DataStore::Instance().getRelationWith(c_FromSide, toObject, storeEntry, index, FROM::Class(),
                                fromName).object);
    }

    /** Get the object to or from which another object has a relation.
     *
     *  @note If possible, use RelationsObject members instead, as they allow more efficent caching. Currently this should only be necessary for genfit objects.
     *
     *  @param object  Pointer to the object to or from which the relation points.
     *  @tparam T      The class of objects to or from which the relation points.
     *  @param name    The name of the store array to or from which the relation points.
     *                 If empty the default store array name for class T will be used.
     *                 If the special name "ALL" is given all store arrays containing objects of type T are considered.
     *  @return        The related object or a null pointer.
     */
    template <class T> static T* getRelated(const TObject* object, const std::string& name = "")
    {
      if (!object) return nullptr;
      StoreEntry* storeEntry = nullptr;
      int index = -1;
      return static_cast<T*>(DataStore::Instance().getRelationWith(c_BothSides, object, storeEntry, index, T::Class(), name).object);
    }

#if defined(__CINT__) || defined(__ROOTCLING__) || defined(R__DICTIONARY_FILENAME)

    /**
     * @{
     * Define versions without template arguments, only available from python modules.
     */
    static RelationVector<TObject> getRelationsToObj(const TObject* toObject, const std::string& name)
    {
      return getRelationsToObj<TObject>(toObject, name);
    }
    static RelationVector<TObject> getRelationsFromObj(const TObject* fromObject, const std::string& name)
    {
      return getRelationsFromObj<TObject>(fromObject, name);
    }
    static RelationVector<TObject> getRelationsWithObj(const TObject* object, const std::string& name)
    {
      return getRelationsWithObj<TObject>(object, name);
    }
    static TObject* getRelatedToObj(const TObject* toObject, const std::string& name) { return getRelatedToObj<TObject>(toObject, name); }
    static TObject* getRelatedFromObj(const TObject* fromObject, const std::string& name) { return getRelatedFromObj<TObject>(fromObject, name); }
    static TObject* getRelated(const TObject* object, const std::string& name) { return getRelated<TObject>(object, name); }
    /** @} */
#endif

    /** Find an object in an array in the data store.
     *
     *  entry/index are used to return the found array and index,
     *  should be set to NULL and -1, or some cached values (will be checked).
     *
     *  Given non-NULL entry and index>=0, this function returns immediately if the object is
     *  found at the given position.
     *
     *  @param object     Pointer to the object.
     *  @param entry      The DataStore entry that contains the object.
     *  @param index      The index of the object in the array.
     *  @return           True if the object was found in the data store
     */
    bool findStoreEntry(const TObject* object, StoreEntry*& entry, int& index);

    /** Returns a list of names of arrays which have registered relations that point to or from 'array'.
     *
     * This may be useful for internal purposes when modifying array contents.
     */
    std::vector<std::string> getListOfRelatedArrays(const StoreAccessorBase& array) const;

    /** Returns a list of names of arrays which are of type (or inherit from) arrayClass. */
    std::vector<std::string> getListOfArrays(const TClass* arrayClass, EDurability durability) const;

    /** Returns a list of names of StoreObjPtr-objects whose class is (or inherits from) objClass.
     *
     * (this does NOT include the internal RelationContainer objects.)
     */
    std::vector<std::string> getListOfObjects(const TClass* objClass, EDurability durability) const;


    //------------------------------ For internal use --------------------------------------------------
    /** Setter for m_initializeActive.
     *
     *  This should only be called by EventProcessor.
     */
    void setInitializeActive(bool active);

    /** Are we currently initializing modules? */
    bool getInitializeActive() const { return m_initializeActive; }

    /** Clears all registered StoreEntry objects of a specified durability, invalidating all objects.
     *
     *  Called by the framework once the given durability is over. Users should usually not use this function without a good reason.
     *
     *  Memory occupied by objects/arrays is only freed once a new object is created in its place (in createObject()).
     */
    void invalidateData(EDurability durability);

    /** Frees memory occopied by data store items and removes all objects from the map.
     *
     *  Afterwards, m_storeEntryMap[durability] is empty.
     *  Called by the framework. Users should usually not use this function without good reason.
     */
    void reset(EDurability durability);

    /** Clears contents of the datastore (all durabilities)
     *
     *  Called by the framework. Users should usually not use this function without good reason.
     */
    void reset();

    /** Return map of depedencies between modules. */
    DependencyMap& getDependencyMap() { return *m_dependencyMap; }


  private:
    /** Hidden constructor, as it is a singleton.*/
    explicit DataStore();
    /** same for copy constructor */
    DataStore(const DataStore&);
    /** Destructor. */
    ~DataStore();

    /** Check whether the given entry and the requested class match.
     *
     *  Name and durability are not checked.
     *
     *  @param entry      The existing DataStore entry.
     *  @param accessor   Encapsulates name, durability, and type
     *  @return           True if both types match.
     */
    bool checkType(const StoreEntry& entry, const StoreAccessorBase& accessor) const;

    /** Fill the vector with the names of store arrays.
     *
     *  @param names      The resulting vector of array names.
     *  @param arrayName  A given array name, the special string "ALL" for all arrays deriving from the given class, or an empty string for the default array name.
     *  @param arrayClass The class of the array(s).
     */
    void getArrayNames(std::vector<std::string>& names, const std::string& arrayName, const TClass* arrayClass,
                       EDurability durability = c_Event) const;

    /** For an array containing RelationsObjects, update index and entry cache for entire contents.
     *
     * You must ensure the array actually contains objects inheriting from RelationObject!
     */
    void updateRelationsObjectCache(StoreEntry& entry);

    /** Maps (name, durability) key to StoreEntry objects. */
    StoreEntryMap m_storeEntryMap[c_NDurabilityTypes];

    /** True if modules are currently being initialized.
     *
     * Creating new map slots is only allowed in a Module's initialize() function.
     */
    bool m_initializeActive;

    /** Collect information about the dependencies between modules. */
    DependencyMap* m_dependencyMap;
  };
} // namespace Belle2
