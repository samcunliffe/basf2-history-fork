#include <tracking/trackFindingCDC/trackFinderOutputCombining/Lookups.h>
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

void SegmentLookUp::fillWith(std::vector<CDCRecoSegment2D>& segments)
{
  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

  // Mark the segments which are fully found by the legendre track finder as taken
  for (const CDCRecoSegment2D& segment : segments) {
    bool oneHitDoesNotHaveTakenFlag = false;
    for (const CDCRecoHit2D& recoHit : segment) {
      if (not recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
        oneHitDoesNotHaveTakenFlag = true;
        break;
      }
    }

    if (not oneHitDoesNotHaveTakenFlag) {
      segment.getAutomatonCell().setTakenFlag();
    }
  }

  // Calculate a lookup SuperLayerID -> Segments
  m_lookup.clear();
  m_lookup.resize(wireTopology.N_SUPERLAYERS);

  // Calculate a lookup cdcHit-> Segment (we use cdcHits here, not cdcwirehits)
  m_hitSegmentLookUp.clear();

  for (CDCRecoSegment2D& segment : segments) {
    if (segment.getAutomatonCell().hasTakenFlag())
      continue;
    ILayerType superlayerID = segment.getISuperLayer();
    SegmentInformation* newSegmentInformation = new SegmentInformation(&segment);
    m_lookup[superlayerID].push_back(newSegmentInformation);

    for (const CDCRecoHit2D& recoHit : segment) {
      const CDCHit* cdcHit = recoHit.getWireHit().getHit();
      m_hitSegmentLookUp.insert(std::make_pair(cdcHit, newSegmentInformation));
    }

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

void SegmentTrackCombiner::match(BaseSegmentTrackChooser& segmentTrackChooserFirstStep)
{
  // prepare lookup
  std::map<SegmentInformation*, std::set<TrackInformation*>> segmentTrackLookUp;
  segmentTrackLookUp.clear();
  for (const std::vector<SegmentInformation*>& segments : m_segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      if (segment->isAlreadyTaken())
        continue;
      segmentTrackLookUp.insert(std::make_pair(segment, std::set<TrackInformation*>()));
    }
  }

  for (TrackInformation* track : m_trackLookUp) {
    for (const CDCRecoHit3D& recoHit : track->getTrackCand()->items()) {
      SegmentInformation* matchingSegment = m_segmentLookUp.findSegmentForHit(recoHit);
      if (matchingSegment != nullptr
          and not isNotACell(segmentTrackChooserFirstStep(std::make_pair(matchingSegment->getSegment(), track->getTrackCand())))) {
        segmentTrackLookUp[matchingSegment].insert(track);
      }
    }
  }

  for (auto& segmentTrackCombination : segmentTrackLookUp) {
    SegmentInformation* segment = segmentTrackCombination.first;
    if (segment->isAlreadyTaken())
      continue;

    const std::set<TrackInformation*>& matchingTracks = segmentTrackCombination.second;
    if (matchingTracks.size() == 1) {
      TrackInformation* track = *(matchingTracks.begin());
      SegmentTrackCombiner::addSegmentToTrack(segment, track);
    } else {
      // Take the one with the most hits in common
      TrackInformation* trackWithTheMostHitsInCommon = nullptr;
      unsigned int mostHitsInCommon = 0;

      for (TrackInformation* track : matchingTracks) {
        unsigned int hitsInCommon = 0;
        for (const CDCRecoHit3D& recoHit : track->getTrackCand()->items()) {
          SegmentInformation* matchingSegment = m_segmentLookUp.findSegmentForHit(recoHit);
          if (matchingSegment != nullptr and matchingSegment == segment) {
            hitsInCommon++;
          }
        }
        if (hitsInCommon > mostHitsInCommon) {
          mostHitsInCommon = hitsInCommon;
          trackWithTheMostHitsInCommon = track;
        }
      }

      if (trackWithTheMostHitsInCommon != nullptr) {
        SegmentTrackCombiner::addSegmentToTrack(segment, trackWithTheMostHitsInCommon);
      }
    }
  }
}


void SegmentTrackCombiner::combine(BaseSegmentTrackChooser& segmentTrackChooserSecondStep,
                                   BaseSegmentTrainFilter& segmentTrainFilter,
                                   BaseSegmentTrackFilter& segmentTrackFilter)
{
  B2DEBUG(100, "Starting combining work.");

  for (std::vector<SegmentInformation*>& segmentsList : m_segmentLookUp) {
    B2DEBUG(100, "Starting next superlayer.");

    // Search for all matching tracks for a given segment
    for (SegmentInformation* segmentInformation : segmentsList) {
      if (segmentInformation->isAlreadyTaken()) {
        continue;
      }
      matchTracksToSegment(segmentInformation, segmentTrackChooserSecondStep);

      if (segmentInformation->getMatches().size() == 1) {
        SegmentTrackCombiner::addSegmentToTrack(segmentInformation, segmentInformation->getMatches()[0]);
        segmentInformation->clearMatches();
      }
    }

    // Go through all tracks and delete the cases were we have more than one train/segment
    for (TrackInformation* trackInformation : m_trackLookUp) {
      // Try to find the longest trains of segments
      std::list<TrainOfSegments> trainsOfSegments;

      // TODO: Be careful with curlers here!!!!
      B2DEBUG(100, "Track matches to " << trackInformation->getMatches().size() << " segments before train creation.")
      createTrainsOfMatchedSegments(trainsOfSegments, trackInformation, segmentTrainFilter);

      if (trainsOfSegments.size() == 1) {
        // If there is only one matching segment/train, mark this as matched
        B2DEBUG(100, "Only one train/segment which matches! Good!")
        trackInformation->setGoodSegmentTrain(trainsOfSegments.front());
      } else if (trainsOfSegments.size() > 1) {
        // There are no good reasons why we should have more than one train/segment that matches. We have to find the best one.
        B2DEBUG(100, "Number of possible trains/segments still left: " << trainsOfSegments.size() <<
                ". As these are too much we will only us the best one and delete the rest.");
        const TrainOfSegments* bestFittingTrain = findBestFittingSegmentTrain(trainsOfSegments, trackInformation, segmentTrackFilter);
        if (bestFittingTrain != nullptr)
          trackInformation->setGoodSegmentTrain(*bestFittingTrain);
      }

      // Reset the matching segments lists
      trackInformation->clearMatches();
    }

    // Go through the tracks and see if we can combine the remaining segment-track combinations
    for (TrackInformation* trackInformation : m_trackLookUp) {
      B2DEBUG(100, "Looking for possible combinations..")
      const TrainOfSegments& goodTrain = trackInformation->getGoodSegmentTrain();
      if (goodTrain.size() > 0)
        tryToCombineSegmentTrainAndMatchedTracks(goodTrain, segmentTrackFilter);

      trackInformation->clearGoodSegmentTrain();
    }
  }
}

const SegmentTrackCombiner::TrainOfSegments* SegmentTrackCombiner::findBestFittingSegmentTrain(
  std::list<TrainOfSegments>& trainsOfSegments, TrackInformation* trackInformation, BaseSegmentTrackFilter& segmentTrackFilter)
{
  // We can easily delete all matches here as we have them in the list anyway
  trackInformation->clearMatches();

  TrainOfSegments* bestFittingTrain = nullptr;
  double bestProbability = -1;
  for (TrainOfSegments& trainOfSegments : trainsOfSegments) {
    if (trainOfSegments.size() > 0) {
      for (SegmentInformation* segmentInformation : trainOfSegments) {
        std::vector<TrackInformation*>& matchedTracks = segmentInformation->getMatches();
        matchedTracks.erase(std::remove(matchedTracks.begin(), matchedTracks.end(), trackInformation), matchedTracks.end());
      }

      CellWeight currentProbability = segmentTrackFilter(std::make_pair(trainOfSegments, trackInformation->getTrackCand()));
      if (not isNotACell(currentProbability) and currentProbability > bestProbability) {
        bestFittingTrain = &trainOfSegments;
        bestProbability = currentProbability;
      }
    }
  }

  if (bestFittingTrain != nullptr) {
    // We have to readd the matches we want to keep
    for (SegmentInformation* segmentInformation : *bestFittingTrain) {
      segmentInformation->addMatch(trackInformation);
    }

    return bestFittingTrain;
  } else {
    return nullptr;
  }
}

void SegmentTrackCombiner::clearSmallerCombinations(std::list<TrainOfSegments>& trainsOfSegments)
{
  // Sort by size descending (biggest at the front)
  trainsOfSegments.sort([](const TrainOfSegments & first, const TrainOfSegments & second) {
    return first.size() > second.size();
  });

  // Can not used a c++-11 range based for loop here, as I edit the container!
  for (auto testTrain = trainsOfSegments.begin(); testTrain != trainsOfSegments.end(); ++testTrain) {
    trainsOfSegments.erase(std::remove_if(trainsOfSegments.begin(),
    trainsOfSegments.end(), [&testTrain](const TrainOfSegments & train) -> bool {
      if (train.size() >= testTrain->size())
        return false;

      bool oneIsNotFound = false;
      for (const SegmentInformation* segmentInformation : train)
      {
        if (std::find(testTrain->begin(), testTrain->end(), segmentInformation) == train.end()) {
          oneIsNotFound = true;
          break;
        }
      }

      return not oneIsNotFound;
    }), trainsOfSegments.end());
  }
}

void SegmentTrackCombiner::makeAllCombinations(std::list<TrainOfSegments>& trainsOfSegments,
                                               const TrackInformation* trackInformation,
                                               BaseSegmentTrainFilter& segmentTrainFilter)
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
      segmentTrainFilter.clear();
      if (segmentTrainFilter(std::make_pair(x, trackInformation->getTrackCand())) != NOT_A_CELL)
        innerSet.push_back(x);
    }
    trainsOfSegments.insert(trainsOfSegments.end(), innerSet.begin(), innerSet.end());
    if (trainsOfSegments.size() > 1000) {
      B2ERROR("Too much trains: " << trainsOfSegments.size() << "!")
      trainsOfSegments.clear();
      return;
    }
  }

  // Delete the first one: it is empty by definition
  trainsOfSegments.pop_front();

  // Delete all trains which are also found as a bigger one
  clearSmallerCombinations(trainsOfSegments);
}

