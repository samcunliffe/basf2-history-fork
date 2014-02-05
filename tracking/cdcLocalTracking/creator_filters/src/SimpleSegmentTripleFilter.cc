/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/SimpleSegmentTripleFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;

SimpleSegmentTripleFilter::SimpleSegmentTripleFilter() : m_riemannFitter(), m_szFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

SimpleSegmentTripleFilter::~SimpleSegmentTripleFilter()
{
}


void SimpleSegmentTripleFilter::clear()
{

}


bool SimpleSegmentTripleFilter::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("SimpleSegmentTripleFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return false;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("SimpleSegmentTripleFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return false;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  //do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  //B2DEBUG(100,"    startSegment.front() = " <<  startSegment.front() );
  //B2DEBUG(100,"    startSegment.back()  = " <<  startSegment.back() );

  //B2DEBUG(100,"    endSegment.front()   = " <<  endSegment.front() );
  //B2DEBUG(100,"    endSegment.back()    = " <<  endSegment.back() );

  //B2DEBUG(100,"    startSegment.getStartPerpS(startFit) = " <<  startSegment.getStartPerpS(startFit) );
  //B2DEBUG(100,"    startSegment.getEndPerpS(startFit)   = " <<  startSegment.getEndPerpS(startFit) );

  //B2DEBUG(100,"    endSegment.getStartPerpS(startFit)   = " <<  endSegment.getStartPerpS(startFit) );
  //B2DEBUG(100,"    endSegment.getEndPerpS(startFit)     = " <<  endSegment.getEndPerpS(startFit) );

  //B2DEBUG(100,"    Check startSegment isForwardFit " << startSegment.isForwardFit(startFit));
  //B2DEBUG(100,"    Check endSegment isForwardFit " << endSegment.isForwardFit(startFit));

  //alignment cut - nonnegotiable
  if (not endSegment.isForwardTrajectory(startFit)) return false;

  //FloatType distanceAverage = ( endSegment.getStartPerpS(startFit) +
  //                              endSegment.getEndPerpS(startFit)    ) / 2;


  //check if end segment is in forward direction of startSegment
  //reference point was already set in the fit to the first hit of the startSegment
  //alignment cut - nonnegotiable
  if (endSegment.getEndPerpS(startFit)   < startSegment.getEndPerpS(startFit) or
      endSegment.getStartPerpS(startFit) < startSegment.getStartPerpS(startFit)) return false;


  //check if the last hit of the reco hit lies further in travel direction than the first
  //FloatType distanceLastToFirst = endSegment.getEndPerpS(startFit) -
  //                                endSegment.getStartPerpS(startFit) ;

  Vector2D startCOM = startSegment.getCenterOfMass2D();
  Vector2D endCOM   = endSegment.getCenterOfMass2D();

  //B2DEBUG(100,"  Check distanceAverage " << distanceAverage);
  //B2DEBUG(100,"  Check distanceLastToFirst " << distanceLastToFirst);

  //difference in position at the forward extrapolation point
  Vector2D endCenter = endFit.getClosest(endCOM);
  Vector2D pointOnFromTrack = startFit.getCloseSamePolarR(endCenter);

  //difference in flight direction
  Vector2D startDirection  = startFit.getUnitMom2D(pointOnFromTrack);
  Vector2D endDirection    = endFit.getUnitMom2D(endCenter);

  //difference in momentum ?
  //marked as usused, because they should not generate a warning
  //the variables are a reminder which variables can be used in the cut
  FloatType startMom __attribute__((unused)) = startFit.getAbsMom2D();
  FloatType endMom __attribute__((unused)) = endFit.getAbsMom2D();

  //check if end segment is in forward direction of startSegment


  //make a cut - make this more sophisticated at some point
  double cosDeviation = endCenter.cosWith(pointOnFromTrack);
  double tolerance = cos(PI / 180);

  return true;

  return cosDeviation > tolerance;


}


