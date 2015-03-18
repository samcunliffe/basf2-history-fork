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
#include <framework/datastore/StoreObjPtr.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

using namespace std;
using namespace Belle2;

ClassImp(SpacePoint)

SpacePoint::SpacePoint(const Belle2::PXDCluster* pxdCluster,
                       const Belle2::VXD::SensorInfoBase* aSensorInfo) :
  m_clustersAssigned( {true, true}),
                    m_vxdID(pxdCluster->getSensorID()),
                    m_qualityIndicator(0.5),
                    m_isAssigned(false)
{
  if (pxdCluster == NULL) { throw InvalidNumberOfClusters(); }

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  m_position = aSensorInfo->pointToGlobal(
                 TVector3(
                   pxdCluster->getU(),
                   pxdCluster->getV(),
                   0
                 )
               );

  setPositionError(pxdCluster->getUSigma(), pxdCluster->getVSigma(), aSensorInfo);

  m_normalizedLocal = convertLocalToNormalizedCoordinates({ pxdCluster->getU(), pxdCluster->getV() } , m_vxdID, aSensorInfo);

  m_sensorType = aSensorInfo->getType();
}



SpacePoint::SpacePoint(std::vector<const Belle2::SVDCluster*>& clusters,
                       const Belle2::VXD::SensorInfoBase* aSensorInfo) :
  m_clustersAssigned( {false, false}),
m_vxdID(clusters.at(0)->getSensorID()),
m_qualityIndicator(0.5),
m_isAssigned(false)
{
  unsigned int nClusters = clusters.size();
  SpacePoint::SpBaseType uCoord = 0; // 0 = center of Sensor
  SpacePoint::SpBaseType vCoord = 0; // 0 = center of Sensor
  SpacePoint::SpBaseType uSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster
  SpacePoint::SpBaseType vSigma = -1; // negative sigmas are not possible, setting to -1 for catching cases of missing Cluster

  // do checks for sanity of input:
  if (nClusters == 0 or nClusters > 2) {
    throw InvalidNumberOfClusters();
  } else {
    vector<VxdID::baseType> vxdIDs;
    vector<bool> isUType;
    for (const SVDCluster* aCluster : clusters) {
      if (aCluster == NULL) throw InvalidNumberOfClusters();
      vxdIDs.push_back(aCluster->getSensorID());
      isUType.push_back(aCluster->isUCluster());
    }

    auto newEndVxdID = std::unique(vxdIDs.begin(), vxdIDs.end());
    vxdIDs.resize(std::distance(vxdIDs.begin(), newEndVxdID));

    auto newEndUType = std::unique(isUType.begin(), isUType.end());
    isUType.resize(std::distance(isUType.begin(), newEndUType));

    if (vxdIDs.size() != 1 or isUType.size() != nClusters) throw IncompatibleClusters();
  }

  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(m_vxdID);
  }

  // retrieve position and sigma-values
  for (const SVDCluster* aCluster : clusters) {
    if (aCluster->isUCluster() == true) {
      m_clustersAssigned.first = true;
      uCoord = aCluster->getPosition();
      uSigma = aCluster->getPositionSigma();
    } else {
      m_clustersAssigned.second = true;
      vCoord = aCluster->getPosition();
      vSigma = aCluster->getPositionSigma();
    }
  }

  if ((aSensorInfo->getBackwardWidth() > aSensorInfo->getForwardWidth()) == true) { // isWedgeSensor
    SpBaseType uWedged = getUWedged({ uCoord, vCoord } , m_vxdID, aSensorInfo);
    m_position = aSensorInfo->pointToGlobal(
                   TVector3(
                     uWedged,
                     vCoord,
                     0
                   )
                 );
    m_normalizedLocal = convertLocalToNormalizedCoordinates({ uWedged, vCoord } , m_vxdID, aSensorInfo);
  } else {
    m_position = aSensorInfo->pointToGlobal(
                   TVector3(
                     uCoord,
                     vCoord,
                     0
                   )
                 );
    m_normalizedLocal = convertLocalToNormalizedCoordinates({ uCoord, vCoord } , m_vxdID, aSensorInfo);
  }


  // if sigma for a coordinate is not known, a uniform distribution over the whole sensor is asumed:
  if (uSigma < 0) { uSigma = aSensorInfo->getUSize(vCoord) / sqrt(12.); }
  if (vSigma < 0) { vSigma = aSensorInfo->getVSize() / sqrt(12.); }

  setPositionError(uSigma, vSigma, aSensorInfo);


  m_sensorType = aSensorInfo->getType();
}



