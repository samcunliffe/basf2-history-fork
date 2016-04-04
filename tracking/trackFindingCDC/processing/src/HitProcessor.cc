/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/collections/CDCTrackList.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>

#include <tracking/trackFindingCDC/processing/TrackProcessor.h>

#include <TMath.h>

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


void HitProcessor::appendUnusedHits(std::vector<CDCTrack>& trackCandidates, const std::vector<ConformalCDCWireHit*>& axialHitList)
{
  for (CDCTrack& trackCandidate : trackCandidates) {
    if (trackCandidate.size() < 5) continue;

    const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
//    ESign trackCharge = TrackMergerNew::getChargeSign(trackCandidate);
    CDCTrajectory2D trackTrajectory2D = trackCandidate.getStartTrajectory3D().getTrajectory2D();


    for (const ConformalCDCWireHit* hit : axialHitList) {
      if (hit->getUsedFlag() || hit->getMaskedFlag()) continue;

      ERightLeft rlInfo = trackTrajectory2D.isRightOrLeft(hit->getCDCWireHit()->getRefPos2D());
      // Is this lookup really necessary?
      const CDCWireHit* wireHit = wireHitTopology.getWireHit(hit->getCDCWireHit()->getHit());
      CDCRLTaggedWireHit rlWireHit(wireHit, rlInfo);
      if (wireHit->getAutomatonCell().hasTakenFlag())
        continue;

      //        if(fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
      //          if(TrackMergerNew::getCurvatureSignWrt(cdcRecoHit3D, track.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) continue;

      const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trackTrajectory2D);

      if (fabs(trackTrajectory2D.getDist2D(cdcRecoHit3D.getRecoPos2D())) < 0.1) {
        trackCandidate.push_back(std::move(cdcRecoHit3D));
        cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag(true);
      }
    }

  }
}