CellWeight SimpleSegmentTripleFilter::isGoodSegmentTriple(const CDCSegmentTriple& segmentTriple)
{

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStart();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as start segment");
    return NOT_A_CELL;
  }
  if (ptrMiddleSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as middle segment");
    return NOT_A_CELL;
  }
  if (ptrEndSegment == nullptr) {
    B2ERROR("MCSegmentTripleFilter::isGoodSegmentTriple invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;


  //check if the middle segment lies within the acceptable bounds in angular deviation
  {
    //get the remembered fits
    const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
    const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

    //use only the first and last hit for this check
    const CDCRecoHit2D& firstHit = middleSegment.front();
    const CDCRecoHit2D& lastHit = middleSegment.back();

    Vector2D firstPosition = firstHit.getRecoPos2D();
    Vector2D lastPosition  = lastHit.getRecoPos2D();

    Vector2D firstReconstructed = startFit.getCloseSamePolarR(firstPosition);
    Vector2D lastReconstructed = endFit.getCloseSamePolarR(lastPosition);

    FloatType first_refToReconstructedAngle = firstPosition.angleWith(firstReconstructed);
    FloatType last_refToReconstructedAngle = lastPosition.angleWith(lastReconstructed);

    const CDCWire& firstWire = firstHit.getWire();
    const CDCWire& lastWire  = lastHit.getWire();

    //Both wires are stereo wires by construction
    //So they have both non zero twist
    //check if the first and last are within the acceptable twist bounds
    const std::pair<FloatType, FloatType>& first_phiRangeToRef = firstWire.getPhiRangeToRef();
    const std::pair<FloatType, FloatType>& last_phiRangeToRef  = lastWire.getPhiRangeToRef();

    const FloatType tolerance = 0.0;

    const bool agrees = (first_phiRangeToRef.first - tolerance) < first_refToReconstructedAngle and
                        first_refToReconstructedAngle < (first_phiRangeToRef.second + tolerance) and

                        (last_phiRangeToRef.first - tolerance) < last_refToReconstructedAngle and
                        last_refToReconstructedAngle < (last_phiRangeToRef.second + tolerance) ;

    //B2DEBUG(100,"    first_rangeToRefAngle.first < first_refToReconstructedAngle  = "
    //              << (first_rangeToRefAngle.first < first_refToReconstructedAngle) );

    //B2DEBUG(100,"    first_refToReconstructedAngle < first_rangeToRefAngle.second = "
    //              << (first_refToReconstructedAngle < first_rangeToRefAngle.second) );

    //B2DEBUG(100,"    last_rangeToRefAngle.first < last_refToReconstructedAngle    = "
    //              << (last_rangeToRefAngle.first < last_refToReconstructedAngle) );

    //B2DEBUG(100,"    last_refToReconstructedAngle < last_rangeToRefAngle.second   = "
    //              << (last_refToReconstructedAngle < last_rangeToRefAngle.second) );


    if (not agrees) {
      B2DEBUG(100, "    first_rangeToRefAngle.first   = " <<  first_phiRangeToRef.first);
      B2DEBUG(100, "    first_refToReconstructedAngle = " <<  first_refToReconstructedAngle);
      B2DEBUG(100, "    first_rangeToRefAngle.second  = " <<  first_phiRangeToRef.second);

      B2DEBUG(100, "    firstPosition                 = " <<  firstPosition);
      B2DEBUG(100, "    firstReconstructed            = " <<  firstReconstructed);

      B2DEBUG(100, "    last_rangeToRefAngle.first    = " <<  last_phiRangeToRef.first);
      B2DEBUG(100, "    last_refToReconstructedAngle  = " <<  last_refToReconstructedAngle);
      B2DEBUG(100, "    last_rangeToRefAngle.second   = " <<  last_phiRangeToRef.second);

      B2DEBUG(100, "    lastPosition                  = " <<  firstPosition);
      B2DEBUG(100, "    lastReconstructed             = " <<  lastReconstructed);

      B2DEBUG(100, "  MiddleSegment rejected because of angular mismatch");


      //double d;
      //cin >> d;
      return NOT_A_CELL;
    }
    //now check if the the fit and the middle segment are coaligned - is here the right point to check for this ??
    //if ( not middleSegment.isForwardFit(startFit) ) return false;

  }

  // make more complex judgement on fitness

  // Get the combined fit of start and end axial segment
  const CDCTrajectory2D& fit = getFittedTrajectory2D(segmentTriple);

  // Check if the middle segment is actually coaligned with the trajectory
  if (not middleSegment.isForwardTrajectory(fit)) return NOT_A_CELL;

  //Reconstruct the
  CDCRecoSegment3D reconstructedMiddle;
  for (CDCStereoRecoSegment2D::const_iterator itRecoHits = middleSegment.begin();
       itRecoHits != middleSegment.end(); ++itRecoHits) {
    reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(*itRecoHits, fit));
    if (not reconstructedMiddle.back().isInCDC()) {
      B2DEBUG(100, "  RecoHit out of CDC");
      //double d;
      //cin >> d;
      return NOT_A_CELL;
    }
  }

  const CDCTrajectorySZ& trajectorySZ = getFittedTrajectorySZ(segmentTriple);

  FloatType squaredDistance = reconstructedMiddle.getSquaredZDist(trajectorySZ);

  {
    B2DEBUG(100, "  ZDistance of the middleSegment = " << squaredDistance);

  }

  CellState result = startSegment.size() + middleSegment.size() + endSegment.size();

  if (not isNotACell(result)) {

    getFittedTrajectory2D(segmentTriple);
    getFittedTrajectorySZ(segmentTriple);

  }

  return result;

}


const CDCTrajectory2D& SimpleSegmentTripleFilter::getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}



const CDCTrajectory2D& SimpleSegmentTripleFilter::getFittedTrajectory2D(const CDCAxialAxialSegmentPair& axialAxialSegmentPair) const
{
  CDCTrajectory2D& trajectory2D = axialAxialSegmentPair.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, axialAxialSegmentPair);
  }
  return trajectory2D;
}



const CDCTrajectorySZ& SimpleSegmentTripleFilter::getFittedTrajectorySZ(const CDCSegmentTriple& segmentTriple) const
{

  CDCTrajectorySZ& trajectorySZ = segmentTriple.getTrajectorySZ();

  if (not trajectorySZ.isFitted()) {

    const CDCTrajectory2D& trajectory2D = getFittedTrajectory2D(segmentTriple);

    const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddle();
    const CDCStereoRecoSegment2D middleSegment = *ptrMiddleSegment;

    //recostruct the stereo segment with the hit
    CDCRecoSegment3D reconstructedMiddle;
    for (CDCStereoRecoSegment2D::const_iterator itRecoHits = middleSegment.begin();
         itRecoHits != middleSegment.end(); ++itRecoHits) {
      reconstructedMiddle.push_back(CDCRecoHit3D::reconstruct(*itRecoHits, trajectory2D));
    }

    getSZFitter().update(trajectorySZ, reconstructedMiddle);

  }

  return trajectorySZ;

}



