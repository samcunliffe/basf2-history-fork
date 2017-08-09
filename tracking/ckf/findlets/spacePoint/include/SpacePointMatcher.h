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
#include <tracking/trackFindingCDC/utilities/ChainedIterator.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  class SpacePointMatcher : public TrackFindingCDC::ProcessingSignalListener {
    /// Shortcut for a part of the return type
    using RangeType = TrackFindingCDC::VectorRange<const SpacePoint*>;

  public:
    /// return the next hits for a given state, which are the hits on the next layer (or the same for overlaps)
    template<class AStateObject>
    TrackFindingCDC::ChainedArray<RangeType> getMatchingHits(AStateObject& currentState);

    /// Fill the cache of hits for each event
    void initializeEventCache(std::vector<RecoTrack*>& seedsVector, std::vector<const SpacePoint*>& filteredHitVector);

    /// Expose parameters (if we would have such a thing)
    void exposeParameters(ModuleParamList* moduleParamList __attribute__((unused)),
                          const std::string& prefix __attribute__((unused)))
    {
    }

    /// Calculate the sensor mapping based on the geometry of the detector
    void beginRun();

  private:
    /// Cache for sorted hits
    std::map<VxdID, RangeType> m_cachedHitMap;
    /// "Sectormap"-like structure, which defines, which sensors can be reached from which. Is calculated once per run
    std::multimap<VxdID, VxdID> m_sensorMapping;

    void fillInAllRanges(std::vector<RangeType>& ranges,
                         unsigned short layer, unsigned short ladder = 0);

    void fillInAllRanges(std::vector<RangeType>& ranges, const VxdID& vxdID)
    {
      const auto& nextSensorIDs = m_sensorMapping.equal_range(vxdID);
      for (const auto& nextSensorIDPair : TrackFindingCDC::asRange(nextSensorIDs)) {
        const auto& nextSensorID = nextSensorIDPair.second;
        if (m_cachedHitMap.find(nextSensorID) != m_cachedHitMap.end()) {
          ranges.push_back(m_cachedHitMap[nextSensorID]);
        }
      }
    }
  };

  template<class AStateObject>
  TrackFindingCDC::ChainedArray<SpacePointMatcher::RangeType> SpacePointMatcher::getMatchingHits(AStateObject& currentState)
  {
    const unsigned int currentNumber = currentState.getNumber();
    const unsigned int currentLayer = extractGeometryLayer(currentState);
    const unsigned int nextLayer = currentLayer - 1;
    const SpacePoint* lastAddedSpacePoint = currentState.getHit();

    std::vector<RangeType> nextRanges;

    if (currentNumber == currentState.getMaximumNumber()) {
      // we came directly from a CDC seed, here. We just return all hits on the most outer layer
      const auto* seed = currentState.getSeedRecoTrack();
      const auto& svdHits = seed->getSortedSVDHitList();
      if (svdHits.size() > 0) {
        const SVDCluster* firstSVDCluster = svdHits.front();
        const auto& currentID = firstSVDCluster->getSensorID();
        if (currentID.getLayerNumber() == 3) {
          fillInAllRanges(nextRanges, currentID);
        } else {
          fillInAllRanges(nextRanges, nextLayer);
        }
      } else {
        // TODO: We can do better here, e.g use the CDC trajectory!
        fillInAllRanges(nextRanges, nextLayer);
      }
    } else if (isOnOverlapLayer(currentState)) {
      // next layer is not an overlap one, so we can just return all hits of the next layer,
      // that are in our sensor mapping. If there is no hit on this (overlap) layer, we use the hit
      // from the parent,
      const auto* parent = currentState.getParent();
      if (not lastAddedSpacePoint and parent) {
        lastAddedSpacePoint = parent->getHit();
      }
      // However, if there was still no former hit (no hit in this current layer and on the parent)
      // we just return all hits of the next layer.
      if (lastAddedSpacePoint) {
        const auto& currentID = lastAddedSpacePoint->getVxdID();
        fillInAllRanges(nextRanges, currentID);
      } else {
        fillInAllRanges(nextRanges, nextLayer);
      }
    } else {
      // next layer is an overlap one, so lets return all hits from the same layer, that are on a
      // ladder which is one below the last added hit.
      if (lastAddedSpacePoint) {
        // No hit was added on the layer, so no overlap can occur.
        const auto& vxdID = lastAddedSpacePoint->getVxdID();
        const unsigned int ladderNumber = vxdID.getLadderNumber();
        const unsigned int maximumLadderNumber = VXD::GeoCache::getInstance().getLadders(vxdID).size();

        int direction = 1;
        if (currentLayer > 2) {
          direction = -1;
        }

        // the reason for this strange formula is the numbering scheme in the VXD.
        // we first substract 1 from the ladder number to have a ladder counting from 0 to N - 1,
        // then we add (PXD)/subtract(SVD) one to get to the next (overlapping) ladder and do a % N to also cope for the
        // highest number. Then we add 1 again, to go from the counting from 0 .. N-1 to 1 .. N.
        // The + maximumLadderNumber in between makes sure, we are not ending with negative numbers
        const unsigned int overlappingLadder = ((ladderNumber + maximumLadderNumber - 1) + direction) % maximumLadderNumber + 1;

        B2DEBUG(100, "Overlap check on " << ladderNumber << " using from " << overlappingLadder);
        fillInAllRanges(nextRanges, currentLayer, overlappingLadder);
      }
    }

    return TrackFindingCDC::ChainedArray<SpacePointMatcher::RangeType>(nextRanges);
  }
}
