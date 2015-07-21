/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/hough/phi0_curv/HitInPhi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/WeightedFastHoughTree.h>
#include <tracking/trackFindingCDC/hough/LinearDivision.h>
#include <tracking/trackFindingCDC/hough/DiscreteAngles.h>

#include <cmath>
#include <vector>
#include <memory>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HitPtr,
             size_t phi0Divisions = 2, // Division at each level
             size_t curvDivisions = 2> // Division at each level
    class HitPhi0CurvLegendre {

    public:
      /// Type of the box division strategy
      using Phi0CurvBoxDivision = LinearDivision<Phi0CurvBox, phi0Divisions, curvDivisions>;

      /// Type of the fast hough tree structure
      using HitPhi0CurvFastHoughTree = WeightedFastHoughTree<HitPtr, Phi0CurvBox, Phi0CurvBoxDivision>;

      /// Type of the nodes used in the tree for the search.
      using Node = typename HitPhi0CurvFastHoughTree::Node;

    public:
      /// Constructor using the default values
      HitPhi0CurvLegendre()
      {;}

      /// Constructor using the given maximal level.
      HitPhi0CurvLegendre(size_t maxLevel) : m_maxLevel(maxLevel)
      {;}

    public:
      /// Initialise the algorithm by constructing the hough tree from the parameters
      void initialize()
      {
        // Setup the discrete values for phi0
        const size_t nPhi0Bins = std::pow(phi0Divisions, m_maxLevel);
        m_discretePhi0s = DiscreteAngleArray::forBinsWithOverlaps(nPhi0Bins,
                                                                  m_discretePhi0Width,
                                                                  m_discretePhi0Overlap);

        // Setup the discrete values for the two dimensional curvature
        assert(m_discreteCurvWidth > m_discreteCurvOverlap);
        const size_t nCurvBins = std::pow(curvDivisions, m_maxLevel);
        m_discreteCurvs =
          DiscreteCurvatureArray::forPositiveCurvatureBinsWithOverlap(m_maxCurv,
              nCurvBins,
              m_discreteCurvWidth,
              m_discreteCurvOverlap);

        // Compose the hough space
        std::pair<DiscreteAngle, DiscreteAngle> phi0Range(m_discretePhi0s.getRange());
        std::pair<DiscreteCurvature, DiscreteCurvature > curvRange(m_discreteCurvs.getRange());
        m_phi0CurvHoughPlain = Phi0CurvBox(phi0Range, curvRange);

        Phi0CurvBox::Delta phi0CurvOverlaps{m_discretePhi0Overlap, m_discreteCurvOverlap};
        Phi0CurvBoxDivision phi0CurvBoxDivision(phi0CurvOverlaps);

        m_hitPhi0CurvFastHoughTree.reset(new HitPhi0CurvFastHoughTree(m_phi0CurvHoughPlain,
                                         phi0CurvBoxDivision));


        B2INFO("Initialize hough plan");

        B2INFO("First curvature bin should be symmetric around zero");
        B2INFO("First curvature bin lower bound " << m_discreteCurvs[0].getValue());
        B2INFO("First curvature bin upper bound " << m_discreteCurvs[m_discreteCurvWidth].getValue());
        B2INFO("Curvature bin width " << m_discreteCurvs[m_discreteCurvWidth].getValue() - m_discreteCurvs[0].getValue());
        B2INFO("Curvature bin overlap " << m_discreteCurvs[m_discreteCurvOverlap].getValue() - m_discreteCurvs[0].getValue());

        double phi0Width = m_discretePhi0s[m_discretePhi0Width].getAngleVec().phi() - m_discretePhi0s[0].getAngleVec().phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Width = std::remainder(phi0Width, 2 * PI);

        double phi0Overlap = m_discretePhi0s[m_discretePhi0Overlap].getAngleVec().phi() - m_discretePhi0s[0].getAngleVec().phi();
        // need modulo 2 * Pi because of the wrap around of the lowest bin.
        phi0Overlap = std::remainder(phi0Overlap, 2 * PI);

        B2INFO("Phi0 bin width " << phi0Width);
        B2INFO("Phi0 bin overlap " << phi0Overlap);
      }

      /// Prepare the leave finding by filling the top node with given hits
      template<class HitPtrs>
      void seed(HitPtrs& hits)
      {
        if (not m_hitPhi0CurvFastHoughTree) { initialize(); }
        m_hitPhi0CurvFastHoughTree->seed(hits);
      }

      /// Find disjoint leaves heavier than minWeight
      std::vector<std::pair<Phi0CurvBox,  std::vector<HitPtr> > >
      find(const Weight& minWeight, const double& maxCurv = NAN)
      {
        typedef typename HitPhi0CurvFastHoughTree::Node Node;

        const bool refined = false;
        HitInPhi0CurvBox<refined> hitInPhi0CurvBox;

        if (not std::isnan(maxCurv)) {
          auto skipHighCurvatureAndLowWeightNode = [minWeight, maxCurv](const Node * node) {
            return (not(node->getWeight() >= minWeight) or
                    static_cast<float>(node->template getLowerBound<1>()) > maxCurv);
          };
          return m_hitPhi0CurvFastHoughTree->findLeavesDisjoint(hitInPhi0CurvBox, m_maxLevel, skipHighCurvatureAndLowWeightNode);

        } else {
          auto skipLowWeightNode = [minWeight](const Node * node) {
            return not(node->getWeight() >= minWeight);
          };
          return m_hitPhi0CurvFastHoughTree->findLeavesDisjoint(hitInPhi0CurvBox, m_maxLevel, skipLowWeightNode);
        }
      }

      /// Terminates the processing by striping all hit information from the tree
      void fell()
      { m_hitPhi0CurvFastHoughTree->fell(); }

      /// Release all memory that the tree aquired during the runs.
      void raze()
      { m_hitPhi0CurvFastHoughTree->raze(); }

    public:
      /// Getter for the tree used in the search in the hough plane.
      HitPhi0CurvFastHoughTree* getTree() const
      {
        return m_hitPhi0CurvFastHoughTree.get();
      }

    private:
      // Default parameters
      // TODO: Cross check this values with old implementation, room further tuning?
      // TODO: Expose these parameters to the constructor?

      /// Maximal level of the tree to investigate
      size_t m_maxLevel = 13;

      /// Overlap of the leaves in phi0 counted in number of discrete values.
      size_t m_discretePhi0Overlap = 2;

      /// Width of the leaves at the maximal level in phi0 counted in number of discrete values.
      size_t m_discretePhi0Width = 3;

      /// Overlap of the leaves in the curvature counted in number of discrete values
      size_t m_discreteCurvOverlap = 4;

      /// Width of the leaves at the maximal level in teh curvature counted in number of discrete values.
      size_t m_discreteCurvWidth = 5;

      /// Maximal curvature value the tree should cover.
      double m_maxCurv = 2.75;

      // Dummy initialisation of the other constructs

      /// Space for the discrete values that mark the usable bin bound in phi0
      DiscreteAngleArray m_discretePhi0s{NAN, NAN, 1};

      /// Space for the discrete values that mark the usable bin bound in the curvature
      DiscreteCurvatureArray m_discreteCurvs{NAN, NAN, 1};

      /// The top level hough plain
      Phi0CurvBox m_phi0CurvHoughPlain{m_discretePhi0s.getRange(), m_discreteCurvs.getRange()};

      /// Dynamic hough tree structure traversed in the leave search.
      std::unique_ptr<HitPhi0CurvFastHoughTree> m_hitPhi0CurvFastHoughTree{nullptr};

    };
  }
}