void HitProcessor::reassignHitsFromOtherTracks(CDCTrackList& cdcTrackList)
{
  std::vector<std::pair<CDCRecoHit3D, CDCTrack>> assignedHits;
  cdcTrackList.doForAllTracks([&assignedHits](CDCTrack & cand) {
    for (CDCRecoHit3D& recoHit : cand) {
      recoHit.getWireHit().getAutomatonCell().setTakenFlag(true);
      recoHit.getWireHit().getAutomatonCell().setMaskedFlag(false);

      assignedHits.push_back(std::make_pair(recoHit, cand));
    }
  });


  B2DEBUG(100, "NCands = " << cdcTrackList.getCDCTracks().size());

  for (std::pair<CDCRecoHit3D, CDCTrack>& itemWithCand : assignedHits) {

    CDCRecoHit3D& item = itemWithCand.first;
    CDCTrack& cand = itemWithCand.second;


    CDCTrajectory2D trajectory = cand.getStartTrajectory3D().getTrajectory2D();

    HitProcessor::updateRecoHit3D(trajectory, item);
    double dist = fabs(trajectory.getDist2D(item.getRecoPos2D()));

    double bestHitDist = dist;
    CDCTrack* bestCandidate = NULL;

    cdcTrackList.doForAllTracks([&cand, &item, &bestHitDist, &bestCandidate](CDCTrack & candInner) {
      if (candInner == cand) return;
      CDCTrajectory2D trajectoryInner = candInner.getStartTrajectory3D().getTrajectory2D();

      HitProcessor::updateRecoHit3D(trajectoryInner, item);
      double distTemp = fabs(trajectoryInner.getDist2D(item.getRecoPos2D()));

      if (distTemp < bestHitDist) {
        bestCandidate = &candInner;
        bestHitDist = distTemp;
      }
    });

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

std::vector<const CDCWireHit*> HitProcessor::splitBack2BackTrack(CDCTrack& trackCandidate)
{
  std::vector<const CDCWireHit*> removedHits;

  if (trackCandidate.size() < 5) return removedHits;

  for (CDCRecoHit3D& hit : trackCandidate) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }


  if (isBack2BackTrack(trackCandidate)) {

    ESign trackCharge = getChargeSign(trackCandidate);

    for (const CDCRecoHit3D& hit : trackCandidate) {

      if (getCurvatureSignWrt(hit, trackCandidate.getStartTrajectory3D().getGlobalCircle().center()) != trackCharge) {
        hit.getWireHit().getAutomatonCell().setMaskedFlag(true);
        hit.getWireHit().getAutomatonCell().setTakenFlag(false);
      }

    }

    for (CDCRecoHit3D& hit : trackCandidate) {
      if (hit.getWireHit().getAutomatonCell().hasMaskedFlag())
        removedHits.push_back(&(hit.getWireHit()));
    }

    deleteAllMarkedHits(trackCandidate);

    for (const CDCWireHit* hit : removedHits) {
      hit->getAutomatonCell().setMaskedFlag(false);
      hit->getAutomatonCell().setTakenFlag(false);
    }


  }

  return removedHits;
}

bool HitProcessor::isBack2BackTrack(CDCTrack& track)
{
  int vote_pos = 0;
  int vote_neg = 0;

  for (const CDCRecoHit3D& hit : track) {
    int curve_sign = getCurvatureSignWrt(hit, track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().center());

    if (curve_sign == ESign::c_Plus)
      ++vote_pos;
    else if (curve_sign == ESign::c_Minus)
      ++vote_neg;
    else {
      B2ERROR(
        "Strange behaviour of TrackHit::getCurvatureSignWrt");
      exit(EXIT_FAILURE);
    }
  }

  if ((fabs(vote_pos - vote_neg) / (double)(vote_pos + vote_neg) < 1.)
      && fabs(track.getStartTrajectory3D().getTrajectory2D().getGlobalCircle().radius()) > 60.)
    return true;

  return false;
}

void HitProcessor::deleteAllMarkedHits(CDCTrack& track)
{
  track.erase(
  std::remove_if(track.begin(), track.end(), [](const CDCRecoHit3D & hit) {
    return hit.getWireHit().getAutomatonCell().hasMaskedFlag();
  }), track.end());
}

void HitProcessor::deleteAllMarkedHits(std::vector<const CDCWireHit*>& wireHits)
{
  wireHits.erase(std::remove_if(wireHits.begin(), wireHits.end(),
  [&](const CDCWireHit * hit) {
    return hit->getAutomatonCell().hasMaskedFlag();
  }), wireHits.end());
}


ESign HitProcessor::getChargeSign(CDCTrack& track)
{
  int vote_pos(0), vote_neg(0);

  Vector2D center(track.getStartTrajectory3D().getGlobalCircle().center());

  if (std::isnan(center.x())) {
    B2WARNING("Trajectory is not setted or wrong!");
    return track.getStartTrajectory3D().getChargeSign();
  }

  for (const CDCRecoHit3D& hit : track) {
    ESign curve_sign = getCurvatureSignWrt(hit, center);

    if (curve_sign == ESign::c_Plus)
      ++vote_pos;
    else if (curve_sign == ESign::c_Minus)
      ++vote_neg;
    else {
      B2FATAL("Strange behaviour of TrackHit::getCurvatureSignWrt");
    }
  }

  if (vote_pos > vote_neg)
    return ESign::c_Plus;
  else
    return ESign::c_Minus;
}

ESign HitProcessor::getCurvatureSignWrt(const CDCRecoHit3D& hit, Vector2D xy)
{
  double phi_diff = atan2(xy.y(), xy.x()) - hit.getRecoPos3D().phi();

  while (phi_diff > /*2 */ TMath::Pi())
    phi_diff -= 2 * TMath::Pi();
  while (phi_diff < -1. * TMath::Pi())
    phi_diff += 2 * TMath::Pi();

  if (phi_diff > 0 /*TMath::Pi()*/)
    return ESign::c_Minus;
  else
    return ESign::c_Plus;

}

void HitProcessor::resetMaskedHits(CDCTrackList& cdcTrackList, std::vector<ConformalCDCWireHit>& conformalCDCWireHitList)
{
  for (ConformalCDCWireHit& hit : conformalCDCWireHitList) {
    hit.setMaskedFlag(false);
    hit.setUsedFlag(false);
  }

  cdcTrackList.doForAllTracks([](const CDCTrack & cdcTrack) {
    cdcTrack.forwardTakenFlag();
  });
}

void HitProcessor::unmaskHitsInTrack(CDCTrack& track)
{
  for (const CDCRecoHit3D& hit : track) {
    hit.getWireHit().getAutomatonCell().setTakenFlag(true);
    hit.getWireHit().getAutomatonCell().setMaskedFlag(false);
  }
}

void HitProcessor::deleteHitsFarAwayFromTrajectory(CDCTrack& track, double maximum_distance)
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  for (CDCRecoHit3D& recoHit : track) {
    const Vector2D& recoPos2D = recoHit.getRecoPos2D();
    if (fabs(trajectory2D.getDist2D(recoPos2D)) > maximum_distance)
      recoHit->getWireHit().getAutomatonCell().setMaskedFlag(true);
  }

  deleteAllMarkedHits(track);
}

