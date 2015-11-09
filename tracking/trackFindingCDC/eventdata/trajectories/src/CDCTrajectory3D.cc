/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCBField.h>


#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <genfit/TrackCand.h>

#include <mdst/dataobjects/MCParticle.h>

#include <cmath>
#include <cassert>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;



CDCTrajectory3D::CDCTrajectory3D(const Vector3D& pos3D,
                                 const Vector3D& mom3D,
                                 const double charge) :
  m_localOrigin(pos3D),
  m_localHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D),
               mom3D.xy().unit(),
               0.0,
               mom3D.cotTheta(),
               0.0)
{
}


CDCTrajectory3D::CDCTrajectory3D(const Vector3D& pos3D,
                                 const Vector3D& mom3D,
                                 const double charge,
                                 const double bZ) :
  m_localOrigin(pos3D),
  m_localHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, bZ),
               mom3D.xy().unit(),
               0.0,
               mom3D.cotTheta(),
               0.0)
{
}

CDCTrajectory3D::CDCTrajectory3D(const MCParticle& mcParticle, const double bZ) :
  CDCTrajectory3D(Vector3D{mcParticle.getProductionVertex()},
                  Vector3D{mcParticle.getMomentum()},
                  mcParticle.getCharge(),
                  bZ)
{
}

CDCTrajectory3D::CDCTrajectory3D(const MCParticle& mcParticle) :
  CDCTrajectory3D(Vector3D{mcParticle.getProductionVertex()},
                  Vector3D{mcParticle.getMomentum()},
                  mcParticle.getCharge())
{
}



CDCTrajectory3D::CDCTrajectory3D(const CDCTrajectory2D& trajectory2D,
                                 const CDCTrajectorySZ& trajectorySZ) :
  m_localOrigin(trajectory2D.getLocalOrigin()),
  m_localHelix(trajectory2D.getLocalCircle(), trajectorySZ.getSZLine())
{
}

CDCTrajectory3D::CDCTrajectory3D(const CDCTrajectory2D& trajectory2D) :
  CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption())
{
}

CDCTrajectory3D::CDCTrajectory3D(const genfit::TrackCand& gfTrackCand) :
  CDCTrajectory3D(gfTrackCand,
                  getBFieldZ(Vector3D{gfTrackCand.getPosSeed()}))
{
}

CDCTrajectory3D::CDCTrajectory3D(const genfit::TrackCand& gfTrackCand, const double bZ) :
  CDCTrajectory3D(Vector3D{gfTrackCand.getPosSeed()},
                  Vector3D{gfTrackCand.getMomSeed()},
                  gfTrackCand.getChargeSeed(),
                  bZ)
{
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  TMatrixDSym cov6 = gfTrackCand.getCovSeed();

  // 1. Rotate to a system where phi0 = 0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  const double px = gfTrackCand.getStateSeed()[iPx];
  const double py = gfTrackCand.getStateSeed()[iPy];
  const double pt = hypot(px, py);

  const double cosPhi0 = px / pt;
  const double sinPhi0 = py / pt;

  // Passive rotation matrix by phi0:
  jacobianRot(iX, iX) = cosPhi0;
  jacobianRot(iX, iY) = sinPhi0;
  jacobianRot(iY, iX) = -sinPhi0;
  jacobianRot(iY, iY) = cosPhi0;
  jacobianRot(iZ, iZ) = 1.0;

  jacobianRot(iPx, iPx) = cosPhi0;
  jacobianRot(iPx, iPy) = sinPhi0;
  jacobianRot(iPy, iPx) = -sinPhi0;
  jacobianRot(iPy, iPy) = cosPhi0;
  jacobianRot(iPz, iPz) = 1.0;

  cov6.Similarity(jacobianRot);

  // 2. Translate to perigee parameters
  TMatrixD jacobianReduce(5, 6);
  jacobianReduce.Zero();

  const double invPt = 1 / pt;
  const double invPtSquared = invPt * invPt;
  const double pz = gfTrackCand.getStateSeed()[iPz];
  const double alpha = getAlphaFromBField(bZ);
  const double charge = gfTrackCand.getChargeSeed();

  using namespace NHelixParameter;
  jacobianReduce(c_Curv, iPx) = charge * invPtSquared / alpha ;
  jacobianReduce(c_Phi0, iPy) = invPt;
  jacobianReduce(c_I, iY) = 1;
  jacobianReduce(c_TanL, iPx) = - pz * invPtSquared;
  jacobianReduce(c_TanL, iPz) = invPt;
  jacobianReduce(c_Z0, iZ) = 1;
  // Note the column corresponding to iX is completely zero as expectable.

  cov6.Similarity(jacobianReduce);

  // The covariance should now be the correct 5x5 covariance matrix.
  m_localHelix.setHelixCovariance(HelixCovariance(cov6));
}



void CDCTrajectory3D::setPosMom3D(const Vector3D& pos3D,
                                  const Vector3D& mom3D,
                                  const double charge)
{
  m_localOrigin = pos3D;
  m_localHelix = UncertainHelix(absMom2DToCurvature(mom3D.xy().norm(), charge, pos3D),
                                mom3D.xy().unit(),
                                0.0,
                                mom3D.cotTheta(),
                                0.0);
}





