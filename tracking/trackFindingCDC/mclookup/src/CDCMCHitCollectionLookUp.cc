/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/mclookup/CDCMCHitCollectionLookUp.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <TDatabasePDG.h>
#include <boost/range/adaptor/reversed.hpp>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


template<class ACDCHitCollection>
void
CDCMCHitCollectionLookUp<ACDCHitCollection>
::clear()
{
  B2DEBUG(100, "Clearing CDCMCHitCollectionLookUp<ACDCHitCollection>");
  m_mcTrackIds.clear();
  B2DEBUG(100, "m_mcTrackIds.size(): " <<  m_mcTrackIds.size());
}




template<class ACDCHitCollection>
const float
CDCMCHitCollectionLookUp<ACDCHitCollection>
::s_minimalMatchPurity = 0.5;



template<class ACDCHitCollection>
ITrackType
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getMCTrackId(const ACDCHitCollection* ptrHits) const
{

  if (not ptrHits) return INVALID_ITRACK;

  const ACDCHitCollection& hits = *ptrHits;
  auto itFound = m_mcTrackIds.find(ptrHits);

  if (itFound == m_mcTrackIds.end()) {
    MCTrackIdPurityPair mcTrackIdAndPurity = getHighestPurity(hits);
    if (mcTrackIdAndPurity.getPurity() >= s_minimalMatchPurity) {
      m_mcTrackIds[ptrHits] = mcTrackIdAndPurity.getMCTrackId();
      return mcTrackIdAndPurity.getMCTrackId();
    } else {
      m_mcTrackIds[ptrHits] = INVALID_ITRACK;
      return INVALID_ITRACK;
    }

  } else {
    return itFound->second;
  }
}


template<class ACDCHitCollection>
const MCParticle*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getMCParticle(const ACDCHitCollection* ptrHits) const
{
  const CDCHit* ptrHit = getFirstHit(ptrHits);
  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();
  return mcHitLookUp.getMCParticle(ptrHit);
}


template<class ACDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getFirstHit(const ACDCHitCollection* ptrHits) const
{
  if (not ptrHits) return nullptr;
  const ACDCHitCollection& hits = *ptrHits;

  ITrackType mcTrackId = getMCTrackId(ptrHits);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCHit* hit : hits) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
  }
  return nullptr;

}




template<class ACDCHitCollection>
const CDCHit*
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getLastHit(const ACDCHitCollection* ptrHits) const
{

  if (not ptrHits) return nullptr;
  const ACDCHitCollection& hits = *ptrHits;

  ITrackType mcTrackId = getMCTrackId(ptrHits);
  if (mcTrackId == INVALID_ITRACK) return nullptr;

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  for (const CDCHit* hit : boost::adaptors::reverse(hits)) {
    if (mcTrackId == mcHitLookUp.getMCTrackId(hit)) return hit;
  }
  return nullptr;

}




template<class ACDCHitCollection>
EForwardBackward
CDCMCHitCollectionLookUp<ACDCHitCollection>
::isForwardOrBackwardToMCTrack(const ACDCHitCollection* ptrHits) const
{
  Index firstInTrackId = getFirstInTrackId(ptrHits);
  Index lastInTrackId = getLastInTrackId(ptrHits);
  if (firstInTrackId == INVALID_INDEX or lastInTrackId == INVALID_INDEX) return EForwardBackward::c_Invalid;
  else if (firstInTrackId < lastInTrackId) return EForwardBackward::c_Forward;
  else if (firstInTrackId > lastInTrackId) return EForwardBackward::c_Backward;
  else if (firstInTrackId == lastInTrackId) return EForwardBackward::c_Unknown;
  return EForwardBackward::c_Invalid;
}




