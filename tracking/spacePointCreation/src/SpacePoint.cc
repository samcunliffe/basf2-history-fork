/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>

using namespace std;
using namespace Belle2;

ClassImp(SpacePoint)

SpacePoint::SpacePoint(const PXDCluster& pxdCluster, unsigned int indexNumber, const VXD::SensorInfoBase* aSensorInfo) :
  m_vxdID(pxdCluster.getSensorID())
{
  m_indexNumbers.push_back(indexNumber);

  // TODO missing, detector type import... (distinguishing between TeLescope and PXD Hits)

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  m_position = aSensorInfo->pointToGlobal(
                 TVector3(
                   pxdCluster.getU(),
                   pxdCluster.getV(),
                   0
                 )
               );

  //As only variances, but not the sigmas transform linearly,
  // we need to use some acrobatics
  // (and some more (abs) since we do not really transform a vector).
  TVector3 globalizedVariances = aSensorInfo->vectorToGlobal(
                                   TVector3(
                                     pxdCluster.getUSigma() * pxdCluster.getUSigma(),
                                     pxdCluster.getVSigma() * pxdCluster.getVSigma(),
                                     0
                                   )
                                 );
  for (int i = 0; i < 3; i++) {
    m_positionError[i] = sqrt(abs(globalizedVariances[i]));
  }

  m_normalizedLocal = convertToNormalizedCoordinates(make_pair(pxdCluster.getU(), pxdCluster.getV()), m_vxdID, aSensorInfo);
//   double halfSensorSizeU = 0.5 *  aSensorInfo->getUSize();
//   double halfSensorSizeV = 0.5 *  aSensorInfo->getVSize();
//   double localUPosition = pxdCluster.getU() + halfSensorSizeU;
//   double localVPosition = pxdCluster.getV() + halfSensorSizeV;
//   m_normalizedLocal[0] = localUPosition / aSensorInfo->getUSize();
//   m_normalizedLocal[1] = localVPosition / aSensorInfo->getVSize();
}



std::pair<double, double> SpacePoint::convertToNormalizedCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  //As the 0 is in the middle of sensor in the geometry, and we want
  // to normalize all positions to numbers between [0,1],
  // where the middle will be 0.5,
  // we need to do some calculation.


  double sensorSizeU =  aSensorInfo->getUSize(hitLocal.second); // this deals with the case of trapezoidal sensors too
  double sensorSizeV =  aSensorInfo->getVSize();

  double localUPosition = hitLocal.first +  0.5 * sensorSizeU;
  localUPosition /= sensorSizeU;
  boundaryCheck(localUPosition, 0, 1);
  double localVPosition = hitLocal.second +  0.5 * sensorSizeV;
  localVPosition /= sensorSizeV;
  boundaryCheck(localVPosition, 0, 1);

  // old ones:
  // double sensorSizeU =  aSensorInfo->getUSize(hitLocal.second);
  // double sensorSizeV = 0.5 * aSensorInfo->getVSize();

//   B2INFO("localUPosition: " << localUPosition);
//   B2INFO("localVPosition: " << localVPosition);


  return make_pair(localUPosition, localVPosition);
}



std::pair<double, double> SpacePoint::convertToLocalCoordinates(const std::pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  // Changed by Stefan F
  double localUPosition = hitNormalized.first - (0.5 * aSensorInfo->getUSize());
  double localVPosition = hitNormalized.second - (0.5 * aSensorInfo->getVSize());

  // old ones:
  //   double localVPosition = (hitNormalized.second - 0.5) * aSensorInfo->getVSize();
  //   double localUPosition = (hitNormalized.first - 0.5) * aSensorInfo->getUSize();

  return (make_pair(localUPosition, localVPosition));
}



TVector3 SpacePoint::getGlobalCoordinates(const std::pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  return aSensorInfo->pointToGlobal(
           TVector3(
             hitLocal.first,
             hitLocal.second,
             0
           )
         );
}
