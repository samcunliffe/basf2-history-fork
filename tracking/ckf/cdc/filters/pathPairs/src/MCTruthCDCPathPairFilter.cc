/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/pathPairs/MCTruthCDCPathPairFilter.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;

namespace {
  unsigned int countCorrectHits(const CDCCKFPath& path, const RecoTrack* mcRecoTrack)
  {
    auto correctHits = mcRecoTrack->getCDCHitList();
    std::sort(correctHits.begin(), correctHits.end());

    unsigned int numberOfCorrectHits = 0;

    for (const auto& state : path) {
      if (state.isSeed()) {
        continue;
      }

      const auto* wireHit = state.getWireHit();
      const auto* cdcHit = wireHit->getHit();

      if (std::binary_search(correctHits.begin(), correctHits.end(), cdcHit)) {
        numberOfCorrectHits++;
      }
    }

    return numberOfCorrectHits;
  }
}

TrackFindingCDC::Weight MCTruthCDCPathPairFilter::operator()(const BaseCDCPathPairFilter::Object& pair)
{
  const auto& lhs = *pair.first;
  const auto& rhs = *pair.second;

  const auto& lhsSeed = lhs.front();
  const auto& rhsSeed = rhs.front();

  const auto* lhsMCRecoTrack = lhsSeed.getMCRecoTrack();
  const auto* rhsMCRecoTrack = rhsSeed.getMCRecoTrack();

  if (not lhsMCRecoTrack and rhsMCRecoTrack) {
    return -1;
  } else if (lhsMCRecoTrack and not rhsMCRecoTrack) {
    return 1;
  } else if (not lhsMCRecoTrack and not rhsMCRecoTrack) {
    // Well, we do not care...
    return 0;
  }

  // Return the one of the highest number of correct hits
  const unsigned int lhsCorrectHits = countCorrectHits(lhs, lhsMCRecoTrack);
  const unsigned int rhsCorrectHits = countCorrectHits(rhs, rhsMCRecoTrack);

  if (lhsCorrectHits > rhsCorrectHits) {
    return 1;
  } else if (lhsCorrectHits < rhsCorrectHits) {
    return -1;
  }

  // In case both have the same number of correct hits, return the shortest (which has the highest purity)
  const unsigned int lhsSize = lhs.size();
  const unsigned int rhsSize = rhs.size();

  if (lhsSize > rhsSize) {
    return -1;
  } else if (lhsSize < rhsSize) {
    return 1;
  }

  // In case both have the same number of hits, use the one which has advanced less in time
  // TODO: better use the correct hit time here!
  const double lhsLastArcLength = lhs.back().getArcLength();
  const double rhsLastArcLength = rhs.back().getArcLength();

  return lhsLastArcLength < rhsLastArcLength;
}
