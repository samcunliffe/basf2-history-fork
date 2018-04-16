/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015-2016 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tobias Schlüter, Thomas Hauth, Nils Braun                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackTimeExtraction/FullGridTrackTimeExtractionModule.h>
#include <tracking/timeExtraction/TimeExtractionUtils.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>

#include <numeric>

using namespace Belle2;

REG_MODULE(FullGridTrackTimeExtraction);

namespace {
  /// Helper Structure holding one extracted time together with their chi^2.
  struct T0Try {
    double m_extractedT0;
    double m_chi2;

    T0Try(const double& extractedT0, const double& chi2) :
      m_extractedT0(extractedT0), m_chi2(chi2) {}
  };

  /// Make a list of extracted times sortable by their chi^2.
  bool operator<(const T0Try& lhs, const T0Try& rhs)
  {
    return lhs.m_chi2 < rhs.m_chi2;
  }

  /// Helper function to map a function over a list and sum the results for double types
  template<class T, class AFunction>
  double sum(StoreArray<T>& list, const AFunction& function)
  {
    return std::accumulate(std::begin(list), std::end(list), 0.0, [&function](double value, T & item) {
      return value + function(item);
    });
  }

  /// Helper function to map a function over a list and sum the results for std::pair types
  template<class T, class AFunction>
  std::pair<double, double> sumPaired(StoreArray<T>& list, const AFunction& function)
  {
    return std::accumulate(std::begin(list), std::end(list), std::make_pair(0.0, 0.0), [&function](std::pair<double, double>& value,
    T & item) {
      const auto& result = function(item);
      value.first += result.first;
      value.second += result.second;
      return value;
    });
  }

  /// Extract the first and second chi^2 derivatives from each fittable track and build the mean.
  std::pair<double, double> extractChi2DerivativesHelper(StoreArray<RecoTrack>& recoTracks,
                                                         std::map<RecoTrack*, bool>& fittableRecoTracks)
  {
    unsigned int numberOfFittableRecoTracks = 0;
    auto chi2Derivates = sumPaired(recoTracks, [&](RecoTrack & recoTrack) {
      if (not fittableRecoTracks[&recoTrack]) {
        return std::make_pair(0.0, 0.0);
      }

      numberOfFittableRecoTracks++;
      return TimeExtractionUtils::getChi2Derivatives(recoTrack);
    });

    chi2Derivates.first /= numberOfFittableRecoTracks;
    chi2Derivates.second /= numberOfFittableRecoTracks;

    return chi2Derivates;
  }

  /// Extract the chi^2 from the fittable reco tracks
  double extractChi2Helper(double value, std::vector<std::pair<RecoTrack*, double>>& recoTracksWithInitialValue,
                           std::map<RecoTrack*, bool>& fittableRecoTracks)
  {
    TrackFitter trackFitter;
    unsigned int numberOfFittableRecoTracks = 0;
    double chi2Sum = 0;

    for (const auto& recoTrackWithTimeSeed : recoTracksWithInitialValue) {
      RecoTrack* recoTrack = recoTrackWithTimeSeed.first;
      double initialValue = recoTrackWithTimeSeed.second;

      recoTrack->setTimeSeed(initialValue + value);
      genfit::AbsTrackRep* cardinalRep = recoTrack->getCardinalRepresentation();
      B2ASSERT("There is no cardinal representation?", cardinalRep);
      if (not trackFitter.fit(*recoTrack, cardinalRep)) {
        fittableRecoTracks[recoTrack] = false;
      } else {
        fittableRecoTracks[recoTrack] = true;
        chi2Sum += TimeExtractionUtils::extractReducedChi2(*recoTrack);
        numberOfFittableRecoTracks++;
      }
    }
    B2DEBUG(100, "Chi2 extraction finished with time seed of " << value << " and the result is " << chi2Sum);

    return chi2Sum / numberOfFittableRecoTracks;
  }

  /**
   * Main function used in the module: Repeat the time extraction of the reco tracks starting from the given start
   * value until:
   * (a) the maximum number of steps is reached
   * (b) the extracted time is not in the range [t0 min, t0 max]
   * (c) the result is non-sense (e.g. NaN)
   *
   * Each extracted time is either added to the tries or the converged tries list, depending on if the chi^2 is small
   * and d^2 chi^2 / (d alpha)^2 is large enough.
   *
   * In the end, the tracks are reset to have the initial time seed.
   */
  std::vector<T0Try> extractTrackTimeFrom(StoreArray<RecoTrack>& recoTracks, const double& startValue, const unsigned int steps,
                                          std::vector<T0Try>& convergedTries,
                                          const double& minimalT0, const double& maximalT0)
  {
    // Store the initial reco track time values to (a) subtract them on each step and (b) reset the tracks afterwards
    std::vector<std::pair<RecoTrack*, double>> recoTracksWithInitialValue;

    std::vector<T0Try> tries;

    for (RecoTrack& recoTrack : recoTracks) {
      recoTracksWithInitialValue.emplace_back(&recoTrack, recoTrack.getTimeSeed());
    }

    // Store which reco tracks we should use (and where the fit already failed)
    std::map<RecoTrack*, bool> fittableRecoTracks;
    for (RecoTrack& recoTrack : recoTracks) {
      fittableRecoTracks[&recoTrack] = true;
    }

    // Store the first try ( = start value) with the chi^2
    const double firstChi2 = extractChi2Helper(startValue, recoTracksWithInitialValue, fittableRecoTracks);

    if (std::isnan(firstChi2)) {
      return tries;
    }

    tries.emplace_back(startValue, firstChi2);
    B2DEBUG(100, "Initial state: " << startValue << " with " << firstChi2);

    double extracted_time = startValue;

    for (unsigned int i = 0; i < steps; i++) {
      // Extract the time for the next time step
      std::pair<double, double> extractedDerivativePair = extractChi2DerivativesHelper(recoTracks, fittableRecoTracks);
      extracted_time += extractedDerivativePair.first / extractedDerivativePair.second;

      if (extracted_time > maximalT0 or extracted_time < minimalT0 or std::isnan(extracted_time)) {
        break;
      }

      // Apply this new extracted time and extract the chi^2
      const double chi2 = extractChi2Helper(extracted_time, recoTracksWithInitialValue, fittableRecoTracks);

      if (chi2 > 10 or std::isnan(chi2)) {
        break;
      }

      // Decide if we are already finished or not depending on the extracted values.
      const bool finished = extractedDerivativePair.second > 2.7122 and chi2 < 1.739;

      if (finished) {
        convergedTries.emplace_back(extracted_time, chi2);
        B2DEBUG(100, "Converged result: " << startValue << " with " << firstChi2);
        break;
      }

      tries.emplace_back(extracted_time, chi2);
      B2DEBUG(100, "Unconverged result: " << startValue << " with " << firstChi2);
    }

    // Reset all RecoTracks
    for (const auto& recoTrackWithTimeSeed : recoTracksWithInitialValue) {
      RecoTrack* recoTrack = recoTrackWithTimeSeed.first;
      double initialValue = recoTrackWithTimeSeed.second;

      recoTrack->setTimeSeed(initialValue);
    }

    return tries;
  }
}


