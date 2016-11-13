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

#include <tracking/trackFindingCDC/filters/segmentPair/HitGapSegmentPairVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     *  Class that computes floating point variables from segment pairs.
     *  Is the same as the HitGapSegmentPairVarSet but skimmed for non sensical combinations
     *  without the expensive common fit.
     */
    class  SkimmedHitGapSegmentPairVarSet : public HitGapSegmentPairVarSet {

    private:
      /// Name of the super class.
      using Super = HitGapSegmentPairVarSet;

    public:
      /// Implement the skim cut.
      bool extract(const CDCSegmentPair* ptrSegmentPair) final;
    };
  }
}
