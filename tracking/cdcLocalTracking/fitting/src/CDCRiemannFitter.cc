/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/CDCRiemannFitter.h"

#include <Eigen/Dense>


using namespace std;
using namespace Belle2;
using namespace Eigen;

using namespace CDCLocalTracking;

ClassImpInCDCLocalTracking(CDCRiemannFitter)


namespace {
  CDCRiemannFitter* g_fitter = nullptr;
  CDCRiemannFitter* g_lineFitter = nullptr;
  CDCRiemannFitter* g_originCircleFitter = nullptr;
}



const CDCRiemannFitter& CDCRiemannFitter::getFitter()
{
  if (not g_fitter) {
    g_fitter = new CDCRiemannFitter();
  }
  return *g_fitter;
}



const CDCRiemannFitter& CDCRiemannFitter::getLineFitter()
{
  if (not g_lineFitter) {
    g_lineFitter = new CDCRiemannFitter();
    g_lineFitter->setLineConstrained();
  }
  return *g_lineFitter;
}



const CDCRiemannFitter& CDCRiemannFitter::getOriginCircleFitter()
{
  if (not g_originCircleFitter) {
    g_originCircleFitter = new CDCRiemannFitter();
    g_originCircleFitter->setOriginConstrained();
  }
  return *g_originCircleFitter;
}




CDCRiemannFitter::CDCRiemannFitter() : m_usePosition(true), m_useOrientation(true),
  m_lineConstrained(false) , m_originConstrained(false) {;}

CDCRiemannFitter::~CDCRiemannFitter()
{
}











CDCTrajectory2D CDCRiemannFitter::fit(const CDCRLWireHitTriple& rlWireHitTriple) const
{
  CDCTrajectory2D result;
  update(result, rlWireHitTriple);
  return result;
}


CDCTrajectory2D CDCRiemannFitter::fit(const CDCRecoSegment2D& recoSegment2D) const
{
  CDCTrajectory2D result;
  update(result, recoSegment2D);
  return result;
}


CDCTrajectory2D CDCRiemannFitter::fit(const CDCSegmentTriple& segmentTriple) const
{
  CDCTrajectory2D result;
  update(result, segmentTriple);
  return result;
}


void CDCRiemannFitter::update(CDCTrajectory2D& fit, const CDCRLWireHitTriple& rlWireHitTriple) const
{

  CDCObservations2D observations2D;
  observations2D.append(rlWireHitTriple);

  B2ERROR("CDCRiemannFitter ask to fit a oriented wire hit triple without line constraint.");
  update(fit, observations2D);

  //Set transverse s reference
  fit.setStartPos2D(rlWireHitTriple.getStartWire().getRefPos2D());

  //Check if fit is forward
  if (not rlWireHitTriple.isForwardTrajectory(fit)) fit.reverse();
  if (not rlWireHitTriple.isForwardTrajectory(fit)) B2WARNING("Fit cannot be oriented correctly");

}

void CDCRiemannFitter::update(CDCTrajectory2D& fit, const CDCRecoSegment2D& recoSegment2D) const
{

  CDCObservations2D observations2D;
  if (m_usePosition) {
    observations2D.append(recoSegment2D, true);
  }
  if (m_useOrientation) {
    observations2D.append(recoSegment2D, false);
  }

  update(fit, observations2D);

  //Set transverse s reference
  fit.setStartPos2D(recoSegment2D.front().getRecoPos2D()) ;

  //Check if fit is forward
  if (not recoSegment2D.isForwardTrajectory(fit)) fit.reverse();
  if (not recoSegment2D.isForwardTrajectory(fit)) B2WARNING("Fit cannot be oriented correctly");

}

void CDCRiemannFitter::update(CDCTrajectory2D& fit,
                              const CDCRecoSegment2D& firstRecoSegment2D,
                              const CDCRecoSegment2D& secondRecoSegment2D) const
{

  CDCObservations2D observations2D;
  if (m_usePosition) {
    observations2D.append(firstRecoSegment2D, true);
    observations2D.append(secondRecoSegment2D, true);
  }
  if (m_useOrientation) {
    observations2D.append(firstRecoSegment2D, false);
    observations2D.append(secondRecoSegment2D, false);
  }

  update(fit, observations2D);

  //set transverse s reference
  fit.setStartPos2D(firstRecoSegment2D.front().getRecoPos2D()) ;

  //check if fit is forward
  if (not firstRecoSegment2D.isForwardTrajectory(fit)) fit.reverse();

  if (not(firstRecoSegment2D.isForwardTrajectory(fit) and secondRecoSegment2D.isForwardTrajectory(fit)))
    B2WARNING("Fit cannot be oriented correctly");

}

void CDCRiemannFitter::update(CDCTrajectory2D& fit, const CDCSegmentTriple& segmentTriple) const
{
  update(fit, *(segmentTriple.getStart()), *(segmentTriple.getEnd()));
}





void CDCRiemannFitter::update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const
{

  if (observations2D.getNObservationsWithDriftRadius() > 0) {
    updateWithRightLeft(fit, observations2D);
  } else {
    updateWithOutRightLeft(fit, observations2D);
  }

}





