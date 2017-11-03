
#include <framework/dataobjects/EventT0.h>
#include <cmath>
#include <gtest/gtest.h>

using namespace Belle2;

namespace {

  /** Testing the event T0 uncertainty calculation */
  TEST(EventT0, UncertaintyCalculation)
  {
    EventT0 t0;

    auto extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 0);
    ASSERT_EQ(extractedT0.second, 0);

    ASSERT_EQ(t0.getDetectors().size(), 0);

    // Add a first event t0
    t0.addEventT0(2, 1, Const::CDC);

    extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 2);
    ASSERT_EQ(extractedT0.second, 1);

    extractedT0 = t0.getEventT0WithUncertainty(Const::PXD);
    ASSERT_EQ(extractedT0.first, 0);
    ASSERT_EQ(extractedT0.second, 0);

    ASSERT_EQ(t0.getDetectors().size(), 1);

    // Add a first integer event t0 (should not change anything)
    t0.addEventT0(3, Const::SVD);

    extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 2);
    ASSERT_EQ(extractedT0.second, 1);

    extractedT0 = t0.getEventT0WithUncertainty(Const::PXD);
    ASSERT_EQ(extractedT0.first, 0);
    ASSERT_EQ(extractedT0.second, 0);

    ASSERT_EQ(t0.getDetectors().size(), 2);

    int binnedT0 = t0.getBinnedEventT0();
    ASSERT_EQ(binnedT0, 3);

    // Add a second event t0
    t0.addEventT0(1, 0.5, Const::PXD);

    extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(extractedT0.first, 1.2);
    ASSERT_EQ(extractedT0.second, 1 / std::sqrt(5));

    extractedT0 = t0.getEventT0WithUncertainty(Const::CDC);
    ASSERT_EQ(extractedT0.first, 2);
    ASSERT_EQ(extractedT0.second, 1);

    extractedT0 = t0.getEventT0WithUncertainty(Const::PXD);
    ASSERT_EQ(extractedT0.first, 1);
    ASSERT_EQ(extractedT0.second, 0.5);

    ASSERT_EQ(t0.getDetectors().size(), 3);

    ASSERT_TRUE(t0.hasEventT0(Const::PXD));
    ASSERT_TRUE(t0.hasEventT0(Const::CDC));
  }

  /** Testing to override and remove existing t0 measurements*/
  TEST(EventT0, AddingRemoving)
  {
    EventT0 t0;

    ASSERT_EQ(t0.getDetectors().size(), 0);

    // Add a first event t0
    t0.addEventT0(2, 1, Const::CDC);

    ASSERT_EQ(t0.getDetectors().size(), 1);

    // Add a new ECL info and overwrite the CDC information
    t0.addEventT0(10, 2, Const::ECL);
    t0.addEventT0(10, 2, Const::CDC);

    auto extractedT0 = t0.getEventT0WithUncertainty();
    ASSERT_EQ(t0.getDetectors().size(), 2);
    ASSERT_EQ(extractedT0.first, 10);

    t0.removeEventT0(Const::CDC);
    ASSERT_EQ(t0.getDetectors().size(), 1);
    ASSERT_FALSE(t0.hasEventT0(Const::CDC));
  }
}
