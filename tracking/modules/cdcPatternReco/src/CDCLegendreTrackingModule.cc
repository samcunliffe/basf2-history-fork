/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcPatternReco/CDCLegendreTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLegendreTracking/CDCLegendreTrackHit.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackCandidate.h>
#include <tracking/cdcLegendreTracking/CDCLegendreTrackFitter.h>
#include <tracking/cdcLegendreTracking/CDCLegendrePatternChecker.h>
#include <tracking/cdcLegendreTracking/CDCLegendreFastHough.h>

#include "genfit/TrackCand.h"

#include <cstdlib>
#include <iomanip>
#include <string>

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>

using namespace std;
using namespace Belle2;

#define SQR(x) ((x)*(x)) //we will use it in least squares fit

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_rMin(-0.15), m_rMax(0.15)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("CDCHitsColName", m_cdcHitsColName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_gfTrackCandsColName,
           "Output GFTrackCandidates collection",
           string("TrackCands"));

  addParam("Threshold", m_threshold, "Threshold for peak finder", 10);

  addParam("InitialAxialHits", m_initialAxialHits,
           "Starting value of axial hits for the stepped Hough", 48);

  addParam("StepScale", m_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("Resolution StereoHits", m_resolutionStereo,
           "Total resolution, used for the assignment of stereo hits to tracks (in sigma)",
           2.);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("Reconstruct Curler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", true);

  addParam("Fit tracks", m_fitTracks,
           "Flag, whether candidates should be fitted with circle", false);

  addParam("Early track merging", m_earlyMerge,
           "Try to merge hit pattern after FastHough with any found track candidate", false);
}

CDCLegendreTrackingModule::~CDCLegendreTrackingModule()
{

}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for genfit::TrackCandidates
  StoreArray<genfit::TrackCand>::registerPersistent(m_gfTrackCandsColName);

  m_nbinsTheta = static_cast<int>(std::pow(2.0, m_maxLevel + 3)); //+3 needed for make bin overlapping;

  cdcLegendreTrackFitter = new CDCLegendreTrackFitter(m_nbinsTheta, m_rMax, m_rMin);
  cdcLegendrePatternChecker = new CDCLegendrePatternChecker();

  m_AxialHitList.reserve(1024);
  m_StereoHitList.reserve(1024);

  cdcLegendreFastHough = new CDCLegendreFastHough(m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMin, m_rMax);
}

void CDCLegendreTrackingModule::beginRun()
{

}

void CDCLegendreTrackingModule::event()
{
  B2INFO("**********   CDCTrackingModule  ************");

  //StoreArray with digitized CDCHits, should already be created by CDCDigitizer module
  StoreArray<CDCHit> cdcHits(m_cdcHitsColName);
  B2DEBUG(100,
          "CDCTracking: Number of digitized Hits: " << cdcHits.getEntries());
  if (cdcHits.getEntries() == 0)
    B2WARNING("CDCTracking: cdcHitsCollection is empty!");

//  if (cdcHits.getEntries() > 1500) {
//    B2INFO("** Skipping track finding due to too large number of hits **");
//    return;
//  }

  //Convert CDCHits to own Hit class
  for (int iHit = 0; iHit < cdcHits.getEntries(); iHit++) {
    CDCLegendreTrackHit* trackHit = new CDCLegendreTrackHit(cdcHits[iHit],
                                                            iHit);
    if (trackHit->getIsAxial())
      m_AxialHitList.push_back(trackHit);
    else
      m_StereoHitList.push_back(trackHit);
  }

  //perform track finding
  DoSteppedTrackFinding();
//  MergeTracks();
//    MergeCurler();
//  AsignStereoHits();

//  checkHitPattern();

  //create GenFit Track candidates
  createGFTrackCandidates();

  //memory management
  clear_pointer_vectors();
}

