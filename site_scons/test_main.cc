/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/** @file
 *
 * Main function to unit test executables.
 * This file contains basically a copy of the gtest_main.cc provided by the
 * gtest framework but extends it with a custom event listener to set the
 * random state before each test to a well defined state.
 */

#include "gtest/gtest.h"
#include "framework/core/RandomNumbers.h"

namespace {
  /** Class to set up random numbers before running each test.
   * The random generator will be set to event independent mode with the name
   * of the test "<testcase>/<test>" as seed.
   */
  class TestEventListener: public ::testing::EmptyTestEventListener {
    /** Set the random state before the test is run */
    virtual void OnTestStart(const ::testing::TestInfo& test) final override
    {
      std::string name = test.test_case_name();
      name += "/";
      name += test.name();
      Belle2::RandomNumbers::initialize(name);
    }
  };
}

/** Run all tests */
int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::UnitTest& unit_test = *::testing::UnitTest::GetInstance();
  unit_test.listeners().Append(new TestEventListener());
  return RUN_ALL_TESTS();
}
