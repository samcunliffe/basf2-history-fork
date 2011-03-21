/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oksana Brovchenko                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/AxialTrackFinder.h"

#include <framework/datastore/StoreArray.h>
#include <framework/logging/Logger.h>

#include <cmath>

#include "TMath.h"
#include "TGraph.h"
#include "TFile.h"
#include "TAxis.h"
#include "TF1.h"

using namespace std;
using namespace Belle2;


AxialTrackFinder::AxialTrackFinder()
{
}

AxialTrackFinder::~AxialTrackFinder()
{
}


double AxialTrackFinder::ShortestDistance(TVector3 firstPoint, TVector3 firstDirection, TVector3 secondPoint)
{

  TVector3 perpDir; //direction perpendicular to firstDirection
  TVector3 perpPoint; //intersection from "first line" with a to it perpendicular line through second point

  //direction perpendicular to first line direction
  double perpDirY = -firstDirection.x() / firstDirection.y();
  perpDir.SetX(1);
  perpDir.SetY(perpDirY);
  perpDir.SetZ(0);

  //calculate the intersection point
  double relation = firstDirection.y() / firstDirection.x();

  double enumerator = (secondPoint.y() - firstPoint.y())
                      + (firstPoint.x() - secondPoint.x()) * relation;
  double denominator = perpDir.x() * relation - perpDir.y();

  double factor = enumerator / denominator;
  perpPoint.SetX(secondPoint.x() + factor * perpDir.x());
  perpPoint.SetY(secondPoint.y() + factor * perpDir.y());
  perpPoint.SetZ(0);

  //B2INFO("Intersection Point: "<<perpPoint.X()<<"  "<<perpPoint.Y()<<"  "<<perpPoint.Z());

  //distance between the second point und the intersection point ( = shortest distance from second point to "first line")
  double distance = (secondPoint - perpPoint).Mag();

  return distance;
}


double AxialTrackFinder::ShortestDistance(CDCSegment segment1,
                                          CDCSegment segment2)
{
  TVector3 FirstSegmentPoint; //starting point of the one segment
  TVector3 SecondSegmentPoint; //point of the  other segment
  TVector3 FirstSegmentDirection = segment1.getDirection(); //direction of the first segment, builds together with starting point a "segment straight line" in the conformal plane


  FirstSegmentPoint.SetX(segment1.getOuterMostHit().getConformalX());
  FirstSegmentPoint.SetY(segment1.getOuterMostHit().getConformalY());
  FirstSegmentPoint.SetZ(0);


  SecondSegmentPoint.SetX(segment2.getOuterMostHit().getConformalX());
  SecondSegmentPoint.SetY(segment2.getOuterMostHit().getConformalY());
  SecondSegmentPoint.SetZ(0);

  double distance = ShortestDistance(FirstSegmentPoint, FirstSegmentDirection, SecondSegmentPoint);

  return distance ;

}



double AxialTrackFinder::SimpleDistance(CDCSegment segment1,
                                        CDCSegment segment2)
{

  double segment1_X = (segment1.getOuterMostHit().getWirePosition().x()
                       + segment1.getInnerMostHit().getWirePosition().x()) / 2;
  double segment1_Y = (segment1.getOuterMostHit().getWirePosition().y()
                       + segment1.getInnerMostHit().getWirePosition().y()) / 2;

  double segment2_X = (segment2.getOuterMostHit().getWirePosition().x()
                       + segment2.getInnerMostHit().getWirePosition().x()) / 2;
  double segment2_Y = (segment2.getOuterMostHit().getWirePosition().y()
                       + segment2.getInnerMostHit().getWirePosition().y()) / 2;

  double simpleDistance = sqrt(pow((segment1_X - segment2_X), 2) + pow(
                                 (segment1_Y - segment2_Y), 2));

  return simpleDistance;
}