void CDCLegendreTrackingModule::DoSteppedTrackFinding()
{
  std::sort(m_AxialHitList.begin(), m_AxialHitList.end());

  std::set<CDCLegendreTrackHit*> hits_set;
  std::set<CDCLegendreTrackHit*>::iterator it = hits_set.begin();
  BOOST_FOREACH(CDCLegendreTrackHit * trackHit, m_AxialHitList) {
    it = hits_set.insert(it, trackHit);
  }

  int n_hits = 999;
  double limit = m_initialAxialHits;

  //Start loop, where tracks are searched for
  do {
    std::vector<CDCLegendreTrackHit*> hits_vector;
    std::copy_if(hits_set.begin(), hits_set.end(), std::back_inserter(hits_vector), [](CDCLegendreTrackHit * hit) {return (hit->isUsed() == CDCLegendreTrackHit::not_used);});

    std::vector<CDCLegendreTrackHit*> c_list;
    std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate =
      std::make_pair(c_list, std::make_pair(-999, -999));

    cdcLegendreFastHough->MaxFastHough(&candidate, hits_vector, 1, 0, m_nbinsTheta, m_rMin, m_rMax,
                                       static_cast<unsigned>(limit));

    n_hits = candidate.first.size();

    //if no track is found
    if (n_hits == 0) {
      limit *= m_stepScale;
      n_hits = 999;
    }


    // if track is found and has enough hits
    else if (n_hits >= m_threshold) {
//      cdcLegendreTrackFitter->fitTrackCandidateStepped(&candidate);
      std::pair<double, double> ref_point = std::make_pair(0., 0.);
      double chi2_cand;
//      cdcLegendrePatternChecker->checkCandidate(&candidate);

      if (candidate.first.size() > 0) {
//        cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate, ref_point, chi2_cand);
//      cdcLegendrePatternChecker->clearBadHits(&candidate, ref_point);

        cout << "for hit removing: R:" << 1. / candidate.second.second <<
             "theta:" << candidate.second.first <<
             "ref_x:" << ref_point.first <<
             "ref_y" << ref_point.second << endl;
        /*
              for (CDCLegendreTrackHit* hit: candidate.first){
                double R = fabs(1. / candidate.second.second);
                double x0_track = cos(candidate.second.first) / candidate.second.second + ref_point.first;
                double y0_track = sin(candidate.second.first) / candidate.second.second + ref_point.second;
                double x0_hit = hit->getOriginalWirePosition().X();
                double y0_hit = hit->getOriginalWirePosition().Y();
                double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
                cout << "dist=" << dist << "||" << hit->getDriftTime() <<endl;
              }
              candidate.first.erase(std::remove_if(candidate.first.begin(), candidate.first.end(),
                  [&candidate,&ref_point](CDCLegendreTrackHit * hit) {
                        double R = fabs(1. / candidate.second.second);
                        double x0_track = cos(candidate.second.first) / candidate.second.second + ref_point.first;
                        double y0_track = sin(candidate.second.first) / candidate.second.second + ref_point.second;
                        double x0_hit = hit->getOriginalWirePosition().X();
                        double y0_hit = hit->getOriginalWirePosition().Y();
                        double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
                        return hit->getDriftTime()/2. < dist;
                    }), candidate.first.end());
              */

        double x0_cand = cos(candidate.second.first) / candidate.second.second + ref_point.first;
        double y0_cand = sin(candidate.second.first) / candidate.second.second + ref_point.second;

        bool merged = false;
        bool make_merge = false;

        double chi2_track;
        //loop over all candidates
        for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
               m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
          CDCLegendreTrackCandidate* cand1 = *it1;

          chi2_track = cand1->getChi2();

          double x0_track = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
          double y0_track = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();

//        if((fabs(x0_track-x0_cand)/x0_track < 0.1)&&(fabs(y0_track-y0_cand)/y0_track < 0.1))make_merge = true;
//        else

          {
            int n_overlapp = 0;
            double R = fabs(1. / cand1->getR());
            for (CDCLegendreTrackHit * hit : candidate.first) {
              double x0_hit = hit->getOriginalWirePosition().X();
              double y0_hit = hit->getOriginalWirePosition().Y();
              double dist = fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime();
              if (dist < hit->getDriftTime() * 3.)n_overlapp++;
            }
            if (n_overlapp > 3)make_merge = true;
            else {
              n_overlapp = 0;
              R = fabs(1. / candidate.second.second);
              for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
                double x0_hit = hit->getOriginalWirePosition().X();
                double y0_hit = hit->getOriginalWirePosition().Y();
                double dist = fabs(R - sqrt(SQR(x0_cand - x0_hit) + SQR(y0_cand - y0_hit))) - hit->getDriftTime();
                if (dist < hit->getDriftTime() * 3.)n_overlapp++;
              }
              if (n_overlapp > 3)make_merge = true;

            }
          }

          if (/*true*/make_merge/*false*/) {
            std::vector<CDCLegendreTrackHit*> c_list_temp;
            std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
              std::make_pair(c_list_temp, std::make_pair(-999, -999));
            for (CDCLegendreTrackHit * hit : candidate.first) {
              candidate_temp.first.push_back(hit);
            }
            for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
              candidate_temp.first.push_back(hit);
            }

            double chi2_temp;
            std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
            cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
//          cdcLegendrePatternChecker->clearBadHits(&candidate_temp, ref_point_temp);
//          cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
            cout << "clist_temp.size = " << candidate_temp.first.size() << endl;
            cout << "chi_cand=" << chi2_cand << " chi2_track=" << chi2_track << " chi2_temp=" << chi2_temp << endl;
            if (candidate_temp.first.size() == 0) {
              merged = true;
              break;
            }

            if (chi2_temp < SQR(sqrt(chi2_track) + sqrt(chi2_cand)) * 3.) {

              cand1->setR(candidate_temp.second.second);
              cand1->setTheta(candidate_temp.second.first);
//            cand1->setReferencePoint(ref_point_temp.first, ref_point_temp.second);
              mergeTracks(cand1, candidate, hits_set);

              cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
              cand1->setReferencePoint(ref_point.first, ref_point.second);

              merged = true;
//            cand1->clearBadHits(ref_point);
              cout << "MERGED!" << endl;
              break;
            } else make_merge = false;
          }


          /*
          //check only curler
          if (fabs(cand1->getR()) > m_rc) {

            if (fabs(candidate.second.second) > m_rc) {

              cout << "R: " << cand1->getR() << "  " << candidate.second.second << endl;
              cout << "theta: " << cand1->getTheta() << "  " << candidate.second.first << endl;

              //check if the two tracks lie next to each other
              if (fabs(cand1->getR() - candidate.second.second) < 0.03
                  && fabs(cand1->getTheta() - candidate.second.first) < 0.03) {
                mergeTracks(cand1, candidate);
                BOOST_FOREACH(CDCLegendreTrackHit * hit, candidate.first) {
                  hits_set.erase(hit);
                }
                cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
                cand1->setReferencePoint(ref_point.first, ref_point.second);
                merged = true;
                cand1->clearBadHits(ref_point);
                cout << "MERGED!" << endl;
                break;
              }
            }
          }

          */
        }


        if (!merged) createLegendreTrackCandidate(candidate, &hits_set, ref_point);

        limit = n_hits * m_stepScale;

      }
    }

    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (n_hits >= m_threshold
           && (limit / m_stepScale >= m_threshold || n_hits != 999)
           && hits_set.size() >= (unsigned) m_threshold);

}

