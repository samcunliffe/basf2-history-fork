/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    class CDCRecoHit2D;
    class CDCRecoSegment2D;

    /// Helper construct implementing the (re)estimation of the drift length for various hit objects
    struct DriftLengthEstimator {

      /// Add the parameters of the estimator to the module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "");

      /// Update the drift length of the reconstructed hit in place.
      void updateDriftLength(CDCRecoHit2D& recoHit2D);

      /// Update the drift length of the contained reconstructed hit in place.
      void updateDriftLength(CDCRecoSegment2D& segment);

      /// Parameter : Switch to serve the alpha angle to the drift length translator.
      bool m_param_useAlphaInDriftLength = true;

      /// Parameter : Mass to estimate the velocity in the flight time to the hit
      double m_param_tofMassScale = NAN;

    }; // struct
  } // namespace TrackFindingCDC
} // namespace Belle2
