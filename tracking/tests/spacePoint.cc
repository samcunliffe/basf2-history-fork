/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/SensorPlane.h>
#include <gtest/gtest.h>


using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SpacePointTest : public ::testing::Test {

  public:
    VXD::SensorInfoBase createSensorInfo(VxdID aVxdID, double width = 1., double length = 1., double width2 = -1.) {
      // (SensorType type, VxdID id, double width, double length, double thickness, int uCells, int vCells, double width2=-1, double splitLength=-1, int vCells2=0)
      VXD::SensorInfoBase sensorInfoBase(VXD::SensorInfoBase::PXD, aVxdID, width, length, 0.3, 2, 4, width2);

      TGeoRotation r1;
      r1.SetAngles(45, 20, 30);      // rotation defined by Euler angles
      TGeoTranslation t1(-10, 10, 1);
      TGeoCombiTrans c1(t1, r1);
      TGeoHMatrix transform = c1;
      sensorInfoBase.setTransformation(transform);

      return sensorInfoBase;
    }
  protected:
  };

  /** Test constructor for PXDClsuters
   * tests the constructor importing a PXDCluster and tests results by
   * using the getters of the spacePoint...
   */
  TEST_F(SpacePointTest, testConstructorPXD)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);

    // create new PXDCluster and fill it with Info getting a Hit which is not at the origin (here, first layer)

    PXDCluster aCluster = PXDCluster(aVxdID, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
    SpacePoint testPoint = SpacePoint(aCluster, 3, &sensorInfoBase);
//     SpacePoint testPoint = SpacePoint(aCluster, 3);

    EXPECT_DOUBLE_EQ(aVxdID, testPoint.getVxdID());

    // needing globalized position and error:
    TVector3 aPosition = sensorInfoBase.pointToGlobal(TVector3(aCluster.getU(), aCluster.getV(), 0));
    TVector3 globalizedVariances = sensorInfoBase.vectorToGlobal(
                                     TVector3(
                                       aCluster.getUSigma() * aCluster.getUSigma(),
                                       aCluster.getVSigma() * aCluster.getVSigma(),
                                       0
                                     )
                                   );

    TVector3 globalError;
    for (int i = 0; i < 3; i++) { globalError[i] = sqrt(abs(globalizedVariances[i])); }

    EXPECT_DOUBLE_EQ(aPosition.X(), testPoint.getPosition().X());
    EXPECT_DOUBLE_EQ(aPosition.Y(), testPoint.getPosition().Y());
    EXPECT_DOUBLE_EQ(aPosition.Z(), testPoint.getPosition().Z());
    EXPECT_DOUBLE_EQ(globalError.X(), testPoint.getPositionError().X());
    EXPECT_DOUBLE_EQ(globalError.Y(), testPoint.getPositionError().Y());
    EXPECT_DOUBLE_EQ(globalError.Z(), testPoint.getPositionError().Z());
    // normalized coordinates, center of Plane should be at 0.5:
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalU());
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalV());

  }



  /** Testing member of spacePoint: convertToNormalizedCoordinates
   *
   * convertToNormalizedCoordinates converts a local hit into sensor-independent relative coordinates defined between 0 and 1.
   */
  TEST_F(SpacePointTest, testConvertLocalToNormalizedCoordinates)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);

    pair<double, double> sensorCenter = {1.15, 2.1};

    pair<double, double> hitLocal05 = {0, 0}; // sensorCenter is at 0, 0 in local coordinates
    pair<double, double> resultNormalized05 = {0.5, 0.5};

    pair<double, double> hitLocal001 = {0.023, 0.042};
    hitLocal001.first -= sensorCenter.first;
    hitLocal001.second -= sensorCenter.second;
    pair<double, double> resultNormalized001 = {0.01, 0.01};

    pair<double, double> hitLocal088 = {2.024, 3.696};
    hitLocal088.first -= sensorCenter.first;
    hitLocal088.second -= sensorCenter.second;
    pair<double, double> resultNormalized088 = {0.88, 0.88};

    pair<double, double> hitLocal001088 = {0.023, 3.696};// asymmetric example verifying that values are not accidentally switched
    hitLocal001088.first -= sensorCenter.first;
    hitLocal001088.second -= sensorCenter.second;
    pair<double, double> resultNormalized001088 = {0.01, 0.88};

    pair<double, double> hitLocalMinMax = { -1.16, 500}; // hit lies way beyond sensor edges (first is below lower threshold, second above higher one)
    pair<double, double> resultNormalizedMinMax = {0., 1.};

    pair<double, double> hitNormalized05 = SpacePoint::convertLocalToNormalizedCoordinates(hitLocal05, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultNormalized05.first, hitNormalized05.first);
    EXPECT_FLOAT_EQ(resultNormalized05.second, hitNormalized05.second);

    pair<double, double> hitNormalized001 = SpacePoint::convertLocalToNormalizedCoordinates(hitLocal001, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultNormalized001.first, hitNormalized001.first);
    EXPECT_FLOAT_EQ(resultNormalized001.second, hitNormalized001.second);

    pair<double, double> hitNormalized088 = SpacePoint::convertLocalToNormalizedCoordinates(hitLocal088, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultNormalized088.first, hitNormalized088.first);
    EXPECT_FLOAT_EQ(resultNormalized088.second, hitNormalized088.second);

    pair<double, double> hitNormalized001088 = SpacePoint::convertLocalToNormalizedCoordinates(hitLocal001088, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultNormalized001088.first, hitNormalized001088.first);
    EXPECT_FLOAT_EQ(resultNormalized001088.second, hitNormalized001088.second);

    pair<double, double> hitNormalizedMinMax = SpacePoint::convertLocalToNormalizedCoordinates(hitLocalMinMax, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultNormalizedMinMax.first, hitNormalizedMinMax.first);
    EXPECT_FLOAT_EQ(resultNormalizedMinMax.second, hitNormalizedMinMax.second);
  }



  /**  Testing member of spacePoint: convertToLocalCoordinates
   *
   * convertToLocalCoordinates converts a hit in sensor-independent relative (def. 0-1) coordinates into local coordinate of given sensor
   */
  TEST_F(SpacePointTest, testConvertNormalizedToLocalCoordinates)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);

    pair<double, double> sensorCenter = {1.15, 2.1};

    pair<double, double> hitNormalized05 = {0.5, 0.5};
    pair<double, double> resultLocal05 = {0, 0}; // sensorCenter is at 0, 0 in local coordinates

    pair<double, double> hitNormalized001 = {0.01, 0.01};
    pair<double, double> resultLocal001 = {0.023, 0.042};
    resultLocal001.first -= sensorCenter.first;
    resultLocal001.second -= sensorCenter.second;

    pair<double, double> hitNormalized088 = {0.88, 0.88};
    pair<double, double> resultLocal088 = {2.024, 3.696};
    resultLocal088.first -= sensorCenter.first;
    resultLocal088.second -= sensorCenter.second;

    pair<double, double> hitNormalized001088 = {0.01, 0.88};
    pair<double, double> resultLocal001088 = {0.023, 3.696};// asymmetric example verifying that values are not accidentally switched
    resultLocal001088.first -= sensorCenter.first;
    resultLocal001088.second -= sensorCenter.second;

    pair<double, double> hitNormalizedMinMax = { -0.1, 4.2}; // hit lies way beyond sensor edges (first is below lower threshold, second above higher one)
    pair<double, double> resultLocalMinMax = { -1.15, 2.1}; // reduced to sensor borders

    pair<double, double> hitLocal05 = SpacePoint::convertNormalizedToLocalCoordinates(hitNormalized05, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultLocal05.first, hitLocal05.first);
    EXPECT_FLOAT_EQ(resultLocal05.second, hitLocal05.second);

    pair<double, double> hitLocal001 = SpacePoint::convertNormalizedToLocalCoordinates(hitNormalized001, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultLocal001.first, hitLocal001.first);
    EXPECT_FLOAT_EQ(resultLocal001.second, hitLocal001.second);

    pair<double, double> hitLocal088 = SpacePoint::convertNormalizedToLocalCoordinates(hitNormalized088, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultLocal088.first, hitLocal088.first);
    EXPECT_FLOAT_EQ(resultLocal088.second, hitLocal088.second);

    pair<double, double> hitLocal001088 = SpacePoint::convertNormalizedToLocalCoordinates(hitNormalized001088, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultLocal001088.first, hitLocal001088.first);
    EXPECT_FLOAT_EQ(resultLocal001088.second, hitLocal001088.second);

    pair<double, double> hitLocalMinMax = SpacePoint::convertNormalizedToLocalCoordinates(hitNormalizedMinMax, aVxdID, &sensorInfoBase);
    EXPECT_FLOAT_EQ(resultLocalMinMax.first, hitLocalMinMax.first);
    EXPECT_FLOAT_EQ(resultLocalMinMax.second, hitLocalMinMax.second);
  }
