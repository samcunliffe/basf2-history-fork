/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/geometry/HelixCovariance.h>

#include <gtest/gtest.h>

using namespace std;

using namespace Belle2;
using namespace TrackFindingCDC;


TEST(TrackFindingCDCTest, geometry_HelixCovariance_perigeeCovariance)
{
  HelixCovariance helixCovariance;
  PerigeeCovariance perigeeCovariance = helixCovariance.perigeeCovariance();

  EXPECT_EQ(3, perigeeCovariance.matrix().GetNrows());
  EXPECT_EQ(3, perigeeCovariance.matrix().GetNcols());

}



TEST(TrackFindingCDCTest, geometry_HelixCovariance_constructFromPerigeeAndSZCovariance)
{
  PerigeeCovariance perigeeCovariance;
  SZCovariance szCovariance;

  perigeeCovariance(iCurv, iCurv) = 1;
  perigeeCovariance(iCurv, iI) = 1;
  perigeeCovariance(iI, iCurv) = 1;
  perigeeCovariance(iI, iI) = 1;


  szCovariance(iTanL, iTanL) = 1;
  szCovariance(iZ0, iZ0) = 1;


  HelixCovariance helixCovariance(perigeeCovariance, szCovariance);

  EXPECT_EQ(1, helixCovariance(iCurv, iCurv));
  EXPECT_EQ(1, helixCovariance(iI, iI));
  EXPECT_EQ(1, helixCovariance(iCurv, iI));
  EXPECT_EQ(1, helixCovariance(iI, iCurv));

  EXPECT_EQ(1, helixCovariance(iTanL, iTanL));
  EXPECT_EQ(1, helixCovariance(iZ0, iZ0));


  EXPECT_EQ(0, helixCovariance(iPhi0, iPhi0));
  EXPECT_EQ(0, helixCovariance(iTanL, iCurv));

}