void CDCLegendreTrackingModule::MergeCurler()
{
  /*
    //loop over all candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
           m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //check only curler
      if (fabs(cand1->getR()) > m_rc) {

        //loop over remaining candidates
        std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
        while (it2 != m_trackList.end()) {
          CDCLegendreTrackCandidate* cand2 = *it2;
          ++it2;

          if (fabs(cand2->getR()) > m_rc) {

            //check if the two tracks lie next to each other
            if (fabs(cand1->getR() - cand2->getR()) < 0.03
                && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)
              return;
            //mergeTracks(cand1, cand2);
          }
        }
      }
    }
    */

  bool merged = false;
  bool make_merge = false;
  std::pair<double, double> ref_point = std::make_pair(0., 0.);

  double chi2_track1, chi2_track2;
  double x0_track1, y0_track1;
  double x0_track2, y0_track2;
  //loop over all candidates
  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
         m_trackList.begin(); it1 != m_trackList.end(); ++it1) {
    CDCLegendreTrackCandidate* cand1 = *it1;

    chi2_track1 = cand1->getChi2();

    double x0_track1 = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X();
    double y0_track1 = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();

    //loop over remaining candidates
    std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
    while (it2 != m_trackList.end()) {
      CDCLegendreTrackCandidate* cand2 = *it2;
      ++it2;

      chi2_track2 = cand2->getChi2();

      double x0_track2 = cos(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().X();
      double y0_track2 = sin(cand2->getTheta()) / cand2->getR() + cand2->getReferencePoint().Y();


//    if((fabs(x0_track1-x0_track2)/x0_track1 < 0.1)&&(fabs(y0_track1-y0_track2)/y0_track1 < 0.1))make_merge = true;
//    else
      {
        int n_overlapp = 0;
        double R = fabs(1. / cand1->getR());
        for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
          double x0_hit = hit->getOriginalWirePosition().X();
          double y0_hit = hit->getOriginalWirePosition().Y();
          double dist = fabs(R - sqrt(SQR(x0_track1 - x0_hit) + SQR(y0_track1 - y0_hit))) - hit->getDriftTime();
          if (dist < hit->getDriftTime() * 3.)n_overlapp++;
        }
        if (n_overlapp > 3)make_merge = true;
        else {
          n_overlapp = 0;
          R = fabs(1. / cand2->getR());
          for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
            double x0_hit = hit->getOriginalWirePosition().X();
            double y0_hit = hit->getOriginalWirePosition().Y();
            double dist = fabs(R - sqrt(SQR(x0_track2 - x0_hit) + SQR(y0_track2 - y0_hit))) - hit->getDriftTime();
            if (dist < hit->getDriftTime() * 3.)n_overlapp++;
          }
          if (n_overlapp > 3)make_merge = true;

        }
      }

      if (true/*make_merge*/) {
        std::vector<CDCLegendreTrackHit*> c_list_temp;
        std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> > candidate_temp =
          std::make_pair(c_list_temp, std::make_pair(-999, -999));
        for (CDCLegendreTrackHit * hit : cand2->getTrackHits()) {
          candidate_temp.first.push_back(hit);
        }
        for (CDCLegendreTrackHit * hit : cand1->getTrackHits()) {
          candidate_temp.first.push_back(hit);
        }

        double chi2_temp;
        std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
        cdcLegendreTrackFitter->fitTrackCandidateFast(&candidate_temp, ref_point_temp, chi2_temp);
        if (chi2_temp < SQR(sqrt(chi2_track1) + sqrt(chi2_track2)) * 2.) {

          mergeTracks(cand1, cand2);
          cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point, true);
          cand1->setReferencePoint(ref_point.first, ref_point.second);
          it2 = boost::next(it1);
          /*        merged = true;
                  cand1->clearBadHits(ref_point);
                  cout << "MERGED!" << endl;
                  break;
          */

        } else make_merge = false;
      }

    }
    cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point);
    cdcLegendreTrackFitter->fitTrackCandidateFast(cand1, ref_point, true);

  }

}

