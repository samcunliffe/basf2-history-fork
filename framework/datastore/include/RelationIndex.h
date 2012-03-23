/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 **************************************************************************/

#ifndef RelationIndex_H
#define RelationIndex_H

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndexManager.h>

namespace Belle2 {

  /** Class to ease use of relations.
   *
   *  This class provides a bidirectional access to a given Relation to ease use
   *  of Relations for the normal user. There is no support for changing or adding
   *  Elements of the relation, this should be done directly using RelationArray
   */
  template<class FROM, class TO> class RelationIndex {
  public:
    /** Struct representing a single element in the index. */
    typedef typename RelationIndexContainer<FROM, TO>::Element Element;

    /** Class representing a the index on the relation. */
    typedef typename RelationIndexContainer<FROM, TO>::ElementIndex ElementIndex;

    /** Typedef for easy access to the from side of the index. */
    typedef typename ElementIndex::template index<FROM>::type index_from;

    /** Typedef for easy access to the to side of the index. */
    typedef typename ElementIndex::template index<TO>::type   index_to;

    /** Iterator of the from side. */
    typedef typename index_from::const_iterator iterator_from;

    /** Iterator of the to side. */
    typedef typename index_to::const_iterator iterator_to;

    /** Pair of iterators specifing the range [first,second) of the from side. */
    typedef std::pair<iterator_from, iterator_from> range_from;

    /** Pair of iterators specifing the range [first,second) of the to side. */
    typedef std::pair<iterator_to, iterator_to> range_to;

    /** Constructor.
     *
     *  @param name       Name of the relation. Empty string will be
     *                    replaced with the default relation name for
     *                    the given types
     *  @param durability Durabiliy of the relation
     */
    RelationIndex(std::string name = (DataStore::defaultRelationName<FROM, TO>()), DataStore::EDurability durability = DataStore::c_Event):
      m_index(RelationIndexManager::get<FROM, TO>(RelationArray(name, durability))),
      m_from(m_index.index().template get<FROM>()),
      m_to(m_index.index().template get<TO>()) {}

    /** Constructor with checks.
     *
     *  This constructor takes the StoreArrays as arguments to perform
     *  additional sanity checks if the relation is correct.
     *
     *  @param from       StoreArray the relation is supposed to point from
     *  @param to         StoreArray the relation is supposed to point to
     *  @param name       Name of the relation. Empty string will be
     *                    replaced with the default relation name for
     *                    the given types
     *  @param durability Durabiliy of the relation
     */
    RelationIndex(const StoreArray<FROM> &from, const StoreArray<TO> &to, std::string name = "",
                  DataStore::EDurability durability = DataStore::c_Event):
      m_index(RelationIndexManager::get<FROM, TO>(RelationArray(from, to, name, durability, false))),
      m_from(m_index.index().template get<FROM>()),
      m_to(m_index.index().template get<TO>()) {}

    /** check if index is based on valid relation. */
    operator bool() const { return m_index; }

    /** Return a range of all elements pointing from the given object.
     *
     *  @param   from Pointer for which to get the relation.
     *  @returns pair of iterators specifing the range [first,second) of
     *           elements which point from this object.
     */
    range_from getFrom(const FROM* from) const { return m_from.equal_range(from);  }

    /** Return a range of all elements pointing from the given object.
     *
     *  @param from Reference for which to get the relation
     *  @returns Pair of iterators specifing the range [first,second) of
     *           elements which point from this object
     */
    range_from getFrom(const FROM& from) const { return m_from.equal_range(&from); }

    /** Return a range of all elements pointing to the given object
     *
     *  @param from Pointer for which to get the relation
     *  @returns Pair of iterators specifing the range [first,second) of
     *           elements which point to this object
     */
    range_to   getTo(const TO* to)       const { return m_to.equal_range(to);      }

    /** Return a range of all elements pointing to the given object
     *
     *  @param from Reference for which to get the relation
     *  @returns Pair of iterators specifing the range [first,second) of
     *           elements which point to this object
     */
    range_to   getTo(const TO& to)       const { return m_to.equal_range(&to);     }

    /** Return a pointer to the first Relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Reference for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstTo(const FROM& from) const { return getFirst(&from); }

    /** Return a pointer to the first Relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Pointer for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstTo(const FROM* from) const {
      iterator_from it = m_from.find(from);
      if (it == m_from.end()) return 0;
      return &(*it);
    }

    /** Return a pointer to the first Relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Reference for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstFrom(const TO& to)   const { return getFirst(&to);   }

    /** Return a pointer to the first Relation Element of the given object.
     *
     *  Useful if there is at most one relation
     *  @param from Pointer for which to get the Relation
     *  @returns Pointer to the RelationIndex<FROM,TO>::Element, can be
     *           NULL if no relation exists
     */
    const Element* getFirstFrom(const TO* to)   const {
      iterator_to it = m_to.find(to);
      if (it == m_to.end()) return 0;
      return &(*it);
    }

    /** Get the AccessorParams of the underlying relation. */
    const AccessorParams getAccessorParams()     const { return m_index.getAccessorParams(); }

    /** Get the AccessorParams of the StoreArray the relation points from. */
    const AccessorParams getFromAccessorParams() const { return m_index.getFromAccessorParams(); }

    /** Get the AccessorParams of the StoreArray the relation points to. */
    const AccessorParams getToAccessorParams()   const { return m_index.getToAccessorParams(); }

    /** Get the size of the index. */
    size_t size() const { return m_index.index().size(); }

  protected:
    /** Reference to the IndexContainer. */
    const RelationIndexContainer<FROM, TO> &m_index;

    /** Reference to the from index. */
    const index_from& m_from;

    /** Reference to the to index. */
    const index_to&   m_to;
  };

} // end namespace Belle2

#endif