//     static pair<double, double> convertToLocalCoordinates(const pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



  /**  Testing member of spacePoint: getGlobalCoordinates
   */
  TEST_F(SpacePointTest, testGetGlobalCoordinates)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);
  }
  /** converts a local hit on a given sensor into global coordinates.
   *
   * first parameter is the local hit stored as a pair of doubles.
   * second parameter is the coded vxdID, which carries the sensorID.
   * third parameter, a sensorInfo can be passed for testing purposes.
   *  If no sensorInfo is passed, the member gets its own pointer to it.
   */
//     static TVector3 getGlobalCoordinates(const pair<double, double>& hitLocal, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);



  /**  Testing member of spacePoint: convertToLocalCoordinatesNormalized
   */
  TEST_F(SpacePointTest, testConvertToLocalCoordinatesNormalized)
  {
    VxdID aVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);
  }
  /** converts a hit in sensor-independent relative coordinates into local coordinate of given sensor.
  *
  * first parameter is the hit in sensor-independent normalized ! coordinates stored as a pair of floats.
  * second parameter is the coded vxdID, which carries the sensorID.
  * third parameter, a sensorInfo can be passed for testing purposes.
  *  If no sensorInfo is passed, the member gets its own pointer to it.
  */
//     static pair<double, double> convertToLocalCoordinatesNormalized(const pair<double, double>& hitNormalized, VxdID::baseType vxdID, const VXD::SensorInfoBase* aSensorInfo = NULL);




}  // namespace
