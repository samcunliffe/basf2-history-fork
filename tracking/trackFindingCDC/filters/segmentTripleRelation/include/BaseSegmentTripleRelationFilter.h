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
#include <tracking/trackFindingCDC/utilities/Range.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instatiations
    extern template class Filter<Relation<const CDCSegmentTriple> >;

    /// Base class for filtering the neighborhood of segment triples
    class BaseSegmentTripleRelationFilter : public Filter<Relation<const CDCSegmentTriple>> {

    public:
      /// Returns a two iterator range covering the range of possible neighboring segment triples of the given facet out of the sorted range given by the two other argumets.
      template<class ACDCSegmentTripleIterator>
      Range<ACDCSegmentTripleIterator>
      getPossibleNeighbors(const CDCSegmentTriple& triple,
                           const ACDCSegmentTripleIterator& itBegin,
                           const ACDCSegmentTripleIterator& itEnd)
      {

        const CDCAxialSegment2D* endSegment = triple.getEndSegment();
        std::pair<ACDCSegmentTripleIterator,  ACDCSegmentTripleIterator> itPairPossibleNeighbors = std::equal_range(itBegin, itEnd,
            endSegment);
        return Range<ACDCSegmentTripleIterator>(itPairPossibleNeighbors.first, itPairPossibleNeighbors.second);

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
