#include <framework/gearbox/Unit.h>

#include <framework/logging/Logger.h>
#include <framework/utilities/TestHelpers.h>

#include <gtest/gtest.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  /** check manual conversion. */
  TEST(UnitTest, MultiplyDivide)
  {
    //set to 10mm in standard units
    double length = 10.0 * Unit::mm;
    //=1.0 cm
    EXPECT_DOUBLE_EQ(1.0, length);

    EXPECT_DOUBLE_EQ(10.0, length / Unit::mm);
    EXPECT_DOUBLE_EQ(10000.0, length / Unit::um);

    EXPECT_DOUBLE_EQ(1e9, 1 * Unit::s);
  }

  /** check conversions between different units. */
  TEST(UnitTest, ConvertValue)
  {
    //check standard units
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "cm"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "ns"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "rad"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "GeV"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "K"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "T"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "e"));
    EXPECT_DOUBLE_EQ(1.0, Unit::convertValue(1.0, "g/cm3"));

    //different orders of magnitude
    EXPECT_DOUBLE_EQ(1e2, Unit::convertValue(1.0, "m"));
    EXPECT_DOUBLE_EQ(1e5, Unit::convertValue(1.0, "km"));
    EXPECT_DOUBLE_EQ(1e-1, Unit::convertValue(1.0, "mm"));
    EXPECT_DOUBLE_EQ(1e-4, Unit::convertValue(1.0, "um"));
    EXPECT_DOUBLE_EQ(1e-7, Unit::convertValue(1.0, "nm"));

    //convert some actual values
    EXPECT_DOUBLE_EQ(5e3 * 1e2, Unit::convertValue(5e3, "m"));
    EXPECT_DOUBLE_EQ(5e-3 * 1e2, Unit::convertValue(5e-3, "m"));
    EXPECT_DOUBLE_EQ(M_PI, Unit::convertValue(180.0, "deg"));

    //test fall-back behaviour (return value unchanged, throw B2ERROR)
    EXPECT_B2ERROR(EXPECT_DOUBLE_EQ(5e3, Unit::convertValue(5e3, "nonexistingunit")));
  }

}  // namespace
