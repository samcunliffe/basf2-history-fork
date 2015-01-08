/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

// tracking
// #include <tracking/spacePointCreation/SpacePointMetaInfo.h>
#include <tracking/vectorTools/B2Vector3.h>
// #include <TVector3.h>

// framework
#include <framework/datastore/RelationsObject.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/FrameworkExceptions.h>
#include <framework/logging/Logger.h>
// vxd
#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>
// pxd
#include <pxd/dataobjects/PXDCluster.h>
//svd
#include <svd/dataobjects/SVDCluster.h>
// genfit
#include <genfit/PlanarMeasurement.h>


// stl:
#include <vector>
#include <string>
#include <utility> // std::pair
#include <math.h>

namespace Belle2 {
  /** The SpacePoint class.
   *
   *  This class stores a global space point with its position error and some extra infos
   */
  class SpacePoint: public RelationsObject {
  public:

    /** setting the baseType for storing coordinate-related infos */
    typedef float SpBaseType;

    /** exception for the case that the user filled an invalid number of Clusters into the Constructor */
    BELLE2_DEFINE_EXCEPTION(InvalidNumberOfClusters, "SpacePoint::Constructor: invalid numbers of Clusters given!");

    /** exception for the case that the user filled an invalid combination of clusters into the constructor (e.g. they are not from the same sensor or both are u clusters) */
    BELLE2_DEFINE_EXCEPTION(IncompatibleClusters, "SpacePoint::Constructor: given combination of SVDCluster is not allowed!");

    /** exception for the case that the detectorType is not supported by the SpacePoint */
    BELLE2_DEFINE_EXCEPTION(InvalidDetectorType, ("SpacePoint: the detector type given is not supported!"));



    /** Default constructor for the ROOT IO. */
    SpacePoint() :
      m_qualityIndicator(0.5),
      m_isAssigned(false) {}



    /** Constructor for the case of PXD Hits.
    *
    * For the case of TelHits, there will be a SpacePoint-Inheriting Class adding a TelCluster-feature
     *
     * first parameter is pointer to cluster (passing a null-pointer will throw an exception)
     * second, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the constructor gets its own pointer to it.
     */
    SpacePoint(const Belle2::PXDCluster* pxdCluster,
               const Belle2::VXD::SensorInfoBase* aSensorInfo = NULL);



    /** Constructor for the case of SVD Hits.
    *
    * 1-2 clusters can be added this way
     *
     * first parameter is a container carrying pointers to the svdClusters,
    *   and .second provides its indexNumber for the StoreArray.
     * It should _not_ be filled with NULL-Pointers (passing a null-pointer will throw an exception).
     * 1 - 2 Clusters are allowed that way, if there are passed more than that or less, an exception will be thrown.
    * second, a sensorInfo can be passed for testing purposes.
     * If no sensorInfo is passed, the constructor gets its own pointer to it.
     *
     */
    SpacePoint(std::vector<const Belle2::SVDCluster*>& clusters,
               const Belle2::VXD::SensorInfoBase* aSensorInfo = NULL);



    /** virtual destructor to prevent undefined behavior for inherited classes */
    virtual ~SpacePoint() {}



// getter:

    /** return the x-value of the global position of the SpacePoint */
    SpBaseType X() const { return m_position.X(); }



    /** return the x-value of the global position of the SpacePoint */
    SpBaseType x() const { return m_position.X(); }



    /** return the y-value of the global position of the SpacePoint */
    SpBaseType Y() const { return m_position.Y(); }



    /** return the y-value of the global position of the SpacePoint */
    SpBaseType y() const { return m_position.Y(); }



    /** return the z-value of the global position of the SpacePoint */
    SpBaseType Z() const { return m_position.Z(); }



    /** return the z-value of the global position of the SpacePoint */
    SpBaseType z() const { return m_position.Z(); }



    /** return the position vector in global coordinates */
    const B2Vector3F& getPosition() const { return m_position; }



    /** return the hitErrors in sigma of the global position */
    const B2Vector3F& getPositionError() const { return m_positionError; }



    /** return the VxdID of the sensor inhabiting the Cluster of the SpacePoint */
    VxdID::baseType getVxdID() const { return m_vxdID; }



    /** return the normalized local coordinates of the cluster in u (0 <= posU <= 1) */
    SpBaseType getNormalizedLocalU() const { return m_normalizedLocal.first/*m_normalizedLocal[0]*/; }



    /** return the normalized local coordinates of the cluster in v (0 <= posV <= 1) */
    SpBaseType getNormalizedLocalV() const { return m_normalizedLocal.second/*m_normalizedLocal[1]*/; }



    /** return the sensorType of the current spacePoint.
    *
    * The return type is equivalent of the type given by SensorInfoBase
    */
    Belle2::VXD::SensorInfoBase::SensorType getType() const { return m_sensorType; }



