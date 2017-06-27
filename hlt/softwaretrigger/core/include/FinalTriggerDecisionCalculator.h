/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <mdst/dataobjects/SoftwareTriggerResult.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    /// Helper class to give the getFinalTriggerDecision to python
    class FinalTriggerDecisionCalculator {
    public:
      /**
       * Calculate the final cut decision using all "total_results" of all sub triggers in the software trigger
       * (fast reco and HLT). The return value is a bool, which has the values accept (true) and reject (false):
       * * accept if and only if both the fast_reco and the HLT has accepted the event or (in the case HLT was not
       *   already evaluated) fast_reco accepted the event. The event is also accepted if none of the triggers have run.
       * * reject if fast_reco or HLT rejected the event
       */
      static bool getFinalTriggerDecision(const SoftwareTriggerResult& result);
    };
  }
}
