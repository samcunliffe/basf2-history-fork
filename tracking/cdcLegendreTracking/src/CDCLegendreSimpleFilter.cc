/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <tracking/cdcLegendreTracking/CDCLegendreSimpleFilter.h>


using namespace std;

using namespace Belle2;
using namespace TrackFinderCDCLegendre;


double SimpleFilter::getAssigmentProbability(TrackHit* hit, TrackCandidate* track)
{

  double prob = 0;

  double x0_track = track->getXc();
  double y0_track = track->getYc();
  double R = track->getRadius();


  double x0_hit = hit->getOriginalWirePosition().X();
  double y0_hit = hit->getOriginalWirePosition().Y();
  double dist = fabs(fabs(R - sqrt((x0_track - x0_hit) * (x0_track - x0_hit) + (y0_track - y0_hit) * (y0_track - y0_hit))) - hit->getDriftLength());



  prob = exp(-1. * dist / hit->getSigmaDriftLength());

  if (dist < hit->getSigmaDriftLength() * 2.) prob = 1.;

  return prob;
}



void SimpleFilter::processTracks(std::list<TrackCandidate*>& m_trackList)
{

  /*
  double total_prob = 1.;
  for (CDCLegendreFilterCandidate* filterCandidate : m_cdcLegendreFilterCandidateList) {

    for(auto& hitMap: filterCandidate->getHitsMap()){
      CDCLegendreTrackHit* hit = hitMap.first;
      double prob = hitMap.second;

  //     if(prob<m_minProb){

        double bestHitProb = prob;
        TrackCandidate* BestCandidate;

        for (TrackCandidate * cand : m_trackList) {
          double probTemp = getAssigmentProbability(hit, cand);

          int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

          if(probTemp > bestHitProb && curve_sign == cand->getCharge()) {
            BestCandidate = cand;
            bestHitProb = probTemp;
          }
        }

        if(bestHitProb > prob) {
          filterCandidate->getLegendreCandidate()->removeHit(hit);
          BestCandidate->addHit(hit);
  //         filterCandidate->removeHit(hit);
          B2INFO("Hit has been reassigned.");
        }


  //     }

    }

  }

  */

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

    if (cand->getTrackHits().size() == 0)continue;

    for (TrackHit * hit : cand->getTrackHits()) {
      double prob = getAssigmentProbability(hit, cand);;

//      if(prob<m_minProb){

      double bestHitProb = prob;
      TrackCandidate* BestCandidate = NULL;

      for (TrackCandidate * candInner : m_trackList) {
        if (candInner == cand) continue;
        double probTemp = getAssigmentProbability(hit, candInner);

//        int curve_sign = hit->getCurvatureSignWrt(cos(candInner->getTheta()) / candInner->getR(), sin(candInner->getTheta()) / candInner->getR());

        if (probTemp > bestHitProb /*&& curve_sign == candInner->getCharge()*/) {
          BestCandidate = candInner;
          bestHitProb = probTemp;
        }
      }

      if (bestHitProb > prob) {
//          filterCandidate->getLegendreCandidate()->removeHit(hit);
        BestCandidate->addHit(hit);
//        cand->removeHit(hit);
        hit->setHitUsage(TrackHit::bad);
//        B2INFO("Hit has been reassigned.");
      }


//      }

    }
    cand->getTrackHits().erase(std::remove_if(cand->getTrackHits().begin(), cand->getTrackHits().end(),
    [&](TrackHit * hit) {
      /*
            if(hit->getHitUsage() == TrackHit::bad) {
              hit->setHitUsage(TrackHit::used_in_track);
              return true;
            } else {
              return false;
            }
      */
      return hit->getHitUsage() == TrackHit::bad;
    }),
    cand->getTrackHits().end());

  }


  for (TrackCandidate * cand : m_trackList) {
    for (TrackHit * hit : cand->getTrackHits()) {
      hit->setHitUsage(TrackHit::used_in_track);
    }
  }
}

/*
void SimpleFilter::trackCore()
{

  for (TrackCandidate * cand : m_trackList) {

//    if(cand->getTrackHits().size() < 10)

    for (CDCLegendreTrackHit * hit : cand->getTrackHits()) {
      bool removeHit = false;
      double prob = getAssigmentProbability(hit, cand);;

      if (prob < 0.9) {
        removeHit = true;
      } else {

        double otherProbs = 0;

        for (TrackCandidate * candInner : m_trackList) {
          if (candInner == cand) continue;
          double probTemp = getAssigmentProbability(hit, candInner);

          if (probTemp > otherProbs && curve_sign == candInner->getCharge()) {

            otherProbs = probTemp;
          }
        }

        if (otherProbs > prob || otherProbs > 0.6) {
          removeHit = true;
        }
      }

      if (removeHit) {
        cand->removeHit(hit);
//        B2INFO("Hit has been removed!");
      }


    }

  }



}

*/


void SimpleFilter::appenUnusedHits(std::list<TrackCandidate*>& m_trackList, std::vector<TrackHit*>& AxialHitList)
{

  for (TrackHit * hit : AxialHitList) {
//    if (hit->getHitUsage() != TrackHit::not_used) continue;
    double bestHitProb = 0;
    TrackCandidate* BestCandidate = NULL;

    for (TrackCandidate * cand : m_trackList) {
      double probTemp = getAssigmentProbability(hit, cand);

//      int curve_sign = hit->getCurvatureSignWrt(cos(cand->getTheta()) / cand->getR(), sin(cand->getTheta()) / cand->getR());

      if (probTemp > bestHitProb /*&& curve_sign == cand->getCharge()*/) {
        BestCandidate = cand;
        bestHitProb = probTemp;
      }
    }

    if (bestHitProb > 0.8) {
      BestCandidate->addHit(hit);
      hit->setHitUsage(TrackHit::used_in_track);
//      B2INFO("Unused hit has been assigned.");
    }

  }
}

