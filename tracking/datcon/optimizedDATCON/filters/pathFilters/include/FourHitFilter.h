/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/datcon/optimizedDATCON/filters/pathFilters/BasePathFilter.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/FourHitVariables.h>
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <math.h>

namespace Belle2 {
  /// Filter for four hits.
  /// Basic working principle: use FourHitVariables and ThreeHitVariables and provide four (three B2Vector3D) to each variable.
  /// These are oHit (outer hit), coHit (middle outer hit), ciHit (middle inner hit) and iHit (inner hit) and then calculate
  /// the variables specified in FourHitVariables and ThreeHitVariables for the four positions
  class FourHitFilter : public BasePathFilter {
  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// set BField value for estimator
    void beginRun() override;

  private:
    /// TODO: Cut on relations in theta for overlay region on same layer but different ladder
    double m_param_CircleRadiusDifferenceCut = 10.;
    /// TODO: Filter relations in theta between hit states where the layer difference is +-1
    double m_param_CircleCenterPositionDifferenceCut = 10.;

    /// Construct empty ThreeHitVariables instance
    FourHitVariables m_fourHitVariables;
  };
}
