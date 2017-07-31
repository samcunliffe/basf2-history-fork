/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/utilities/ProcessingSignalListener.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <tracking/trackFindingCDC/utilities/VectorRange.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class CDCToSpacePointMatcher : public TrackFindingCDC::ProcessingSignalListener {
  public:
    /// Maximal number of ladders per layer
    constexpr static unsigned int maximumLadderNumbers[6] = {8, 12, 7, 10, 12, 16};

    /// return the next hits for a given state, which are the hits on the next layer (or the same for overlaps)
    template<class AStateObject>
    TrackFindingCDC::VectorRange<const SpacePoint*> getMatchingHits(AStateObject& currentState);

    /// Fill the cache of hits for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector);

    /// Expose parameters (if we would have such a thing)
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {
    }

  private:
    /// Cache for sorted hits
    std::map<unsigned int, TrackFindingCDC::VectorRange<const SpacePoint*>> m_cachedHitMap;
  };

  template<class AStateObject>
  TrackFindingCDC::VectorRange<const SpacePoint*> CDCToSpacePointMatcher::getMatchingHits(AStateObject& currentState)
  {
    const unsigned int currentNumber = currentState.getNumber();

    // TODO
    if (currentNumber == 12 or isOnOverlapLayer(currentState)) {
      // next layer is not an overlap one, so we can just return all hits of this layer.
      const unsigned int nextLayer = extractGeometryLayer(currentState) - 1;
      B2DEBUG(100, "Will return all hits from layer " << nextLayer << ", which are " << m_cachedHitMap[nextLayer].size());
      return m_cachedHitMap[nextLayer];
    } else {
      // next layer is an overlap one, so lets return all hits from the same layer, that are on a
      // ladder which is one below the last added hit.
      const SpacePoint* lastAddedSpacePoint = currentState.getHit();
      if (not lastAddedSpacePoint) {
        // No hit was added on the layer, so no overlap can occur.
        return TrackFindingCDC::VectorRange<const SpacePoint*>();
      }

      const unsigned int ladderNumber = lastAddedSpacePoint->getVxdID().getLadderNumber();
      const unsigned int currentLayer = extractGeometryLayer(currentState);
      const unsigned int maximumLadderNumber = CDCToSpacePointMatcher::maximumLadderNumbers[currentLayer];

      // the reason for this strange formula is the numbering scheme in the VXD.
      // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
      // then we subtract one to get to the next (overlapping) ladder and % N, to also cope for the
      // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
      const unsigned int overlappingLadder = ((ladderNumber - 1) - 1) % maximumLadderNumber + 1;

      B2DEBUG(100, "Overlap check on " << ladderNumber << " using from " << overlappingLadder);

      const auto& onNextLadderCheck = [overlappingLadder](const SpacePoint * spacePoint) {

        return spacePoint->getVxdID().getLadderNumber() == overlappingLadder;
      };

      const auto& hitsOfCurrentLayer = m_cachedHitMap[currentLayer];
      const auto first = std::find_if(hitsOfCurrentLayer.begin(), hitsOfCurrentLayer.end(), onNextLadderCheck);
      const auto last = std::find_if_not(first, hitsOfCurrentLayer.end(), onNextLadderCheck);

      B2DEBUG(100, "Overlap " << currentLayer << " " << lastAddedSpacePoint->getVxdID() <<  " " << std::distance(first, last));
      for (auto it = first; it != last; it++) {
        B2DEBUG(100, (*it)->getVxdID());
      }
      return TrackFindingCDC::VectorRange<const SpacePoint*>(first, last);
    }
  }
}
