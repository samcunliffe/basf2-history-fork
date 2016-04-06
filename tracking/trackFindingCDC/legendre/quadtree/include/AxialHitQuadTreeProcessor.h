/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth, Nils Braun                  *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/


#pragma once
#include <tracking/trackFindingCDC/legendre/quadtree/QuadTreeProcessorTemplate.h>
#include <tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/hits/ConformalCDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit2D.h>


#include <TF1.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TAxis.h>
#include <TLine.h>

#include <array>
#include <utility>

namespace Belle2 {
  namespace TrackFindingCDC {
    class ConformalCDCWireHit;

    /** A QuadTreeProcessor for TrackHits */
    class AxialHitQuadTreeProcessor : public QuadTreeProcessorTemplate<unsigned long, float, ConformalCDCWireHit, 2, 2> {

    public:

      AxialHitQuadTreeProcessor(unsigned char lastLevel, const ChildRanges& ranges, std::function< double(double) >& lmdFunctLevel,
                                bool standartBinning = false) :
        QuadTreeProcessorTemplate(lastLevel, ranges), m_standartBinning(standartBinning), m_lmdFunctLevel(lmdFunctLevel) { }

    private:

      bool m_standartBinning;

      std::function< double(double) >& m_lmdFunctLevel;

      inline bool sameSign(double n1, double n2, double n3, double n4) const
      {return ((n1 > 0 && n2 > 0 && n3 > 0 && n4 > 0) || (n1 < 0 && n2 < 0 && n3 < 0 && n4 < 0));};

      std::vector<QuadTree> m_seededTree;

    public:

      /**
       * lastLevel depends on curvature of the track candidate
       */
      bool checkIfLastLevel(QuadTree* node) override final
      {
        if (node->getLevel() <= 6) return false;
        if (node->getLevel() >= getLastLevel()) return true;

        double nodeResolution = fabs(node->getYMin() - node->getYMax());

        double resolution = m_lmdFunctLevel(node->getYMean());
        if (resolution >= nodeResolution) return true;

        return false;
      }

      /**
       * Do only insert the hit into a node if sinogram calculated from this hit belongs into this node
       */
      inline bool insertItemInNode(QuadTree* node, ConformalCDCWireHit* hit, unsigned int /*t_index*/,
                                   unsigned int /*r_index*/) const override final
      {
        typedef std::array< std::array<float, 2>, 2> Quadlet;
        Quadlet dist_1;
        Quadlet dist_2;

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();

        unsigned long thetaMin = node->getXMin();
        unsigned long thetaMax = node->getXMax();
        float rMin = node->getYMin();
        float rMax = node->getYMax();
        float cosThetaMin = trigonometricalLookupTable.cosTheta(thetaMin);
        float sinThetaMin = trigonometricalLookupTable.sinTheta(thetaMin);
        float cosThetaMax = trigonometricalLookupTable.cosTheta(thetaMax);
        float sinThetaMax = trigonometricalLookupTable.sinTheta(thetaMax);

        float rHitMin = hit->getConformalX() * cosThetaMin + hit->getConformalY() * sinThetaMin;
        float rHitMax = hit->getConformalX() * cosThetaMax + hit->getConformalY() * sinThetaMax;

        float rHitMin1 = rHitMin - hit->getConformalDriftLength();
        float rHitMin2 = rHitMin + hit->getConformalDriftLength();
        float rHitMax1 = rHitMax - hit->getConformalDriftLength();
        float rHitMax2 = rHitMax + hit->getConformalDriftLength();

        // this has some explicit cppcheck suppressions to silence false-positives about
        // array bounds
        dist_1[0][0] = rMin - rHitMin1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[0][1] = rMin - rHitMax1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[1][0] = rMax - rHitMin1;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_1[1][1] = rMax - rHitMax1;

        dist_2[0][0] = rMin - rHitMin2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[0][1] = rMin - rHitMax2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[1][0] = rMax - rHitMin2;
        // cppcheck-suppress arrayIndexOutOfBounds
        dist_2[1][1] = rMax - rHitMax2;

        bool valueToReturn(false);

        // cppcheck-suppress arrayIndexOutOfBounds
        if (! sameSign(dist_1[0][0], dist_1[0][1], dist_1[1][0], dist_1[1][1])) {
          valueToReturn = true;
        }
        // cppcheck-suppress arrayIndexOutOfBounds
        else if (! sameSign(dist_2[0][0], dist_2[0][1], dist_2[1][0], dist_2[1][1])) {
          valueToReturn = true;
        } else {

          float rHitMinExtr = -1.*hit->getConformalX() * sinThetaMin + hit->getConformalY() * cosThetaMin;
          float rHitMaxExtr = -1.*hit->getConformalX() * sinThetaMax + hit->getConformalY() * cosThetaMax;

          if (rHitMinExtr * rHitMaxExtr < 0.) valueToReturn = checkExtremum(node, hit);
        }

        return valueToReturn;
      }

