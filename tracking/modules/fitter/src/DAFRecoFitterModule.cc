/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/fitter/DAFRecoFitterModule.h>
using namespace Belle2;

REG_MODULE(DAFRecoFitter)

DAFRecoFitterModule::DAFRecoFitterModule() : BaseRecoFitterModule()
{
  setDescription("DAF Fitter using Genfit. If you have TrackCands, you have to convert them to RecoTracks"
                 "using the RecoTrackCreatorModule first. After that, you probably want to add the measurements from the"
                 "hits using the MeasurementCreatorModule.");

  addParam("ProbCut", m_param_probabilityCut,
           "Probability cut for the DAF. Any value between 0 and 1 is possible. Common values are between 0.01 and 0.001",
           double(0.001));

  addParam("MinimumIterations", m_param_minimumIterations,
           "Minimum number of iterations for the Kalman filter", static_cast<unsigned int>(2));
  addParam("MaximumIterations", m_param_maximumIterations,
           "Maximum number of iterations for the Kalman filter", static_cast<unsigned int>(4));

  addParam("numberOfFailedHits", m_param_maxNumberOfFailedHits,
           "Maximum number of failed hits before aborting the fit.", static_cast<int>(-1));
}

/** Create a DAF fitter */
std::shared_ptr<genfit::AbsFitter> DAFRecoFitterModule::createFitter() const
{
  std::shared_ptr<genfit::DAF> fitter = std::make_shared<genfit::DAF>();
  fitter->setMaxFailedHits(m_param_maxNumberOfFailedHits);
  fitter->setMinIterations(m_param_minimumIterations);
  fitter->setMaxIterations(m_param_maximumIterations);

  fitter->setProbCut(m_param_probabilityCut);

  return fitter;
}
