/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackVarSet.h>

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

bool SegmentTrackVarSet::extract(const std::pair<const CDCRecoSegment2D*, const CDCTrack*>* testPair)
{
  extractNested(testPair);
  const CDCRecoSegment2D* segment = testPair->first;
  const CDCTrack* track = testPair->second;

  double maxmimumTrajectoryDistanceFront = 0;
  double maxmimumTrajectoryDistanceBack = 0;
  double maxmimumHitDistanceFront = 0;
  double maxmimumHitDistanceBack = 0;
  double outOfCDC = 0; // 0 means no, 1 means yes
  double hitsInSameRegion = 0;

  const CDCRecoHit2D& front = segment->front();
  const CDCRecoHit2D& back = segment->back();

  // Calculate distances
  const CDCTrajectory2D& trajectoryTrack = track->getStartTrajectory3D().getTrajectory2D();


  maxmimumTrajectoryDistanceFront = trajectoryTrack.getDist2D(front.getWireHit().getRefPos2D());
  maxmimumTrajectoryDistanceBack = trajectoryTrack.getDist2D(back.getWireHit().getRefPos2D());

  var<named("z_distance")>() = 0;
  var<named("theta_distance")>() = 0;

  if (segment->getStereoType() == AXIAL) {
    CDCTrajectory2D& trajectorySegment = segment->getTrajectory2D();
    if (not trajectoryTrack.isFitted()) {
      const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
      fitter.update(trajectorySegment, *segment);
    }
  } else {
    const CDCTrajectorySZ& szTrajectoryTrack = track->getStartTrajectory3D().getTrajectorySZ();

    CDCObservations2D observations;
    for (const CDCRecoHit2D& recoHit : segment->items()) {
      CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectoryTrack);
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observations.append(s, z);
    }

    if (observations.size() > 3) {
      const CDCSZFitter& fitter = CDCSZFitter::getFitter();
      const CDCTrajectorySZ& szTrajectorySegments = fitter.fit(observations);

      double startZTrack = szTrajectoryTrack.getStartZ();
      double startZSegments = szTrajectorySegments.getStartZ();

      var<named("z_distance")>() = startZTrack - startZSegments;
      var<named("theta_distance")>() = szTrajectoryTrack.getSZSlope() - szTrajectorySegments.getSZSlope();
    }
  }

  // Calculate if it is out of the CDC
  Vector3D frontRecoPos3D = front.reconstruct3D(trajectoryTrack);
  Vector3D backRecoPos3D = back.reconstruct3D(trajectoryTrack);

  if (segment->getStereoType() != AXIAL) {
    double forwardZ = front.getWire().getSkewLine().forwardZ();
    double backwardZ = front.getWire().getSkewLine().backwardZ();

    if (frontRecoPos3D.z() > forwardZ or frontRecoPos3D.z() < backwardZ or backRecoPos3D.z() > forwardZ
        or backRecoPos3D.z() < backwardZ) {
      outOfCDC = 1.0;
    }
  }

  // Count number of hits in the same region
  for (const CDCRecoHit3D& recoHit : track->items()) {
    if (recoHit.getISuperLayer() == segment->getISuperLayer()) {
      hitsInSameRegion++;
    } else if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) == 1) {
      double distanceFront = (front.getWireHit().getRefPos2D() - recoHit.getRecoPos2D()).norm();
      if (distanceFront > maxmimumHitDistanceFront) {
        maxmimumHitDistanceFront = distanceFront;
      }
      double distanceBack = (back.getWireHit().getRefPos2D() - recoHit.getRecoPos2D()).norm();
      if (distanceBack > maxmimumHitDistanceBack) {
        maxmimumHitDistanceBack = distanceBack;
      }
    }
  }

  // Make a fit with all the hits and one with only the hits in the near range
  CDCObservations2D observationsFull;
  CDCObservations2D observationsNeigh;

  // Collect the observations
  bool isAxialSegment = segment->getStereoType() != AXIAL;

  for (const CDCRecoHit3D& recoHit : track->items()) {
    if (isAxialSegment and recoHit.getStereoType() == AXIAL) {
      observationsFull.append(recoHit.getWireHit().getRefPos2D());
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.append(recoHit.getWireHit().getRefPos2D());
      }
    } else if (not isAxialSegment and recoHit.getStereoType() != AXIAL) {
      double s = recoHit.getPerpS();
      double z = recoHit.getRecoZ();
      observationsFull.append(s, z);
      if (abs(recoHit.getISuperLayer() - segment->getISuperLayer()) < 3) {
        observationsNeigh.append(s, z);
      }
    }
  }


  for (const CDCRecoHit2D& recoHit : segment->items()) {
    if (isAxialSegment) {
      observationsFull.append(recoHit.getRecoPos2D());
      observationsNeigh.append(recoHit.getRecoPos2D());
    } else {
      CDCRLWireHit rlWireHit(recoHit.getWireHit(), recoHit.getRLInfo());
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectoryTrack);
      double s = recoHit3D.getPerpS();
      double z = recoHit3D.getRecoZ();
      observationsFull.append(s, z);
      observationsNeigh.append(s, z);
    }
  }

  // Do the fit
  if (segment->getStereoType() == AXIAL) {
    const CDCRiemannFitter& fitter = CDCRiemannFitter::getFitter();
    var<named("fit_full")>() = fitter.fit(observationsFull).getPValue();
  } else {
    const CDCSZFitter& fitter = CDCSZFitter::getFitter();
    var<named("fit_full")>() = fitter.fit(observationsFull).getPValue();

    if (observationsNeigh.size() > 3) {
      var<named("fit_neigh")>() = fitter.fit(observationsNeigh).getPValue();
    } else {
      var<named("fit_neigh")>() = 0;
    }
  }

  if (observationsFull.size() == observationsNeigh.size()) {
    var<named("fit_neigh")>() = -1;
  }

  //var<named("is_stereo")>() = segment->getStereoType() != AXIAL;
  var<named("segment_size")>() = segment->size();
  var<named("track_size")>() = track->size();

  var<named("pt_of_track")>() = std::isnan(trajectoryTrack.getAbsMom2D()) ? 0.0 : trajectoryTrack.getAbsMom2D();
  var<named("track_is_curler")>() = trajectoryTrack.getExit().hasNAN();

  var<named("superlayer_already_full")>() = not trajectoryTrack.getOuterExit().hasNAN() and hitsInSameRegion > 5;

  if (std::isnan(maxmimumTrajectoryDistanceFront)) {
    var<named("maxmimum_trajectory_distance_front")>() = 999;
  } else {
    var<named("maxmimum_trajectory_distance_front")>() = maxmimumTrajectoryDistanceFront;
  }

  if (std::isnan(maxmimumTrajectoryDistanceBack)) {
    var<named("maxmimum_trajectory_distance_back")>() = 999;
  } else {
    var<named("maxmimum_trajectory_distance_back")>() = maxmimumTrajectoryDistanceBack;
  }

  var<named("maxmimum_hit_distance_front")>() = maxmimumHitDistanceFront;
  var<named("maxmimum_hit_distance_back")>() = maxmimumHitDistanceBack;

  var<named("out_of_CDC")>() = outOfCDC;
  var<named("hits_in_same_region")>() = hitsInSameRegion;

  return true;
}