void CDCLegendreTrackingModule::MergeTracks()
{
  //loop over all full candidates
  /*  for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
        m_fullTrackList.begin(); it1 != m_fullTrackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //loop over remaining candidates

      std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
      while (it2 != m_fullTrackList.end()) {
        CDCLegendreTrackCandidate* cand2 = *it2;
        ++it2;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand2->getR()) < 0.03
            && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)

          mergeTracks(cand1, cand2);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_shortTrackList.begin(); it_short != m_shortTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_trackletTrackList.begin(); it_short != m_trackletTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

    }

    //loop over all short candidates
    for (std::list<CDCLegendreTrackCandidate*>::iterator it1 =
        m_shortTrackList.begin(); it1 != m_shortTrackList.end(); ++it1) {
      CDCLegendreTrackCandidate* cand1 = *it1;

      //loop over remaining candidates

      std::list<CDCLegendreTrackCandidate*>::iterator it2 = boost::next(it1);
      while (it2 != m_shortTrackList.end()) {
        CDCLegendreTrackCandidate* cand2 = *it2;
        ++it2;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand2->getR()) < 0.03
            && fabs(cand1->getTheta() - cand2->getTheta()) < 0.15)

          mergeTracks(cand1, cand2);
      }

      for (std::list<CDCLegendreTrackCandidate*>::iterator it_short =
          m_trackletTrackList.begin(); it_short != m_trackletTrackList.end(); ++it_short) {
        CDCLegendreTrackCandidate* cand_short = *it_short;

        //check if the two tracks lie next to each other
        if (fabs(cand1->getR() - cand_short->getR()) < 0.03
            && fabs(cand1->getTheta() - cand_short->getTheta()) < 0.15)

          mergeTracks(cand1, cand_short);
      }

    }*/

  for (CDCLegendreTrackCandidate * trackCand : m_fullTrackList) {
    m_trackList.push_back(new CDCLegendreTrackCandidate(*trackCand));
  }
  for (CDCLegendreTrackCandidate * trackCand : m_shortTrackList) {
    m_trackList.push_back(new CDCLegendreTrackCandidate(*trackCand));
  }

//  std::copy(m_fullTrackList.begin(), m_fullTrackList.end(), std::back_inserter(m_trackList));
//  std::copy(m_shortTrackList.begin(), m_shortTrackList.end(), std::back_inserter(m_trackList));

//  m_trackList.insert( m_trackList.end(), m_fullTrackList.begin(), m_fullTrackList.end() );
//  m_trackList.insert( m_trackList.end(), m_shortTrackList.begin(), m_shortTrackList.end() );

}


