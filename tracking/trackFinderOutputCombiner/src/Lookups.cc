#include <tracking/trackFinderOutputCombiner/Lookups.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <framework/logging/LogSystem.h>

using namespace Belle2;
using namespace TrackFindingCDC;
using namespace TrackFinderOutputCombining;

void SegmentLookUp::fillWith(std::vector<CDCRecoSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Calculate a lookup SuperLayerID -> Segments
  m_lookup.clear();
  m_lookup.resize(wireTopology.N_SUPERLAYERS);

  for (CDCRecoSegment2D& segment : segments) {
    ILayerType superlayerID = segment.getISuperLayer();
    m_lookup[superlayerID].push_back(new SegmentInformation(&segment));
    B2DEBUG(200, "Added new segment to segment lookup: " << segment.getTrajectory2D())
  }
}

void TrackLookUp::fillWith(std::vector<CDCTrack>& tracks)
{
  // Calculate a lookup for Tracks
  m_lookup.clear();
  m_lookup.reserve(tracks.size());

  // Calculate a lookup Track -> TrackInformation
  for (CDCTrack& trackCand : tracks) {
    TrackInformation* trackInformation = new TrackInformation(&trackCand);

    std::vector<double>& perpSList = trackInformation->getPerpSList();
    perpSList.reserve(trackCand.size());
    for (const CDCRecoHit3D& recoHit : trackCand) {
      perpSList.push_back(recoHit.getPerpS());
    }
    trackInformation->calcPerpS();
    m_lookup.push_back(trackInformation);
    B2DEBUG(200, "Added new track to track lookup: " << trackCand.getStartTrajectory3D().getTrajectory2D())
  }
}

void SegmentTrackCombiner::combine()
{
  B2DEBUG(100, "Starting combining work.");

  for (std::vector<SegmentInformation*>& segmentsList : m_segmentLookUp) {
    B2DEBUG(100, "Starting next superlayer.");

    // Search for all matching tracks for a given segment
    for (SegmentInformation* segmentInformation : segmentsList) {
      matchTracksToSegment(segmentInformation);
    }

    // Go through all tracks.
    for (TrackInformation* trackInformation : m_trackLookUp) {

      // Try to find the longest trains of segments
      std::list<TrainOfSegments> trainsOfSegments;
      createTrainsOfSegments(trainsOfSegments, trackInformation);

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        // TODO: Better: Add this to a list!
        combineSegmentTrainAndMatchedTracks(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // If there are more possibilities use the one which fills in the empty space of the track best
        B2WARNING("Number of possible trains still left: " << trainsOfSegments.size());
      }
    }

    // Reset the matching segments lists
    for (TrackInformation* trackInformation : m_trackLookUp) {
      trackInformation->clearMatches();
    }

    unsigned int notUsedSegments = std::count_if(segmentsList.begin(),
    segmentsList.end(), [](const SegmentInformation * segmentInformation) -> bool {
      if (segmentInformation->isUsedInTrack())
      {
        segmentInformation->getSegment()->getAutomatonCell().setTakenFlag();
        return false;
      } else {
        segmentInformation->getSegment()->getAutomatonCell().unsetTakenFlag();
        return true;
      }
    });

    if (notUsedSegments > 0) {
      B2WARNING("Still " << notUsedSegments << " not used in this superlayer.")
    }

  }
}

