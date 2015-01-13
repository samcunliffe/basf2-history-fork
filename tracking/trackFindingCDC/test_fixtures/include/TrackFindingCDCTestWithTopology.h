/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost <oliver.frost@desy.de>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKFINDINGCDCTESTWITHTOPOLOGY_H_
#define TRACKFINDINGCDCTESTWITHTOPOLOGY_H_

#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace TrackFindingCDC {
    /** This class provides the declaration of the common test fixture to all
       test of the CDCLocalTracking code.

       Reminder: You can filter for specific test by running

       $ test_tracking --gtest_filter="TrackFindingCDCTest*"

       which runs only CDCLocalTracking test (positiv match).

       Run

       $ test_tracking --gtest_filter="-TrackFindingCDCTest*"

       to exclude test with this fixture. Also consider
       test_tracking --help for more details
    */
    class TrackFindingCDCTestWithTopology : public ::testing::Test {

    public:
      static void SetUpTestCase();
      static void TearDownTestCase();

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //TRACKFINDINGCDCTESTWITHTOPOLOGY_H_
