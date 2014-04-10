#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <tracking/dataobjects/SectorTFInfo.h>
// #include <TMatrixF.h>
// #include <RKTrackRep.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  /** command x should exit using B2FATAL. */
#define EXPECT_FATAL(x) EXPECT_EXIT(x,::testing::KilledBySignal(SIGABRT),"");

  /** Set up a few arrays and objects in the datastore */
  class SectorTFInfoTest : public ::testing::Test {
  protected:
  };




  /** Test simple Setters and Getters. */
  TEST_F(SectorTFInfoTest, testEmptyFilter)
  {
    SectorTFInfo aSector = SectorTFInfo();

    ASSERT_EQ(0., aSector.getSectorID());

  }

  TEST_F(SectorTFInfoTest, testPoints)
  {
    TVector3 testPoint(2, 1, 3);

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);
    aSector.setPoint(0, testPoint);

    ASSERT_EQ(12., aSector.getSectorID());
    ASSERT_EQ(testPoint, aSector.getPoint(0));

  }

  TEST_F(SectorTFInfoTest, testFriends)
  {
    std::vector<unsigned int> testFriends = {1, 2, 3};
    bool false_item = false;

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setAllFriends(testFriends);
    aSector.setIsOnlyFriend(false_item);

    ASSERT_EQ(1., aSector.getFriends().at(0));
    ASSERT_EQ(false_item, aSector.getIsOnlyFriend());
    ASSERT_EQ(3., aSector.sizeFriends());

  }



  TEST_F(SectorTFInfoTest, testOverlapped)
  {
    bool false_item = false;
    bool true_item = true;

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);

    aSector.setUseCounter(1);
    aSector.changeUseCounter(1);

    ASSERT_EQ(true_item, aSector.isOverlapped());

    aSector.changeUseCounter(-1);

    ASSERT_EQ(false_item, aSector.isOverlapped());
    ASSERT_EQ(false_item, aSector.isOverlapped());

    ASSERT_EQ(2., aSector.getMaxCounter());
    ASSERT_EQ(1., aSector.getUseCounter());

  }


  TEST_F(SectorTFInfoTest, testDisplay)
  {
    TVector3 testPoint(2, 1, 3);

    SectorTFInfo aSector = SectorTFInfo();
    aSector.setSectorID(12);
    aSector.setPoint(0, testPoint);

    ASSERT_EQ(12., aSector.getSectorID());
    ASSERT_EQ(testPoint, aSector.getPoint(0));
    ASSERT_EQ(testPoint, aSector.getCoordinates()[0]);
    ASSERT_EQ(testPoint, aSector.getCoordinates()[4]);
    ASSERT_EQ(5., aSector.getCoordinates().size());

    aSector.getDisplayAlternativeBox();
    aSector.getDisplayInformation();

  }



}
