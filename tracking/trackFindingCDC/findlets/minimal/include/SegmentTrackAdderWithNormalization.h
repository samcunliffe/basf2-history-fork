/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

#include <tracking/trackFindingCDC/findlets/minimal/TrackNormalizer.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCSegment2D;

    /**
     * Add the matched segments to the tracks and normalize the tracks afterwards.
     * Also deletes all hits from the tracks, that are part of segments, that were not matched to these tracks.
     */
    class SegmentTrackAdderWithNormalization
      : public Findlet<WeightedRelation<CDCTrack, const CDCSegment2D>&, CDCTrack&, const CDCSegment2D> {

    private:
      /// Type of the base class
      using Super = Findlet<WeightedRelation<CDCTrack, const CDCSegment2D>&, CDCTrack&, const CDCSegment2D>;

    public:
      /// Constructor for registering the sub-findlets
      SegmentTrackAdderWithNormalization();

      /// Expose the parameters of the sub-findlets.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

      /// Short description of the findlet
      std::string getDescription() override;

      /// Apply the findlet
      void apply(std::vector<WeightedRelation<CDCTrack, const CDCSegment2D>>& relations,
                 std::vector<CDCTrack>& tracks, const std::vector<CDCSegment2D>& segment) override;

    private:
      // Findlets
      /// Findlet for performing the normalization of the tracks afterwards
      TrackNormalizer m_trackNormalizer;
    };
  }
}
