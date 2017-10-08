/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>

#include <algorithm>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instantiations
    extern template class Filter<Relation<const CDCSegment2D> >;

    /// Base class for filtering the neighborhood of segments
    class BaseSegmentRelationFilter : public Filter<Relation<const CDCSegment2D>> {

    public:
      /// Default constructor
      BaseSegmentRelationFilter();

      /// Default destructor
      virtual ~BaseSegmentRelationFilter();

      /// Returns all equivalent segment ins the range.
      std::vector<const CDCSegment2D*> getPossibleNeighbors(
        const CDCSegment2D* segment,
        const std::vector<const CDCSegment2D*>::const_iterator& itBegin,
        const std::vector<const CDCSegment2D*>::const_iterator& itEnd) const
      {
        assert(std::is_sorted(itBegin, itEnd, LessOf<Deref>()) && "Expected segments to be sorted");
        ConstVectorRange<const CDCSegment2D*> neighbors{
          std::equal_range(itBegin, itEnd, segment, LessOf<Deref>())};
        return {neighbors.begin(),  neighbors.end()};
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all segment neighbors.
       */
      virtual Weight operator()(const CDCSegment2D& from, const CDCSegment2D& to);

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCSegment2D>& relation) override;
    };
  }
}