    /** returns a vector of genfit::PlanarMeasurement, which is needed for genfit::track.
    *
    * This member ensures compatibility with genfit2.
    * The return type is detector independent,
    * but each entry will be of the same detector type,
    * since a spacePoint can not contain clusters of different sensors
    * and therefore of different detector types.
    */
    virtual std::vector<genfit::PlanarMeasurement> getGenfitCompatible() const ;



    /** returns the current state of assignment - returns true if it is assigned and therefore blocked for reuse. */
    bool getAssignmentState() const {return m_isAssigned; }



    /** returns the current estimation for the quality of that spacePoint.
     *
     * returns value between 0-1, 1 means "good", 0 means "bad".
     * */
    unsigned int getQualityEstimation() const {return m_qualityIndicator; }



// static converter functions:

    /** converts a local hit into sensor-independent relative coordinates.
     *
     * first parameter is the local hit (as provided by getU and getV!) stored as a pair of SpBaseTypes.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
     */
    static std::pair<SpBaseType, SpBaseType> convertLocalToNormalizedCoordinates(const std::pair<SpBaseType, SpBaseType>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** converts a local hit on a given sensor into global coordinates.
     *
     * so this practically does what sensorInfo::pointToGlobal is doing, the difference is, that you do not need to have the sensorInfo beforehand (it will be retrieved using the VxdID)
     * first parameter is the local hit (as provided by getU and getV!) stored as a pair of SpBaseTypes.
     * second parameter is the coded vxdID, which carries the sensorID.
     * third parameter, a sensorInfo can be passed for testing purposes.
     *  If no sensorInfo is passed, the member gets its own pointer to it.
     */
    static B2Vector3F getGlobalCoordinates(const std::pair<SpBaseType, SpBaseType>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



    /** converts a hit in sensor-independent relative coordinates into local coordinate of given sensor.
    *
    * first parameter is the hit in sensor-independent normalized ! coordinates stored as a pair of floats.
    * second parameter is the coded vxdID, which carries the sensorID.
    * third parameter, a sensorInfo can be passed for testing purposes.
    *  If no sensorInfo is passed, the member gets its own pointer to it.
    */
    static std::pair<SpBaseType, SpBaseType> convertNormalizedToLocalCoordinates(const std::pair<SpBaseType, SpBaseType>& hitNormalized, Belle2::VxdID::baseType vxdID, const Belle2::VXD::SensorInfoBase* aSensorInfo = NULL);



    /** checks first parameter for boundaries.
     *
     * does take second/third argument for checking for lower/upper boundary.
     * if boundary is crossed, value gets reset to boundary value
     * */
    static void boundaryCheck(SpBaseType& value, SpBaseType lower = 0, SpBaseType higher = 1) {
      if (value < lower) { value = lower; }
      if (value > higher) { value = higher; }
    }



// setter:

    /** sets the state of assignment - set true if it is assigned and therefore blocked for reuse. */
    void setAssignmentState(bool newState) { m_isAssigned = newState; }



    /** sets the estimation for the quality of that spacePoint.
     *
     * set value between 0-1, 1 means "good", 0 means "bad".
     * */
    void setQualityEstimation(unsigned int newQI) {m_qualityIndicator = newQI; }


  protected:


    /** protected function to set the global position error.
     *
     * It takes care for the transformation of the local sigmas to global error values.
     */
    void setPositionError(SpBaseType uSigma, SpBaseType vSigma, const VXD::SensorInfoBase* aSensorInfo) {
      //As only variances, but not the sigmas transform linearly,
      // we need to use some acrobatics
      // (and some more (abs) since we do not really transform a vector).
      m_positionError = aSensorInfo->vectorToGlobal(
                          TVector3(
                            uSigma * uSigma,
                            vSigma * vSigma,
                            0
                          )
                        );
      m_positionError.Sqrt();
    }



    /** Global position vector.
     *
     *  [0]: x , [1] : y, [2] : z
     */
    B2Vector3<SpBaseType> m_position;



    /** Error "Vector" of global position in sigma.
     *
     *  [0]: x-uncertainty , [1] : y-uncertainty, [2] : z-uncertainty
     */
    B2Vector3<SpBaseType> m_positionError;



    /** Position in local coordinates normalized to the sensor size between 0 and 1.
     *
     *  First entry is u, second is v
     */
    std::pair<SpBaseType, SpBaseType> m_normalizedLocal;
//     SpBaseType m_normalizedLocal[2];



    /** stores the vxdID */
    VxdID::baseType m_vxdID;



    /** stores the SensorType using the scheme of sensorInfoBase.
     *
     * Currently there are the following types possible:
     * PXD, SVD, TEL, VXD
     */
    VXD::SensorInfoBase::SensorType m_sensorType;



    /** stores a quality indicator.
     *
     * The value shall be between 0-1, where 1 means "good" and 0 means "bad".
     * Standard is 0.5.
     * */
    SpBaseType m_qualityIndicator;



    /** stores whether this spacePoint has already been assigned or not */
    bool m_isAssigned;



    ClassDef(SpacePoint, 8) // last member changed: m_position, m_positionError, m_qualityIndicator
  };
}