float AxialTrackFinder::UsedSegmentsFraction(CDCTrackCandidate track,
                                             vector<int> & UsedSegmentId)
{

  int used = 0;

  for (int i = 0; i < track.getNSegments(); i++) { //loop over all segments and compare their Id's to those which were already used
    for (unsigned int j = 0; j < UsedSegmentId.size(); j++) {

      if (track.getSegments().at(i).getId() == UsedSegmentId.at(j)) {
        used++;
      }
    }
  }

  float fraction = float(used / track.getNSegments());

  return fraction;

}


vector<CDCTrackCandidate> AxialTrackFinder:: FindTrackCandidates(vector<CDCSegment> & cdcAxialSegments, double SimpleDistanceCut, double AngleCut, double ShortDistanceCut, int StartSLId)
{

  vector<CDCTrackCandidate> TrackCandidates;

  int nSegments = cdcAxialSegments.size();
  int NTracks = 0;
  int SLId = StartSLId; //start value for the superlayerId
  int trackcounter = -1; //count the number of track candidates

  //Step 1: Count all good segments in the start superlayer, create the same number of track candidates and assign the segment from the start superlayer to them

  //loop over all segment and count good segments in the given superlayer ( = starting points for the track candidates)
  for (int i = 0; i < nSegments; i++) {
    CDCSegment* segmentCandidate = & cdcAxialSegments[i];
    if (segmentCandidate->getSuperlayerId() == StartSLId && segmentCandidate->getIsGood() == true) {

      trackcounter++;
      segmentCandidate->setTrackCandId(trackcounter);
    }
  }//end loop over all segments

  //B2INFO("****Create Tracks****");
  for (int i = 0; i <= trackcounter; i++) {
    //create a TrackCandidate and place it in the corresponding vector
    CDCTrackCandidate track(i);
    TrackCandidates.push_back(track);

    //loop over all segments, add segments with correct SLId (= StartSlId) and corresponding TrackCandId to created track
    for (int j = 0; j < nSegments; j++) {
      for (unsigned k = 0; k  < cdcAxialSegments[j].getTrackCandId().size(); k++) {

        if (cdcAxialSegments[j].getSuperlayerId() == StartSLId
            && cdcAxialSegments[j].getTrackCandId().at(k) == i) {
          TrackCandidates.at(i).addSegment(cdcAxialSegments[j]);
          //B2INFO("Create Track "<<i<<"  with Segment "<<cdcSegmentsArray[j]->getId()<<"  from SL "<<cdcSegmentsArray[j]->getSuperlayerId());
        }
      }
    }//end loop over all segments
  } //end loop over over all track candidates


  //Step 2: search for further segments for already created tracks in the superlayers below

  while (SLId > 0) {  //while loop over all axial superlayers

    //loop over all Segments, if two neighbouring segments are found, the TrackCandId from the 'higher' one is assigned to the 'lower' one
    for (int i = 0; i < nSegments; i++) {
      // B2INFO("Candidate Segment "<<cdcSegmentsArray[i]->getId()<<" from SL "<<cdcSegmentsArray[i]->getSuperlayerId());
      CDCSegment* segmentCandidate = & cdcAxialSegments[i];

      for (int j = 0; j < nSegments; j++) {
        // B2INFO("Check Segment "<<cdcSegmentsArray[j]->getId()<<" from SL "<<cdcSegmentsArray[j]->getSuperlayerId());
        double angle = segmentCandidate->getDirection().Angle(cdcAxialSegments[j].getDirection());
        if (angle > TMath::Pi() / 2)  angle = angle - TMath::Pi(); // -90 < angle < 90
        //first necessary neighbouring conditions
        //B2INFO("SimpleDistance: "<<SimpleDistance(*segmentCandidate, *cdcSegmentsArray[j])<<"  Angle: "<<angle<<"  ShortestDistance: "<<ShortestDistance(*segmentCandidate,*cdcSegmentsArray[j]));
        if (cdcAxialSegments[j].getSuperlayerId()
            == segmentCandidate->getSuperlayerId() - 2
            && SimpleDistance(*segmentCandidate, cdcAxialSegments[j]) < SimpleDistanceCut
            && angle < AngleCut && cdcAxialSegments[j].getIsGood() == true) {
          //the decisive neighbouring condition
          if (ShortestDistance(*segmentCandidate,
                               cdcAxialSegments[j]) < ShortDistanceCut) {

            cdcAxialSegments[j].setTrackCandId(segmentCandidate->getTrackCandId()); //copy the TrackCandId from one segment to another
          }
        }

      }

    }//end loop over all segments

    NTracks = TrackCandidates.size();

    //Now after the correct TrackCandIds are assigned, the corresponding segments can be added to the tracks

    for (int i = 0; i < NTracks; i++) { //loop over all track candidates
      CDCTrackCandidate candidate = TrackCandidates.at(i);
      //B2INFO("Track "<<i);
      for (int j = 0; j < nSegments; j++) { //loop over all segments

        //boolean to mark the case, when two different segments from the same superlayer are matching the same track candidate
        //if already == true, the track candidate should be split in two different candidates
        bool already = false;
        for (unsigned k = 0; k
             < cdcAxialSegments[j].getTrackCandId().size(); k++) { //loop over all TrackCandIds assigned to this segment
          // B2INFO("TrackIds from segment: "<<cdcSegmentsArray[j]->getTrackCandId().at(k));
          if (cdcAxialSegments[j].getSuperlayerId() == SLId - 2
              && cdcAxialSegments[j].getTrackCandId().at(k) == i) {

            for (int sl = 0; sl
                 < TrackCandidates.at(i).getNSegments(); sl++) { //check, if there were already another segment from the same superlayer assigned to this candidate
              if (cdcAxialSegments[j].getSuperlayerId()
                  == TrackCandidates.at(i).getSegments().at(
                    sl).getSuperlayerId()) {

                already = true;
              }
            }


            if (already == false) {
              TrackCandidates.at(i).addSegment(
                cdcAxialSegments[j]);
              //  B2INFO("Add to candidate "<<i<<" a segment with Id "<<cdcSegmentsArray[j]->getId()<<"  from SL "<<cdcSegmentsArray[j]->getSuperlayerId());

            } else { // Split the track candidate

              //create a new candidate by copying the old one (new Id) and adding to it the new segment
              trackcounter++;

              CDCTrackCandidate SplitTrack(candidate, trackcounter);
              TrackCandidates.push_back(SplitTrack);

              TrackCandidates.at(trackcounter).addSegment(
                cdcAxialSegments[j]);
              cdcAxialSegments[j].setTrackCandId(trackcounter);
              //  B2INFO("Add to candidate "<<trackcounter<<" a segment with Id "<<cdcSegmentsArray[j]->getId()<<"  from SL "<<cdcSegmentsArray[j]->getSuperlayerId());

            }

          } //endif

        }  //end for loop over all TrackCandIds
      }

    } //end for loop over all track candidates

    SLId = SLId - 2; //go one axial supelayer lower

  } //end while loop over all axial superlayers


  return TrackCandidates;
}


