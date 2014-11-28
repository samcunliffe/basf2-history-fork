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

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>

#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/RKTrackRep.h>

#include <gtest/gtest.h>


using namespace std;

namespace Belle2 {



  /** Set up a few arrays and objects in the datastore */
  class SpacePointTest : public ::testing::Test {

  public:
    /** this is a small helper function to create a sensorInfo to be used */
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
    SpacePoint testPoint = SpacePoint(&aCluster, &sensorInfoBase);

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
    EXPECT_FLOAT_EQ(globalError.X(), testPoint.getPositionError().X());
    EXPECT_FLOAT_EQ(globalError.Y(), testPoint.getPositionError().Y());
    EXPECT_FLOAT_EQ(globalError.Z(), testPoint.getPositionError().Z());
    // normalized coordinates, center of Plane should be at 0.5:
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalU());
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalV());

  }



  /** Test constructor for SVDClsuters
   * tests the constructor importing a SVDCluster and tests results by
   * using the getters of the spacePoint...
   */
  TEST_F(SpacePointTest, testConstructorSVD)
  {
    VxdID aVxdID = VxdID(3, 3, 3), anotherVxdID = VxdID(1, 1, 1);
    VXD::SensorInfoBase sensorInfoBase = createSensorInfo(aVxdID, 2.3, 4.2);
    VXD::SensorInfoBase anotherSensorInfoBase = createSensorInfo(anotherVxdID, 2.3, 4.2);

    // create new SVDClusters and fill it with Info getting a Hit which is not at the origin
    // SVDCluster (VxdID sensorID, bool isU, float position, float positionSigma, double clsTime, double clsTimeSigma, float seedCharge, float clsCharge, unsigned short clsSize)
    SVDCluster clusterU1 = SVDCluster(aVxdID, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    SVDCluster clusterV1 = SVDCluster(aVxdID, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1);
    SVDCluster clusterU2 = SVDCluster(aVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1);
    SVDCluster clusterU3 = SVDCluster(anotherVxdID, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1);


    // normal u+v = 2D Cluster (order of input irrelevant):
    std::vector<const SVDCluster*> good2D = { &clusterU1, &clusterV1 };
    SpacePoint testPoint2D = SpacePoint(good2D, &sensorInfoBase);

    // normal u-only = 1D Cluster, sensorInfoBase is normally not needed, since constructor can create it on its own, but here the geometry is not set up, therefore we have to pass the infoBase:
    std::vector<const SVDCluster*> good1D = { &clusterU3 };
    SpacePoint testPoint1D = SpacePoint(good1D, &anotherSensorInfoBase);


    // should throw, since no clusters given:
    std::vector<const SVDCluster*> badNoClusters;
//  SpacePoint blaBla = SpacePoint(badNoClusters, &sensorInfoBase);
    EXPECT_THROW(SpacePoint(badNoClusters, &sensorInfoBase), std::out_of_range);

    // should throw, since too many clusters (of same sensor) given:
    std::vector<const SVDCluster*> bad3Clusters = { &clusterU1, &clusterV1, &clusterU2 };
    EXPECT_THROW(SpacePoint(bad3Clusters, &sensorInfoBase), std::runtime_error);

    // should throw, since two clusters of same type (but on same sensor) given:
    std::vector<const SVDCluster*> badSameType = { &clusterU1, &clusterU2 };
    EXPECT_THROW(SpacePoint(badSameType, &sensorInfoBase), std::runtime_error);

    // should throw, since two clusters of different sensors given:
    std::vector<const SVDCluster*> badDifferentSensors = { &clusterV1, &clusterU3 };
    EXPECT_THROW(SpacePoint(badDifferentSensors, &sensorInfoBase), std::runtime_error);


    // check results for full 2D cluster-combi:
    TVector3 aPositionFor2D = sensorInfoBase.pointToGlobal(TVector3(clusterU1.getPosition(), clusterV1.getPosition(), 0));
    TVector3 globalizedVariancesFor2D = sensorInfoBase.vectorToGlobal(
                                          TVector3(
                                            clusterU1.getPositionSigma() * clusterU1.getPositionSigma(),
                                            clusterV1.getPositionSigma() * clusterV1.getPositionSigma(),
                                            0
                                          )
                                        );
    TVector3 globalErrorFor2D;
    for (int i = 0; i < 3; i++) { globalErrorFor2D[i] = sqrt(abs(globalizedVariancesFor2D[i])); }

    // vxdID:
    EXPECT_DOUBLE_EQ(aVxdID, testPoint2D.getVxdID());
    // global position:
    EXPECT_DOUBLE_EQ(aPositionFor2D.X(), testPoint2D.getPosition().X());
    EXPECT_DOUBLE_EQ(aPositionFor2D.Y(), testPoint2D.getPosition().Y());
    EXPECT_DOUBLE_EQ(aPositionFor2D.Z(), testPoint2D.getPosition().Z());
    //global error:
    EXPECT_FLOAT_EQ(globalErrorFor2D.X(), testPoint2D.getPositionError().X());
    EXPECT_FLOAT_EQ(globalErrorFor2D.Y(), testPoint2D.getPositionError().Y());
    EXPECT_FLOAT_EQ(globalErrorFor2D.Z(), testPoint2D.getPositionError().Z());
    //local normalized position:
    EXPECT_FLOAT_EQ(0.4, testPoint2D.getNormalizedLocalU());
    EXPECT_FLOAT_EQ(0.6, testPoint2D.getNormalizedLocalV());


    // check results for single-cluster-only-case:
    TVector3 aPositionFor1D = anotherSensorInfoBase.pointToGlobal(TVector3(clusterU3.getPosition(), 0, 0));
    TVector3 globalizedVariancesFor1D = anotherSensorInfoBase.vectorToGlobal(
                                          TVector3(
                                            clusterU3.getPositionSigma() * clusterU3.getPositionSigma(),
                                            anotherSensorInfoBase.getVSize() * anotherSensorInfoBase.getVSize() / 12.,
                                            0
                                          )
                                        );
    TVector3 globalErrorFor1D;
    for (int i = 0; i < 3; i++) { globalErrorFor1D[i] = sqrt(abs(globalizedVariancesFor1D[i])); }


    // vxdID:
    EXPECT_DOUBLE_EQ(anotherVxdID, testPoint1D.getVxdID());
    // global position:
    EXPECT_DOUBLE_EQ(aPositionFor1D.X(), testPoint1D.getPosition().X());
    EXPECT_DOUBLE_EQ(aPositionFor1D.Y(), testPoint1D.getPosition().Y());
    EXPECT_DOUBLE_EQ(aPositionFor1D.Z(), testPoint1D.getPosition().Z());
    //global error:
    EXPECT_FLOAT_EQ(globalErrorFor1D.X(), testPoint1D.getPositionError().X());
    EXPECT_FLOAT_EQ(globalErrorFor1D.Y(), testPoint1D.getPositionError().Y());
    EXPECT_FLOAT_EQ(globalErrorFor1D.Z(), testPoint1D.getPositionError().Z());
    //local normalized position:
    EXPECT_FLOAT_EQ(0.6, testPoint1D.getNormalizedLocalU());
    EXPECT_FLOAT_EQ(0.5, testPoint1D.getNormalizedLocalV()); // center of sensor since v-cluster was not given
    /*

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
    EXPECT_FLOAT_EQ(globalError.X(), testPoint.getPositionError().X());
    EXPECT_FLOAT_EQ(globalError.Y(), testPoint.getPositionError().Y());
    EXPECT_FLOAT_EQ(globalError.Z(), testPoint.getPositionError().Z());
    // normalized coordinates, center of Plane should be at 0.5:
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalU());
    EXPECT_DOUBLE_EQ(0.5, testPoint.getNormalizedLocalV());*/

  }



  /** test conversion to genfit-compatible output and relations WARNING NOT WORKING ATTENTION! reason: Mockup for testing relations of StoreArrays could not be created (not solved yet)*/
//   TEST_F(SpacePointTest, testGenfitCompatibility)
//   {
//  B2INFO("testGenfitCompatibility: prepare StoreArray-stuff - clusters")
//  // prepare StoreArray-stuff:
//
//  //Clusters
//  StoreArray<PXDCluster> pxdCL("pxdStuff", DataStore::c_Persistent);
//
//
//  StoreArray<SVDCluster> svdCL("svdStuff", DataStore::c_Persistent);
//
//
//
//  B2INFO("testGenfitCompatibility: prepare StoreArray-stuff - SpacePoints including relations")
//  // SpacePoints including relations
//  StoreArray<SpacePoint> sps("spacePoints", DataStore::c_Persistent);
//
//
//
// //   pxdCL.registerRelationTo(sps, DataStore::c_Persistent);
// //   svdCL.registerRelationTo(sps, DataStore::c_Persistent);
//
//
//  B2INFO("testGenfitCompatibility: unique StoreArrayPtr to carry the names of the Cluster-storeArrays")
//  // unique StoreArrayPtr to carry the names of the Cluster-storeArrays:
//  StoreObjPtr<SpacePointMetaInfo> spMI("", DataStore::c_Persistent);
//
//
//
//
//  DataStore::Instance().setInitializeActive(true);
//  pxdCL.registerInDataStore(pxdCL.getName(), DataStore::c_Persistent);
//  svdCL.registerInDataStore(svdCL.getName(), DataStore::c_Persistent);
//  sps.registerInDataStore(sps.getName(), DataStore::c_Persistent);
//  sps.registerRelationTo(pxdCL, DataStore::c_Persistent);
//  sps.registerRelationTo(svdCL, DataStore::c_Persistent);
//  spMI.registerInDataStore("", DataStore::c_Persistent);
//  DataStore::Instance().setInitializeActive(false);
//
//  EXPECT_TRUE(pxdCL.isValid());
//  EXPECT_TRUE(svdCL.isValid());
//  EXPECT_TRUE(sps.isValid());
//
//  spMI.create();
//  EXPECT_TRUE(spMI.isValid());
//
//
//  unsigned int indexOfpxdCL = spMI->addName(pxdCL.getName());
//  unsigned int indexOfsvdCL = spMI->addName(svdCL.getName());
//
//  B2INFO("testGenfitCompatibility: creating 1 PXD and 3 SVD Cluster ")
//  // creating 1 PXD and 3 SVD Cluster
//  VxdID aVxdIDL1 = VxdID(1, 1, 1);
//  VxdID aVxdIDL3 = VxdID(3, 3, 3);
//  VxdID aVxdIDL4 = VxdID(4, 4, 4);
//  VXD::SensorInfoBase sensorInfoBase1 = createSensorInfo(aVxdIDL1, 2.3, 4.2);
//  VXD::SensorInfoBase sensorInfoBase3 = createSensorInfo(aVxdIDL3, 5., 23.42);
//  VXD::SensorInfoBase sensorInfoBase4 = createSensorInfo(aVxdIDL3, 23.42, 42.23);
//
//
//  B2INFO("testGenfitCompatibility: create new PXDCluster, fill it in storeArray, link it with newly created spacePoint")
//  // create new PXDCluster, fill it in storeArray, link it with newly created spacePoint
//  PXDCluster aCluster = PXDCluster(aVxdIDL1, 0., 0., 0.1, 0.1, 0, 0, 1, 1, 1, 1, 1, 1);
//  unsigned int indexOfPxdCluster = pxdCL.getEntries(); // getting the index before adding, singe its minus one
//  B2INFO("indexOfPxdCluster: " << indexOfPxdCluster)
//  pxdCL.appendNew(aCluster);
//
//  SpacePoint testPoint = SpacePoint(&aCluster, indexOfPxdCluster, indexOfpxdCL, &sensorInfoBase1);
//  SpacePoint* newSP = sps.appendNew(testPoint);
//  newSP->addRelationTo(pxdCL[indexOfPxdCluster]);
//
//
//  B2INFO("testGenfitCompatibility: create SVDClusters, two of them are on the same sensor")
//  // create SVDClusters, two of them are on the same sensor
//  SVDCluster clusterU1 = SVDCluster(aVxdIDL3, true, -0.23, 0.1, 0.01, 0.001, 1, 1, 1);
//  unsigned int indexOfSvdClusterU1 = svdCL.getEntries();
//  B2INFO("indexOfSvdClusterU1: " << indexOfSvdClusterU1)
//  svdCL.appendNew(clusterU1);
//
//  SVDCluster clusterV1 = SVDCluster(aVxdIDL3, false, 0.42, 0.1, 0.01, 0.001, 1, 1, 1);
//  unsigned int indexOfSvdClusterV1 = svdCL.getEntries();
//  B2INFO("indexOfSvdClusterV1: " << indexOfSvdClusterV1)
//  svdCL.appendNew(clusterV1);
//
//  SpacePoint testPoint2D = SpacePoint({ {&clusterU1, indexOfSvdClusterU1}, {&clusterV1, indexOfSvdClusterV1} }, indexOfsvdCL, &sensorInfoBase3);
//  newSP = sps.appendNew(testPoint2D);
//  newSP->addRelationTo(svdCL[indexOfSvdClusterU1]);
//  newSP->addRelationTo(svdCL[indexOfSvdClusterV1]);
//
//
//  B2INFO("testGenfitCompatibility: create SVDCluster, which is alone on its sensor")
//  // create SVDCluster, which is alone on its sensor
//  SVDCluster clusterU2 = SVDCluster(aVxdIDL4, true, 0.23, 0.1, 0.01, 0.001, 1, 1, 1);
//  unsigned int indexOfSvdClusterU2 = svdCL.getEntries();
//  B2INFO("indexOfSvdClusterU2: " << indexOfSvdClusterU2)
//  svdCL.appendNew(clusterU2);
//  SpacePoint testPoint1D = SpacePoint({ {&clusterU2, indexOfSvdClusterU2} }, indexOfsvdCL, &sensorInfoBase4);
//  newSP = sps.appendNew(testPoint1D);
//  newSP->addRelationTo(svdCL[indexOfSvdClusterU2]);
//
//
//
//  /** Situation so far:
//   * now the mockup is fully prepared.
//   * it contains 2 storeArrays, carrying PXD- and SVDClusters and SpacePoints based on them.
//   * The spacePoints are related to them and have a metaInfo-StoreObjPtr telling them, where to retrieve the clusters again.
//   * This is now used to get genfit-compatible output and fill it in a genfit::Track
//   */
//
//  B2INFO("testGenfitCompatibility: preparing genfit track filled with arbitrary input")
//  // preparing genfit track filled with arbitrary input
//  genfit::AbsTrackRep* trackRep = new genfit::RKTrackRep(211);
// //   TVector3 posSeed =
// //   TVector3 posSeed =
//  genfit::Track track(trackRep, newSP->getPosition(), TVector3(23., 42., 5.));
//
//  B2INFO("testGenfitCompatibility: feed the track with spacePoints ported to genfit compatible stuff")
//  // feed the track with spacePoints ported to genfit compatible stuff:
//  // is rather complicated since the assignment operator is protected:
//  std::vector<genfit::AbsMeasurement*> hitOutput;
//  for (auto& aSP : sps) {
//    std::vector<genfit::PlanarMeasurement> tempMeasurements = aSP.getGenfitCompatible();
//    for (genfit::PlanarMeasurement& measurement : tempMeasurements) {
//    hitOutput.emplace_back(measurement.clone());
//    }
// //     hitOutput.insert(hitOutput.end(), temp.begin(), temp.end());
//  }
//
//  for (unsigned i = 0; i < hitOutput.size(); i++) {
//    track.insertMeasurement(hitOutput[i]);
//  }
//
//  EXPECT_EQ(track.getNumPoints(), unsigned(pxdCL.getEntries() + svdCL.getEntries()));
//
//  // garbage collection:
//  delete trackRep;
//  for (auto& hit : hitOutput) { delete hit; }
//   }



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