void CDCLegendreTrackingModule::AsignStereoHits()
{
  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_StereoHitList) {
    CDCLegendreTrackCandidate* best = NULL;
    double best_chi2 = 999;

    BOOST_FOREACH(CDCLegendreTrackCandidate * candidate, m_trackList) {
      //precut on distance between track and stereo hit
      if (candidate->DistanceTo(*hit) <= 5.) {
        //Hit needs to have the correct curvature
        if ((candidate->getCharge() == CDCLegendreTrackCandidate::charge_curler) || hit->getCurvatureSignWrt(candidate->getXc(), candidate->getYc()) == candidate->getCharge()) {
          //check nearest position of the hit to the track
          if (hit->approach2(*candidate)) {
            double chi2 = candidate->DistanceTo(*hit) / sqrt(hit->getDeltaDriftTime());

            if (chi2 < m_resolutionStereo) {
              //search for minimal distance
              if (chi2 < best_chi2) {
                best = candidate;
                best_chi2 = chi2;
              }
            }
          }
        }
      }
    }

    //if there is track near enough
    if (best != NULL) {
      best->addHit(hit);
    }
  }

  BOOST_FOREACH(CDCLegendreTrackCandidate * cand, m_trackList) {
    cand->CheckStereoHits();
  }

}

void CDCLegendreTrackingModule::mergeTracks(CDCLegendreTrackCandidate* cand1,
                                            const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
                                            std::set<CDCLegendreTrackHit*>& hits_set)
{
  /*
    cand1->setR(
      (cand1->getR() * cand1->getNHits() + track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));
    cand1->setTheta(
      (cand1->getTheta() * cand1->getNHits()
       + track.second.first * track.second.second * track.first.size())
      / (cand1->getNHits() + track.second.second * track.first.size()));
  */
  BOOST_FOREACH(CDCLegendreTrackHit * hit, track.first) {
    /*    cand1->addHit(hit);
        hit->setUsed(CDCLegendreTrackHit::used_in_track);
    */
    double R = fabs(1. / cand1->getR());
    double x0_track = cos(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().X() ;
    double y0_track = sin(cand1->getTheta()) / cand1->getR() + cand1->getReferencePoint().Y();
    double x0_hit = hit->getOriginalWirePosition().X();
    double y0_hit = hit->getOriginalWirePosition().Y();
    double dist = SQR(fabs(R - sqrt(SQR(x0_track - x0_hit) + SQR(y0_track - y0_hit))) - hit->getDriftTime());
    if (hit->getDriftTime() / 2. < dist) {
      cand1->addHit(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
//        hits_set.erase(hit);
    }
  }

//  m_trackList.remove(cand2);
//  delete cand2;
//  cand2 = NULL;
}

void CDCLegendreTrackingModule::mergeTracks(CDCLegendreTrackCandidate* cand1, CDCLegendreTrackCandidate* cand2)
{

  cand1->setR(
    (cand1->getR() * cand1->getNHits() + cand2->getR() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));
  cand1->setTheta(
    (cand1->getTheta() * cand1->getNHits()
     + cand2->getTheta() * cand2->getNHits())
    / (cand1->getNHits() + cand2->getNHits()));

  BOOST_FOREACH(CDCLegendreTrackHit * hit, cand2->getTrackHits()) {
    cand1->addHit(hit);
    hit->setUsed(true);
  }

  m_trackList.remove(cand2);
  delete cand2;
  cand2 = NULL;
}
void CDCLegendreTrackingModule::checkHitPattern()
{
  int candType;
  for (CDCLegendreTrackCandidate * trackCand : m_trackList) {
    cout << "pattern:" << trackCand->getHitPatternAxial().getHitPattern() << endl;
    candType = trackCand->getCandidateType();
    cout << "candType = " << candType << endl;
//    if(candType == 1)

  }
}

void CDCLegendreTrackingModule::createLegendreTrackCandidate(
  const std::pair<std::vector<CDCLegendreTrackHit*>, std::pair<double, double> >& track,
  std::set<CDCLegendreTrackHit*>* trackHitList, std::pair<double, double>& ref_point)
{

  //get theta and r values for each track candidate
  double track_theta = track.second.first;
  double track_r = track.second.second;

  //get charge estimation for the track candidate
  int charge = CDCLegendreTrackCandidate::getChargeAssumption(track_theta,
                                                              track_r, track.first);

  //for curlers, negative, and positive tracks we want to create one track candidate
  if (charge == CDCLegendreTrackCandidate::charge_positive
      || charge == CDCLegendreTrackCandidate::charge_negative
      || charge == CDCLegendreTrackCandidate::charge_curler) {
    CDCLegendreTrackCandidate* trackCandidate = new CDCLegendreTrackCandidate(
      track_theta, track_r, charge, track.first);
    trackCandidate->setReferencePoint(ref_point.first, ref_point.second);
    cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate, ref_point);
//    trackCandidate->clearBadHits(ref_point);

    processTrack(trackCandidate, trackHitList);

  }

  //here we create two oppositely charged tracks (with the same theta and r value)
  else if (charge == CDCLegendreTrackCandidate::charge_two_tracks) {
    CDCLegendreTrackCandidate* trackCandidate_pos =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_positive, track.first);
    trackCandidate_pos->setReferencePoint(ref_point.first, ref_point.second);

    CDCLegendreTrackCandidate* trackCandidate_neg =
      new CDCLegendreTrackCandidate(track_theta, track_r,
                                    CDCLegendreTrackCandidate::charge_negative, track.first);
    trackCandidate_neg->setReferencePoint(ref_point.first, ref_point.second);
    cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_pos, ref_point);
    cdcLegendreTrackFitter->fitTrackCandidateFast(trackCandidate_neg, ref_point);
//    trackCandidate_pos->clearBadHits(ref_point);
//    trackCandidate_neg->clearBadHits(ref_point);

    processTrack(trackCandidate_pos, trackHitList);

    processTrack(trackCandidate_neg, trackHitList);
  }
  //This shouldn't happen, check CDCLegendreTrackCandidate::getChargeAssumption()
  else {
    B2ERROR(
      "Strange behavior of CDCLegendreTrackCandidate::getChargeAssumption");
    exit(EXIT_FAILURE);
  }
}

