/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/CurlerCloneTruthVarSet.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCCurlerCloneLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void CurlerCloneTruthVarSet::initialize()
{
  CDCMCManager::getInstance().requireTruthInformation();
  Super::initialize();
}

void CurlerCloneTruthVarSet::beginEvent()
{
  CDCMCManager::getInstance().fill();
  Super::beginEvent();
}

bool CurlerCloneTruthVarSet::extract(const CDCTrack* ptrCDCTrack)
{
  if (not ptrCDCTrack) return false;

  /// Find the MC track with the highest number of hits in the segment
  const CDCMCTrackLookUp& mcTrackLookUp = CDCMCTrackLookUp::getInstance();
  const CDCMCHitLookUp& hitLookUp = CDCMCHitLookUp::getInstance();
  CDCMCCurlerCloneLookUp& curlerCloneLookUp = CDCMCCurlerCloneLookUp::getInstance();

  ITrackType trackMCMatch = mcTrackLookUp.getMCTrackId(ptrCDCTrack);
  bool trackIsFake = false;

  /// Flag if CDCTrack is matched in CDCMCTrackLookUp, which uses m_minimalMatchPurity = 0.5
  bool trackHasMinimalMatchPurity = (trackMCMatch != INVALID_ITRACK);

  if (not trackHasMinimalMatchPurity) {
    trackIsFake = true;
  } else {
    // count number of correct hits
    auto hitIsCorrect = [&hitLookUp, &trackMCMatch](const CDCRecoHit3D & recoHit) {
      return hitLookUp.getMCTrackId(recoHit.getWireHit().getHit()) == trackMCMatch;
    };
    unsigned int numberOfCorrectHits =
      std::count_if(ptrCDCTrack->begin(), ptrCDCTrack->end(), hitIsCorrect);

    /// For information if CDCTrack is fake or true, use stricter 80% threshold
    if ((double)numberOfCorrectHits / ptrCDCTrack->size() < 0.8) {
      trackIsFake = true;
    } else {
      trackIsFake = false;
    }
  }
  bool trackIsClone = curlerCloneLookUp.isTrackCurlerClone(*ptrCDCTrack);

  var<named("weight")>() = trackHasMinimalMatchPurity;
  var<named("track_is_fake")>() = trackIsFake;
  var<named("track_is_matched")>() = not trackIsFake;
  var<named("track_is_curler_clone_truth")>() = trackIsClone;
  var<named("truth")>() = not trackIsClone;
  var<named("truth_first_nloops")>() = mcTrackLookUp.getFirstNLoops(ptrCDCTrack);
  var<named("truth_event_id")>() = m_eventMetaData->getEvent();
  var<named("truth_MCTrackID")>() = trackMCMatch;
  return true;
}