void AxialTrackFinder::FitTrackCandidate(CDCTrackCandidate & candidate)
{

  TGraph * graph;
  double x[100];
  double y[100];
  int nHits = candidate.getNHits(); //Nr of hits in a track
  int temp_nHits; //Nr of hit in a track *excluding* the hits in the first superlayer
  temp_nHits = 0;

  //loop over all hits and get their conformal coordinates (exclude first layer!)
  for (int j = 0; j < nHits; j++) {

    if (candidate.getTrackHits().at(j).getSuperlayerId() != 1) {
      temp_nHits++;
      x[j] = candidate.getTrackHits().at(j).getConformalX();
      y[j] = candidate.getTrackHits().at(j).getConformalY();
      //B2INFO("x: "<<x[j]<<"  y: "<<y[j]);

    }
  }

  graph = new TGraph(temp_nHits, x, y);

  double min = graph->GetXaxis()->GetXmin();
  double max = graph->GetXaxis()->GetXmax();

  //Fit the conformal coordinates with simple linear fit
  graph->Fit("pol1", "Q", "", min, max);
  TF1 *fit = graph->GetFunction("pol1");

  //TFile * output = new TFile("Warum.root", "RECREATE");

  double Chi = fit->GetChisquare();

  candidate.setChiSquare(Chi);

  //First estimation of the absolut momentum value
  double x0 = -fit->GetParameter(0) / fit->GetParameter(1);  // x- and y - axis section
  double y0 = fit->GetParameter(0);
  //B2INFO("x0: "<<x0<<"  y0: "<<y0);
  double R = sqrt(1 / x0 * 1 / x0 + 1 / y0 * 1 / y0); //Radius of the track circle in the normal plane
  double p = R * 1.5 / 299.792458;  //Preliminary(!) calculation of momentum in GeV, magnetic field and c hardcoded for now...
  //B2INFO("Radius: "<<R<<"  Momentum: "<<p);
  candidate.setMomentumValue(p);

  //Preliminary estimation of the charge of the track
  double phi1 = candidate.getInnerMostSegment().getInnerMostHit().getPhi();
  double phi2 = candidate.getOuterMostSegment().getOuterMostHit().getPhi();

  if ((phi1 - phi2) > 0) candidate.setChargeSign(1);
  else candidate.setChargeSign(-1);

}



