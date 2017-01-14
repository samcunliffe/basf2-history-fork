/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/mcMatcher/TrackMatchLookUp.h>

#include <framework/datastore/DataStore.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

TrackMatchLookUp::TrackMatchLookUp(const std::string& mcRecoTrackStoreArrayName,
                                   const std::string& prRecoTrackStoreArrayName)
  : m_mcTracksStoreArrayName(DataStore::arrayName<RecoTrack>(mcRecoTrackStoreArrayName))
  , m_prTracksStoreArrayName(DataStore::arrayName<RecoTrack>(prRecoTrackStoreArrayName))
{
  if (m_mcTracksStoreArrayName == m_prTracksStoreArrayName) {
    B2WARNING("Pattern recognition and Monte Carlo track StoreArray are the same.");
  }
}

bool TrackMatchLookUp::isMCRecoTrack(const RecoTrack& recoTrack)
{
  return recoTrack.getArrayName() == getMCTracksStoreArrayName();
}

bool TrackMatchLookUp::isPRRecoTrack(const RecoTrack& recoTrack)
{
  return recoTrack.getArrayName() == getPRTracksStoreArrayName();
}

const RecoTrack*
TrackMatchLookUp::getRelatedMCRecoTrack(const RecoTrack& prRecoTrack, float& purity)
{
  std::pair<RecoTrack*, double> mcRecoTrackAndWeight =
    prRecoTrack.getRelatedWithWeight<RecoTrack>(getMCTracksStoreArrayName());
  const RecoTrack* mcRecoTrack = mcRecoTrackAndWeight.first;
  if (mcRecoTrack) {
    purity = mcRecoTrackAndWeight.second;
  } else {
    purity = NAN;
  }
  return mcRecoTrack;
}

const RecoTrack*
TrackMatchLookUp::getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack, float& efficiency)
{
  std::pair<RecoTrack*, double> prRecoTrackAndWeight =
    mcRecoTrack.getRelatedWithWeight<RecoTrack>(getPRTracksStoreArrayName());
  const RecoTrack* prRecoTrack = prRecoTrackAndWeight.first;
  if (prRecoTrack) {
    efficiency = prRecoTrackAndWeight.second;
  } else {
    efficiency = NAN;
  }
  return prRecoTrack;
}

TrackMatchLookUp::MCToPRMatchInfo
TrackMatchLookUp::extractMCToPRMatchInfo(const RecoTrack* prRecoTrack, const float& efficiency)
{
  if (not prRecoTrack) return MCToPRMatchInfo::c_Missing;
  if (efficiency < 0) {
    return MCToPRMatchInfo::c_Merged;
  } else if (efficiency > 0) {
    return MCToPRMatchInfo::c_Matched;
  }
  return MCToPRMatchInfo::c_Invalid;
}

TrackMatchLookUp::PRToMCMatchInfo
TrackMatchLookUp::extractPRToMCMatchInfo(const RecoTrack& prRecoTrack,
                                         const RecoTrack* mcRecoTrack,
                                         const float& purity)
{
  if (not mcRecoTrack) {
    // The patter recognition track has no associated Monte Carlo track.
    // Figure out of it is a clone or a match by the McTrackId property assigned to the track.
    // That is also why we need the pattern recogntion track in this method as well.
    const RecoTrack::MatchingStatus matchingStatus = prRecoTrack.getMatchingStatus();

    if (matchingStatus == RecoTrack::MatchingStatus::c_ghost) {
      return PRToMCMatchInfo::c_Ghost;
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_background) {
      return PRToMCMatchInfo::c_Background;
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_clone) {
      // MCTrackMatcher is running without
      // RelateClonesToMcParticles
      return PRToMCMatchInfo::c_Clone;
    } else if (matchingStatus == RecoTrack::MatchingStatus::c_undefined) {
      return PRToMCMatchInfo::c_Invalid;
    }

  } else {
    // The patter recognition track has an associated Monte Carlo track.
    // Figure out of it is a clone or a match by the sign of the purity.
    if (purity < 0) {
      return PRToMCMatchInfo::c_Clone;
    } else if (purity > 0) {
      return PRToMCMatchInfo::c_Matched;
    } else {
      return PRToMCMatchInfo::c_Invalid;
    }
  }
  return PRToMCMatchInfo::c_Invalid;
}

