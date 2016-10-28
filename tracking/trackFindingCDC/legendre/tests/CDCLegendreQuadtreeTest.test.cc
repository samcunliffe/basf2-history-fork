/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth <thomas.hauth@kit.edu>                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/legendre/testFixtures/CDCLegendreTestFixture.h>

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/quadtree/AxialHitQuadTreeProcessor.h>
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeItem.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/OriginPrecisionFunction.h>
#include <tracking/trackFindingCDC/legendre/precisionFunctions/NonOriginPrecisionFunction.h>

#include <set>
#include <cmath>
#include <vector>
#include <memory>
#include <chrono>
#include <algorithm>
#include <gtest/gtest.h>


using namespace Belle2;
using namespace TrackFindingCDC;

TEST_F(CDCLegendreTestFixture, legendre_QuadTreeTest)
{
  //high-pt candidate
  AxialHitQuadTreeProcessor::ChildRanges ranges1(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2,
                                                 BasePrecisionFunction::getLookupGridLevel())),
                                                 AxialHitQuadTreeProcessor::rangeY(0., 0.15));
  OriginPrecisionFunction originPrecisionFunction;
  BasePrecisionFunction::PrecisionFunction highPtPrecisionFunction = originPrecisionFunction.getFunction();

  //low-pt candidate
  AxialHitQuadTreeProcessor::ChildRanges ranges2(AxialHitQuadTreeProcessor::rangeX(0, std::pow(2,
                                                 BasePrecisionFunction::getLookupGridLevel())),
                                                 AxialHitQuadTreeProcessor::rangeY(0., 0.30));
  NonOriginPrecisionFunction nonOriginPrecisionFunction;
  BasePrecisionFunction::PrecisionFunction lowPtPrecisionFunction = nonOriginPrecisionFunction.getFunction();


  std::vector<AxialHitQuadTreeProcessor::ReturnList> candidates;

  markAllHitsAsUnused();
  std::vector<CDCConformalHit*>& hitsVector = getHitVector();

  AxialHitQuadTreeProcessor::CandidateProcessorLambda lmdProcessor = [&candidates](const AxialHitQuadTreeProcessor::ReturnList & hits,
  AxialHitQuadTreeProcessor::QuadTree*) {
    candidates.push_back(hits);
  };



  auto now = std::chrono::high_resolution_clock::now();
  AxialHitQuadTreeProcessor qtProcessor1(13, ranges1, highPtPrecisionFunction);
  qtProcessor1.provideItemsSet(hitsVector);

  // actual filling of the hits into the quad tree structure
  qtProcessor1.fillGivenTree(lmdProcessor, 30);

  AxialHitQuadTreeProcessor qtProcessor2(11, ranges2, lowPtPrecisionFunction);
  qtProcessor2.provideItemsSet(hitsVector);

  // actual filling of the hits into the quad tree structure
  qtProcessor2.fillGivenTree(lmdProcessor, 30);
  auto later = std::chrono::high_resolution_clock::now();

  ASSERT_EQ(numberOfPossibleTrackCandidate, candidates.size());

  std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(later - now);
  B2INFO("QuadTree took " << time_span.count() << " seconds, found " << candidates.size() << " candidates");

  // Check for the parameters of the track candidates
  // The actual hit numbers are more than 30, but this is somewhat a lower bound
  EXPECT_GE(candidates[0].size(), 30);
  EXPECT_GE(candidates[1].size(), 30);
}