void CDCRiemannFitter::updateWithOutRightLeft(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const
{

  CDCObservations2D::EigenObservationMatrix&&  eigenObservation = observations2D.getObservationMatrix();
  size_t nObservations = observations2D.size();


  if (isLineConstrained()) {

    //do a normal line fit
    Matrix<FloatType, Dynamic, 2> points = eigenObservation.leftCols<2>();

    Matrix<FloatType, 1, 2> pointMean;
    //RowVector2d pointMean;
    pointMean << 0.0, 0.0;
    if (!(isOriginConstrained())) {
      // subtract the offset from the origin
      pointMean = points.colwise().mean();

      points = points.rowwise() - pointMean;
    }
    Matrix<FloatType, 2, 2> covarianceMatrix = points.transpose() * points;

    SelfAdjointEigenSolver< Matrix<FloatType, 2, 2> > eigensolver(covarianceMatrix);

    if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");

    //the eigenvalues are generated in increasing order
    //we are interested in the lowest one since we want to compute the normal vector of the plane

    Matrix<FloatType, 2, 1> normalToLine = eigensolver.eigenvectors().col(0);

    FloatType offset = -pointMean * normalToLine;

    // set the generalized circle parameters
    // last set to zero constrains to a line
    fit.setGenCircle(GeneralizedCircle(offset, normalToLine(0), normalToLine(1), 0));

  } else {

    //lift the points to the projection space
    Matrix<FloatType, Dynamic, 3> projectedPoints(nObservations, 3);

    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();


    Matrix<FloatType, 1, 3> pointMean;
    pointMean << 0.0, 0.0, 0.0;
    if (!(isOriginConstrained())) {
      // subtract the offset from the origin
      pointMean = projectedPoints.colwise().mean();

      projectedPoints = projectedPoints.rowwise() - pointMean;
    }

    Matrix<FloatType, 3, 3> covarianceMatrix = projectedPoints.transpose() * projectedPoints;

    SelfAdjointEigenSolver< Matrix<FloatType, 3, 3> > eigensolver(covarianceMatrix);

    if (eigensolver.info() != Success) B2WARNING("SelfAdjointEigenSolver could not compute the eigen values of the observation matrix");

    //the eigenvalues are generated in increasing order
    //we are interested in the lowest one since we want to compute the normal vector of the plane

    Matrix<FloatType, 3, 1> normalToPlane = eigensolver.eigenvectors().col(0);

    FloatType offset = -pointMean * normalToPlane;

    fit.setGenCircle(GeneralizedCircle(offset, normalToPlane(0), normalToPlane(1), normalToPlane(2)));
    //fit.setParameters();

  }

  //check if the orientation is alright
  Vector2D directionAtCenter = fit.getUnitMom2D(Vector2D(0.0, 0.0));


  size_t voteForChangeSign = 0;
  for (size_t iPoint = 0; iPoint < nObservations; ++iPoint) {
    FloatType pointInSameDirection = eigenObservation(iPoint, 0) * directionAtCenter.x() +
                                     eigenObservation(iPoint, 1) * directionAtCenter.y();
    if (pointInSameDirection < 0) ++voteForChangeSign;
  }

  if (voteForChangeSign > nObservations / 2.0) fit.reverse();

}





void CDCRiemannFitter::updateWithRightLeft(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const
{

  CDCObservations2D::EigenObservationMatrix && eigenObservation = observations2D.getObservationMatrix();
  size_t nObservations = observations2D.size();

  //cout << "updateWithRightLeft : " << endl;
  //observations always have the structure
  /*
  observables[0][0] == x of first point
  observables[0][1] == y of first point
  observables[0][2] == desired distance of first point
  */

  Matrix< FloatType, Dynamic, 1 > distances = eigenObservation.col(2);

  //cout << "distances : " << endl << distances << endl;

  if ((isLineConstrained()) && (isOriginConstrained())) {


    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 2);

    //all coordiates
    //projectedPoints.col(0) = Matrix<FloatType,Dynamic,1>::Constant(nObservations,1.0);
    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    //projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 2, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(0.0, parameters(0), parameters(1), 0.0));
  }

  else if ((! isLineConstrained()) && (isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 3);

    //all coordiates
    //projectedPoints.col(0) = Matrix<FloatType,Dynamic,1>::Constant(1.0);
    projectedPoints.col(0) = eigenObservation.col(0);
    projectedPoints.col(1) = eigenObservation.col(1);
    projectedPoints.col(2) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 3, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(0.0, parameters(0), parameters(1), parameters(2)));
  }

  else if ((isLineConstrained()) && (! isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 3);

    //all coordiates
    projectedPoints.col(0) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
    projectedPoints.col(1) = eigenObservation.col(0);
    projectedPoints.col(2) = eigenObservation.col(1);
    //projectedPoints.col(3) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    Matrix< FloatType, 3, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(parameters(0), parameters(1), parameters(2), 0.0));
    //fit.setParameters(parameters(0),parameters(1),parameters(2),0.0);

  }

  else if ((! isLineConstrained()) && (! isOriginConstrained())) {

    Matrix< FloatType, Dynamic, Dynamic > projectedPoints(nObservations, 4);

    //all coordiates
    projectedPoints.col(0) = Matrix<FloatType, Dynamic, 1>::Constant(nObservations, 1.0);
    projectedPoints.col(1) = eigenObservation.col(0);
    projectedPoints.col(2) = eigenObservation.col(1);
    projectedPoints.col(3) = eigenObservation.leftCols<2>().rowwise().squaredNorm();

    //cout << "points : " << endl << projectedPoints << endl;

    Matrix< FloatType, 4, 1> parameters = projectedPoints.jacobiSvd(ComputeThinU | ComputeThinV).solve(distances);

    fit.setGenCircle(GeneralizedCircle(parameters(0), parameters(1), parameters(2), parameters(3)));

  }


}