      /**
       * Return the new ranges. We do not use the standard ranges for the lower levels.
       */
      ChildRanges createChildWithParent(QuadTree* node, unsigned int i, unsigned int j) const override final
      {
        if (not m_standartBinning) {
          if ((node->getLevel() > (getLastLevel() - 7)) && (fabs(node->getYMean()) > 0.005)) {
            if (node->getLevel() < (getLastLevel() - 5)) {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 4.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 4.;
              unsigned long extension = static_cast<unsigned long>(pow(2, getLastLevel() - node->getLevel()) / 4);

              unsigned long theta1 = node->getXBin(i);
              if (theta1 <= extension) theta1 = 0;
              else theta1 -= extension;

              unsigned long theta2 = node->getXBin(i + 1) + extension;
              if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta())
                theta2 = node->getXBin(i + 1);

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            } else {
              float r1 = node->getYBin(j) - fabs(node->getYBin(j + 1) - node->getYBin(j)) / 8.;
              float r2 = node->getYBin(j + 1) + fabs(node->getYBin(j + 1) - node->getYBin(j))  / 8.;
              unsigned long extension = static_cast<unsigned long>(pow(2, getLastLevel() - node->getLevel()) / 8);

              unsigned long theta1 = node->getXBin(i);
              if (theta1 <= extension) theta1 = 0;
              else theta1 -= extension;

              unsigned long theta2 = node->getXBin(i + 1) + extension;
              if (theta2 >= TrigonometricalLookupTable<>::Instance().getNBinsTheta())
                theta2 = node->getXBin(i + 1);

              return ChildRanges(rangeX(theta1, theta2), rangeY(r1, r2));
            }
          } else {
            return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
          }
        } else {
          return ChildRanges(rangeX(node->getXBin(i), node->getXBin(i + 1)), rangeY(node->getYBin(j), node->getYBin(j + 1)));
        }
      }


      void seedQuadTree(int lvl)
      {
        bool twoSidedPhasespace(false);

        if ((m_quadTree->getYMin() * m_quadTree->getYMax()) < 0)twoSidedPhasespace = true;

        unsigned long nbins = pow(2, lvl);

        m_seededTree.reserve(nbins * nbins);

        ChildRanges ranges = std::make_pair(rangeX(m_quadTree->getXMin(), m_quadTree->getXMax()),
                                            rangeY(m_quadTree->getYMin(), m_quadTree->getYMax()));


        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;

        unsigned long binSizeX = (x.second - x.first) / nbins;
        float binSizeY = (y.second - y.first) / nbins;

        std::vector<ItemType*> items = m_quadTree->getItemsVector();

        for (unsigned long xIndex = 0; xIndex < nbins; xIndex++) {
          for (unsigned long yIndex = 0; yIndex < nbins; yIndex++) {
            unsigned long xMin = xIndex * binSizeX + x.first;
            unsigned long xMax = (xIndex + 1) * binSizeX + x.first;
            float yMin = yIndex * binSizeY + y.first;
            float yMax = (yIndex + 1) * binSizeY + y.first;

            m_seededTree.emplace_back(xMin, xMax, yMin, yMax, lvl, nullptr);
            QuadTree& newQuadTree = m_seededTree.back();
            if ((newQuadTree.getYMin() < -0.02)  && (newQuadTree.getYMax() < -0.02)) continue;
            newQuadTree.reserveHitsVector(m_quadTree->getNItems() * 2);

            for (ItemType* item : items) {
              if (item->isUsed()) continue;

              if (insertItemInNode(&newQuadTree, item->getPointer(), 0, 0)) {
                if (twoSidedPhasespace && (newQuadTree.getYMin() < 0.02)  && (newQuadTree.getYMax() < 0.02)) {
                  if (checkDerivative(&newQuadTree, item->getPointer()))
                    newQuadTree.insertItem(item);
                } else {
                  newQuadTree.insertItem(item);
                }
              }
            }

            if (newQuadTree.getNItems() < 10) m_seededTree.pop_back();

          }
        }

        sortSeededTree();

      }


      void sortSeededTree()
      {
        std::sort(m_seededTree.begin(), m_seededTree.end(), [](QuadTree & quadTree1, QuadTree & quadTree2) { return quadTree1.getNItems() > quadTree2.getNItems();});
      }

      void fillSeededTree(CandidateProcessorLambda& lmdProcessor,
                          unsigned int nHitsThreshold, float rThreshold)
      {
        sortSeededTree();
//        B2INFO("Size of the tree : " << m_seededTree.size());

        for (QuadTree& tree : m_seededTree) {
//          B2INFO("NItems in the node: " << tree.getNItems());
//          fillGivenTreeWithSegments(&tree, lmdProcessor, nHitsThreshold, rThreshold, true);
          tree.cleanUpItems(*this);
          fillGivenTree(&tree, lmdProcessor, nHitsThreshold, rThreshold, true);
        }

      }


