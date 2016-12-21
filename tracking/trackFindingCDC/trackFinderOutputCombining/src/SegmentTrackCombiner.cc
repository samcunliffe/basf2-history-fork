#include <tracking/trackFindingCDC/trackFinderOutputCombining/SegmentTrackCombiner.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void SegmentTrackCombiner::match(BaseSegmentTrackFilter& segmentTrackFilter, std::vector<CDCTrack>& trackList,
                                 std::vector<CDCSegment2D>& segmentList)
{
  TrackLookUp trackLookUp;
  SegmentLookUp segmentLookUp;

  trackLookUp.fillWith(trackList);
  segmentLookUp.fillWith(segmentList);

  // Mark the segments which are fully found by the legendre track finder as taken
  for (const std::vector<SegmentInformation*>& segments : segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      const bool isFullyTaken = segment->getSegment()->isFullyTaken(1);

      if (isFullyTaken) {
        // Ensure that all hits belong to the same track!
        TrackInformation* singleTrackWithHitsInCommon = nullptr;
        for (const CDCRecoHit2D& recoHit : * (segment->getSegment())) {
          TrackInformation* trackWithHit = trackLookUp.findTrackForHit(recoHit);
          if (trackWithHit != nullptr) {
            if (singleTrackWithHitsInCommon == nullptr) {
              singleTrackWithHitsInCommon = trackWithHit;
            } else if (singleTrackWithHitsInCommon != trackWithHit) {
              singleTrackWithHitsInCommon = nullptr;
              break;
            }
          }
        }

        if (singleTrackWithHitsInCommon != nullptr) {
          addSegmentToTrack(*(segment->getSegment()), *(singleTrackWithHitsInCommon->getTrackCand()));
        }
      }
    }
  }

  // prepare lookup
  for (TrackInformation* track : trackLookUp) {
    for (const CDCRecoHit3D& recoHit : * (track->getTrackCand())) {
      SegmentInformation* matchingSegment = segmentLookUp.findSegmentForHit(recoHit);

      if (matchingSegment == nullptr) {
        continue;
      }

      if (matchingSegment->isAlreadyTaken()) {
        continue;
      }

      // Check if we did not already have the track in the list
      const SegmentInformation::ListOfMatchCandidates& currentlyMatched = matchingSegment->getMatches();
      if (std::find_if(currentlyMatched.begin(),
      currentlyMatched.end(), [&track](const std::pair<TrackInformation*, double>& pair) -> bool {
      return pair.first == track;
    }) != currentlyMatched.end()) {
        continue;
      }

      // Call the filter and add the match
      double filterResult = segmentTrackFilter(
                              std::make_pair(matchingSegment->getSegment(), track->getTrackCand()));
      if (not std::isnan(filterResult)) {
        matchingSegment->addMatch(track, filterResult);
      }
    }
  }

  for (const std::vector<SegmentInformation*>& segments : segmentLookUp) {
    for (SegmentInformation* segment : segments) {
      if (segment->isAlreadyTaken()) {
        continue;
      }

      const SegmentInformation::ListOfMatchCandidates& matchingTracks = segment->getMatches();

      if (matchingTracks.size() == 0) {
        continue;
      }

      TrackInformation* bestMatch = segment->getBestMatch();

      if (matchingTracks.size() > 1) {
        // Delete the hits from the other track(s)
        for (const std::pair<TrackInformation*, double>& matchingPair : matchingTracks) {
          TrackInformation* notBestTrack = matchingPair.first;

          if (notBestTrack == bestMatch) {
            continue;
          }

          CDCTrack* cdcTrack = notBestTrack->getTrackCand();

          cdcTrack->erase(std::remove_if(cdcTrack->begin(), cdcTrack->end(),
          [&segmentLookUp, &segment](const CDCRecoHit3D & recoHit) -> bool {
            if (segmentLookUp.findSegmentForHit(recoHit) == segment)
            {
              recoHit.getWireHit().getAutomatonCell().unsetTakenFlag();
              return true;
            } else {
              return false;
            }
          }), cdcTrack->end());
        }
      }

      // Add the segment to the track with the highest probability to match
      SegmentTrackCombiner::addSegmentToTrack(*(segment->getSegment()), *(bestMatch->getTrackCand()));
    }
  }

  trackLookUp.clear();
  segmentLookUp.clear();
}

void SegmentTrackCombiner::addSegmentToTrack(const CDCSegment2D& segment, CDCTrack& track)
{
  if (segment.getAutomatonCell().hasTakenFlag()) {
    return;
  }

  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  for (const CDCRecoHit2D& recoHit : segment) {
    if (recoHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      continue;
    }
    CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(recoHit.getRLWireHit(), trajectory2D);
    track.push_back(recoHit3D);
    recoHit.getWireHit().getAutomatonCell().setTakenFlag();
  }

  track.setHasMatchingSegment();
  segment.getAutomatonCell().setTakenFlag();
}