void CDCLegendreTrackingModule::processTrack(
  CDCLegendreTrackCandidate* trackCandidate,
  std::set<CDCLegendreTrackHit*>* trackHitList)
{
  //check if the number has enough axial hits (might be less due to the curvature check).
  if (fullfillsQualityCriteria(trackCandidate)) {
    /*    int candType = trackCandidate->getCandidateType();
        if(candType == CDCLegendreTrackCandidate::fullTrack)m_fullTrackList.push_back(trackCandidate);
        else if(candType == CDCLegendreTrackCandidate::curlerTrack)m_shortTrackList.push_back(trackCandidate);
        else if(candType == CDCLegendreTrackCandidate::tracklet)m_trackletTrackList.push_back(trackCandidate);
        else {
          BOOST_FOREACH(CDCLegendreTrackHit * hit, trackCandidate->getTrackHits()) {
            trackHitList->erase(hit);
          }


          //memory management, since we cannot use smart pointers in function interfaces
          delete trackCandidate;
          trackCandidate = NULL;
        }
    */
    m_trackList.push_back(trackCandidate);

    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackCandidate->getTrackHits()) {
//      trackHitList->erase(hit);
      hit->setUsed(CDCLegendreTrackHit::used_in_track);
    }
  }

  else {
    BOOST_FOREACH(CDCLegendreTrackHit * hit, trackCandidate->getTrackHits()) {
//      trackHitList->erase(hit);
      hit->setUsed(CDCLegendreTrackHit::used_bad);
    }

    //memory management, since we cannot use smart pointers in function interfaces
    delete trackCandidate;
    trackCandidate = NULL;
  }

}

