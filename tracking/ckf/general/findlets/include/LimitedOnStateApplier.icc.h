/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/ckf/general/findlets/LimitedOnStateApplier.dcl.h>
#include <tracking/trackFindingCDC/utilities/Functional.h>
#include <tracking/trackFindingCDC/numerics/WeightComperator.h>

namespace Belle2 {
  template <class AState, class AFilter>
  void LimitedOnStateApplier<AState, AFilter>::apply(const std::vector<const AState*>& currentPath,
                                                     std::vector<TrackFindingCDC::WithWeight<AState*>>& childStates)
  {
    Super::apply(currentPath, childStates);

    if (m_param_useNStates > 0 and childStates.size() > static_cast<unsigned int>(m_param_useNStates)) {
      std::sort(childStates.begin(), childStates.end(), TrackFindingCDC::LessOf<TrackFindingCDC::GetWeight>());
      childStates.erase(childStates.begin() + m_param_useNStates, childStates.end());
    }
  };

  template <class AState, class AFilter>
  TrackFindingCDC::Weight LimitedOnStateApplier<AState, AFilter>::operator()(const Object& object)
  {
    return m_filter(object);
  };

  template <class AState, class AFilter>
  bool LimitedOnStateApplier<AState, AFilter>::needsTruthInformation()
  {
    return m_filter.needsTruthInformation();
  };

  template <class AState, class AFilter>
  void LimitedOnStateApplier<AState, AFilter>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    m_filter.exposeParameters(moduleParamList, prefix);
  };
}
