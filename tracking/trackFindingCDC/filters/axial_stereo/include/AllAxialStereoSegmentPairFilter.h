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

#include "BaseAxialStereoSegmentPairFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the constuction of axial to stereo segment pairs based on simple criteria.
    class AllAxialStereoSegmentPairFilter : public Filter<CDCAxialStereoSegmentPair> {

    public:
      /** Checks if a pair of segments is a good combination.
       *  All implementation always accepts the total number of hits.
       */
      virtual
      CellWeight operator()(const CDCAxialStereoSegmentPair& axialStereoSegmentPair) override final
      {
        const CDCAxialRecoSegment2D* ptrStartSegment = axialStereoSegmentPair.getStartSegment();
        const CDCAxialRecoSegment2D* ptrEndSegment = axialStereoSegmentPair.getEndSegment();

        assert(ptrStartSegment);
        assert(ptrEndSegment);

        const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
        const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;
        return startSegment.size() + endSegment.size();
      }

    }; // end class AllAxialStereoSegmentPairFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2
