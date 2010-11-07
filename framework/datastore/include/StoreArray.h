/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef STOREARRAY_H
#define STOREARRAY_H

#include <framework/datastore/StoreAccessorAbs.h>
#include <TClonesArray.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreDefs.h>
#include <framework/datastore/Relation.h>
#include <framework/datastore/RelationArray.h>

namespace Belle2 {

  /*! Accessor to stored TClonesArrays. */
  /*! This is an accessor class for the TClonesArrays saved in the DataStore.
      To add new objects, please use the TClonesArray function
      <a href="http://root.cern.ch/root/htmldoc/TClonesArray.html#TClonesArray:New">New</a>.
      The TClonesArrays are never deleted, but their content is deleted according to the EDurability type,
      that is given to them.
      \author <a href="mailto:martin.heck@kit.edu?subject=StoreArray">Martin Heck</a>
  */
  template <class T>
  class StoreArray : public StoreAccessorAbs<TClonesArray> {
  public:

    /*!Constructor. */
    /*! \param name Name with which the TClonesArray is saved.
        \param durability Specifies lifetime of array in question.
        \param generate Shall array be created, if none with name exists so far.
    */
    StoreArray(const std::string& name, const EDurability& durability = c_Event, bool generate = true) {
      if (assignArray(name, durability, generate)) {
        BELLE2_DEBUG(100, "A TClonesArray with name " + name + " has been generated");
      }
    }

    /*!Switch the array, the StoreArray points to. */
    /*! \param name Name with which the TClonesArray is saved.
        \param durability Specifies lifetime of array in question.
        \param generate Shall array be created, if none with name exists so far.
    */
    bool assignArray(const std::string& name, const EDurability& durability = c_Event, bool generate = false);

    /*! Imitate array functionality. */
    TClonesArray& operator *() const {return *m_storeArray;}

    /*! Imitate array functioanlity. */
    TClonesArray* operator ->() const {return m_storeArray;}

    /*!
    */
    TClonesArray* getPtr() {return m_storeArray;}

    /*! Imitate array functionality. */
    operator bool() const {return m_storeArray;}

    /*! Imitate array functionality. */
    /*! By default the TClonesArray would return TObjects, so a cast is necessary.
        The static cast is save here, because at a previous stage, it is already checked,
        that the TClonesArray contains type T.
    */
    T* operator [](int i) const {return static_cast<T*>(m_storeArray->At(i));}

    /*! Get the number of occupied slots in the array. */
    int GetEntries() const {return m_storeArray->GetEntriesFast();}

    /*! Convinient Relation creating.
        Using this way to create Relations is safer than direct creation,
        because in this case you use definitively an object, that is already stored
        in the DataStore.
        \par to   Object towards which the relation shall be.
        \par from index of the object, from which the relation shall point. 0 means, the Relation has the whole TClonesArray stored in the StoreArray as from.
        \par weight Assign a weight to the Relation.
    */
    Relation* relateTo(StoreAccessorAbs<TObject>& to, const int& from = 0, float& weight = 1);

    /*! Convenient RelationArray creating.
        This way of creation can be used, if all weights are the same.
    */
    RelationArray* relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<int>& indexList, const int& from = 0, float& weight = 1);

    /*! RelationArray creation in case of multiple weights.
    */
    RelationArray* relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<std::pair<int, float> > indexWeightList, const int& from = 0);


  private:

    /*! Pointer that actually holds the TClonesArray. */
    TClonesArray* m_storeArray;
  };


} // end namespace Belle2

//-------------------Implementation of template part of the class ---------------------------------
using namespace std;


template <class T>
bool StoreArray<T>::assignArray(const std::string& name, const EDurability& durability, bool generate)
{
  if (name == "") {BELLE2_FATAL("No name was specified!");}

  m_storeArray =  DataStore::Instance().getArray<T>(name, durability);

  if (m_storeArray) {
    return (false);
  }

  if (generate) {
    m_storeArray = DataStore::Instance().createArray<T>(name, durability);
    return(true);
  }
  return (false);

}

template <class T>
Relation* StoreArray<T>::relateTo(StoreAccessorAbs<TObject>& to, const int& from, float& weight)
{
  if (from == 0) {
    return new Relation(m_storeArray, to.getPtr(), weight);
  } else {
    return new Relation(m_storeArray->At(from), to.getPtr());
  }
}

template <class T>
RelationArray* StoreArray<T>::relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<int>& indexList, const int& from, float& weight)
{
  if (from == 0) {
    return new RelationArray(dynamic_cast<TObject*>(m_storeArray), to.getPtr(), indexList, weight);
  } else {
    return new RelationArray(m_storeArray->At(from), to.getPtr(), indexList, weight);
  }
}

template <class T>
RelationArray* StoreArray<T>::relateTo(StoreAccessorAbs<TClonesArray>& to, std::list<std::pair<int, float> > indexWeightList, const int& from)
{
  if (from == 0) {
    return new RelationArray(m_storeArray, to.getPtr(), indexWeightList);
  } else {
    return new RelationArray(m_storeArray->At(from), to.getPtr(), indexWeightList);
  }
}

#endif