bool CDCTrajectory3D::fillInto(genfit::TrackCand& gfTrackCand) const
{
  Vector3D position = getSupport();
  return fillInto(gfTrackCand, getBFieldZ(position));
}

bool CDCTrajectory3D::fillInto(genfit::TrackCand& gfTrackCand, const double bZ) const
{
  // Set the start parameters
  Vector3D position = getSupport();
  Vector3D momentum = bZ == 0 ? getUnitMom3DAtSupport() : getMom3DAtSupport(bZ);
  ESign charge = getChargeSign();

  // Do not propagate invalid fits, signal that the fit is invalid to the caller.
  if (not isValidSign(charge) or momentum.hasNAN() or position.hasNAN()) {
    // B2INFO("Charge " <<  charge);
    // B2INFO("Position " <<  position);
    // B2INFO("Local origin " <<  getLocalOrigin());
    // B2INFO("Momentum " <<  momentum);
    return false;
  }

  gfTrackCand.setPosMomSeed(position, momentum, charge);

  // Now translate and set the covariance matrix.
  const UncertainHelix& localHelix = getLocalHelix();

  const double impactXY = localHelix.impactXY();
  const Vector2D& tangentialXY = localHelix.tangentialXY();

  const double cosPhi0 = tangentialXY.x();
  const double sinPhi0 = tangentialXY.y();

  const double curvatureXY = localHelix.curvatureXY();
  const double tanLambda = localHelix.tanLambda();

  // 0. Define indices
  // Maybe push these out of this function:
  // Indices of the cartesian coordinates
  const int iX = 0;
  const int iY = 1;
  const int iZ = 2;
  const int iPx = 3;
  const int iPy = 4;
  const int iPz = 5;

  TMatrixDSym cov5 = localHelix.helixCovariance();

  // 1. Inflat the perigee covariance to a cartesian covariance where phi0 = 0 is assumed
  // Jacobian matrix for the translation
  TMatrixD jacobianInflate(6, 5);
  jacobianInflate.Zero();

  const double alpha = getAlphaFromBField(bZ);
  const double chargeAlphaCurv = charge * alpha * curvatureXY;
  const double chargeAlphaCurv2 = charge * alpha * std::pow(curvatureXY, 2);

  const double invChargeAlphaCurv = 1.0 / chargeAlphaCurv;
  const double invChargeAlphaCurv2 = 1.0 / chargeAlphaCurv2;


  using namespace NHelixParameter;
  // Position
  jacobianInflate(iX, c_Phi0) = -impactXY;
  jacobianInflate(iY, c_I) = 1.0;
  jacobianInflate(iZ, c_Z0) = 1.0;

  // Momentum
  if (bZ == 0) {
    jacobianInflate(iPx, c_Curv) = 0;
    jacobianInflate(iPy, c_Phi0) = momentum.cylindricalR();
    jacobianInflate(iPz, c_Curv) = 0;
    jacobianInflate(iPz, c_TanL) = momentum.cylindricalR();
  } else {
    jacobianInflate(iPx, c_Curv) = invChargeAlphaCurv2;
    jacobianInflate(iPy, c_Phi0) = - invChargeAlphaCurv;
    jacobianInflate(iPz, c_Curv) = tanLambda * invChargeAlphaCurv2;
    jacobianInflate(iPz, c_TanL) = - invChargeAlphaCurv;
  }
  // Transform
  TMatrixDSym cov6 = cov5; //copy
  cov6.Similarity(jacobianInflate);

  /// 2. Rotate to the right phi0
  TMatrixD jacobianRot(6, 6);
  jacobianRot.Zero();

  // Active rotation matrix by phi0:
  jacobianRot(iX, iX) = cosPhi0;
  jacobianRot(iX, iY) = -sinPhi0;
  jacobianRot(iY, iX) = sinPhi0;
  jacobianRot(iY, iY) = cosPhi0;
  jacobianRot(iZ, iZ) = 1.0;

  jacobianRot(iPx, iPx) = cosPhi0;
  jacobianRot(iPx, iPy) = -sinPhi0;
  jacobianRot(iPy, iPx) = sinPhi0;
  jacobianRot(iPy, iPy) = cosPhi0;
  jacobianRot(iPz, iPz) = 1.0;

  cov6.Similarity(jacobianRot);

  // 3. Forward the covariance matrix.
  gfTrackCand.setCovSeed(cov6);

  return true;
}




ESign CDCTrajectory3D::getChargeSign() const
{
  return ccwInfoToChargeSign(getLocalHelix().circleXY().orientation());
}

double CDCTrajectory3D::getAbsMom3D(const double bZ) const
{
  double tanLambda = getLocalHelix().tanLambda();

  double factor2DTo3D = hypot(1, tanLambda);

  double curvatureXY = getLocalHelix().curvatureXY();

  double absMom2D =  curvatureToAbsMom2D(curvatureXY, bZ);

  return factor2DTo3D * absMom2D;
}

double CDCTrajectory3D::getAbsMom3D() const
{
  Vector3D position = getSupport();

  double tanLambda = getLocalHelix().tanLambda();

  double factor2DTo3D = hypot(1, tanLambda);

  double curvatureXY = getLocalHelix().curvatureXY();

  double absMom2D =  curvatureToAbsMom2D(curvatureXY, position);

  return factor2DTo3D * absMom2D;
}
