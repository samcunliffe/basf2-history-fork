/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/trackFinderOutputCombining/FittingMatrix.h>
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>
#include <vector>


namespace Belle2 {

// Forward declarations
  namespace TrackFindingCDC {
    class CDCTrack;
    class CDCRecoSegment2D;
  }

  class NotAssignedHitsCombinerModule : public TrackFinderCDCFromSegmentsModule {

  public:

    /**
     * Constructor to set the module parameters.
     */
    NotAssignedHitsCombinerModule();

  private:
    /**
     * Try to combine the segments and the tracks
     */
    void generate(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks) override;
    double calculateThetaOfTrackCandidate(const TrackFindingCDC::CDCTrack& trackCandidate);
    void findEasyCandidates(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments,
                            std::vector<TrackFindingCDC::CDCTrack>& tracks);
    void findHarderCandidates(std::vector<TrackFindingCDC::CDCRecoSegment2D>& segments, std::vector<TrackFindingCDC::CDCTrack>& tracks);


    /// Object Pool
    TrackFindingCDC::FittingMatrix m_fittingMatrix; /**< The fitting matrix we use to calculate the track - segment combinations */

    /// Parameters
    bool m_param_useSecondStage; /**< Flag to also append "harder" cases with a higher fake rate */
  };
}
