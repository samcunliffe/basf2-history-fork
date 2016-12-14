/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/matchers/MatcherInterface.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  class SimpleMatcher : public MatcherInterface<int, double> {
    Weight match(int& a, const double& b) override
    {
      return a + b;
    }
  };

  TEST(TrackFindingCDCTest, matcher_interface)
  {
    SimpleMatcher matcher;

    std::vector<WeightedRelation<int, const double>> relations;

    std::vector<int> collectorItems = {1, 2};
    std::vector<double> collectionItems = {2.1, 2.2, 2.3};

    matcher.apply(collectorItems, collectionItems, relations);

    ASSERT_EQ(relations.size(), 6);
    ASSERT_EQ(relations[5].getWeight(), 3.1);
    ASSERT_EQ(relations[4].getWeight(), 3.2);
    ASSERT_EQ(relations[3].getWeight(), 3.3);
    ASSERT_EQ(relations[2].getWeight(), 4.1);
    ASSERT_EQ(relations[1].getWeight(), 4.2);
    ASSERT_EQ(relations[0].getWeight(), 4.3);
  }
}