void AxialTrackFinder::FitTrackCandidates(vector<CDCTrackCandidate> & candidates)
{

  for (unsigned int i = 0; i < candidates.size(); i++) {
    FitTrackCandidate(candidates.at(i));

  }
}

void AxialTrackFinder::CollectTrackCandidates(vector<CDCSegment> & cdcAxialSegments,
                                              string CDCTrackCandidates)
{

  if (cdcAxialSegments.size() == 0) B2WARNING("AxialTrackFinder: cdcAxialSegments collection is empty!");

  StoreArray<CDCTrackCandidate> cdcTrackCandidates(CDCTrackCandidates.c_str());  //Output TrackCandidates

  vector<CDCTrackCandidate> TrackCandidates;  //vector to hold Track Candidates, is emptied and refilled for each start superlayer
  vector<CDCTrackCandidate> FinalTrackCandidates;  //vector to gather Track Candidates (who passed the Chi^2 test) from all start superlayers

  int nSegments = cdcAxialSegments.size();
  int NTracks = 0;

  vector<int> UsedSegmentId; //vector to store the Ids of segments, which were already used to reconstruct a track candidate

  int startSLId = 9;

  //int trackcounter = -1; //count the number of track candidates
  int counter = -1;

  B2INFO("Find possible track candidates...");

  //loop over all possible starting superlayers, starting with the outermost superlayer
  //first all track candidates are found, after a simple fit those with too large Chi2 are discarded, all the other candidates are collected in FinalTrackCandidates
  while (startSLId > 1) {

    TrackCandidates = FindTrackCandidates(cdcAxialSegments, 28, 0.3, 0.005,
                                          startSLId);

    NTracks = TrackCandidates.size();
    //B2INFO("Number of TrackCandidates starting in SL "<<startSLId<<" : "<<NTracks);

    FitTrackCandidates(TrackCandidates);

    //Discard bad chi2 and tracks with unrealistic momentum
    for (int i = 0; i < NTracks; i++) {
      //B2INFO("Track Nr "<<i<<"  Chi2: "<<TrackCandidates.at(i).getChiSquare());
      if (TrackCandidates.at(i).getChiSquare() < 0.003 && TrackCandidates.at(i).getMomentumValue() < 10) { //what value???
        FinalTrackCandidates.push_back(TrackCandidates.at(i));

      }

    }
    //B2INFO("Number of TrackCandidates after Chi2 test: "<<FinalTrackCandidates.size());

    startSLId = startSLId - 2;  //go one axial supelayer lower

    //cleanup
    TrackCandidates.erase(TrackCandidates.begin(), TrackCandidates.end());
    for (int i = 0; i < nSegments; i++) {
      cdcAxialSegments[i].clearTrackCandId();
    }

  }//end while loop over superlayers


  B2INFO("Select the best track candidates...");
  //Now the best ( = longest) TrackCandidates are chosen from the FinalTrackCandidates

  int NFinalTracks = FinalTrackCandidates.size();

  int NumberOfSegments = 5; //maximum possible track candidate length

  while (NumberOfSegments > 1) {  //loop over possible track lengths
    for (int i = 0; i < NFinalTracks; i++) {  //loop over all track candidates

      //check for correct number of segments and the fraction of already used segments
      if (FinalTrackCandidates.at(i).getNSegments() == NumberOfSegments
          && UsedSegmentsFraction(FinalTrackCandidates.at(i),
                                  UsedSegmentId) < 0.3) {
        counter++;
        //B2INFO("Nr "<<counter<<"   Final track with "<<NumberOfSegments<<" found");

        //Add the Ids of all used segments to the UsedSegmentId vector
        for (int j = 0; j < FinalTrackCandidates.at(i).getNSegments(); j++) {
          UsedSegmentId.push_back(
            FinalTrackCandidates.at(i).getSegments().at(j).getId());
        }

        //extension for special cases, where short tracks found separately can be combined to one track

        if (NumberOfSegments == 3 || NumberOfSegments == 2) {
          int additionalSegments = 2;

          while (additionalSegments > 0) { //while loop, to be performed two times, first searching for 2 segment tracks, then 1 segment tracks
            bool sameSegment = false;
            for (int i_add = 0; i_add < NFinalTracks; i_add++) {  //loop over all candidates
              for (int id = 0; id < FinalTrackCandidates.at(i).getNSegments(); id++) { //check, if there were already the same segment assigned to this candidate
                for (int id2 = 0; id2 < FinalTrackCandidates.at(i_add).getNSegments(); id2++)
                  if (FinalTrackCandidates.at(i).getSegments().at(id).getId() == FinalTrackCandidates.at(i_add).getSegments().at(id2).getId()) {
                    sameSegment = true;
                    //B2INFO("This segment ist already there!!!");
                  }
              }


              //check for right segments number and used fraction
              if (i_add != i && sameSegment == false
                  && FinalTrackCandidates.at(i_add).getNSegments()
                  == additionalSegments
                  && UsedSegmentsFraction(
                    FinalTrackCandidates.at(i_add),
                    UsedSegmentId) < 0.3) {

                //additional variable to check if the short tracks can be combined to one
                double angle = FinalTrackCandidates.at(i_add).getSegments().at(0).getDirection().Angle(FinalTrackCandidates.at(i).getSegments().at(0).getDirection());
                if (angle > TMath::Pi() / 2) angle = angle - TMath::Pi() ; // -90 < angle < 90

                //check for shortest distance and angle
                if (ShortestDistance(
                      FinalTrackCandidates.at(i_add).getSegments().at(
                        0),
                      FinalTrackCandidates.at(i).getSegments().at(
                        0)) < 0.005 && abs(angle) < 0.3 && SimpleDistance(FinalTrackCandidates.at(i_add).getSegments().at(
                                                                            0), FinalTrackCandidates.at(i).getSegments().at(
                                                                            0)) < 70) {
                  B2INFO("Two short tracks combined to one");

                  //Add all segment from one short track to the other and mark segment Ids as used
                  for (int s = 0; s
                       < FinalTrackCandidates.at(i_add).getNSegments(); s++) {

                    FinalTrackCandidates.at(i).addSegment(FinalTrackCandidates.at(i_add).getSegments().at(s));
                    UsedSegmentId.push_back(
                      FinalTrackCandidates.at(i_add).getSegments().at(
                        s).getId());

                  }
                  //Refit the candidate
                  FitTrackCandidate(FinalTrackCandidates.at(i));

                }
              }
            }  //loop over all candidates
            additionalSegments--;
          }
        } //end extension

        //Finally add the candidate to the StoreArray
        new(cdcTrackCandidates->AddrAt(counter)) CDCTrackCandidate(
          FinalTrackCandidates.at(i), counter);
        B2INFO("Candidate " << counter << " with " << cdcTrackCandidates[counter]->getNSegments() << " Segments added to final array ...");

      }
    } //end for loop over all candidates

    NumberOfSegments--; //decrease required track length

  } //end while

}





