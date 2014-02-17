/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCMCTrackStore.h"

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/topology/CDCWireTopology.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <mdst/dataobjects/MCParticle.h>


using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

CDCMCTrackStore::CDCMCTrackStore()
{
}



CDCMCTrackStore::~CDCMCTrackStore()
{
}



void CDCMCTrackStore::clear()
{

  m_ptrMCMap = nullptr;

  m_mcTracksByMCParticleIdx.clear();
  m_mcSegmentsByMCParticleIdx.clear();

  m_inTrackIds.clear();
  m_inTrackSegmentIds.clear();
  m_nPassedSuperLayers.clear();

}



void CDCMCTrackStore::fill(const CDCMCMap* ptrMCMap)
{

  B2DEBUG(200, "In CDCMCTrackStore::fill()");
  clear();

  m_ptrMCMap = ptrMCMap;

  // Put the right hits into the rigth track
  fillMCTracks();

  // Split the tracks into segments
  fillMCSegments();

  // Assigne the reverse mapping from CDCHits to position in track
  fillInTrackId();

  // Assigne the reverse mapping from CDCHits to segment ids
  fillInTrackSegmentId();

  // Assigne the reverse mapping from CDCHits to the number of already traversed superlayers
  fillNPassedSuperLayers();

  B2INFO("m_mcTracksByMCParticleIdx.size(): " << m_mcTracksByMCParticleIdx.size());
  B2INFO("m_mcSegmentsByMCParticleIdx.size(): " << m_mcSegmentsByMCParticleIdx.size());

  B2INFO("m_inTrackSegmentIds.size() " << m_inTrackSegmentIds.size());
  B2INFO("m_inTrackIds.size(): " << m_inTrackIds.size());
  B2INFO("m_nPassedSuperLayers.size(): " << m_nPassedSuperLayers.size());

}


void CDCMCTrackStore::fillMCTracks()
{

  //StoreArray<CDCHit> hits;
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot create tracks");
    return;
  }

  const CDCMCMap& mcMap = *m_ptrMCMap;

  for (const CDCMCMap::MCParticleByCDCHitRelation & relation : mcMap.getMCParticleByHitRelations()) {

    const CDCHit* ptrHit = relation.get<CDCHit>();
    const MCParticle* ptrMCParticle = relation.get<MCParticle>();

    if (not mcMap.isBackground(ptrHit) and ptrMCParticle) {

      int mcParticleIdx = ptrMCParticle->getArrayIndex();
      //Append hit to its own track
      m_mcTracksByMCParticleIdx[mcParticleIdx].push_back(ptrHit);
    }
  } //end for wire hits


  //Sort the tracks along the time of flight
  for (std::pair<const int, CDCHitVector>& mcTrackAndMCParticleIdx : m_mcTracksByMCParticleIdx) {

    //int mcParticleIdx = mcTrackAndMCParticleIdx.first;
    CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;
    arrangeMCTrack(mcTrack);

  }

}

void CDCMCTrackStore::fillMCSegments()
{

  for (std::pair<const int, CDCHitVector>& mcTrackAndMCParticleIdx : m_mcTracksByMCParticleIdx) {

    int mcParticleIdx = mcTrackAndMCParticleIdx.first;
    CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;

    std::vector<CDCHitVector>& mcSegments = m_mcSegmentsByMCParticleIdx[mcParticleIdx];
    mcSegments.clear();

    if (mcTrack.empty()) continue;

    //Savest way is to cluster the elements in the track for their nearest neighbors
    WeightedNeighborhood<const CDCHit> hitNeighborhood;
    const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();

    for (const CDCHit * ptrHit : mcTrack) {

      const CDCHit& hit = *ptrHit;
      WireID wireID(hit.getISuperLayer(), hit.getILayer(), hit.getIWire());

      for (const CDCHit * ptrNeighborHit : mcTrack) {

        if (ptrHit == ptrNeighborHit) continue;

        const CDCHit& neighborHit = *ptrNeighborHit;
        WireID neighborWireID(neighborHit.getISuperLayer(), neighborHit.getILayer(), neighborHit.getIWire());

        if (wireTopology.areNeighbors(wireID, neighborWireID) or wireID == neighborWireID) {
          hitNeighborhood.insert(ptrHit, ptrNeighborHit);
        }

      }

    }

    Clusterizer<CDCHit, CDCHitVector> hitClusterizer;
    hitClusterizer.createFromPointers(mcTrack, hitNeighborhood, mcSegments);
    // mcSegments are not sorted for their time of flight internally, but they are in the right order

    // Lets sort them along for the time of flight.
    for (CDCHitVector & mcSegment : mcSegments) {
      arrangeMCTrack(mcSegment);
    }
  }

}

