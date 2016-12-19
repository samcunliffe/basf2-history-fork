/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>
#include <tracking/trackFindingCDC/hough/algorithms/InPhi0CurvBox.h>

#include <tracking/trackFindingCDC/utilities/ParameterVariant.h>

#include <vector>
#include <map>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCWireHit;
    class CDCTrack;

    /// Generates axial tracks from hit using the special leaf processing inspired by the legendre algorithm.
    class AxialTrackCreatorHitLegendre : public Findlet<const CDCWireHit, CDCTrack> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCWireHit, CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize the findlet before event processing
      void initialize() final;

      /// Generates the tracks from the given segments into the output argument.
      void apply(const std::vector<CDCWireHit>& wireHits, std::vector<CDCTrack>& tracks) final;

      /// Cleanup the findlet after event processing
      void terminate() final;

    public:
      /// Get a series of parameters to be set for each pass over the fine hough space
      std::vector<ParameterVariantMap> getDefaultFineRelaxationSchedule() const;

      /// Get a series of parameters to be set for each pass over the rough hough space
      std::vector<ParameterVariantMap> getDefaultRoughRelaxationSchedule() const;

    private:
      /// Parameter: Level of divisions in the hough space.
      int m_param_granularityLevel = 12;

      /// Parameter: Number of levels to be skipped on the first level to form sectors
      int m_param_sectorLevelSkip = 0;

      /// Parameter: Curvature bounds of the hough space.
      // std::vector<float> m_param_curvBounds{{ -0.018, 0.75}};

      // Parameter: Fine hough bounds.
      std::vector<float> m_param_fineCurvBounds{{ -0.02, 0.14}};

      // Parameter: Rough hough bounds.
      std::vector<float> m_param_roughCurvBounds{{ 0.0, 0.30}};

      /// Parameter: Width of the phi0 bins at the lowest level of the hough space.
      size_t m_param_discretePhi0Width = 5;

      /// Parameter: Overlap of the phi0 bins at the lowest level of the hough space.
      size_t m_param_discretePhi0Overlap = 4;

      /// Parameter: Width of the curvature bins at the lowest level of the hough space.
      size_t m_param_discreteCurvWidth = 5;

      /// Parameter: Overlap of the curvature bins at the lowest level of the hough space.
      size_t m_param_discreteCurvOverlap = 4;

      /// Fixed parameter: Number of divisions in the phi0 direction
      static const size_t c_phi0Divisions = 2;

      /// Fixed parameter: Number of divisions in the curv direction
      static const size_t c_curvDivisions = 2;

      /// Parameter: Absolute minimal number of hits to make an axial track.
      double m_param_minNHits = 30;

      /// Parameter: Relaxation schedule for the leaf processor in the fine hough tree
      std::vector<ParameterVariantMap> m_param_fineRelaxationSchedule;

      /// Parameter: Relaxation schedule for the leaf processor in the fine hough tree
      std::vector<ParameterVariantMap> m_param_roughRelaxationSchedule;

      const double m_maxCurvAcceptance = 0.13;
      const double m_curlCurv = 0.018;

    private:
      /// Type of the hough space tree search
      using SimpleRLTaggedWireHitPhi0CurvHough =
        SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, c_phi0Divisions, c_curvDivisions>;

      /// The fine hough space tree search
      std::unique_ptr<SimpleRLTaggedWireHitPhi0CurvHough> m_fineHoughTree;

      /// The rough space tree search
      std::unique_ptr<SimpleRLTaggedWireHitPhi0CurvHough> m_roughHoughTree;
    };
  }
}
