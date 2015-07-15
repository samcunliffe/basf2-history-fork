/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/tests_fixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/hough/phi0_curv/HitInPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHough.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>

#include <tracking/trackFindingCDC/utilities/TimeIt.h>

#include <set>
#include <cmath>
#include <vector>
#include <gtest/gtest.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(CDCLegendreTestFixture, phi0CurvHoughTreeOnTrackHits)
{
  // Prepare the hough algorithm
  const size_t maxLevel = 13;
  const size_t phi0Divisions = 2; // Division at each level
  const size_t curvDivisions = 2; // Division at each level

  // const size_t maxLevel = 8;
  // const size_t phiDivisions = 3;
  // const size_t curvDivisions = 3;

  const size_t discretePhi0Overlap = 1;
  const size_t discretePhi0Width = 2;

  const size_t discreteCurvOverlap = 1;
  const size_t discreteCurvWidth = 2;
  const double maxCurv = 3.0;


  // Setup thre discrete values for phi0
  assert(discretePhi0Width > discretePhi0Overlap);
  const size_t nPhi0Bins = std::pow(phi0Divisions, maxLevel);
  const size_t nDiscretePhi0 = (discretePhi0Width - discretePhi0Overlap) * nPhi0Bins + discretePhi0Overlap + 1;

  const double phi0Overlap = 2 * PI / (nPhi0Bins * (static_cast<double>(discretePhi0Width) / discretePhi0Overlap - 1) + 1);
  B2INFO("phi0Overlap " << phi0Overlap);

  // Adjust the phi0 bounds such that overlap occures at the wrap around of the phi0 range as well
  const double phi0LowerBound = -PI - phi0Overlap;
  const double phi0UpperBound = +PI + phi0Overlap;

  DiscreteAngleArray discreteAngles(phi0LowerBound, phi0UpperBound, nDiscretePhi0);
  std::pair<DiscreteAngle, DiscreteAngle> phi0Range(discreteAngles.getRange());

  // Setup thre discrete values for the two dimensional curvature
  assert(discreteCurvWidth > discreteCurvOverlap);
  const size_t nCurvBins = std::pow(curvDivisions, maxLevel);
  const size_t nDiscreteCurv = (discreteCurvWidth - discreteCurvOverlap) * nCurvBins + discreteCurvOverlap + 1;

  const double curvOverlap = maxCurv / (nCurvBins * (static_cast<double>(discreteCurvWidth) / discreteCurvOverlap - 1) + 1);
  B2INFO("curvOverlap " << curvOverlap);


  // Since the lower bound is slightly prefered we can bias to high momenta by putting them at the lower bound.
  const double curvLowerBound = +0 - curvOverlap;
  const double curvUpperBound = maxCurv + curvOverlap;

  DiscreteFloatArray discreteCurvs(curvLowerBound, curvUpperBound, nDiscreteCurv);
  std::pair<DiscreteFloat, DiscreteFloat > curvRange(discreteCurvs.getRange());


  // Compose the hough space
  Phi0CurvBox phi0CurvHoughPlain(phi0Range, curvRange);

  using Phi0CurvBoxDivision = LinearDivision<Phi0CurvBox, phi0Divisions, curvDivisions>;
  Phi0CurvBox::Delta phi0CurvOverlaps{discretePhi0Overlap, discreteCurvOverlap};
  Phi0CurvBoxDivision phi0CurvBoxDivision(phi0CurvOverlaps);

  using HitPhi0CurvFastHough =
    WeightedFastHough<TrackHit, Phi0CurvBox, Phi0CurvBoxDivision>;
  HitPhi0CurvFastHough hitPhi0CurvHough(phi0CurvHoughPlain, phi0CurvBoxDivision);

  const bool refined = false;
  HitInPhi0CurvBox<refined> hitInPhi0CurvBox;


  // Get the hits form the test event
  markAllHitsAsUnused();
  std::set<TrackHit*>& hits_set = getHitSet();
  std::vector<TrackHit*> hitVector;

  for (TrackHit* trackHit : hits_set) {
    if (trackHit->getSuperlayerId() % 2 == 0)
      hitVector.push_back(trackHit);
  }

  // Execute the finding a couple of time to find a stable execution time.
  vector< pair<Phi0CurvBox, vector<TrackHit*> > > candidates;

  // Is this still C++? Looks like JavaScript to me.
  TimeItResult timeItResult = timeIt(100, true, [&]() {
    hitPhi0CurvHough.seed(hitVector);

    const double minWeight = 30.0;
    candidates = hitPhi0CurvHough.findHeavyLeavesDisjoint(hitInPhi0CurvBox, maxLevel, minWeight);

    // B2INFO("Execution " << iExecution);
    /// Check if exactly two candidates have been found
    ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

    // Check for the parameters of the track candidates
    // The actual hit numbers are more than 30, but this is somewhat a lower bound
    EXPECT_GE(candidates[0].second.size(), 30);
    EXPECT_GE(candidates[1].second.size(), 30);

    // Exclude the timing of the resource release for comparision with the legendre test.
    hitPhi0CurvHough.fell();
  });

  hitPhi0CurvHough.raze();

  for (std::pair<Phi0CurvBox, std::vector<TrackHit*> >& candidate : candidates) {
    B2INFO("Candidate");
    B2INFO("size " << candidate.second.size());
    B2INFO("Phi " << candidate.first.getLowerBound<0>().getAngle());
    B2INFO("Curv " << static_cast<float>(candidate.first.getLowerBound<1>()));
  }

  B2INFO("First execution took " << timeItResult.getSeconds(0) << " seconds ");
  B2INFO("On average execution took " << timeItResult.getAverageSeconds() << " seconds " <<
         "in " << timeItResult.getNExecutions() << " executions.");

}
