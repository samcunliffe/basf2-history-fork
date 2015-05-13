/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/segment_track/BaseSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/trackFinderOutputCombining/MatchingInformation.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter for the construction of good segment trains - track pairs
    class SimpleSegmentTrackFilter : public BaseSegmentTrackFilter {

    public:
      /// Constructor
      SimpleSegmentTrackFilter() : BaseSegmentTrackFilter() { }

    public:
      virtual CellWeight operator()(const std::pair<std::vector<SegmentInformation*>, const CDCTrack*>& testPair) IF_NOT_CINT(
        override final);
    };
  }
}
