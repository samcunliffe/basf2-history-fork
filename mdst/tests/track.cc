#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {



  /** Test class for the Track object.
   */
  class TrackTest : public ::testing::Test {
  protected:
  };

  /** Test simple Setters and Getters.
   */
  TEST_F(TrackTest, settersNGetters)
  {
    //Create some TrackFitResults in the the DataStore.
    //PDGCode of the TrackFitResult will be used in the test to identify the TFR.
    DataStore::Instance().setInitializeActive(true);
    StoreArray<TrackFitResult>::registerPersistent();
    StoreArray<TrackFitResult> myResults;

    const TVector3 dummyVector3;
    const TMatrixDSym dummyMatrix(6);
    const int charge = 1;
    const float pValue = 1.;
    const float bField = 1.5;

    myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::pion,     pValue, bField, 0, 0);

    myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::kaon,     pValue, bField, 0, 0);

    myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::electron, pValue, bField, 0, 0);

    myResults.appendNew(dummyVector3, dummyVector3, dummyMatrix, charge, Const::muon,     pValue, bField, 0, 0);

    Track mytrack1;
    mytrack1.setTrackFitResultIndex(Const::pion, 0);
    mytrack1.setTrackFitResultIndex(Const::muon, 3);
    mytrack1.setTrackFitResultIndex(Const::kaon, 2);
    EXPECT_EQ(mytrack1.getNumberOfFittedHypotheses(), 3);
    // If the index of the corresponding particel is set, the correct particle should be returned.
    EXPECT_EQ(mytrack1.getTrackFitResult(Const::pion)->getParticleType(), Const::pion);

    // make some test of the logic of return statements in case the primary hypothesis is not available.
    Track mytrack2;

  }
}  // namespace
