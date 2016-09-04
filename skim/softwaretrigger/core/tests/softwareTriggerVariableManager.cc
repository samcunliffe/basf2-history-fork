/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/core/SoftwareTriggerVariableManager.h>
#include <skim/softwaretrigger/core/SoftwareTriggerCut.h>
#include <skim/softwaretrigger/dataobjects/SoftwareTriggerResult.h>
#include <gtest/gtest.h>

using namespace std;

namespace Belle2 {
  namespace SoftwareTrigger {

    /** Test the software trigger variable manager. */
    TEST(SoftwareTriggerVarialeManagerTest, pickVariables)
    {
      SoftwareTriggerObject softwareTriggerObject;
      softwareTriggerObject["one_variable"] = 1.1;

      // Normal user case
      const auto compiledFirstCut = SoftwareTriggerCut::compile("one_variable == 1.1", 1);
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, compiledFirstCut->checkPreScaled(softwareTriggerObject));

      // Unregistered variable
      EXPECT_THROW(SoftwareTriggerCut::compile("two_variable == 1.1", 1)->checkPreScaled(softwareTriggerObject),
                   std::out_of_range);

      softwareTriggerObject["two_variable"] = 2.2;

      // Now it is registered
      const auto compiledSecondCut = SoftwareTriggerCut::compile("two_variable == 2.2", 1);
      EXPECT_EQ(SoftwareTriggerCutResult::c_accept, compiledSecondCut->checkPreScaled(softwareTriggerObject));

      // Check should fail
      softwareTriggerObject["two_variable"] = 2.3;
      EXPECT_EQ(SoftwareTriggerCutResult::c_noResult, compiledSecondCut->checkPreScaled(softwareTriggerObject));
    }
  }
}
