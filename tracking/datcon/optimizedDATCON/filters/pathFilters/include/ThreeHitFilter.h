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
#include <tracking/datcon/optimizedDATCON/entities/HitData.h>

#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
// #include <tracking/trackFindingVXD/utilities/CalcCurvatureSignum.h>

#include <math.h>

namespace Belle2 {
  /// Base filter for three hits.
  /// Basic working principle: use ThreeHitVariables and provide three B2Vector3D to each variable.
  /// These are oHit (outer hit), cHit (middle hit), and iHit (inner hit) and then calculate
  /// the variables specified in ThreeHitVariables for the three positions, often using the difference
  /// (oHit - cHit) and (cHit - iHit).
  class ThreeHitFilter : public BasePathFilter {

  public:
    /// Return the weight based on azimuthal-angle separation
    TrackFindingCDC::Weight operator()(const BasePathFilter::Object& pair) override;
    /// Expose the parameters.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

  private:
    /// cut for cosine in RZ between the two vectors (oHit - cHit) and (cHit - iHit)
    double m_cosRZCut = 0.95;
    /// cut on the difference between circle radius and circle center position in the x-y plane
    /// to check if the track is compatible with passing through the IP
    double m_circleIPDistanceCut = 2.0;
    /// cut on the POCA distance in xy obtained from the helixFitEstimator
    double m_helixFitPocaDCut = 2.0;

    /// Get track quality estimate from a circle fit
    QualityEstimatorCircleFit circleFitEstimator;
    /// Get track quality estimate from a helix fit
    QualityEstimatorRiemannHelixFit helixFitEstimator;
    /// Get track quality estimate from a triplet fit
    QualityEstimatorTripletFit tripletFitEstimator;
  };
}
