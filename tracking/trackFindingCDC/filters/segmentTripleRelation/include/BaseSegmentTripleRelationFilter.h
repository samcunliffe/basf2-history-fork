/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCSegmentTriple> >;

    /// Base class for filtering the neighborhood of segment triples
    class BaseSegmentTripleRelationFilter : public Filter<Relation<const CDCSegmentTriple>> {

    public:
      /// Returns the segment triples form the range that continue on the to site of the given segment triple.
      std::vector<const CDCSegmentTriple*> getPossibleNeighbors(
        const CDCSegmentTriple* segmentTriple,
        const std::vector<const CDCSegmentTriple*>::const_iterator& itBegin,
        const std::vector<const CDCSegmentTriple*>::const_iterator& itEnd) const
      {
        assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected segment triples to be sorted");
        const CDCSegment2D* endSegment = segmentTriple->getEndSegment();

        ConstVectorRange<const CDCSegmentTriple*> neighbors{
          std::equal_range(itBegin, itEnd, &endSegment, LessOf<Deref>())};
        return {neighbors.begin(),  neighbors.end()};
      }

      /// Main filter method returning the weight of the neighborhood relation. Return NAN if relation shall be rejected.
      virtual Weight operator()(const CDCSegmentTriple& from  __attribute__((unused)),
                                const CDCSegmentTriple& to  __attribute__((unused)))
      {
        return 1;
      }

      /** Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.*/
      Weight operator()(const Relation<const CDCSegmentTriple>& relation) override
      {
        const CDCSegmentTriple* ptrFrom(relation.first);
        const CDCSegmentTriple* ptrTo(relation.second);
        if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
        return this->operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