void SegmentTrackCombiner::addSegmentToTrack(const CDCRecoSegment2D& segment, CDCTrack& track)
{
  if (segment.getAutomatonCell().hasTakenFlag())
    return;

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  for (const CDCRecoHit2D& recoHit : segment) {
    if (recoHit.getWireHit().getAutomatonCell().hasTakenFlag())
      continue;
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    track.push_back(recoHit3D);
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  segment.getAutomatonCell().setTakenFlag();
}


void SegmentTrackCombiner::addSegmentToTrack(SegmentInformation* segmentInformation, TrackInformation* matchingTrack)
{
  addSegmentToTrack(*(segmentInformation->getSegment()), *(matchingTrack->getTrackCand()));

  for (const CDCRecoHit3D& recoHit : matchingTrack->getTrackCand()->items()) {
    matchingTrack->getPerpSList().push_back(recoHit.getPerpS());
  }
  matchingTrack->calcPerpS();
}

void SegmentTrackCombiner::tryToCombineSegmentTrainAndMatchedTracks(const TrainOfSegments& trainOfSegments,
    TrackFindingCDC::BaseSegmentTrackFilter& segmentTrackFilter)
{
  B2DEBUG(100, "Trying to combine " << trainOfSegments.size() << " segment(s) with their track(s)...")
  for (SegmentInformation* segmentInformation : trainOfSegments) {

    std::vector<TrackInformation*>& matchingTracks = segmentInformation->getMatches();
    B2DEBUG(100, "Segment has " << matchingTracks.size() << " partners.")

    if (matchingTracks.size() == 0) {
      B2DEBUG(100, "Match was deleted.")
      return;
    }

    // Try to fiddle out which is the best one!
    double bestFitProb = 0;
    TrackInformation* bestMatch = nullptr;

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

    if (matchingTracks.size() == 1) {
      bestMatch = matchingTracks[0];
      B2DEBUG(100, "Combining segment with track: " << bestMatch->getTrackCand())
    } else if (matchingTracks.size() > 1) {
      B2DEBUG(100, matchingTracks.size() << " are too many possible partners! We choose the best one:")

      for (TrackInformation* trackInformation : matchingTracks) {
        std::vector<SegmentInformation*> temporaryTrain = {segmentInformation};
        CellWeight fitProbability = segmentTrackFilter(std::make_pair(temporaryTrain, trackInformation->getTrackCand()));

        if (not isNotACell(fitProbability) and fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
      } else {
        B2DEBUG(100, "Found no segment with a good fit value in the track.")
        bestMatch = nullptr;
      }
    } else if (matchingTracks.size() == 0) {
      B2DEBUG(100, "None of the matches were in the track. Aborting. There are " << matchesAboveTrack.size() <<
              " matches above the track.")

      // Try to fit them to the track
      double bestFitProb = 0;
      TrackInformation* bestMatch = nullptr;

      for (TrackInformation* trackInformation : matchesAboveTrack) {
        std::vector<SegmentInformation*> temporaryTrain = {segmentInformation};
        CellWeight fitProbability = segmentTrackFilter(std::make_pair(temporaryTrain, trackInformation->getTrackCand()));

        if (not isNotACell(fitProbability) and fitProbability > bestFitProb) {
          bestFitProb = fitProbability;
          bestMatch = trackInformation;
        }
      }

      if (bestMatch != nullptr and bestFitProb > m_param_minimalFitProbability) {
        B2DEBUG(100, "Combining segment with track above/below after fit test: " << bestMatch->getTrackCand() << " with: " << bestFitProb)
      } else {
        B2DEBUG(100, "Found no segment with a good fit value above/below the track.")
        bestMatch = nullptr;
      }
    }

    if (bestMatch != nullptr)
      addSegmentToTrack(segmentInformation, bestMatch);
    matchingTracks.clear();
  }
}

void SegmentTrackCombiner::matchTracksToSegment(SegmentInformation* segmentInformation,
                                                BaseSegmentTrackChooser& segmentTrackChooser)
{
  for (TrackInformation* trackInformation : m_trackLookUp) {
    if (isNotACell(segmentTrackChooser(std::make_pair(segmentInformation->getSegment(), trackInformation->getTrackCand())))) {
      B2DEBUG(110, "Found not matchable in " << segmentInformation->getSegment()->getISuperLayer())
    } else {
      B2DEBUG(110, "Found matchable in " << segmentInformation->getSegment()->getISuperLayer())
      trackInformation->addMatch(segmentInformation);
      segmentInformation->addMatch(trackInformation);
    }
  }
}

void SegmentTrackCombiner::createTrainsOfMatchedSegments(std::list<TrainOfSegments>& trainsOfSegments,
                                                         const TrackInformation* trackInformation,
                                                         BaseSegmentTrainFilter& segmentTrainFilter)
{
  const std::vector<SegmentInformation*>& matchedSegments = trackInformation->getMatches();
  if (matchedSegments.size() == 1) {
    trainsOfSegments.push_back(matchedSegments);
  } else if (matchedSegments.size() > 1 and matchedSegments.size() <= 5) {
    makeAllCombinations(trainsOfSegments, trackInformation, segmentTrainFilter);
  } else if (matchedSegments.size() > 5) {
    B2WARNING("Number of matched segments exceeds 5 with: " << matchedSegments.size())
  }
}
