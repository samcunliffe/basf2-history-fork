/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/modules/fitter/BaseRecoFitterModule.h>
#include <genfit/KalmanFitterRefTrack.h>

namespace Belle2 {

  class KalmanRecoFitterModule : public BaseRecoFitterModule {
  public:
    /** Module for the Kalman Fitter */
    KalmanRecoFitterModule() : BaseRecoFitterModule()
    {
      setDescription("Kalman Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                     "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                     "hits using the MeasurementCreatorModule.");

      addParam("MinimumIterations", m_param_minimumIterations,
               "Minimum number of iterations for the Kalman filter", static_cast<unsigned int>(3));
      addParam("MaximumIterations", m_param_maximumIterations,
               "Maximum number of iterations for the Kalman filter", static_cast<unsigned int>(10));

      addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
               "Maximum number of failed hits before aborting the fit.", static_cast<unsigned int>(5));
    }

  protected:
    /** Create a Kalman fitter */
    std::shared_ptr<genfit::AbsFitter> createFitter() const override
    {
      std::shared_ptr<genfit::KalmanFitterRefTrack> fitter = std::make_shared<genfit::KalmanFitterRefTrack>();
      fitter->setMinIterations(m_param_minimumIterations);
      fitter->setMaxIterations(m_param_maximumIterations);
      fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);

      return fitter;
    }

  private:
    unsigned int m_param_minimumIterations; /**< Minimum number of iterations for the Kalman filter */
    unsigned int m_param_maximumIterations; /**< Maximum number of iterations for the Kalman filter */
    /** Maximum number of failed hits before aborting the fit */
    unsigned int m_param_maxNumberOfFailedHits = 5;
  };
}
