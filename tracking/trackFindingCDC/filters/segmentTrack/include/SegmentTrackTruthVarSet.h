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

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr
    static char const* const segmentTrackTruthVarNames[] = {
      "belongs_to_same_track_truth",
      "segment_is_fake_truth",
      "segment_purity_truth",
      "track_purity_truth",
      "truth",
    };

    /// Vehicle class to transport the variable names
    struct SegmentTrackTruthVarNames : public VarNames<std::pair<const CDCTrack*, const CDCSegment2D*>> {

      /// Number of variables to be generated
      static const size_t nVars = size(segmentTrackTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return segmentTrackTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a segment to track match
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class SegmentTrackTruthVarSet : public VarSet<SegmentTrackTruthVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const std::pair<const CDCTrack*, const CDCSegment2D*>* testPair) final;
    };
  }
}
