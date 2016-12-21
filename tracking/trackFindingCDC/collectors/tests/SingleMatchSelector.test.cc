/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/collectors/selectors/SingleMatchSelector.h>

#include <gtest/gtest.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /// Test for the SingleMatchSelector
  TEST(TrackFindingCDCTest, single_match_selector)
  {
    SingleMatchSelector<int, double> selector;

    int a = 1, b = 2;
    double c = 3, d = 4, e = 5;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &c),
      WeightedRelation<int, const double>(&a, 1.0, &d),
      WeightedRelation<int, const double>(&b, 0.8, &c),
      WeightedRelation<int, const double>(&b, 0.6, &e)
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 2);
    ASSERT_EQ(relations[0].getWeight(), 1.0);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &d);
    ASSERT_EQ(relations[1].getWeight(), 0.6);
    ASSERT_EQ(relations[1].getFrom(), &b);
    ASSERT_EQ(relations[1].getTo(), &e);

  }

  /// Test for the SingleMatchSelector with use single best set to false
  TEST(TrackFindingCDCTest, single_best_match_selector)
  {
    SingleMatchSelector<int, double> selector;
    selector.setUseOnlySingleBestCandidate(false);

    int a = 1, b = 2, c = 3;
    double d = 4, e = 5, f = 6;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &d),
      WeightedRelation<int, const double>(&a, 1.0, &e),
      WeightedRelation<int, const double>(&b, 0.8, &d),
      WeightedRelation<int, const double>(&b, 0.6, &f),
      WeightedRelation<int, const double>(&c, 0.6, &d),
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 3);
    ASSERT_EQ(relations[0].getWeight(), 1.1);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &d);
    ASSERT_EQ(relations[1].getWeight(), 1.0);
    ASSERT_EQ(relations[1].getFrom(), &a);
    ASSERT_EQ(relations[1].getTo(), &e);
    ASSERT_EQ(relations[2].getWeight(), 0.6);
    ASSERT_EQ(relations[2].getFrom(), &b);
    ASSERT_EQ(relations[2].getTo(), &f);
  }

  /// Custom comperator for the test
  struct CustomComparer {
    bool operator()(const double* lhs, const double* rhs) const
    {
      return (static_cast<int>(*lhs) % 2) < (static_cast<int>(*rhs) % 2);
    }
  };

  /// Test for the SingleMatchSelector using a custom comperator
  TEST(TrackFindingCDCTest, single_match_selector_with_comparer)
  {
    SingleMatchSelector<int, double, CustomComparer> selector;

    int a = 1, b = 2;
    double c = 3, d = 4, e = 5;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &c),
      WeightedRelation<int, const double>(&a, 1.0, &d),
      WeightedRelation<int, const double>(&b, 0.8, &c),
      WeightedRelation<int, const double>(&b, 0.6, &e)
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 1);
    ASSERT_EQ(relations[0].getWeight(), 1.0);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &d);
  }

  class TestSingleBestMatchSelector : public SingleMatchSelector<int, double> {
    void resetNonBestMatches(const WeightedRelationItem& notBestMatch) override
    {
      m_dismissedRelations.push_back(notBestMatch);
    }

  public:
    std::vector<WeightedRelationItem> m_dismissedRelations;
  };

  TEST(TrackFindingCDCTest, single_match_selector_with_reset_function)
  {
    TestSingleBestMatchSelector selector;

    int a = 1, b = 2, c = 3;
    double d = 4, e = 5, f = 6;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &d),
      WeightedRelation<int, const double>(&a, 1.0, &e),
      WeightedRelation<int, const double>(&b, 0.8, &d),
      WeightedRelation<int, const double>(&b, 0.6, &f),
      WeightedRelation<int, const double>(&c, 0.6, &d),
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 2);
    ASSERT_EQ(relations[0].getWeight(), 1.0);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &e);
    ASSERT_EQ(relations[1].getWeight(), 0.6);
    ASSERT_EQ(relations[1].getFrom(), &b);
    ASSERT_EQ(relations[1].getTo(), &f);

    std::sort(selector.m_dismissedRelations.begin(), selector.m_dismissedRelations.end(), GreaterWeight());

    ASSERT_EQ(selector.m_dismissedRelations.size(), 3);
    ASSERT_EQ(selector.m_dismissedRelations[0].getWeight(), 1.1);
    ASSERT_EQ(selector.m_dismissedRelations[0].getFrom(), &a);
    ASSERT_EQ(selector.m_dismissedRelations[0].getTo(), &d);
    ASSERT_EQ(selector.m_dismissedRelations[1].getWeight(), 0.8);
    ASSERT_EQ(selector.m_dismissedRelations[1].getFrom(), &b);
    ASSERT_EQ(selector.m_dismissedRelations[1].getTo(), &d);
    ASSERT_EQ(selector.m_dismissedRelations[2].getWeight(), 0.6);
    ASSERT_EQ(selector.m_dismissedRelations[2].getFrom(), &c);
    ASSERT_EQ(selector.m_dismissedRelations[2].getTo(), &d);
  }

  TEST(TrackFindingCDCTest, single_best_match_selector_with_reset_function)
  {
    TestSingleBestMatchSelector selector;
    selector.setUseOnlySingleBestCandidate(false);

    int a = 1, b = 2, c = 3;
    double d = 4, e = 5, f = 6;

    std::vector<WeightedRelation<int, const double>> relations = {
      WeightedRelation<int, const double>(&a, 1.1, &d),
      WeightedRelation<int, const double>(&a, 1.0, &e),
      WeightedRelation<int, const double>(&b, 0.8, &d),
      WeightedRelation<int, const double>(&b, 0.6, &f),
      WeightedRelation<int, const double>(&c, 0.6, &d),
    };

    std::sort(relations.begin(), relations.end());
    selector.apply(relations);

    std::sort(relations.begin(), relations.end(), GreaterWeight());

    ASSERT_EQ(relations.size(), 3);
    ASSERT_EQ(relations[0].getWeight(), 1.1);
    ASSERT_EQ(relations[0].getFrom(), &a);
    ASSERT_EQ(relations[0].getTo(), &d);
    ASSERT_EQ(relations[1].getWeight(), 1.0);
    ASSERT_EQ(relations[1].getFrom(), &a);
    ASSERT_EQ(relations[1].getTo(), &e);
    ASSERT_EQ(relations[2].getWeight(), 0.6);
    ASSERT_EQ(relations[2].getFrom(), &b);
    ASSERT_EQ(relations[2].getTo(), &f);

    std::sort(selector.m_dismissedRelations.begin(), selector.m_dismissedRelations.end(), GreaterWeight());

    ASSERT_EQ(selector.m_dismissedRelations.size(), 2);
    ASSERT_EQ(selector.m_dismissedRelations[0].getWeight(), 0.8);
    ASSERT_EQ(selector.m_dismissedRelations[0].getFrom(), &b);
    ASSERT_EQ(selector.m_dismissedRelations[0].getTo(), &d);
    ASSERT_EQ(selector.m_dismissedRelations[1].getWeight(), 0.6);
    ASSERT_EQ(selector.m_dismissedRelations[1].getFrom(), &c);
    ASSERT_EQ(selector.m_dismissedRelations[1].getTo(), &d);
  }
}
