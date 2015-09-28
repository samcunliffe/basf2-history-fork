/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <tracking/trackFindingCDC/filters/segment_triple_relation/BaseSegmentTripleRelationFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class excepting all segment triples.
    class AllSegmentTripleRelationFilter : public Filter<Relation<CDCSegmentTriple>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCSegmentTriple>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

      /** Main filter method returning the weight of the neighborhood relation.
       *  Always return the overlap penatlty accepting all relations.*/
      virtual NeighborWeight operator()(const CDCSegmentTriple& from,
                                        const CDCSegmentTriple& to) override final;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2