void CDCMCTrackStore::arrangeMCTrack(CDCHitVector& mcTrack) const
{
  if (not m_ptrMCMap) {
    B2WARNING("CDCMCMap not set. Cannot sort track");
    return;
  }
  const CDCMCMap& mcMap = *m_ptrMCMap;

  std::sort(mcTrack.begin(), mcTrack.end(), [&mcMap](const CDCHit * ptrHit, const CDCHit * ptrOtherHit) -> bool {

    const CDCSimHit* ptrSimHit = mcMap.getSimHit(ptrHit);
    const CDCSimHit* ptrOtherSimHit = mcMap.getSimHit(ptrOtherHit);

    //const CDCSimHit* primarySimHit = getClosestPrimarySimHit(hit);
    //const CDCSimHit* otherPrimarySimHit = getClosestPrimarySimHit(otherHit);

    //double primaryFlightTime = primarySimHit ? primarySimHit->getFlightTime() : simHit->getFlightTime();
    //double otherPrimaryFlightTime = otherPrimarySimHit ? otherPrimarySimHit->getFlightTime() : otherSimHit->getFlightTime();

    double secondaryFlightTime =  ptrSimHit->getFlightTime();
    double otherSecondaryFlightTime =  ptrOtherSimHit->getFlightTime();

    return secondaryFlightTime < otherSecondaryFlightTime;

    //return primaryFlightTime < otherPrimaryFlightTime or
    //(primaryFlightTime == otherPrimaryFlightTime and
    //secondaryFlightTime < otherSecondaryFlightTime);
  });

}






void CDCMCTrackStore::fillInTrackId()
{

  for (const std::pair<int, CDCHitVector>& mcTrackAndMCParticleIdx : getMCTracksByMCParticleIdx()) {

    const CDCHitVector& mcTrack = mcTrackAndMCParticleIdx.second;

    //Fill the in track ids
    int iHit = -1;
    for (const CDCHit * ptrHit : mcTrack) {
      ++iHit;
      m_inTrackIds[ptrHit] = iHit;
    }
  }

}

void CDCMCTrackStore::fillInTrackSegmentId()
{
  for (const std::pair<int, std::vector<CDCHitVector> >& mcSegmentsAndMCParticleIdx : getMCSegmentsByMCParticleIdx()) {
    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;

    int iSegment = -1;
    for (const CDCHitVector &  mcSegment : mcSegments) {
      ++iSegment;
      for (const CDCHit * ptrHit : mcSegment) {

        m_inTrackSegmentIds[ptrHit] = iSegment;
      }
    }
  }

}

void CDCMCTrackStore::fillNPassedSuperLayers()
{

  for (const std::pair<int, std::vector<CDCHitVector> >& mcSegmentsAndMCParticleIdx : getMCSegmentsByMCParticleIdx()) {
    const std::vector<CDCHitVector>& mcSegments = mcSegmentsAndMCParticleIdx.second;

    ILayerType lastISuperLayer = INVALID_ISUPERLAYER;
    int nPassedSuperLayers = 0;

    int iSegment = -1;
    for (const CDCHitVector & mcSegment : mcSegments) {
      ++iSegment;

      for (const CDCHit * ptrHit : mcSegment) {

        if (ptrHit->getISuperLayer() != lastISuperLayer) {
          ++nPassedSuperLayers;
          lastISuperLayer = ptrHit->getISuperLayer();
        }
        m_nPassedSuperLayers[ptrHit] = nPassedSuperLayers;

      }
    }
  }

}



int CDCMCTrackStore::getInTrackId(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_inTrackIds.find(ptrHit);
  return itFoundHit == m_inTrackIds.end() ? -999 : itFoundHit->second;

}



int CDCMCTrackStore::getInTrackSegmentId(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_inTrackSegmentIds.find(ptrHit);
  return itFoundHit == m_inTrackSegmentIds.end() ? -999 : itFoundHit->second;

}


int CDCMCTrackStore::getNPassedSuperLayers(const CDCHit* ptrHit) const
{

  auto itFoundHit = m_nPassedSuperLayers.find(ptrHit);
  return itFoundHit == m_nPassedSuperLayers.end() ? -999 : itFoundHit->second;

}
