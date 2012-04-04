#include <vxd/dataobjects/VxdID.h>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

namespace Belle2 {

  /** Check that both constructors yield the same result */
  TEST(VxdID, Constructor)
  {
    VxdID s("1.1.1");
    VxdID l(1, 1, 1);
    EXPECT_EQ(l, s);
    EXPECT_EQ((string)l, "1.1.1");
  }

  /** Check the string conversion of the VxdID in both directions */
  TEST(VxdID, FromString)
  {
    //Check that all possible values can be converted back and forth
    for (int layer = 0; layer <= VxdID::MaxLayer; ++layer) {
      for (int ladder = 0; ladder <= VxdID::MaxLadder; ++ladder) {
        for (int sensor = 0; sensor <= VxdID::MaxSensor; ++sensor) {
          for (int segment = 0; segment <= VxdID::MaxSegment; ++segment) {
            string sid = (string)VxdID(layer, ladder, sensor, segment);
            VxdID id(layer, ladder, sensor, segment);
            EXPECT_EQ(id, VxdID(sid));
            EXPECT_EQ(id.getLayer(), layer);
            EXPECT_EQ(id.getLadder(), ladder);
            EXPECT_EQ(id.getSensor(), sensor);
            EXPECT_EQ(id.getSegment(), segment);
          }
        }
      }
    }

    //Check some invalid ids
    EXPECT_FALSE(VxdID("1.f"));
    EXPECT_FALSE(VxdID("1.1.f"));
    EXPECT_FALSE(VxdID("1.*.f"));
  }
}  // namespace
