/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/track/BasicTrackVarSet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCHitLookUp.h>
#include <tracking/trackFindingCDC/mclookup/CDCMCTrackLookUp.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>

#include <tracking/trackFindingCDC/numerics/ToFinite.h>

#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include<numeric>

#include <cdc/dataobjects/CDCHit.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicTrackVarSet::extract(const CDCTrack* track)
{
  if (not track) return false;

  if (track->empty()) {
    return false;
  }

  // use boost accumulators, which lazily provide different statistics (mean, variance, ...) for the
  // data that they accumulate (i.e. are "filled" with).
  // TODO Maybe wrap the accumulators code in an interface and put them in some utility file
  statistics_accumulator drift_length_acc;
  statistics_accumulator adc_acc;
  statistics_accumulator empty_s_acc;

  unsigned int size = track->size();

  // Fill accumulators with ADC and drift circle information
  for (const CDCRecoHit3D& recoHit : *track) {
    drift_length_acc(recoHit.getWireHit().getRefDriftLength());
    adc_acc(recoHit.getWireHit().getHit()->getADCCount());
  }

  // Extract empty_s (ArcLength2D gap) information
  double s_range = track->back().getArcLength2D() - track->front().getArcLength2D();

  // fill vector with all 2D arc lengths
  std::vector<double> arc_lengths;
  std::transform(begin(*track),
                 end(*track),
                 back_inserter(arc_lengths),
  [](const CDCRecoHit3D & recoHit) { return recoHit.getArcLength2D(); });
  // Remove all NAN elements. For some reason, last hit in track is sometimes NAN
  erase_remove_if(arc_lengths, [](double x) { return std::isnan(x); });

  // calculate gaps in arc length s between adjacent hits
  // beware: first element not a difference but mapped onto itself, empty_s_gaps[0] = arc_lengths[0]
  if (arc_lengths.size() > 1) {
    std::vector<double> empty_s_gaps;
    std::adjacent_difference(begin(arc_lengths), end(arc_lengths), back_inserter(empty_s_gaps));

    // start filling accumulator with hit gaps, but skip first which is not a difference
    std::for_each(next(begin(empty_s_gaps)), end(empty_s_gaps), [&empty_s_acc](double empty_s) {
      empty_s_acc(empty_s);
    });
  }

  unsigned int empty_s_size = bacc::count(empty_s_acc);

  double drift_length_variance = -1;
  double adc_variance = -1;
  double empty_s_variance = -1;
  double empty_s_sum = -1;
  double empty_s_min = -1;
  double empty_s_max = -1;
  double empty_s_mean = -1;

  if (size > 1) {
    drift_length_variance = std::sqrt(bacc::variance(drift_length_acc) * size / (size - 1));
    adc_variance = std::sqrt(bacc::variance(adc_acc) * size / (size - 1));
  }

  if (empty_s_size > 0) {
    empty_s_sum = bacc::sum(empty_s_acc);
    empty_s_min = bacc::min(empty_s_acc);
    empty_s_max = bacc::max(empty_s_acc);
    empty_s_mean = bacc::mean(empty_s_acc);
    if (empty_s_size > 1) {
      empty_s_variance = std::sqrt(bacc::variance(empty_s_acc) * empty_s_size / (empty_s_size - 1));
    }
  }

  const CDCTrajectory3D& trajectory3D = track->getStartTrajectory3D();
  const CDCTrajectory2D& trajectory2D = trajectory3D.getTrajectory2D();
  const CDCTrajectorySZ& trajectorySZ = trajectory3D.getTrajectorySZ();

  var<named("size")>() = size;
  var<named("pt")>() = toFinite(trajectory2D.getAbsMom2D(), 0);
  // var<named("fit_prob_3d")>() = trajectory3D.getPValue();
  // var<named("fit_prob_2d")>() = trajectory2D.getPValue();
  // var<named("fit_prob_sz")>() = trajectorySZ.getPValue();

  var<named("sz_slope")>() = toFinite(trajectorySZ.getTanLambda(), 0);
  var<named("drift_length_mean")>() = toFinite(bacc::mean(drift_length_acc), 0);
  var<named("drift_length_variance")>() = toFinite(drift_length_variance, 0);
  var<named("drift_length_max")>() = toFinite(bacc::max(drift_length_acc), 0);
  var<named("drift_length_min")>() = toFinite(bacc::min(drift_length_acc), 0);
  var<named("drift_length_sum")>() = toFinite(bacc::sum(drift_length_acc), 0);

  var<named("adc_mean")>() = toFinite(bacc::mean(adc_acc), 0);
  var<named("adc_variance")>() = toFinite(adc_variance, 0);
  var<named("adc_max")>() = toFinite(bacc::max(adc_acc), 0);
  var<named("adc_min")>() = toFinite(bacc::min(adc_acc), 0);
  var<named("adc_sum")>() = toFinite(bacc::sum(adc_acc), 0);

  var<named("has_matching_segment")>() = track->getHasMatchingSegment();

  var<named("empty_s_mean")>() = toFinite(empty_s_mean, 0);
  var<named("empty_s_sum")>() = toFinite(empty_s_sum, 0);
  var<named("empty_s_variance")>() = toFinite(empty_s_variance, 0);
  var<named("empty_s_max")>() = toFinite(empty_s_max, 0);
  var<named("empty_s_min")>() = toFinite(empty_s_min, 0);
  var<named("s_range")>() = toFinite(s_range, 0);

  return true;
}
