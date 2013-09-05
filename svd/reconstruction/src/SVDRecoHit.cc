/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter, Moritz Nadler            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/SensorPlane.h>
#include <vxd/geometry/GeoCache.h>

#include <GFDetPlane.h>
#include <TVector3.h>
#include <TRandom.h>
#include <cmath>

using namespace std;
using namespace Belle2;



ClassImp(SVDRecoHit)


const double SVDRecoHit::c_HMatrixUContent[5] = {0, 0, 0, 1, 0};
const TMatrixD SVDRecoHit::c_HMatrixU = TMatrixD(HIT_DIMENSIONS, 5, c_HMatrixUContent);

const double SVDRecoHit::c_HMatrixVContent[5] = {0, 0, 0, 0, 1};
const TMatrixD SVDRecoHit::c_HMatrixV = TMatrixD(HIT_DIMENSIONS, 5, c_HMatrixVContent);

SVDRecoHit::SVDRecoHit():
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0),
  m_cluster(0), m_energyDep(0), m_rotationPhi(0)
{}

SVDRecoHit::SVDRecoHit(const SVDTrueHit* hit, bool uDirection, float sigma):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(hit),
  m_cluster(0), m_isU(uDirection), m_energyDep(0), m_rotationPhi(0)
{
  // Smear the coordinate when constructing from a TrueHit.
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");

  // Set the sensor UID
  m_sensorID = hit->getSensorID();

  //If no error is given, estimate the error to pitch/sqrt(12)
  if (sigma < 0) {
    const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
    sigma = (m_isU) ? geometry.getUPitch(hit->getV()) / sqrt(12) : geometry.getVPitch() / sqrt(12);
  }

  // Set positions
  fHitCoord(0) = (m_isU) ? gRandom->Gaus(hit->getU(), sigma) : gRandom->Gaus(hit->getV(), sigma);
  // Set the error covariance matrix
  fHitCov(0, 0) = sigma * sigma;
  // Set physical parameters
  m_energyDep = hit->getEnergyDep();
  // Setup geometry information
  setDetectorPlane();
}

SVDRecoHit::SVDRecoHit(const SVDCluster* hit):
  GFAbsPlanarHit(HIT_DIMENSIONS), m_sensorID(0), m_trueHit(0),
  m_cluster(hit), m_energyDep(0), m_rotationPhi(0)
{
  // Set the sensor UID
  m_sensorID = hit->getSensorID();
  m_isU = hit->isUCluster();

  // Determine if we have a wedge sensor.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));

  bool isWedgeU = m_isU && (geometry.getBackwardWidth() > geometry.getForwardWidth());

  // Set positions
  if (!isWedgeU)
    fHitCoord(0) = hit->getPosition();
  else {
    // For u coordinate in a wedge sensor, the position line is not u = const.
    // We have to rotate the coordinate system to achieve this.
    m_rotationPhi = atan2((geometry.getBackwardWidth() - geometry.getForwardWidth()) / geometry.getWidth(0) * hit->getPosition(), geometry.getLength());
    // Set the position in the rotated coordinate frame.
    fHitCoord(0) = hit->getPosition() * cos(m_rotationPhi);
  }
  // Set the error covariance matrix (this does not scale with position)
  fHitCov(0, 0) = hit->getPositionSigma() * hit->getPositionSigma();
  // Set physical parameters
  m_energyDep = hit->getCharge();
  // Setup geometry information
  setDetectorPlane();
}

void SVDRecoHit::setDetectorPlane()
{
  // Construct a finite detector plane and set it.
  const SVD::SensorInfo& geometry = dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(m_sensorID));
  bool isWedgeU = m_isU && (geometry.getBackwardWidth() > geometry.getForwardWidth());

  // Construct vectors o, u, v
  TVector3 uLocal(1, 0, 0);
  TVector3 vLocal(0, 1, 0);
  if (isWedgeU) {
    double cosPhi = cos(m_rotationPhi);
    double sinPhi = sin(m_rotationPhi);
    uLocal.SetXYZ(cosPhi, sinPhi, 0);
    vLocal.SetXYZ(-sinPhi, cosPhi, 0);
  }
  TVector3 origin  = geometry.pointToGlobal(TVector3(0, 0, 0));
  TVector3 uGlobal = geometry.vectorToGlobal(uLocal);
  TVector3 vGlobal = geometry.vectorToGlobal(vLocal);

  //Construct the detector plane
  VXD::SensorPlane* finitePlane = new VXD::SensorPlane(m_sensorID, 20.0, 20.0);
  if (isWedgeU) finitePlane->setRotation(m_rotationPhi);
  GFDetPlane detPlane(origin, uGlobal, vGlobal, finitePlane);
  setDetPlane(detPlane);
}

GFAbsRecoHit* SVDRecoHit::clone()
{
  return new SVDRecoHit(*this);
}

const TMatrixD& SVDRecoHit::getHMatrix(const GFAbsTrackRep*)
{
  if (m_isU == true) {
    return c_HMatrixU;
  } else {
    return c_HMatrixV;
  }
}
