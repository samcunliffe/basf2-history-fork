/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Christian Wessel                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/svd/filters/relations/DistanceSVDPairFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/spacePointCreation/SpacePoint.h>

using namespace Belle2;
using namespace TrackFindingCDC;

TrackFindingCDC::Weight
DistanceSVDPairFilter::operator()(const std::pair<const CKFToSVDState*, const CKFToSVDState*>& relation)
{
  const CKFToSVDState& fromState = *(relation.first);
  const CKFToSVDState& toState = *(relation.second);

  const CKFToSVDState::stateCache& fromStateCache = fromState.getStateCache();
  const CKFToSVDState::stateCache& toStateCache = toState.getStateCache();

  B2ASSERT("You have filled the wrong states into this!", toStateCache.isHitState);

  double phiDiff = fromStateCache.phi - toStateCache.phi;
  while (phiDiff > M_PI) phiDiff -= 2. * M_PI;
  while (phiDiff < -M_PI) phiDiff += 2. * M_PI;

  if (not fromStateCache.isHitState) {
    // We are coming from a CDC track, so we can use its position to only look for matching ladders
    if (fabs(phiDiff) < 0.2 and fabs(fromStateCache.theta - toStateCache.theta) < 0.2) {
      return 1.0;
    }
    // If the current state (fromState) is a RecoTrack-based state, but no relations could be created
    // don't proceed but return
    return NAN;
  }

  if (fromStateCache.geoLayer == toStateCache.geoLayer and
      fromStateCache.sensorID.getSensorNumber() == toStateCache.sensorID.getSensorNumber()) {
    // TODO: Checking for equality of sensor numbers seems not to harm the hit efficiency,
    // but maybe it's safer to allow for a sensor number difference of 1?
    return 1.0;
  }

  if (abs(phiDiff) < 0.2) {
    return 1.0;
  }

  return NAN;
}