bool CDCLegendreTrackingModule::fullfillsQualityCriteria(CDCLegendreTrackCandidate* trackCandidate)
{
//  if (trackCandidate->getNAxialHits() < m_threshold)
//    return false;

//  if (trackCandidate->getLayerWaight() < 1)
//    return false;

  return true;
}

void CDCLegendreTrackingModule::createGFTrackCandidates()
{
  //StoreArray for genfit::TrackCandidates: interface class to Genfit
  StoreArray<genfit::TrackCand> gfTrackCandidates(m_gfTrackCandsColName);
  gfTrackCandidates.create();

  int i = 0;

  BOOST_FOREACH(CDCLegendreTrackCandidate * trackCand, m_trackList) {
    gfTrackCandidates.appendNew();
    std::pair<double, double> ref_point_temp = std::make_pair(0., 0.);
    cdcLegendreTrackFitter->fitTrackCandidateFast(trackCand, ref_point_temp);

//  testing of track's hit pattern
//    cout << "pattern:" << trackCand->getHitPattern().getHitPattern() << endl;

    //set the values needed as start values for the fit in the genfit::TrackCandidate from the CDCTrackCandidate information
    //variables stored in the genfit::TrackCandidates are: vertex position + error, momentum + error, pdg value, indices for the Hits
    TVector3 position;
//    position.SetXYZ(0.0, 0.0, 0.0);//at the moment there is no vertex determination in the ConformalFinder, but maybe the origin or the innermost hit are good enough as start values...
    //position = cdcTrackCandidates[i]->getInnerMostHit().getWirePosition();
    position = trackCand->getReferencePoint();

    TVector3 momentum =
      trackCand->getMomentumEstimation(true);

    //Pattern recognition can determine only the charge, so here some dummy pdg value is set (with the correct charge), the pdg hypothesis can be then overwritten in the GenFitterModule
    int pdg = trackCand->getChargeSign() * (211);

    //The initial covariance matrix is calculated from these errors and it is important (!!) that it is not completely wrong
    /*TMatrixDSym covSeed(6);
    covSeed(0, 0) = 4; covSeed(1, 1) = 4; covSeed(2, 2) = 4;
    covSeed(3, 3) = 0.1 * 0.1; covSeed(4, 4) = 0.1 * 0.1; covSeed(5, 5) = 0.5 * 0.5;*/

    //set the start parameters
    gfTrackCandidates[i]->setPosMomSeedAndPdgCode(position, momentum, pdg);


    B2DEBUG(100, "Create genfit::TrackCandidate " << i << "  with pdg " << pdg);
    B2DEBUG(100,
            "position seed:  (" << position.x() << ", " << position.y() << ", " << position.z() << ")");//   position variance: (" << covSeed(0, 0) << ", " << covSeed(1, 1) << ", " << covSeed(2, 2) << ") ");
    B2DEBUG(100,
            "momentum seed:  (" << momentum.x() << ", " << momentum.y() << ", " << momentum.z() << ")");//   position variance: (" << covSeed(3, 3) << ", " << covSeed(4, 4) << ", " << covSeed(5, 5) << ") ");

    //find indices of the Hits
    std::vector<CDCLegendreTrackHit*> trackHitVector = trackCand->getTrackHits();

    sortHits(trackHitVector, trackCand->getChargeSign());

    B2DEBUG(100, " Add Hits: hitId rho planeId")

    BOOST_FOREACH(CDCLegendreTrackHit * trackHit, trackHitVector) {
      int hitID = trackHit->getStoreIndex();
      gfTrackCandidates[i]->addHit(Const::CDC, hitID);
    }
    ++i;
  }
}

