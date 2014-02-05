/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NEIGHBORHOODBUILDER_H_
#define NEIGHBORHOODBUILDER_H_

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>


namespace Belle2 {
  namespace CDCLocalTracking {

    /// Builder for a neigborhood over a sorted collection
    /** Often one face the problem of having to build a neighborhood relation between elements of the \n
     *  same kind. To find suitable neighbors in a general container it would take an amount of time \n
     *  proportional to n*n to compare all available elements to each other, which is often to long. \n
     *  However if we can sort the sequence we can improve look up speed to an element by a great deal. \n
     *  All tracking entities we want to build neighborhoods for are therefore already made sortable. \n
     *  But the improved look up speed only helps if the neighbors are not scattered around randomly over \n
     *  the sorted range but are close together in a specific section of the range. The time complexity drops \n
     *  than to n*log n + n * m where is the number of elements in the collection and m the expected \n
     *  of neighbors.
     *
     *  Since we already sorted out the arrangement of entities during their creation, we have to define \n
     *  the region where to look for neighbors. Since we keep the general logic for look up the same \n
     *  but vary the definition of what a neighborhood is supposed to be we factor the later about into \n
     *  a strategy object called the NeighborChooser. \n
     */
    template<class Collection, class NeighborChooser>
    class NeighborhoodBuilder {

    private:
      typedef typename Collection::value_type Item; /// type of the item
      typedef WeightedNeighborhood<const Item> Neighborhood; /// type of the neighborhood to be filled

    private:
      NeighborChooser m_chooser; ///< Memory for the neighborhood chooser

    public:
      ///Empty constructor initialzing the default chooser
      NeighborhoodBuilder() : m_chooser()
      {;}

      /// Constructor initialzing the chooser to the given value. Use in case the chooser has parameters you want to set.
      NeighborhoodBuilder(const NeighborChooser& chooser) : m_chooser(chooser)
      {;}

      ///Empty destructor
      ~NeighborhoodBuilder() {;}

      /// Drop all relations in a neighborhood an create a new one.
      void create(const Collection& collection, Neighborhood& neighborhood) const {
        neighborhood.clear();
        append(collection, neighborhood);
      }
      /// Append the neighborhood relations to an existing neighborhood.
      void append(const Collection& collection, Neighborhood& neighborhood) const {
        //forget everything from former creations
        m_chooser.clear();
        for (const Item & item : collection) {

          for (const Item & possibleNeighbor : m_chooser.getPossibleNeighbors(item, collection.begin(), collection.end())) {

            Weight weight = m_chooser.isGoodNeighbor(item, possibleNeighbor);
            if (not isNotANeighbor(weight)) {
              // The neighborhood takes references and keeps them
              neighborhood.insert(item, possibleNeighbor, weight);
            }

          } //end for possibleNeighbor
        }
      }

    private:

    }; // end class

  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //NEIGHBORHOODBUILDER_H_