template<class ACDCHitCollection>
EForwardBackward
CDCMCHitCollectionLookUp<ACDCHitCollection>
::areAlignedInMCTrack(const ACDCHitCollection* ptrStartSegment2D,
                      const ACDCHitCollection* ptrEndSegment2D) const
{

  // Check if the segments are sensable on their own
  EForwardBackward startFBInfo = isForwardOrBackwardToMCTrack(ptrStartSegment2D);
  if (startFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

  EForwardBackward endFBInfo = isForwardOrBackwardToMCTrack(ptrEndSegment2D);
  if (endFBInfo == EForwardBackward::c_Invalid) return EForwardBackward::c_Invalid;

  if (startFBInfo != endFBInfo) return EForwardBackward::c_Invalid;


  /// Check if the segments are in the same track
  ITrackType startMCTrackId = getMCTrackId(ptrStartSegment2D);
  if (startMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

  ITrackType endMCTrackId = getMCTrackId(ptrEndSegment2D);
  if (endMCTrackId == INVALID_ITRACK) return EForwardBackward::c_Invalid;

  if (startMCTrackId != endMCTrackId) return EForwardBackward::c_Invalid;


  // Now check are aligned with their common track
  // Examine if they are in the same super layer

  Index lastNPassedSuperLayersOfStartSegment = getLastNPassedSuperLayers(ptrStartSegment2D);
  if (lastNPassedSuperLayersOfStartSegment == INVALID_INDEX) return EForwardBackward::c_Invalid;

  Index firstNPassedSuperLayersOfEndSegment = getFirstNPassedSuperLayers(ptrEndSegment2D);
  if (firstNPassedSuperLayersOfEndSegment == INVALID_INDEX) return EForwardBackward::c_Invalid;

  if (lastNPassedSuperLayersOfStartSegment < firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == EForwardBackward::c_Forward and  endFBInfo == EForwardBackward::c_Forward) return EForwardBackward::c_Forward;
    else return EForwardBackward::c_Invalid;
  } else if (lastNPassedSuperLayersOfStartSegment > firstNPassedSuperLayersOfEndSegment) {
    if (startFBInfo == EForwardBackward::c_Backward and endFBInfo == EForwardBackward::c_Backward) return EForwardBackward::c_Backward;
    else return EForwardBackward::c_Invalid;
  }

  // Now we are in the same passed super layer with both segments
  Index lastInTrackIdOfStartSegment = getLastInTrackId(ptrStartSegment2D);
  if (lastInTrackIdOfStartSegment == INVALID_INDEX) return EForwardBackward::c_Invalid;

  Index firstInTrackIdOfEndSegment = getFirstInTrackId(ptrEndSegment2D);
  if (firstInTrackIdOfEndSegment == INVALID_INDEX) return EForwardBackward::c_Invalid;

  if (startFBInfo == EForwardBackward::c_Forward and endFBInfo == EForwardBackward::c_Forward) {
    if (lastInTrackIdOfStartSegment < firstInTrackIdOfEndSegment) return EForwardBackward::c_Forward;
    else return EForwardBackward::c_Invalid;
  } else if (startFBInfo == EForwardBackward::c_Backward and endFBInfo == EForwardBackward::c_Backward) {
    // Test if end segment lies before in the mc track
    // Hence the whole pair of segments is reverse to the track direction of flight
    if (lastInTrackIdOfStartSegment > firstInTrackIdOfEndSegment) return EForwardBackward::c_Backward;
    else return EForwardBackward::c_Invalid;
  }
  return EForwardBackward::c_Invalid;

}




template<class ACDCHitCollection>
CDCTrajectory3D
CDCMCHitCollectionLookUp<ACDCHitCollection>
::getTrajectory3D(const ACDCHitCollection* ptrHits) const
{
  CDCTrajectory3D trajectory3D;

  if (not ptrHits) {
    B2WARNING("Segment is nullptr. Could not get fit.");
    return trajectory3D;
  }

  const CDCMCHitLookUp& mcHitLookUp = CDCMCHitLookUp::getInstance();

  const CDCHit* ptrFirstHit = getFirstHit(ptrHits);
  const CDCSimHit* ptrPrimarySimHit = mcHitLookUp.getClosestPrimarySimHit(ptrFirstHit);

  if (not ptrPrimarySimHit) {
    // If there is no primary SimHit simply use the secondary simhit as reference
    ptrPrimarySimHit = mcHitLookUp.getSimHit(ptrFirstHit);
    if (not ptrPrimarySimHit) {
      B2WARNING("First simhit of CDCRecoSegment is nullptr. Could not get fit.");
      return trajectory3D;
    }
  }

  const CDCSimHit& primarySimHit = *ptrPrimarySimHit;

  Vector3D mom3D{primarySimHit.getMomentum()};
  Vector3D pos3D{primarySimHit.getPosTrack()};

  int pdgCode = primarySimHit.getPDGCode();
  const TParticlePDG* ptrTPDGParticle = TDatabasePDG::Instance()->GetParticle(pdgCode);

  if (not ptrTPDGParticle) {
    B2WARNING("No particle for PDG code " << pdgCode << ". Could not get fit");
    return trajectory3D;
  }

  const TParticlePDG& tPDGParticle = *ptrTPDGParticle;

  double charge = tPDGParticle.Charge() / 3.0;

  ESign chargeSign = sign(charge);

  trajectory3D.setPosMom3D(pos3D, mom3D, charge);

  ESign settedChargeSign = trajectory3D.getChargeSign();

  if (chargeSign != settedChargeSign) {
    B2WARNING("Charge sign of mc particle is not the same as the one of the fit");
  }

  return trajectory3D;
}


// Explicit instantiation for the relevant cases.
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCRecoSegment2D>;
template class Belle2::TrackFindingCDC::CDCMCHitCollectionLookUp<CDCTrack>;
