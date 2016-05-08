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

#include <tracking/trackFindingCDC/filters/segmentPair/BaseSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterOnVarSet.h>

#include <tracking/trackFindingCDC/filters/segmentPair/SkimmedFitlessSegmentPairVarSet.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of segment pairs based on simple criteria without the common fit.
    class FitlessSegmentPairFilter : public FilterOnVarSet<SkimmedFitlessSegmentPairVarSet> {

    private:
      /// Type of the base class
      typedef FilterOnVarSet<SkimmedFitlessSegmentPairVarSet> Super;

    public:
      /// Checks if a pair of segments is a good combination
      virtual Weight operator()(const CDCSegmentPair& segmentPair) override final;

    }; // end class FitlessSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
