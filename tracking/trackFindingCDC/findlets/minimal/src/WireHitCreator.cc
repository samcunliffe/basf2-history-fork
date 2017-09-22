/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/WireHitCreator.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/utils/FlightTimeEstimator.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/findlets/minimal/EPreferredDirection.h>

#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/translators/LinearGlobalADCCountTranslator.h>

#include <cdc/dataobjects/CDCHit.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/utilities/ParamList.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;

WireHitCreator::WireHitCreator() = default;
WireHitCreator::~WireHitCreator() = default;

std::string WireHitCreator::getDescription()
{
  return "Combines the geometrical information and the raw hit information into wire hits, "
         "which can be used from all modules after that";
}

void WireHitCreator::exposeParams(ParamList* paramList, const std::string& prefix)
{
  paramList->addParameter(prefixed(prefix, "wirePosition"),
                          m_param_wirePosition,
                          "Set of geometry parameters to be used in the track finding. "
                          "Either 'base', 'misaligned' or 'aligned'.",
                          m_param_wirePosition);

  paramList->addParameter(prefixed(prefix, "ignoreWireSag"),
                          m_param_ignoreWireSag,
                          "Assume a wire sag coefficient of zero "
                          "such that the wires appear to be straight for "
                          "the track finders",
                          m_param_ignoreWireSag);

  paramList->addParameter(prefixed(prefix, "flightTimeEstimation"),
                          m_param_flightTimeEstimation,
                          "Option which flight direction should be assumed for "
                          "an initial time of flight estimation. Options are: "
                          "'none' (no TOF correction), "
                          "'outwards', "
                          "'downwards'.",
                          m_param_flightTimeEstimation);

  paramList->addParameter(prefixed(prefix, "triggerPoint"),
                          m_param_triggerPoint,
                          "Point relative to which the flight times are calculated",
                          m_param_triggerPoint);

  paramList->addParameter(prefixed(prefix, "useSuperLayers"),
                          m_param_useSuperLayers,
                          "List of super layers to be used - mostly for debugging",
                          m_param_useSuperLayers);

  paramList->addParameter(prefixed(prefix, "useSecondHits"),
                          m_param_useSecondHits,
                          "Use the second hit information in the track finding.",
                          m_param_useSecondHits);

}

void WireHitCreator::initialize()
{
  StoreArray<CDCHit> hits;
  hits.isRequired();

  // Create the wires and layers once during initialisation
  CDCWireTopology::getInstance();

  if (m_param_wirePosition == "base") {
    m_wirePosition = EWirePosition::c_Base;
  } else if (m_param_wirePosition == "misaligned") {
    m_wirePosition = EWirePosition::c_Misaligned;
  } else if (m_param_wirePosition == "aligned") {
    m_wirePosition = EWirePosition::c_Aligned;
  } else {
    B2ERROR("Received unknown wirePosition " << m_param_wirePosition);
  }

  m_tdcCountTranslator.reset(new CDC::RealisticTDCCountTranslator);
  m_adcCountTranslator.reset(new CDC::LinearGlobalADCCountTranslator);

  if (m_param_flightTimeEstimation != std::string("")) {
    try {
      m_flightTimeEstimation = getPreferredDirection(m_param_flightTimeEstimation);
    } catch (std::invalid_argument& e) {
      B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
    }
  }

  m_triggerPoint = Vector3D(std::get<0>(m_param_triggerPoint),
                            std::get<1>(m_param_triggerPoint),
                            std::get<2>(m_param_triggerPoint));

  if (m_flightTimeEstimation == EPreferredDirection::c_Symmetric) {
    B2ERROR("Unexpected 'flightTimeEstimation' parameter : '" << m_param_flightTimeEstimation);
  } else if (m_flightTimeEstimation == EPreferredDirection::c_Downwards) {
    FlightTimeEstimator::instance(std::make_unique<CosmicRayFlightTimeEstimator>(m_triggerPoint));
  } else if (m_flightTimeEstimation == EPreferredDirection::c_Outwards) {
    FlightTimeEstimator::instance(std::make_unique<BeamEventFlightTimeEstimator>());
  }

  if (not m_param_useSuperLayers.empty()) {
    for (const ISuperLayer& iSL : m_param_useSuperLayers) {
      m_useSuperLayers.at(iSL) = true;
    }
  } else {
    m_useSuperLayers.fill(true);
  }

  Super::initialize();
}

