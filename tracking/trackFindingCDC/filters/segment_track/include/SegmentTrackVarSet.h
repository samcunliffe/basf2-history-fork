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

#include <tracking/trackFindingCDC/varsets/EmptyVarSet.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

#include <vector>
#include <string>
#include <assert.h>


namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCWireHitCluster.
    class CDCRecoSegment2D;
    class CDCTrack;


    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const segmentTrackNames[] = {
      //"is_stereo",
      "segment_size",
      "track_size",
      "maxmimum_trajectory_distance_front",
      "maxmimum_trajectory_distance_back",
      "maxmimum_hit_distance_front",
      "maxmimum_hit_distance_back",
      "out_of_CDC",
      "hits_in_same_region",
      "fit_full",
      "fit_neigh",
      "pt_of_track",
      "track_is_curler",
      "superlayer_already_full",
      "z_distance",
      "theta_distance"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a wire hits cluster.
     */
    class SegmentTrackVarNames : public VarNames<std::pair<const CDCRecoSegment2D*, const CDCTrack*>> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 15;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return segmentTrackNames[iName];
      }
    };

    /** Class that computes floating point variables from a pair of track and segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class SegmentTrackVarSet : public VarSet<SegmentTrackVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      SegmentTrackVarSet(const std::string& prefix = "") : VarSet<SegmentTrackVarNames>(prefix) { }

      /// Generate and assign the variables from the pair
      virtual bool extract(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>* testPair) IF_NOT_CINT(override final);
    };
  }
}
