/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

namespace Belle2 {
  /**
   * Findlet for teaching a MC or recording filter, which states are correct (= setting the truth information)
   */
  template<class ASeed, class AHitObject>
  class StateTeacher : public TrackFindingCDC::Findlet<CKFStateObject<ASeed, AHitObject>*> {
  public:
    /// The parent class
    using Super = TrackFindingCDC::Findlet<CKFStateObject<ASeed, AHitObject>*>;
    /// The state class
    using State = CKFStateObject<ASeed, AHitObject>;

    /// Expose the parameters of the filter
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override
    {
      Super::exposeParameters(moduleParamList, prefix);

      moduleParamList->addParameter("enableStateTeacher", m_param_enableStateTeacher,
                                    "Enable adding truth information from the teacher to the states.",
                                    m_param_enableStateTeacher);
    }

    /// Main function of this findlet: add truth information from the MC method
    void apply(std::vector<State*>& states) final {
      if (not m_param_enableStateTeacher)
      {
        return;
      }

      for (State* state : states)
      {
        state->setTruthInformation(allStatesCorrect(*state));
      }
    }

  private:
    // Parameters
    /// Parameter: Enable adding truth information from the teacher to the states.
    bool m_param_enableStateTeacher = false;
  };
}
