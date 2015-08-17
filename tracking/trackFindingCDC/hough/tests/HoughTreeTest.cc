/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Thomas Hauth <thomas.hauth@kit.edu>        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/test_fixtures/TrackFindingCDCTestWithSimpleSimulation.h>

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>

#include <tracking/trackFindingCDC/hough/perigee/InPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/perigee/SimpleRLTaggedWireHitHoughTree.h>

#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <vector>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;
using namespace PerigeeBinSpec;

namespace {
  TEST_F(TrackFindingCDCTestWithSimpleSimulation, hough_HitPhi0CurvLegendre)
  {
    std::string svgFileName = "phi0_curv_on_prepared_event_hits_using_leaf_processing.svg";
    loadPreparedEvent();
    saveDisplay(svgFileName);

    // Prepare the hough algorithm
    using WireHitPhi0CurvQuadLegendre =
      SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    using Node = typename WireHitPhi0CurvQuadLegendre::Node;
    WireHitPhi0CurvQuadLegendre houghTree(maxLevel, curlCurv);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());

    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());
    houghTree.initialize();

    const double minWeight = 30.0;
    AxialLegendreLeafProcessor<Node> leafProcessor(maxLevel);
    leafProcessor.setMinWeight(minWeight);
    leafProcessor.setMaxCurv(maxCurvAcceptance);

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<CDCTrajectory2D, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_axialWireHits);
      leafProcessor.clear();

      houghTree.findUsing(leafProcessor);
      candidates = leafProcessor.getCandidates();

      ASSERT_EQ(m_mcTracks.size(), candidates.size());
      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 30);
      }
    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2DEBUG(100, "Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    size_t iColor = 0;
    for (std::pair<CDCTrajectory2D, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const CDCTrajectory2D& trajectory2D = candidate.first;
      const std::vector<CDCRLTaggedWireHit >& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Phi0 " << trajectory2D.getGlobalCircle().tangentialPhi());
      B2DEBUG(100, "Curv " << trajectory2D.getCurvature());
      B2DEBUG(100, "Tags of the hits");

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    rl = " << rlTaggedWireHit.getRLInfo() <<
                " dl = " << rlTaggedWireHit->getRefDriftLength());
      }

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
        std::string color = "blue";
        if (rlTaggedWireHit.getRLInfo() == RIGHT) {
          color = "green";
        } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
          color = "red";
        }
        //EventDataPlotter::AttributeMap strokeAttr {{"stroke", color}};
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*wireHit, strokeAttr);
      }
      draw(trajectory2D);
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }

  TEST_F(TrackFindingCDCTestWithSimpleSimulation,  hough_phi0_curv_SimpleHitBasedPhi0CurvHough_onLegendreEvent)
  {
    std::string svgFileName = "phi0_curv_on_prepared_event_hits.svg";
    loadPreparedEvent();
    saveDisplay(svgFileName);

    using RLTaggedWireHitPhi0CurvHough = SimpleRLTaggedWireHitHoughTree<InPhi0CurvBox, phi0Divisions, curvDivisions>;
    using Phi0CurvBox = RLTaggedWireHitPhi0CurvHough::HoughBox;
    RLTaggedWireHitPhi0CurvHough houghTree(maxLevel);

    houghTree.assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(),
                                        phi0BinsSpec.getNOverlap());

    houghTree.assignArray<DiscreteCurv>(curvBinsSpec.constructArray(),
                                        curvBinsSpec.getNOverlap());

    houghTree.initialize();
    const double minWeight = 30.0;

    // Execute the finding a couple of time to find a stable execution time.
    vector< pair<Phi0CurvBox, vector<CDCRLTaggedWireHit> > > candidates;

    // Is this still C++? Looks like JavaScript to me :-).
    TimeItResult timeItResult = timeIt(100, true, [&]() {
      // Exclude the timing of the resource release for comparision with the legendre test.
      houghTree.fell();
      houghTree.seed(m_axialWireHits);

      candidates = houghTree.find(minWeight, maxCurvAcceptance);
      //candidates = houghTree.findBest(minWeight, maxCurvAcceptance);

      ASSERT_EQ(m_mcTracks.size(), candidates.size());
      // Check for the parameters of the track candidates
      // The actual hit numbers are more than 30, but this is somewhat a lower bound
      for (size_t iCand = 0; iCand < candidates.size(); ++iCand) {
        EXPECT_GE(candidates[iCand].second.size(), 30);
      }
    });

    /// Test idiom to output statistics about the tree.
    std::size_t nNodes = houghTree.getTree()->getNNodes();
    B2DEBUG(100, "Tree generated " << nNodes << " nodes");
    houghTree.fell();
    houghTree.raze();

    size_t iColor = 0;
    for (std::pair<Phi0CurvBox, std::vector<CDCRLTaggedWireHit> >& candidate : candidates) {
      const Phi0CurvBox& phi0CurvBox = candidate.first;
      const std::vector<CDCRLTaggedWireHit>& taggedHits = candidate.second;

      B2DEBUG(100, "Candidate");
      B2DEBUG(100, "size " << taggedHits.size());
      B2DEBUG(100, "Phi0 " << phi0CurvBox.getLowerBound<DiscretePhi0>()->phi());
      B2DEBUG(100, "Curv " << phi0CurvBox.getLowerBound<DiscreteCurv>());
      B2DEBUG(100, "Tags of the hits");

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        B2DEBUG(100, "    rl = " << rlTaggedWireHit.getRLInfo() <<
                " dl = " << rlTaggedWireHit->getRefDriftLength());
      }

      for (const CDCRLTaggedWireHit& rlTaggedWireHit : taggedHits) {
        const CDCWireHit* wireHit = rlTaggedWireHit.getWireHit();
        std::string color = "blue";
        if (rlTaggedWireHit.getRLInfo() == RIGHT) {
          color = "green";
        } else if (rlTaggedWireHit.getRLInfo() == LEFT) {
          color = "red";
        }
        //EventDataPlotter::AttributeMap strokeAttr {{"stroke", color}};
        EventDataPlotter::AttributeMap strokeAttr {{"stroke", m_colors[iColor % m_colors.size()] }};
        draw(*wireHit, strokeAttr);
      }
      ++iColor;
    }
    saveDisplay(svgFileName);
    timeItResult.printSummary();
  }
}
