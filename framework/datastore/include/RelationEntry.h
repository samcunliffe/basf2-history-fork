/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Kuhr                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RELATIONENTRY_H
#define RELATIONENTRY_H

#include <vector>
#include <string>
#include <TObject.h>

namespace Belle2 {

  /** Struct for relations.
   *
   *  This class is supposed to be used by the data store to answer queries
   *  for objects that are pointed to by relations.
   */
  struct RelationEntry {

    /** Constructor.
     *
     *  @param obj  The object that is pointed to be the relation.
     *  @param e    The weight of the relation.
     */
    RelationEntry(TObject* obj, double w = 1): object(obj), weight(w) {}

    TObject* object;  /**< Pointer to the object. */
    double   weight;  /**< Weight of the relation. */
  };


  /** Class for type safe access to objects that are referred to in relations.
   *
   *  This class is supposed to be used by the RelationsInterface to provide
   *  type safe access to the objects in a vector of relations returned by
   *  the data store.
   */
  template <class T> class RelationVector {
  public:

    /** Constructor.
     *
     *  @param relations  The vector of relation objects.
     */
    RelationVector(const std::vector<RelationEntry>& relations): m_relations(relations) {}

    /** Accessor for the relations vector.
     *
     *  @return           Vector of RelationEntry objects.
     */
    std::vector<RelationEntry> relations() {return m_relations;}


    /** Get number of relations.
     *
     *  @return           Number of relations.
     */
    size_t size() {return m_relations.size();}

    /** Get object with index.
     *
     *  @param index      Index of relation.
     *  @return           Object that the relation points to.
     */
    T*     object(int index) {return static_cast<T*>(m_relations[index].object);}

    /** Get object with index.
     *
     *  @param index      Index of relation.
     *  @return           Object that the relation points to.
     */
    T*     operator[](int index) {return object(index);}

    /** Get weight with index.
     *
     *  @param index      Index of relation.
     *  @return           Weight that the relation has.
     */
    double weight(int index) const {return m_relations[index].weight;}

  private:
    std::vector<RelationEntry> m_relations;  /**< The vector of relation entries */
  };
}

#endif /* RELATIONENTRY_H */