FullGridTrackTimeExtractionModule::FullGridTrackTimeExtractionModule()
{
  setDescription("Build the full covariance matrix for RecoTracks and extract the event time using the CDC drift time information.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("recoTracksStoreArrayName", m_param_recoTracksStoreArrayName, "StoreArray containing the RecoTracks to process",
           m_param_recoTracksStoreArrayName);

  addParam("minimalT0Shift", m_param_minimalT0Shift, "Minimal shift of the event time which is allowed.",
           m_param_minimalT0Shift);
  addParam("maximalT0Shift", m_param_maximalT0Shift, "Maximal shift of the event time which is allowed.",
           m_param_maximalT0Shift);
  addParam("numberOfGrids", m_param_numberOfGrids, "Number of shifts to try out.",
           m_param_numberOfGrids);

  addParam("t0Uncertainty", m_param_t0Uncertainty, "Use this as sigma t0.",
           m_param_t0Uncertainty);

  addParam("overwriteExistingEstimation", m_param_overwriteExistingEstimation,
           "Whether to replace an existing time estimation or not.",
           m_param_overwriteExistingEstimation);

}

void FullGridTrackTimeExtractionModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);
  recoTracks.isRequired();

  m_eventT0.registerInDataStore();
}

void FullGridTrackTimeExtractionModule::event()
{
  StoreArray<RecoTrack> recoTracks(m_param_recoTracksStoreArrayName);

  if (recoTracks.getEntries() == 0) {
    return;
  }

  if (not m_eventT0.isValid()) {
    m_eventT0.create();
  } else if (not m_param_overwriteExistingEstimation) {
    B2WARNING("T0 estimation already present and overwriteExistingEstimation set to false. Skipping.");
    return;
  }

  // Try out phase: test 3 data points between t0 min and t0 max and let them extrapolate a bit.
  std::vector<T0Try> tries;
  tries.reserve(m_param_numberOfGrids * 4);
  std::vector<T0Try> convergedTries;

  const double deltaT0 = 1.0 / (double)m_param_numberOfGrids * (m_param_maximalT0Shift - m_param_minimalT0Shift);

  for (unsigned int i = 1; i < m_param_numberOfGrids; i++) {
    const std::vector<T0Try>& tmpTries = extractTrackTimeFrom(recoTracks, m_param_minimalT0Shift + i * deltaT0, 2,
                                                              convergedTries, m_param_minimalT0Shift, m_param_maximalT0Shift);
    for (const T0Try& t0Try : tmpTries) {
      tries.push_back(t0Try);
    }
  }

  if (not convergedTries.empty()) {
    // If we have found some "converging" extracted t0s, use the one with the lowest chi2.
    const auto& minimalChi2 = std::min_element(convergedTries.begin(), convergedTries.end());

    const double extractedTime = minimalChi2->m_extractedT0;
    // The uncertainty was calculated using a test MC sample
    m_eventT0->addTemporaryEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
    // TODO: until now, we have no combination of different t0s in place, so we just set the final one here.
    m_eventT0->setEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
  } else {
    // If not, start with the lowest extracted chi2 and do another two iteration steps. If it converges then,
    // use this. Else, use the next best guess.
    std::sort(tries.begin(), tries.end());

    for (const auto& tryOut : tries) {
      extractTrackTimeFrom(recoTracks, tryOut.m_extractedT0, 2, convergedTries, m_param_minimalT0Shift, m_param_maximalT0Shift);
      if (not convergedTries.empty()) {
        const double extractedTime = convergedTries.back().m_extractedT0;
        // The uncertainty was calculated using a test MC sample
        m_eventT0->addTemporaryEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
        // TODO: until now, we have no combination of different t0s in place, so we just set the final one here.
        m_eventT0->setEventT0(extractedTime, m_param_t0Uncertainty, Const::EDetector::CDC);
        break;
      }
    }
  }

  if (m_eventT0->hasEventT0()) {
    B2DEBUG(100, "Final EventT0 " << m_eventT0->getEventT0());
  }
}