void WireHitCreator::beginRun()
{
  CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  wireTopology.reinitialize(m_wirePosition, m_param_ignoreWireSag);
}

void WireHitCreator::apply(std::vector<CDCWireHit>& outputWireHits)
{
  // Wire hits have been created before
  if (not outputWireHits.empty()) return;

  const CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  CDC::TDCCountTranslatorBase& tdcCountTranslator = *m_tdcCountTranslator;
  CDC::ADCCountTranslatorBase& adcCountTranslator = *m_adcCountTranslator;

  // Create the wire hits into a vector
  StoreArray<CDCHit> hits;
  std::size_t nHits = hits.getEntries();
  if (nHits == 0) {
    B2WARNING("Event with no hits");
    outputWireHits.clear();
  }

  outputWireHits.reserve(nHits);
  for (const CDCHit& hit : hits) {

    // ignore this hit if it contains the information of a 2nd hit
    if (!m_param_useSecondHits && hit.is2ndHit()) {
      continue;
    }

    WireID wireID(hit.getID());
    if (not wireTopology.isValidWireID(wireID)) {
      B2WARNING("Skip invalid wire id " << hit.getID());
      continue;
    }

    ISuperLayer iSL = wireID.getISuperLayer();
    if (not m_useSuperLayers[iSL]) continue;

    const CDCWire& wire = wireTopology.getWire(wireID);

    const Vector2D& pos2D = wire.getRefPos2D();

    // Consider the particle as incoming in the top part of the CDC for a downwards flight direction
    bool isIncoming = m_flightTimeEstimation == EPreferredDirection::c_Downwards and pos2D.y() > 0;
    const double alpha = isIncoming ?  M_PI : 0;
    const double beta = 1;
    const double flightTimeEstimate =
      FlightTimeEstimator::instance().getFlightTime2D(pos2D, alpha, beta);

    const bool left = false;
    const bool right = true;
    const double theta = M_PI / 2;

    const double leftRefDriftLength =
      tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                        wire.getWireID(),
                                        flightTimeEstimate,
                                        left,
                                        wire.getRefZ(),
                                        alpha,
                                        theta);

    const double rightRefDriftLength =
      tdcCountTranslator.getDriftLength(hit.getTDCCount(),
                                        wire.getWireID(),
                                        flightTimeEstimate,
                                        right,
                                        wire.getRefZ(),
                                        alpha,
                                        theta);

    const double refDriftLength =
      (leftRefDriftLength + rightRefDriftLength) / 2.0;

    const double leftRefDriftLengthVariance =
      tdcCountTranslator.getDriftLengthResolution(refDriftLength,
                                                  wire.getWireID(),
                                                  left,
                                                  wire.getRefZ(),
                                                  alpha,
                                                  theta);

    const double rightRefDriftLengthVariance =
      tdcCountTranslator.getDriftLengthResolution(refDriftLength,
                                                  wire.getWireID(),
                                                  right,
                                                  wire.getRefZ(),
                                                  alpha,
                                                  theta);

    const double refDriftLengthVariance =
      (leftRefDriftLengthVariance + rightRefDriftLengthVariance) / 2.0;

    const double leftRefChargeDeposit =
      adcCountTranslator.getCharge(hit.getADCCount(),
                                   wire.getWireID(),
                                   left,
                                   wire.getRefZ(),
                                   theta);

    const double rightRefChargeDeposit =
      adcCountTranslator.getCharge(hit.getADCCount(),
                                   wire.getWireID(),
                                   right,
                                   wire.getRefZ(),
                                   theta);

    const double refChargeDeposit =
      (leftRefChargeDeposit + rightRefChargeDeposit) / 2.0;

    outputWireHits.emplace_back(&hit, refDriftLength, refDriftLengthVariance, refChargeDeposit);
  }

  std::sort(outputWireHits.begin(), outputWireHits.end());
}