void CDCLegendreTrackingModule::endRun()
{

}

void CDCLegendreTrackingModule::terminate()
{
}

void CDCLegendreTrackingModule::sortHits(
  std::vector<CDCLegendreTrackHit*>& hits, int charge)
{
  CDCTracking_SortHit sorter(charge);
  stable_sort(hits.begin(), hits.end(), sorter);
}

bool CDCTracking_SortHit::operator()(CDCLegendreTrackHit* hit1,
                                     CDCLegendreTrackHit* hit2)
{
  bool result = true;
  boost::tuple<int, double, int, double> tuple1(hit1->getStoreIndex(),
                                                hit1->getWirePosition().Mag(), hit1->getWireId());
  boost::tuple<int, double, int, double> tuple2(hit2->getStoreIndex(),
                                                hit2->getWirePosition().Mag(), hit2->getWireId());

  //the comparison function for the tuples created by the sort function

  if ((int) tuple1.get<1>() == (int) tuple2.get<1>()) {
    //special case: several hits in the same layer
    //now we have to proceed differently for positive and negative tracks
    //in a common case we just have to check the wireIds and decide the order according to the charge
    //if however the track is crossing the wireId 0, we have again to treat it specially
    //the number 100 is just a more or less arbitrary number, assuming that no track will be 'crossing' 100 different wireIds

    //in general this solution does not look very elegant, so if you have some suggestions how to improve it, do not hesitate to tell me

    if (m_charge < 0) {
      //negative charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = false;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = true;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = true;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = false;
      }
    } //end negative charge

    else {
      //positive charge

      //check for special case with wireId 0
      if (tuple1.get<2>() == 0 && tuple2.get<2>() > 100) {
        result = true;
      }
      if (tuple1.get<2>() > 100 && tuple2.get<2>() == 0) {
        result = false;
      }
      //'common' case
      if (tuple1.get<2>() < tuple2.get<2>()) {
        result = false;
      }
      if (tuple1.get<2>() > tuple2.get<2>()) {
        result = true;
      }
    } //end positive charge

  }

  //usual case: hits sorted by the rho value
  else
    result = (tuple1.get<1>() < tuple2.get<1>());

  return result;

}

void CDCLegendreTrackingModule::clear_pointer_vectors()
{

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_AxialHitList) {
    delete hit;
  }
  m_AxialHitList.clear();

  BOOST_FOREACH(CDCLegendreTrackHit * hit, m_StereoHitList) {
    delete hit;
  }
  m_StereoHitList.clear();

  BOOST_FOREACH(CDCLegendreTrackCandidate * track, m_trackList) {
    delete track;
  }
  m_trackList.clear();

  BOOST_FOREACH(CDCLegendreTrackCandidate * track, m_fullTrackList) {
    delete track;
  }
  m_fullTrackList.clear();

  BOOST_FOREACH(CDCLegendreTrackCandidate * track, m_shortTrackList) {
    delete track;
  }
  m_shortTrackList.clear();

  BOOST_FOREACH(CDCLegendreTrackCandidate * track, m_trackletTrackList) {
    delete track;
  }
  m_trackletTrackList.clear();

}