      void clearSeededTree()
      {
        m_seededTree.clear();
        m_seededTree.resize(0);
      }

      QuadTree* createSingleNode(const ChildRanges& ranges)
      {
        std::vector<ItemType*> hitsVector = m_quadTree->getItemsVector();

        const rangeX& x = ranges.first;
        const rangeY& y = ranges.second;
        QuadTree* quadTree = new QuadTree(x.first, x.second, y.first, y.second, 0, nullptr);

        std::vector<ItemType*>& quadtreeItemsVector = quadTree->getItemsVector();
//        quadtreeItemsVector.reserve(hitsVector.size());
        for (ItemType* item : hitsVector) {
          if (item->isUsed()) continue;
          if (insertItemInNode(quadTree, item->getPointer(), 0, 0))
            quadtreeItemsVector.push_back(item);
        }

        return quadTree;
      }


      void drawNode()
      {
        static int nevent(0);

        TCanvas* canv = new TCanvas("canv", "legendre transform", 0, 0, 1200, 600);
        canv->cd(1);
        TGraph* dummyGraph = new TGraph();
        dummyGraph->SetPoint(1, -3.1415, 0);
        dummyGraph->SetPoint(2, 3.1415, 0);
        dummyGraph->Draw("AP");
        dummyGraph->GetXaxis()->SetTitle("#theta");
        dummyGraph->GetYaxis()->SetTitle("#rho");
        dummyGraph->GetXaxis()->SetRangeUser(-3.1415, 3.1415);
        dummyGraph->GetYaxis()->SetRangeUser(0, 0.15);


        //    int nhits = 0;
        for (ItemType* hit : m_quadTree->getItemsVector()) {
          TF1* funct1 = new TF1("funct", "2*[0]*cos(x)/((1-sin(x))*[1]) ", -3.1415, 3.1415);
          funct1->SetLineWidth(1);
          double r2 = (hit->getPointer()->getCDCWireHit()->getRefPos2D().norm() + hit->getPointer()->getCDCWireHit()->getRefDriftLength()) *
                      (hit->getPointer()->getCDCWireHit()->getRefPos2D().norm() - hit->getPointer()->getCDCWireHit()->getRefDriftLength());
          double d2 = hit->getPointer()->getCDCWireHit()->getRefDriftLength() * hit->getPointer()->getCDCWireHit()->getRefDriftLength();
          double x = hit->getPointer()->getCDCWireHit()->getRefPos2D().x();

          funct1->SetParameters(x, r2 - d2);
          funct1->Draw("CSAME");

        }
        canv->Print(Form("legendreHits_%i.root", nevent));
        canv->Print(Form("legendreHits_%i.eps", nevent));
        canv->Print(Form("legendreHits_%i.png", nevent));


        nevent++;
      }

      bool checkDerivative(QuadTree* node, ConformalCDCWireHit* hit) const
      {
        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();

        float rMinD = -1.*hit->getConformalX() * trigonometricalLookupTable.sinTheta(node->getXMin())
                      + hit->getConformalY() * trigonometricalLookupTable.cosTheta(node->getXMin());

        float rMaxD = -1.*hit->getConformalX() * trigonometricalLookupTable.sinTheta(node->getXMax())
                      + hit->getConformalY() * trigonometricalLookupTable.cosTheta(node->getXMax());


//        float rMean = node->getYMean();

        if ((rMinD > 0) && (rMaxD * rMinD >= 0)) return true;
        if ((rMaxD * rMinD < 0)) return true;
        return false;


      }

      bool checkExtremum(QuadTree* node, ConformalCDCWireHit* hit) const
      {

        double thetaExtremum = atan2(hit->getConformalY(), hit->getConformalX());

        double pi = boost::math::constants::pi<double>();

        //        if (thetaExtremum > pi) thetaExtremum -= pi;
        //        if (thetaExtremum < 0.) thetaExtremum += pi;

        TrigonometricalLookupTable<>& trigonometricalLookupTable = TrigonometricalLookupTable<>::Instance();

        unsigned long thetaExtremumLookup = (thetaExtremum + pi) * trigonometricalLookupTable.getNBinsTheta() / (2.*pi) ;

        if ((thetaExtremumLookup > node->getXMax()) || (thetaExtremumLookup < node->getXMin())) return false;

        double rD = hit->getConformalX() * trigonometricalLookupTable.cosTheta(thetaExtremumLookup)
                    + hit->getConformalY() * trigonometricalLookupTable.sinTheta(thetaExtremumLookup);

        if ((rD > node->getYMin()) && (rD < node->getYMax())) return true;


        return false;
      }

    };

  }
}
