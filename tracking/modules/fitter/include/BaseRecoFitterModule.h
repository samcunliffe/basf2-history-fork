/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <genfit/MeasurementFactory.h>
#include <framework/gearbox/Const.h>
#include <framework/core/Module.h>
#include <string>

namespace genfit {
  class AbsMeasurement;
  class AbsKalmanFitter;
}


namespace Belle2 {

  class RecoTrack;

  class BaseRecoFitterModule : public Module {

  public:
    /** Constructor .
     */
    BaseRecoFitterModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize() override;

    /** Called when entering a new run.
     */
    void beginRun() override { }

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event() override;

    /** This method is called if the current run ends.
     */
    void endRun() override { }

    /** This method is called at the end of the event processing.
     */
    void terminate() override { }

  protected:
    /** Method do create the used filter
     * This method has to by implemented by the derived class
     * @return The fitter we will use for fitting.
     */
    virtual std::shared_ptr<genfit::AbsKalmanFitter> createFitter() const = 0;

  private:
    /** StoreArray name of the input and output reco tracks */
    std::string m_param_recoTracksStoreArrayName;
    /** Use this particle hypothesis for fitting. Please use the positive pdg code only. */
    unsigned int m_param_pdgCodeToUseForFitting;
    /** Use the momentum estimation from VXD */
    bool m_param_useVXDMomentumEstimation;

    unsigned int m_param_maxNumberOfFailedHits = 5;
    genfit::MeasurementFactory<genfit::AbsMeasurement> m_measurementFactory;
    std::string m_param_storeArrayNameOfCDCHits = "CDCHits";
    std::string m_param_storeArrayNameOfSVDHits = "SVDClusters";
    std::string m_param_storeArrayNameOfPXDHits = "PXDClusters";

    /**
     * Helper function to construct the measurements for the added hits of a reco track.
     * @param recoTrack
     */
    void constructHitsForTrack(RecoTrack& recoTrack) const;
  };
}

