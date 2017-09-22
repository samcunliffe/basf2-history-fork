/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/filters/pxdSpacePoint/SimpleCKFPXDStateFilter.h>

#include <tracking/ckf/utilities/StateAlgorithms.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <pxd/reconstruction/PXDRecoHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template <class ARhs, class ALhs>
  decltype(ARhs() % ALhs()) mod(ARhs a, ALhs b)
  {
    return (a % b + b) % b;
  }

  unsigned int getPTRange(const Vector3D& momentum)
  {
    const double pT = momentum.xy().norm();
    if (pT > 0.4) {
      return 0;
    } else if (pT > 0.2) {
      return 1;
    } else {
      return 2;
    }
  }
}

constexpr const double SimpleCKFPXDStateFilter::m_param_maximumHelixDistanceXY[][3];
constexpr const double SimpleCKFPXDStateFilter::m_param_maximumResidual[][3];
constexpr const double SimpleCKFPXDStateFilter::m_param_maximumChi2[][3];

Weight SimpleCKFPXDStateFilter::operator()(const BaseCKFCDCToSpacePointStateObjectFilter::Object& currentState)
{
  if (not checkOverlapAndHoles(currentState)) {
    return NAN;
  }

  const auto* spacePoint = currentState.getHit();

  if (not spacePoint) {
    // lets use a very small number here, to always have the empty state in the game
    return 0;
  }

  GeometryLayerExtractor extractGeometryLayer;
  const unsigned int layer = extractGeometryLayer(currentState);
  const Vector3D momentum(currentState.getMSoPMomentum());

  double valueToCheck;
  const MaximalValueArray* maximumValues;

  if (not currentState.isFitted() and not currentState.isAdvanced()) {
    // TODO: Use the sensor information here and cache this!
    // Filter 1
    const RecoTrack* cdcTrack = currentState.getSeedRecoTrack();

    const Vector3D position(currentState.getMSoPPosition());
    const Vector3D hitPosition(spacePoint->getPosition());
    const CDCTrajectory3D trajectory(position, 0, momentum, cdcTrack->getChargeSeed(), m_cachedBField);

    const double arcLength = trajectory.calcArcLength2D(hitPosition);
    const Vector2D& trackPositionAtHit2D = trajectory.getTrajectory2D().getPos2DAtArcLength2D(arcLength);
    const double trackPositionAtHitZ = trajectory.getTrajectorySZ().mapSToZ(arcLength);
    const Vector3D trackPositionAtHit(trackPositionAtHit2D, trackPositionAtHitZ);
    const Vector3D differenceHelix = trackPositionAtHit - hitPosition;

    valueToCheck = differenceHelix.xy().norm();
    maximumValues = &m_param_maximumHelixDistanceXY;
  } else if (not currentState.isFitted()) {
    // Filter 2
    PXDRecoHit recoHit(spacePoint->getRelated<PXDCluster>());
    const auto& measuredStateOnPlane = currentState.getMeasuredStateOnPlane();
    const double residual = m_fitter.calculateResidualDistance<PXDRecoHit, 2>(measuredStateOnPlane, recoHit);

    valueToCheck = residual;
    maximumValues = &m_param_maximumResidual;
  } else {
    // Filter 3
    valueToCheck = currentState.getChi2();
    maximumValues = &m_param_maximumChi2;
  }

  if (valueToCheck > (*maximumValues)[layer - 1][getPTRange(momentum)]) {
    return NAN;
  }

  return valueToCheck;
}