vector< genfit::PlanarMeasurement > SpacePoint::getGenfitCompatible() const
{
  // XYRecoHit will be stored as their base-class, which is detector-independent.
  vector< genfit::PlanarMeasurement > collectedMeasurements;


  // get the related clusters to this spacePoint and create a genfit::PlanarMeasurement for each of them:
  if (getType() == VXD::SensorInfoBase::SensorType::SVD) {

    auto relatedClusters = this->getRelationsTo<SVDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(SVDRecoHit(relatedClusters[i]));
    }

  } else if (getType() == VXD::SensorInfoBase::SensorType::PXD) {

    // since we do not know the name of the attached PXDCluster, getRelatedTo does not work, however, getRelationsTo seems to be less sensible and therefore can be used, but in this case, one has to loop over the entries (which should be only one in this case)
    auto relatedClusters = this->getRelationsTo<PXDCluster>("ALL");
    for (unsigned i = 0; i < relatedClusters.size(); i++) {
      collectedMeasurements.push_back(PXDRecoHit(relatedClusters[i]));
    }

  } else {
    throw InvalidDetectorType();
  }

  B2DEBUG(50, "SpacePoint::getGenfitCompatible(): collected " << collectedMeasurements.size() << " meaturements")

  return move(collectedMeasurements);
}



std::pair<SpacePoint::SpBaseType, SpacePoint::SpBaseType> SpacePoint::convertLocalToNormalizedCoordinates(
  const std::pair<SpacePoint::SpBaseType, SpacePoint::SpBaseType>& hitLocal, Belle2::VxdID::baseType vxdID,
  const Belle2::VXD::SensorInfoBase* aSensorInfo)
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
  SpacePoint::SpBaseType sensorSizeU =  aSensorInfo->getUSize(hitLocal.second); // this deals with the case of trapezoidal sensors too
  SpacePoint::SpBaseType sensorSizeV =  aSensorInfo->getVSize();

  SpacePoint::SpBaseType normalizedUPosition = (hitLocal.first +  0.5 * sensorSizeU) /
                                               sensorSizeU; // indepedent of the trapezoidal sensor-issue by definition
  boundaryCheck(normalizedUPosition, 0, 1);

  SpacePoint::SpBaseType normalizedVPosition = (hitLocal.second +  0.5 * sensorSizeV) / sensorSizeV;
  boundaryCheck(normalizedVPosition, 0, 1);

  return { normalizedUPosition, normalizedVPosition };
}



std::pair<SpacePoint::SpBaseType, SpacePoint::SpBaseType> SpacePoint::convertNormalizedToLocalCoordinates(
  const std::pair<SpacePoint::SpBaseType, SpacePoint::SpBaseType>& hitNormalized, Belle2::VxdID::baseType vxdID,
  const Belle2::VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  // normalized range is 0 to 1, but final coordinates are from - halfSensorSize to + halfSensorSize
  SpacePoint::SpBaseType localVPosition = (hitNormalized.second - 0.5) * aSensorInfo->getVSize();
  boundaryCheck(localVPosition, -0.5 * aSensorInfo->getVSize(), 0.5 * aSensorInfo->getVSize()); // restrain hits to sensor boundaries

  SpacePoint::SpBaseType uSizeAtHit = aSensorInfo->getUSize(localVPosition);
  SpacePoint::SpBaseType localUPosition = (hitNormalized.first - 0.5) * uSizeAtHit;
  boundaryCheck(localUPosition, -0.5 * uSizeAtHit, 0.5 * uSizeAtHit); // restrain hits to sensor boundaries

  return { localUPosition, localVPosition };
}




B2Vector3<SpacePoint::SpBaseType> SpacePoint::getGlobalCoordinates(const std::pair<SpacePoint::SpBaseType, SpacePoint::SpBaseType>&
    hitLocal, Belle2::VxdID::baseType vxdID, const Belle2::VXD::SensorInfoBase* aSensorInfo)
{
  //We need some handle to translate IDs to local and global
  // coordinates.
  if (aSensorInfo == NULL) {
    aSensorInfo = &VXD::GeoCache::getInstance().getSensorInfo(vxdID);
  }

  B2Vector3<SpacePoint::SpBaseType> globalCoords = aSensorInfo->pointToGlobal(
                                                     TVector3(
                                                       hitLocal.first,
                                                       hitLocal.second,
                                                       0
                                                     )
                                                   );
  return std::move(globalCoords);
//   return aSensorInfo->pointToGlobal(
//                 TVector3(
//                   hitLocal.first,
//                   hitLocal.second,
//                   0
//                 )
//               )
//              ;
}



