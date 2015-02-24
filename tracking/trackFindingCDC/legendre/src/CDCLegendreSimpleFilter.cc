/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/trackFindingCDC/legendre/CDCLegendreSimpleFilter.h>


using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


double SimpleFilter::getAssigmentProbability(TrackHit* hit, TrackCandidate* track)
{
  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();

  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());

  return 1.0 - exp(-1 / dist);
}



void SimpleFilter::reassignHitsFromOtherTracks(std::list<TrackCandidate*>& m_trackList)
{
  int ii = 0;

  B2DEBUG(100, "NCands = " << m_trackList.size());

  for (TrackCandidate * cand : m_trackList) {
    ii++;
    B2DEBUG(100, "ii = " << ii);
    B2DEBUG(100, "Processing: Cand hits vector size = " << cand->getTrackHits().size());
    B2DEBUG(100, "Processing: Cand R = " << cand->getR());

    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }

    if (cand->getTrackHits().size() == 0) continue;

    for (TrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);

      double bestHitProb = prob;
      TrackCandidate* BestCandidate = NULL;

      for (TrackCandidate * candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

        // TODO: Do not process this hit of we construct a B2B candidate!

        if (probTemp > bestHitProb) {
          BestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
        BestCandidate->addHit(hit);
        hit->setHitUsage(TrackHit::bad);
      }
    }

    deleteAllMarkedHits(cand);
  }

  for (TrackCandidate * cand : m_trackList) {
    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }
}

void SimpleFilter::deleteAllMarkedHits(TrackCandidate* trackCandidate)
{
  trackCandidate->getTrackHits().erase(
    std::remove_if(trackCandidate->getTrackHits().begin(), trackCandidate->getTrackHits().end(),
  [&](TrackHit * hit) { return hit->getHitUsage() == TrackHit::bad; }),
  trackCandidate->getTrackHits().end());

}

void SimpleFilter::appendUnusedHits(std::list<TrackCandidate*>& trackList, std::vector<TrackHit*>& axialHitList, double minimal_assignment_probability)
{

  for (TrackHit * hit : axialHitList) {
    if (hit->getHitUsage() == TrackHit::used_in_track or hit->getHitUsage() == TrackHit::used_in_cand) continue;

    // Search for best candidate to assign to
    double bestHitProb = 0;
    TrackCandidate* BestCandidate = nullptr;

    for (TrackCandidate * cand : trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

      if (probTemp > bestHitProb) {
        BestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestHitProb > minimal_assignment_probability) {
      BestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);
    }

  }
}

void SimpleFilter::deleteWrongHitsOfTrack(TrackCandidate* trackCandidate, double minimal_assignment_probability)
{
  std::vector<TrackHit*>& trackHits = trackCandidate->getTrackHits();

  if (trackHits.size() == 0) return;

  for (TrackHit * hit : trackHits) {
    hit->setHitUsage(TrackHit::used_in_track);
  }

  int ndf = trackHits.size() - 4;

  if (ndf <= 0) return;

  for (auto hitIterator = trackHits.begin(); hitIterator != trackHits.end(); hitIterator++) {
    double assignment_probability = getAssigmentProbability(*hitIterator, trackCandidate);

    if (assignment_probability < minimal_assignment_probability) {
      (*hitIterator)->setHitUsage(TrackHit::bad);
    }
  }

  deleteAllMarkedHits(trackCandidate);
}
