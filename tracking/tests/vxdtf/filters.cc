/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Eugenio Paoloni (eugenio.paoloni@pi.infn.it              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <gtest/gtest.h>


#include "tracking/trackFindingVXD/FilterTools/Shortcuts.h"

#include <tuple>
#include <iostream>
#include <math.h>

using namespace std;

using namespace Belle2;

namespace VXDTFfilterTest {

  /** just a small proto-container storing coordinates */
  typedef tuple<float, float, float> spacePoint;


  /** a small filter illustrating the behavior of a distance3D-filter */
  class SquaredDistance3D : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2) +
        pow(get<1>(p1) - get<1>(p2) , 2) +
        pow(get<2>(p1) - get<2>(p2) , 2) ;
    }
  };


  /** a small filter illustrating the behavior of a distance2D-filter in XY */
  class SquaredDistance2Dxy : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2) +
        pow(get<1>(p1) - get<1>(p2) , 2) ;
    }
  };


  /** a small filter illustrating the behavior of a distance1D-filter in X */
  class SquaredDistance1Dx : public SelectionVariable< spacePoint , float > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        pow(get<0>(p1) - get<0>(p2) , 2);
    }
  };


  /** a small filter illustrating the behavior of a filter which is compatible with boolean comparisons */
  class BooleanVariable : public SelectionVariable< spacePoint , bool > {
  public:
    static float value(const spacePoint& p1, const spacePoint& p2) {
      return
        get<0>(p1) - get<0>(p2) == 0.;
    }
  };


  template < class T>
  class counter {
  public:
    static int N;
    counter() {};
    ~counter() {};
  };


  template<>
  int counter< SquaredDistance3D   >::N(0);


  template<>
  int counter< SquaredDistance2Dxy >::N(0);


  template<>
  int counter< SquaredDistance1Dx  >::N(0);


  /** this observer does simply count the number of times, the attached Filter was used */
  class Observer : public VoidObserver {
  public:
    template<class Var>
    static void notify(const typename Var::argumentType& ,
                       const typename Var::argumentType& ,
                       const Var& ,
                       typename Var::variableType) {
      counter<Var>::N ++ ;
    }

  };



  /** Test class for Filter object. */
  class FilterTest : public ::testing::Test {
  protected:
  };


  /** shows the functionality of the Range */
  TEST_F(FilterTest, Range)
  {

    Range<double, double> range(0. , 1.);
    EXPECT_TRUE(range.contains(0.5));
    EXPECT_FALSE(range.contains(-1.));
    EXPECT_FALSE(range.contains(0.));
    EXPECT_FALSE(range.contains(1.));
    EXPECT_FALSE(range.contains(2.));
    EXPECT_EQ(0. , range.getInf());
    EXPECT_EQ(1. , range.getSup());
  }

  /** shows the functionality of the Range */
  TEST_F(FilterTest, ClosedRange)
  {

    ClosedRange<double, double> range(0. , 1.);
    EXPECT_TRUE(range.contains(0.5));
    EXPECT_FALSE(range.contains(-1.));
    EXPECT_TRUE(range.contains(0.));
    EXPECT_TRUE(range.contains(1.));
    EXPECT_FALSE(range.contains(2.));
    EXPECT_EQ(0. , range.getInf());
    EXPECT_EQ(1. , range.getSup());
  }


  /** shows the functionality of the UpperBoundedSet */
  TEST_F(FilterTest, UpperBoundedSet)
  {

    UpperBoundedSet<double> upperBoundedSet(0.);
    EXPECT_TRUE(upperBoundedSet.contains(-1.));
    EXPECT_FALSE(upperBoundedSet.contains(0.));
    EXPECT_FALSE(upperBoundedSet.contains(1.));
    EXPECT_EQ(0. , upperBoundedSet.getSup());
  }

  /** shows the functionality of the ClosedUpperBoundedSet */
  TEST_F(FilterTest, ClosedUpperBoundedSet)
  {

    ClosedUpperBoundedSet<double> upperBoundedSet(0.);
    EXPECT_TRUE(upperBoundedSet.contains(-1.));
    EXPECT_TRUE(upperBoundedSet.contains(0.));
    EXPECT_FALSE(upperBoundedSet.contains(1.));
    EXPECT_EQ(0. , upperBoundedSet.getSup());
  }


  /** shows the functionality of the LowerBoundedSet */
  TEST_F(FilterTest, LowerBoundedSet)
  {

    LowerBoundedSet<double> lowerBoundedSet(0.);
    EXPECT_TRUE(lowerBoundedSet.contains(1.));
    EXPECT_FALSE(lowerBoundedSet.contains(0.));
    EXPECT_FALSE(lowerBoundedSet.contains(-1.));
    EXPECT_EQ(0. , lowerBoundedSet.getInf());
  }

  /** shows the functionality of the ClosedLowerBoundedSet */
  TEST_F(FilterTest, ClosedLowerBoundedSet)
  {

    ClosedLowerBoundedSet<double> lowerBoundedSet(0.);
    EXPECT_TRUE(lowerBoundedSet.contains(1.));
    EXPECT_TRUE(lowerBoundedSet.contains(0.));
    EXPECT_FALSE(lowerBoundedSet.contains(-1.));
    EXPECT_EQ(0. , lowerBoundedSet.getInf());
  }


  /** shows the functionality of the auto naming capability of the Filter */
  TEST_F(FilterTest, SelectionVariableName)
  {

    EXPECT_EQ("VXDTFfilterTest::SquaredDistance3D" , SquaredDistance3D().name());

  }


  /** shows how to use a filter in a simple case */
  TEST_F(FilterTest, BasicFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, VoidObserver > filter(Range<double, double>(0., 1.));

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));

  }


  /** shows how to attach an observer to a filter of interest */
  TEST_F(FilterTest, ObservedFilter)
  {
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, VoidObserver > unobservedFilter(Range<double, double>(0., 1.));

    Filter< SquaredDistance3D, Range<double, double>, Observer > filter(unobservedFilter);
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);
    counter< SquaredDistance3D >::N = 0;

    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_FALSE(filter.accept(x1, x3));
    EXPECT_EQ(2 , counter< SquaredDistance3D >::N);
  }


  /** shows how to bypass a filter which itself was not initially planned to be bypassed */
  TEST_F(FilterTest, BypassableFilter)
  {
    bool bypassControl(false);
    // Very verbose declaration, see below for convenient shortcuts
    Filter< SquaredDistance3D, Range<double, double>, Observer > nonBypassableFilter(Range<double, double>(0., 1.));
    auto filter = nonBypassableFilter.bypass(bypassControl);
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(2.0f , 0.0f, 0.0f);
    counter< SquaredDistance3D >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x2));
    EXPECT_EQ(1 , counter< SquaredDistance3D >::N);

    bypassControl = true;
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_EQ(2 , counter< SquaredDistance3D >::N);

  }


  /** shows how to write compact code using the new filter design */
  TEST_F(FilterTest, Shortcuts)
  {

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(0.5f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);
    spacePoint x4(1.0f , 0.0f, 0.0f);

    auto filterSup = (SquaredDistance3D() < 1.) ;
    EXPECT_TRUE(filterSup.accept(x1, x2));
    EXPECT_FALSE(filterSup.accept(x1, x4));
    EXPECT_FALSE(filterSup.accept(x1, x3));

    auto filterMax = (SquaredDistance3D() <= 1.) ;
    EXPECT_TRUE(filterMax.accept(x1, x2));
    EXPECT_TRUE(filterMax.accept(x1, x4));
    EXPECT_FALSE(filterMax.accept(x1, x3));


    auto filterSup2 = (1 > SquaredDistance3D()) ;
    EXPECT_TRUE(filterSup2.accept(x1, x2));
    EXPECT_FALSE(filterSup2.accept(x1, x3));
    EXPECT_FALSE(filterSup2.accept(x1, x4));

    auto filterMax2 = (1 >= SquaredDistance3D()) ;
    EXPECT_TRUE(filterMax2.accept(x1, x2));
    EXPECT_FALSE(filterMax2.accept(x1, x3));
    EXPECT_TRUE(filterMax2.accept(x1, x4));

    auto filterInf = (SquaredDistance3D() > 1.) ;
    EXPECT_TRUE(filterInf.accept(x1, x3));
    EXPECT_FALSE(filterInf.accept(x1, x2));
    EXPECT_FALSE(filterInf.accept(x1, x4));

    auto filterMin = (SquaredDistance3D() >= 1.) ;
    EXPECT_TRUE(filterMin.accept(x1, x3));
    EXPECT_FALSE(filterMin.accept(x1, x2));
    EXPECT_TRUE(filterMin.accept(x1, x4));

    auto filterInf2 = (1 < SquaredDistance3D()) ;
    EXPECT_TRUE(filterInf2.accept(x1, x3));
    EXPECT_FALSE(filterInf2.accept(x1, x2));
    EXPECT_FALSE(filterInf2.accept(x1, x4));

    auto filterMin2 = (1 <= SquaredDistance3D()) ;
    EXPECT_TRUE(filterMin2.accept(x1, x3));
    EXPECT_FALSE(filterMin2.accept(x1, x2));
    EXPECT_TRUE(filterMin2.accept(x1, x4));

    auto filterRange = (0. < SquaredDistance3D() < 1);
    EXPECT_FALSE(filterRange.accept(x1, x1));
    EXPECT_TRUE(filterRange.accept(x1, x2));
    EXPECT_FALSE(filterRange.accept(x1, x3));
    EXPECT_FALSE(filterRange.accept(x1, x4));

    auto filterClosedRange = (0. <= SquaredDistance3D() <= 1);
    EXPECT_TRUE(filterClosedRange.accept(x1, x1));
    EXPECT_TRUE(filterClosedRange.accept(x1, x2));
    EXPECT_FALSE(filterClosedRange.accept(x1, x3));
    EXPECT_TRUE(filterClosedRange.accept(x1, x4));

  }


  /** tests compatibility of filters with boolean operations for easy coupling of filters */
  TEST_F(FilterTest, BooleanOperations)
  {


    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    auto filter = !(SquaredDistance3D() > 1.);
    EXPECT_TRUE(filter.accept(x1, x2));
    EXPECT_TRUE(filter.accept(x1, x1));
    EXPECT_FALSE(filter.accept(x1, x3));

    auto filter2 =
      !(SquaredDistance3D() > 1.) &&
      !(SquaredDistance3D() < 1);
    // i.e. SquaredDistance3D == 1
    EXPECT_TRUE(filter2.accept(x1, x2));
    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_FALSE(filter2.accept(x1, x3));


    auto filter3 =
      (SquaredDistance3D() > 1.) ||
      (SquaredDistance3D() < 1);
    // i.e. SquaredDistance3D != 1
    EXPECT_FALSE(filter3.accept(x1, x2));
    EXPECT_TRUE(filter3.accept(x1, x1));
    EXPECT_TRUE(filter3.accept(x1, x3));


  }


  /** check for shortcircuit evaluation */
  TEST_F(FilterTest, ShortCircuitsEvaluation)
  {
    auto filter(
      (SquaredDistance2Dxy() < 1).observe(Observer()) &&
      (SquaredDistance3D()   < 1).observe(Observer())
    );

    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);
    spacePoint x3(2.0f , 0.0f, 0.0f);

    counter< SquaredDistance3D   >::N = 0;
    counter< SquaredDistance2Dxy >::N = 0;

    EXPECT_FALSE(filter.accept(x1, x3));
    // since the pair x1, x3 does not satisfy the SquaredDistance2Dxy
    // requirement, we do expect SquaredDistance2Dxy evaluated once:
    EXPECT_EQ(1 , counter< SquaredDistance2Dxy >::N);
    // and SquaredDistance3D not evaluated at all
    EXPECT_EQ(0 , counter< SquaredDistance3D >::N);

    EXPECT_TRUE(filter.accept(x1, x1));
    // in this case Distance2Dxy is satisfied
    EXPECT_EQ(2 , counter< SquaredDistance2Dxy >::N);
    // and Distance3D is evaluated
    EXPECT_EQ(1 , counter< SquaredDistance3D >::N);

  }


  /** explains how to use boolean checks with filters */
  TEST_F(FilterTest, BooleanVariableShortcuts)
  {
    auto filter1(BooleanVariable() == true);
    auto filter2(false == BooleanVariable());
    spacePoint x1(0.0f , 0.0f, 0.0f);
    spacePoint x2(1.0f , 0.0f, 0.0f);

    EXPECT_TRUE(filter1.accept(x1, x1));
    EXPECT_FALSE(filter1.accept(x1, x2));


    EXPECT_FALSE(filter2.accept(x1, x1));
    EXPECT_TRUE(filter2.accept(x1, x2));



  }
}
