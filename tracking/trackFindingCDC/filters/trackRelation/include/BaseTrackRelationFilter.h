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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/numerics/Weight.h>
#include <tracking/trackFindingCDC/utilities/Relation.h>
#include <tracking/trackFindingCDC/utilities/Range.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    // Guard to prevent repeated instatiations
    extern template class Filter<Relation<const CDCTrack> >;

    /// Base class for filtering the neighborhood of tracks
    class BaseTrackRelationFilter : public Filter<Relation<const CDCTrack> > {

    public:
      /// Returns the full range of tracks.
      template<class ACDCTrackIterator>
      Range<ACDCTrackIterator>
      getPossibleNeighbors(const CDCTrack& track  __attribute__((unused)),
                           const ACDCTrackIterator& itBegin,
                           const ACDCTrackIterator& itEnd) const
      {
        return Range<ACDCTrackIterator>(itBegin, itEnd);
      }

      /**
       *  Main filter method returning the weight of the neighborhood relation.
       *  Return always returns NAN to reject all track neighbors.
       */
      virtual Weight operator()(const CDCTrack& from  __attribute__((unused)),
                                const CDCTrack& to  __attribute__((unused)))
      {
        return 1;
      }

      /**
       *  Main filter method overriding the filter interface method.
       *  Checks the validity of the pointers in the relation and unpacks the relation to
       *  the method implementing the rejection.
       */
      Weight operator()(const Relation<const CDCTrack>& relation) override
      {
        const CDCTrack* ptrFrom(relation.first);
        const CDCTrack* ptrTo(relation.second);
        if (ptrFrom == ptrTo) return NAN; // Prevent relation to same.
        if ((ptrFrom == nullptr) or (ptrTo == nullptr)) return NAN;
        return this->operator()(*ptrFrom, *ptrTo);
      }
    };
  }
}
