/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/HitProcessor.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include "../include/TrackHitsProcessor.h"
#include "tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h"

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;

void HitProcessor::updateRecoHit3D(CDCTrajectory2D& trackTrajectory2D, CDCRecoHit3D& hit)
{
  hit.setRecoPos3D(hit.getRecoHit2D().getRLWireHit().reconstruct3D(trackTrajectory2D));

  double perpS = trackTrajectory2D.calcArcLength2D(hit.getRecoPos2D());
  if (perpS < 0.) {
    double perimeter = fabs(trackTrajectory2D.getGlobalCircle().perimeter()) / 2.;
    perpS += perimeter;
  }
  // Recalculate the perpS of the hits
  hit.setArcLength2D(perpS);

}


void HitProcessor::appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<QuadTreeHitWrapper*>& axialHitList)
{
  for (CDCTrack& trackCandidate : trackCandidates) {
    if (trackCandidate.size() < 5) continue;

    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//    ESign trackCharge = TrackMergerNew::getChargeSign(trackCandidate);
    CDCTrajectory2D trackTrajectory2D = trackCandidate.getStartTrajectory3D().getTrajectory2D();


    for (const QuadTreeHitWrapper* hit : axialHitList) {
      if (hit->getUsedFlag() || hit->getMaskedFlag()) continue;

      ERightLeft rlInfo = ERightLeft::c_Right;
      if (trackTrajectory2D.getDist2D(hit->getCDCWireHit()->getRefPos2D()) < 0)
        rlInfo = ERightLeft::c_Left;
      const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(hit->getCDCWireHit()->getHit(), rlInfo);
      if (rlWireHit->getWireHit().getAutomatonCell().hasTakenFlag())
        continue;

      //        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
      //          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(*rlWireHit, trackTrajectory2D);

      if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.1) {
        trackCandidate.push_back(std::move(cdcRecoHit3D));
        cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
      }
    }

  }
  /*
  for (TrackHit* hit : axialHitList) {
    if (hit->getHitUsage() == TrackHit::c_usedInTrack or
        hit->getHitUsage() == TrackHit::c_bad) continue;

    // Search for best candidate to assign to
    double bestHitProb = 0;
    TrackCandidate* bestCandidate = nullptr;

    for (TrackCandidate* cand : trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      if (probTemp > bestHitProb) {
        bestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestCandidate != nullptr and bestHitProb > minimal_assignment_probability) {
      bestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::c_usedInTrack);
    }

  }
  */
}

void HitProcessor::deleteAllMarkedHits(CDCTrack& trackCandidate)
{

  trackCandidate.erase(
  std::remove_if(trackCandidate.begin(), trackCandidate.end(), [](const CDCRecoHit3D & hit) {
    if (hit.getWireHit().getAutomatonCell().hasMaskedFlag()) {
      return true;
    }
    return false;
  }),
  trackCandidate.end());

}

void HitProcessor::deleteWrongHitsOfTrack(CDCTrack& trackCandidate)
{
  /*
  assert(trackCandidate);

  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackHits.size() == 0) return;

  for (TrackHit* hit : trackHits) {
    assert(hit);
    hit->setHitUsage(TrackHit::c_usedInTrack);
  }

  int ndf = trackHits.size() - 4;

  if (ndf <= 0) return;

  for (TrackHit* trackHit : trackHits) {
    assert(trackHit);
    double assignment_probability = getAssigmentProbability(trackHit, trackCandidate);

    if (assignment_probability < minimal_assignment_probability) {
      trackHit->setHitUsage(TrackHit::c_bad);
    }
  }
  */
  deleteAllMarkedHits(trackCandidate);

}


/*
double SimpleFilter::getAssigmentProbability(const TrackHit* hit, const TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getWirePosition().X();
  double y0_hit = hit->getWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) *
                                   (y0_track - y0_hit))) - hit->getDriftLength());

  return 1.0 - exp(-1 / dist);
}
*/


void HitProcessor::reassignHitsFromOtherTracks(std::list<CDCTrack>& trackCandidates)
{

  return;
  std::vector<std::pair<CDCRecoHit3D, CDCTrack>> assignedHits;
  for (CDCTrack& cand : trackCandidates) {

    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag(true);
      recoHit.getWireHit().getAutomatonCell().setMaskedFlag(false);

      assignedHits.push_back(std::make_pair(recoHit, cand));
    }
  }


  B2DEBUG(100, "NCands = " << trackCandidates.size());

  for (std::pair<CDCRecoHit3D, CDCTrack>& itemWithCand : assignedHits) {

    CDCRecoHit3D& item = itemWithCand.first;
    CDCTrack& cand = itemWithCand.second;


    CDCTrajectory2D trajectory = cand.getStartTrajectory3D().getTrajectory2D();

    HitProcessor::updateRecoHit3D(trajectory, item);
    double dist = fabs(trajectory.getDist2D(item.getRecoPos2D()));

    double bestHitDist = dist;
    CDCTrack* bestCandidate = NULL;

    for (CDCTrack& candInner : trackCandidates) {
      if (candInner == cand) continue;
      CDCTrajectory2D trajectoryInner = candInner.getStartTrajectory3D().getTrajectory2D();

      HitProcessor::updateRecoHit3D(trajectoryInner, item);
      double distTemp = fabs(trajectoryInner.getDist2D(item.getRecoPos2D()));

      if (distTemp < bestHitDist) {
        bestCandidate = &candInner;
        bestHitDist = distTemp;
      }
    }

    if (bestHitDist < dist) {
      const CDCRecoHit3D& cdcRecoHit3D  =  CDCRecoHit3D::reconstruct(item.getRLWireHit(),
                                           bestCandidate->getStartTrajectory3D().getTrajectory2D());

      bestCandidate->push_back(std::move(cdcRecoHit3D));
      item.getWireHit().getAutomatonCell().setMaskedFlag(true);
      deleteAllMarkedHits(cand);
      cdcRecoHit3D.getWireHit().getAutomatonCell().setMaskedFlag(false);
    }

  }


}

void HitProcessor::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
  }
}