bool SegmentTrackCombiner::segmentMatchesToTrack(const SegmentInformation* segmentInformation,
                                                 const TrackInformation* trackInformation)
{
  const CDCRecoHit2D& front = segmentInformation->getSegment()->front();
  const CDCRecoHit2D& back = segmentInformation->getSegment()->back();

  // Calculate distance
  const CDCTrajectory2D& trajectory = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  if (segmentInformation->getSegment()->getStereoType() != AXIAL) {
    if (fabs(trajectory.getDist2D(front.getRecoPos2D())) > 10)  {
      B2DEBUG(120, "Hits too far away.")
      return false;
    }
  } else {
    if (fabs(trajectory.getDist2D(front.getRecoPos2D())) > 5)  {
      B2DEBUG(120, "Hits too far away.")
      return false;
    }
  }

  Vector3D frontRecoPos3D = front.reconstruct3D(trajectory);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectory);

  if (segmentInformation->getSegment()->getStereoType() != AXIAL) {
    double forwardZ = front.getWire().getSkewLine().forwardZ();
    double backwardZ = front.getWire().getSkewLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ) {
      B2DEBUG(120, "Segment out of CDC after reconstruction.")
      return false;
    }
  }

  // Calculate perpS
  double perpS_min = trajectory.calcPerpS(frontRecoPos3D.xy());
  double perpS_max = trajectory.calcPerpS(backRecoPos3D.xy());

  if (perpS_min > perpS_max) {
    std::swap(perpS_min, perpS_max);
  }

  unsigned int hitsInSameRegion = 0;
  for (double perpS : trackInformation->getPerpSList()) {
    if (perpS < (1 + m_param_percentageForPerpSMeasurements) * perpS_max
        and perpS > (1 - m_param_percentageForPerpSMeasurements) * perpS_min) {
      hitsInSameRegion++;
    }
  }

  if (hitsInSameRegion > 3) {
    B2DEBUG(110, "Too many hits in the same region: " << hitsInSameRegion)
    return false;
  } else {
    B2DEBUG(110, "Hits in the region " << perpS_min << " - " << perpS_max << ": " << hitsInSameRegion << " while hits in segment: " <<
            segmentInformation->getSegment()->size())
  }

  return true;
}

bool SegmentTrackCombiner::doesFitTogether(const TrainOfSegments& trainOfSegments, const TrackInformation* trackInformation)
{
  double lastPerpS;
  bool alreadySet = false;

  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  for (SegmentInformation* segmentInformation : trainOfSegments) {
    double perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
    if (alreadySet and perpSFront < (1 - m_param_percentageForPerpSMeasurements) * lastPerpS) {
      return false;
    }
    alreadySet = true;
    lastPerpS = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
  }
  return true;
}

void SegmentTrackCombiner::makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments,
                                               const TrackInformation* trackInformation)
{
  const std::vector<SegmentInformation*>& matchedSegments = trackInformation->getMatches();

  trainsOfSegments.clear();
  trainsOfSegments.emplace_back();

  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  for (SegmentInformation* segment : matchedSegments) {
    std::list<std::vector<SegmentInformation*>> innerSet;
    for (std::vector<SegmentInformation*> x : trainsOfSegments) {
      x.push_back(segment);
      std::sort(x.begin(), x.end(), [&trajectory2D](SegmentInformation * first, SegmentInformation * second) {
        return trajectory2D.calcPerpS(first->getSegment()->front().getRecoPos2D()) >
               trajectory2D.calcPerpS(second->getSegment()->front().getRecoPos2D());
      });
      if (doesFitTogether(x, trackInformation))
        innerSet.push_back(x);
    }
    trainsOfSegments.insert(trainsOfSegments.end(), innerSet.begin(), innerSet.end());
  }
}

void SegmentTrackCombiner::addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTrack)
{
  const CDCTrajectory2D& trajectory2D = matchingTrack->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  segmentInformation->setUsedInTrack();
  for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
    // TODO

    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    //matchingTrack->getTrackCand()->push_back(recoHit3D);
    matchingTrack->getPerpSList().push_back(
      trajectory2D.calcPerpS(recoHit.getRecoPos2D()));
  }
  matchingTrack->calcPerpS();
}

double SegmentTrackCombiner::testFitSegmentToTrack(const SegmentInformation* segmentInformation,
                                                   const TrackInformation* trackInformation)
{
  const CDCTrajectory2D& trajectory2D = trackInformation->getTrackCand()->getStartTrajectory3D().getTrajectory2D();

  CDCObservations2D observations;

  double perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
  double perpSBack = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());

  CDCTrack* trackCand = trackInformation->getTrackCand();

  bool isAxialSegment = segmentInformation->getSegment()->getStereoType() != AXIAL;

  for (const CDCRecoHit3D& recoHit : trackCand->items()) {
    if (isAxialSegment) {
      observations.append(recoHit.getWireHit().getRefPos2D());
    } else {
      double s = recoHit.getPerpS();
      double z = recoHit.getRecoZ();
      observations.append(s, z);
    }
  }

  for (const CDCRecoHit2D& recoHit : segmentInformation->getSegment()->items()) {
    if (isAxialSegment) {
      observations.append(recoHit.getRecoPos2D());
    } else {
      // TODO: Actually we need the rl-information here!
      CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observations.append(s, z);
    }
  }

  if (isAxialSegment) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    CDCTrajectory2D fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  } else {
    const CDCSZFitter& fitter = CDCSZFitter::getFitter();
    CDCTrajectorySZ fittedTrajectory = fitter.fit(observations);
    return TMath::Prob(fittedTrajectory.getChi2(), fittedTrajectory.getNDF());
  }
}