void HitProcessor::assignNewHitsToTrack(CDCTrack& track, const std::vector<ConformalCDCWireHit>& conformalCDCWireHitList,
                                        double minimal_distance_to_track)
{
  if (track.size() < 10) return;
  unmaskHitsInTrack(track);

  const CDCTrajectory2D& trackTrajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  for (const ConformalCDCWireHit& hit : conformalCDCWireHitList) {
    if (hit.getUsedFlag() or hit.getMaskedFlag()) {
      continue;
    }

    const CDCRecoHit3D& cdcRecoHit3D = CDCRecoHit3D::reconstructNearest(hit.getCDCWireHit(), trackTrajectory2D);
    const Vector2D& recoPos2D = cdcRecoHit3D.getRecoPos2D();

    if (fabs(trackTrajectory2D.getDist2D(recoPos2D)) < minimal_distance_to_track) {
      track.push_back(std::move(cdcRecoHit3D));
      cdcRecoHit3D.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }
}

void HitProcessor::maskHitsWithPoorQuality(CDCTrack& track)
{
  double apogeeArcLenght = fabs(track.getStartTrajectory3D().getGlobalCircle().perimeter()) / 4.;

  std::vector<double> startingArmSLayers;
  std::vector<double> endingArmSLayers;

  for (int ii = 0; ii <= 8; ii++) {
    startingArmSLayers.push_back(0);
    endingArmSLayers.push_back(0);
  }

  // Count the number of hits in the outgoing and ingoing arm per superlayer.
  for (const CDCRecoHit3D& hit : track) {
    if ((hit.getArcLength2D() <= apogeeArcLenght) && (hit.getArcLength2D() > 0)) {
      startingArmSLayers[hit->getISuperLayer()]++;
    } else {
      endingArmSLayers[hit->getISuperLayer()]++;
    }
  }

  std::vector<int> emptyStartingSLayers;
  std::vector<int> emptyEndingSLayers;

  if (hasHoles(startingArmSLayers, endingArmSLayers, emptyStartingSLayers, emptyEndingSLayers)) {
    sort(emptyStartingSLayers.begin(), emptyStartingSLayers.end());

    if (emptyStartingSLayers.size() > 0) {
      const int breakSLayer = emptyStartingSLayers.front();
      for (CDCRecoHit3D& hit : track) {
        if (hit.getArcLength2D() >= apogeeArcLenght || hit.getArcLength2D() < 0) {
          hit.getWireHit().getAutomatonCell().setMaskedFlag();
        }
        if (hit.getISuperLayer() >= breakSLayer) {
          hit.getWireHit().getAutomatonCell().setMaskedFlag();
        }
      }
    }

    if (emptyEndingSLayers.size() > 0) {
      const int breakSLayer = emptyEndingSLayers.back();
      for (CDCRecoHit3D& hit : track) {
        if (hit.getISuperLayer() >= breakSLayer) {
          hit.getWireHit().getAutomatonCell().setMaskedFlag();
        }
      }
    }

  }

  deleteAllMarkedHits(track);
}

int HitProcessor::startingSLayer(const std::vector<double>& startingArmSLayers)
{
  std::vector<double>::const_iterator startSlIt = std::find_if(startingArmSLayers.begin(), startingArmSLayers.end(), [](double val) {
    return val > 0;
  });

  if (startSlIt != startingArmSLayers.end())
    return startSlIt - startingArmSLayers.begin();
  else
    return 8;
}


int HitProcessor::endingSLayer(const std::vector<double>& startingArmSLayers)
{
  std::vector<double>::const_reverse_iterator endSlIt;
  endSlIt = std::find_if(startingArmSLayers.rbegin(), startingArmSLayers.rend(), [](double val) {return val > 0;});

  if (endSlIt != startingArmSLayers.rend())
    return 8 - (endSlIt - startingArmSLayers.rbegin());
  else
    return 0;
}

bool HitProcessor::isTwoSided(const std::vector<double>& startingArmSLayers, const std::vector<double>& endingArmSLayers)
{
  if ((std::accumulate(startingArmSLayers.begin(), startingArmSLayers.end(), 0) > 0) &&
      (std::accumulate(endingArmSLayers.begin(), endingArmSLayers.end(), 0) > 0)) return true;
  else return false;
}

bool HitProcessor::hasHoles(const std::vector<double>& startingArmSLayers,
                            const std::vector<double>& endingArmSLayers,
                            std::vector<int>& emptyStartingSLayers, std::vector<int>& emptyEndingSLayers)
{

  // Find the start end end point.
  int startingSlayer = startingSLayer(startingArmSLayers);
  int endingSlayer = endingSLayer(startingArmSLayers);


  std::vector<double>::const_iterator first = startingArmSLayers.begin() + startingSlayer;
  std::vector<double>::const_iterator last = startingArmSLayers.begin() + endingSlayer;

  for (; first <= last; first += 2) {
    if (*first == 0) {
      emptyStartingSLayers.push_back(first - startingArmSLayers.begin());
    }
  }

  if (isTwoSided(startingArmSLayers, endingArmSLayers)) {

    // Find the start end end point.
    startingSlayer = startingSLayer(endingArmSLayers);
    endingSlayer = endingSLayer(endingArmSLayers);

    std::vector<double>::const_iterator rfirst = endingArmSLayers.begin() + startingSlayer;
    std::vector<double>::const_iterator rlast = endingArmSLayers.begin() + endingSlayer;

    for (; rfirst <= rlast; rfirst += 2) {
      if (*rfirst == 0) {
        emptyEndingSLayers.push_back(rfirst - endingArmSLayers.begin());
      }
    }
  }

  return emptyStartingSLayers.size() > 0 or emptyEndingSLayers.size() > 0;
}
