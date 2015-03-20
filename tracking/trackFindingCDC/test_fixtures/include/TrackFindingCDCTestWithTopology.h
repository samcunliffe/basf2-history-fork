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

#include <framework/utilities/TestHelpers.h>

using namespace std;

namespace Belle2 {
  namespace TrackFindingCDC {
    /** This class provides the declaration of the common test fixture to all
       test of the track finding in the CDC code.

       Reminder: You can filter for specific test by running

       $ test_tracking --gtest_filter="TrackFindingCDCTest*"

       which runs only for track finding in the CDC (positiv match).

       Run

       $ test_tracking --gtest_filter="-TrackFindingCDCTest*"

       to exclude test with this fixture. Also consider
       test_tracking --help for more details

       Alternativelly it is now possible to run

       $ test_tracking_trackFindingCDC

       as a seperate executable.
    */
    class TrackFindingCDCTestWithTopology : public TestHelpers::TestWithGearbox {

    public:
      static void SetUpTestCase();
      static void TearDownTestCase();

    }; //end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //TRACKFINDINGCDCTESTWITHTOPOLOGY_H_