void SegmentTrackCombiner::combineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments)
{
  B2DEBUG(100, "Trying to combine...")
  for (SegmentInformation* segmentInformation : trainOfSegments) {

    std::vector<TrackInformation*>& matchingTracks = segmentInformation->getMatches();

    if (matchingTracks.size() == 0) {
      B2DEBUG(100, "Match was deleted.")
      return;
    }

    // Try to fiddle out which is the best one!
    std::vector<TrackInformation*> matchesAboveTrack;
    matchesAboveTrack.reserve(matchingTracks.size() / 2);

    matchingTracks.erase(
      std::remove_if(matchingTracks.begin(), matchingTracks.end(), [&segmentInformation,
    &matchesAboveTrack](TrackInformation * possiblyMatch) -> bool {
      const CDCTrajectory2D& trajectory2D = possiblyMatch->getTrackCand()->getStartTrajectory3D().getTrajectory2D();
      double perpSFront = trajectory2D.calcPerpS(segmentInformation->getSegment()->front().getRecoPos2D());
      double perpSBack = trajectory2D.calcPerpS(segmentInformation->getSegment()->back().getRecoPos2D());
      if (perpSFront > possiblyMatch->getMaxPerpS() or perpSBack < possiblyMatch->getMinPerpS())
      {
        B2DEBUG(120, "Segment is above or below track.")
        matchesAboveTrack.push_back(possiblyMatch);
        return true;
      } else {
        return false;
      }
    }), matchingTracks.end());

    if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! We choose the best one:")
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchingTracks) {
        double fitProbability = testFitSegmentToTrack(segmentInformation, trackInformation);

        if (fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
        addSegmentToTrack(segmentInformation, bestMatch);
      }
    } else if (matchingTracks.size() == 0) {
      B2WARNING("None of the matches were in the track. Aborting. There are " << matchesAboveTrack.size() << " matches above the track.")

      // Try to fit them to the track
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchesAboveTrack) {
        double fitProbability = testFitSegmentToTrack(segmentInformation, trackInformation);

        if (fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track above/below after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
        addSegmentToTrack(segmentInformation, bestMatch);
      }
    } else {
      B2DEBUG(100, "Combining segment with track: " << matchingTracks[0]->getTrackCand())
      addSegmentToTrack(segmentInformation, matchingTracks[0]);
    }

    matchingTracks.clear();
  }
}

void SegmentTrackCombiner::matchTracksToSegment(SegmentInformation* segmentInformation)
{
  for (TrackInformation* trackInformation : m_trackLookUp) {
    if (segmentMatchesToTrack(segmentInformation, trackInformation)) {
      B2DEBUG(110, "Found matchable in " << segmentInformation->getSegment()->getISuperLayer())
      trackInformation->addMatch(segmentInformation);
      segmentInformation->addMatch(trackInformation);
    } else {
      B2DEBUG(110, "Found not matchable in " << segmentInformation->getSegment()->getISuperLayer())
    }
  }
}

void SegmentTrackCombiner::createTrainsOfSegments(std::list<TrainOfSegments>& trainsOfSegments,
                                                  const TrackInformation* trackInformation)
{
  const std::vector<SegmentInformation*>& matchedSegments = trackInformation->getMatches();

  B2DEBUG(100, "Matched Segments: " << matchedSegments.size());

  for (SegmentInformation* matchedSegment : matchedSegments) {
    B2DEBUG(110, matchedSegment->getMatches().size());
  }

  if (matchedSegments.size() == 0) {
  } else if (matchedSegments.size() == 1) {
    trainsOfSegments.push_back(matchedSegments);
  } else if (matchedSegments.size() > 1) {
    makeAllCombinations(trainsOfSegments, trackInformation);
  }
}