const MCParticle* TrackMatchLookUp::getRelatedMCParticle(const RecoTrack& prRecoTrack)
{
  return prRecoTrack.getRelated<MCParticle>();
}

const RecoTrack* TrackMatchLookUp::getRelatedMCRecoTrack(const RecoTrack& prRecoTrack)
{
  return prRecoTrack.getRelated<RecoTrack>(getMCTracksStoreArrayName());
}

const TrackFitResult* TrackMatchLookUp::getRelatedTrackFitResult(const RecoTrack& prRecoTrack,
    Const::ChargedStable chargedStable)
{
  Belle2::Track* b2track = prRecoTrack.getRelated<Belle2::Track>();
  if (b2track) {
    // Query the Belle2::Track for the selected fit hypothesis
    return b2track->getTrackFitResult(chargedStable);
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getRelatedPRRecoTrack(const RecoTrack& mcRecoTrack)
{
  return mcRecoTrack.getRelated<RecoTrack>(getPRTracksStoreArrayName());
}

float TrackMatchLookUp::getRelatedPurity(const RecoTrack& prRecoTrack)
{
  float purity = NAN;
  getRelatedMCRecoTrack(prRecoTrack, purity);
  return std::fabs(purity);
}

float TrackMatchLookUp::getRelatedEfficiency(const RecoTrack& mcRecoTrack)
{
  float efficiency = NAN;
  getRelatedPRRecoTrack(mcRecoTrack, efficiency);
  return std::fabs(efficiency);
}

const RecoTrack* TrackMatchLookUp::getMatchedMCRecoTrack(const RecoTrack& prRecoTrack)
{
  float purity = NAN;
  const RecoTrack* mcRecoTrack = getRelatedMCRecoTrack(prRecoTrack, purity);

  if (extractPRToMCMatchInfo(prRecoTrack, mcRecoTrack, purity) == PRToMCMatchInfo::c_Matched) {
    return mcRecoTrack;
  } else {
    return nullptr;
  }
}

const RecoTrack* TrackMatchLookUp::getMatchedPRRecoTrack(const RecoTrack& mcRecoTrack)
{
  float efficiency = NAN;
  const RecoTrack* prRecoTrack = getRelatedPRRecoTrack(mcRecoTrack, efficiency);

  if (extractMCToPRMatchInfo(prRecoTrack, efficiency) == MCToPRMatchInfo::c_Matched) {
    return prRecoTrack;
  } else {
    return nullptr;
  }
}

float TrackMatchLookUp::getMatchedPurity(const RecoTrack& recoTrack)
{
  if (isMCRecoTrack(recoTrack)) {
    const RecoTrack& mcRecoTrack = recoTrack;
    const RecoTrack* prRecoTrack = getMatchedPRRecoTrack(mcRecoTrack);
    if (prRecoTrack) {
      return getRelatedPurity(*prRecoTrack);
    } else {
      return NAN;
    }

  } else {
    const RecoTrack& prRecoTrack = recoTrack;
    return getRelatedPurity(prRecoTrack);
  }
}

float TrackMatchLookUp::getMatchedEfficiency(const RecoTrack& recoTrack)
{
  if (isPRRecoTrack(recoTrack)) {
    const RecoTrack& prRecoTrack = recoTrack;
    const RecoTrack* mcRecoTrack = getMatchedMCRecoTrack(prRecoTrack);
    if (mcRecoTrack) {
      return getRelatedEfficiency(*mcRecoTrack);
    } else {
      return NAN;
    }

  } else {
    const RecoTrack& mcRecoTrack = recoTrack;
    return getRelatedPurity(mcRecoTrack);
  }
}
