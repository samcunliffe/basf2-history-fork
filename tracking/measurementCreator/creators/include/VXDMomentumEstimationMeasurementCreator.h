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

#include <tracking/measurementCreator/creators/BaseMeasurementCreatorFromCoordinateMeasurement.h>
#include <tracking/vxdMomentumEstimation/VXDMomentumEstimation.h>
#include <tracking/vxdMomentumEstimation/PlanarMomentumMeasurement.h>
#include <mdst/dataobjects/MCParticle.h>
#include <genfit/PlanarMeasurement.h>

namespace Belle2 {
  template <class HitType, Const::EDetector detector>
  class VXDMomentumEstimationMeasurementCreator : public BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector> {
  public:
    explicit VXDMomentumEstimationMeasurementCreator(const genfit::MeasurementFactory<genfit::AbsMeasurement>& measurementFactory) :
      BaseMeasurementCreatorFromCoordinateMeasurement<HitType, detector>(measurementFactory), m_minimumMomentum(0.1) {}

    virtual ~VXDMomentumEstimationMeasurementCreator() { }

    /** Set the parameters of the fit functions and whether to use the thickness or not or the tracking seeds or not */
    void setParameter(const std::string& parameterName, const std::string& parameterValue) override
    {
      if (parameterName == "MinimumMomentum") {
        m_minimumMomentum = std::stod(parameterValue);
      } else if (parameterName == "aE") {
        m_fitParameters.aE = std::stod(parameterValue);
      } else if (parameterName == "bE") {
        m_fitParameters.bE = std::stod(parameterValue);
      } else if (parameterName == "cE") {
        m_fitParameters.cE = std::stod(parameterValue);
      } else if (parameterName == "dE") {
        m_fitParameters.dE = std::stod(parameterValue);
      } else if (parameterName == "aM") {
        m_correctionFitParameters.aM = std::stod(parameterValue);
      } else if (parameterName == "bM") {
        m_correctionFitParameters.bM = std::stod(parameterValue);
      } else if (parameterName == "cM") {
        m_correctionFitParameters.cM = std::stod(parameterValue);
      } else if (parameterName == "dM") {
        m_correctionFitParameters.dM = std::stod(parameterValue);
      } else if (parameterName == "use_trackfinder_seeds") {
        m_useTrackFinderSeeds = std::stoi(parameterValue);
      } else if (parameterName == "use_thickness") {
        m_useThickness = std::stoi(parameterValue);
      } else {
        B2FATAL("A parameter with the name " << parameterName << " and the value " << parameterValue << " could not be set.")
      }
    }

  protected:
    /**
     * Create a measurement based on the momentum estimation given by the VXDMomentumEstimation class
     */
    virtual std::vector<genfit::AbsMeasurement*> createMeasurementFromCoordinateMeasurement(HitType* hit,
        const RecoTrack& recoTrack, const RecoHitInformation&,
        const std::pair<genfit::AbsMeasurement*, genfit::TrackCandHit*>& coordinateMeasurement) const override
    {
      genfit::AbsMeasurement* absCoordinateMeasurement = coordinateMeasurement.first;
      genfit::PlanarMeasurement* planarMeasurement = dynamic_cast<genfit::PlanarMeasurement*>(absCoordinateMeasurement);
      if (planarMeasurement == nullptr) {
        B2FATAL("Can only add VXD hits which are based on PlanarMeasurements with momentum estimation!")
      }

      TVectorD rawHitCoordinates(1);
      double estimatedMomentum = 0;

      if (m_useThickness) {
        estimatedMomentum = estimateMomentumWithThickness(hit, recoTrack);
      } else {
        if (m_useTrackFinderSeeds) {
          estimatedMomentum = estimateMomentumWithTrackFinderSeeds(hit, recoTrack);
        } else {
          estimatedMomentum = estimateMomentumWithMCParticleSeeds(hit);
        }
      }

      if (not std::isnan(estimatedMomentum)) {
        rawHitCoordinates(0) = estimatedMomentum;

        TMatrixDSym rawHitCovariance(1);
        rawHitCovariance(0, 0) = 0.23;

        PlanarMomentumMeasurement* momentumMeasurement = new PlanarMomentumMeasurement(*planarMeasurement);
        momentumMeasurement->setRawHitCoords(rawHitCoordinates);
        momentumMeasurement->setRawHitCov(rawHitCovariance);
        return {momentumMeasurement};
      } else {
        return {};
      }
    }

  private:
    /** Parameters for the main function */
    typename VXDMomentumEstimation<HitType>::FitParameters m_fitParameters;
    /** Parameters for the correction function. Set them to zero to not use a correction function */
    typename VXDMomentumEstimation<HitType>::CorrectionFitParameters m_correctionFitParameters;
    /** Use the seeds of the track finder or the seeds of the MC particles */
    bool m_useTrackFinderSeeds = true;
    /** Use the thickness of the clusters of the path length for estimating dX */
    bool m_useThickness = false;
    /** Minimal value for the momentum below the estimation is used */
    double m_minimumMomentum;

    /** Use the seeds from the track finder to estimate the momentum from dedx */
    double estimateMomentumWithTrackFinderSeeds(HitType* hit, const RecoTrack& recoTrack) const
    {
      const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

      const TVector3& momentum = recoTrack.getMomentum();
      const TVector3& position = recoTrack.getPosition();
      const short charge = recoTrack.getCharge();

      const double estimation = momentumEstimation.estimateQOverP(*hit, momentum, position, charge, m_fitParameters,
                                                                  m_correctionFitParameters);

      if (momentum.Mag() > m_minimumMomentum or charge / estimation > 1.5 * m_minimumMomentum) {
        return std::nan("");
      } else {
        return estimation;
      }
    }

    /** Use the information of the related MCParticle to estimate the momentum from dedx */
    double estimateMomentumWithMCParticleSeeds(HitType* hit) const
    {
      const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

      MCParticle* relatedMCParticle = hit->template getRelated<MCParticle>("MCParticles");
      if (relatedMCParticle == nullptr) {
        return std::nan("");
      }

      const TVector3& momentum = relatedMCParticle->getMomentum();
      const TVector3& position = relatedMCParticle->getProductionVertex();
      const short charge = relatedMCParticle->getCharge();

      const double estimation = momentumEstimation.estimateQOverP(*hit, momentum, position, charge, m_fitParameters,
                                                                  m_correctionFitParameters);

      if (momentum.Mag() > m_minimumMomentum or charge / estimation > 1.5 * m_minimumMomentum) {
        return std::nan("");
      } else {
        return estimation;
      }
    }

    /** Use the thickness of the sensor to estimate the momentum from dedx */
    double estimateMomentumWithThickness(HitType* hit, const RecoTrack& recoTrack) const
    {
      const VXDMomentumEstimation<HitType>& momentumEstimation = VXDMomentumEstimation<HitType>::getInstance();

      const TVector3& momentum = recoTrack.getMomentum();
      const short charge = recoTrack.getCharge();

      const double estimation = momentumEstimation.estimateQOverPWithThickness(*hit, charge, m_fitParameters, m_correctionFitParameters);

      if (momentum.Mag() > m_minimumMomentum or charge / estimation > 1.5 * m_minimumMomentum) {
        return std::nan("");
      } else {
        return estimation;
      }
    }
  };
}
