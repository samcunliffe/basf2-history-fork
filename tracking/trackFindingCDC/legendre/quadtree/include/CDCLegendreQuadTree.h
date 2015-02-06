/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Viktor Trusov, Thomas Hauth                              *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

/*
 * Object which can store pointers to hits while processing FastHogh algorithm
 *
 * TODO: check if it's possible to store in each hit list of nodes in which we can meet it.
 *
 */


#pragma once

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackCandidate.h>


#include "tracking/trackFindingCDC/legendre/quadtree/TrigonometricalLookupTable.h"

#include "tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeNeighborFinder.h"
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTreeCandidateCreator.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreConformalPosition.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>

#include <framework/logging/Logger.h>

#include <boost/math/constants/constants.hpp>
#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility.hpp>
#include <boost/multi_array.hpp>

#include <set>
#include <vector>
#include <array>
#include <algorithm>

#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cmath>
#include <functional>

namespace Belle2 {
  namespace TrackFindingCDC {

    class QuadTreeNeighborFinder;
    class QuadTree;

    class QuadChildren {
    public:
      QuadChildren();

      void apply(std::function<void(QuadTree*)> lmd);

      ~QuadChildren();

      inline void set(const size_t x, const size_t y, QuadTree* qt) {
        m_children[x][y] = qt;
      }

      inline QuadTree* get(const size_t x, const size_t y) {
        return m_children[x][y];
      }
      static constexpr size_t m_sizeX = 2;
      static constexpr size_t m_sizeY = 2;

    private:

      QuadTree* m_children[m_sizeX][m_sizeY];
    };


    class QuadTree {

    public:

      // a lambda expression with this signature is used to process each of the
      // found candidates during the startFillingTree call
      typedef std::function< void(QuadTree*) > CandidateProcessorLambda;

      typedef std::array<float, 3> FloatBinTuple;
      typedef std::array<int, 3> IntBinTuple;
      typedef std::vector<QuadTree*> NodeList;
      //typedef boost::multi_array< QuadTree *, 2> ChildrenArray;

      QuadTree();

      QuadTree(float rMin, float rMax, int thetaMin, int thetaMax, int level, QuadTree* parent);

//    void setParameters(double rMin, double rMax, int thetaMin, int thetaMax, int level, CDCLegendreQuadTree* parent);

      /** Initialize structure and prepare children */
      void initialize();

      void terminate();

      /** Create vector with children of current node */
      void initializeChildren();

      /** Build neighborhood for leafs */
      void buildNeighborhood(int levelNeighborhood);

      /** Returns level of the node in tree (i.e., how much ancestors the node has) */
      inline int getLevel() const {return m_level;};

      /** Sets threshold on number of hits in the node */
      static void setHitsThreshold(unsigned int hitsThreshold) {s_hitsThreshold = hitsThreshold;};

      /** Sets threshold on pt of candidates */
      static void setRThreshold(double rThreshold) {s_rThreshold = static_cast<float>(rThreshold);};

      /** Sets threshold on pt of candidates */
      static void setLastLevel(double lastLevel) {s_lastLevel = static_cast<float>(lastLevel);};

      /** Sets threshold on pt of candidates */
      int getLastLevel() const {return s_lastLevel;};

      /** Copy information about hits into member of class (node at level 0 should be used  because other levels fills by parents) */
      void provideHitSet(const std::set<TrackHit*>& hits_set);

      // legacy interface, remove
      /*      void startFillingTree() {
              CandidateProcessorLambda lmd = [](QuadTree*)->void {};
              startFillingTree(lmd);
            }*/
      /** legacy interface, remove */
      /*      void startFillingTree(bool returnResult, std::vector<QuadTree*>& nodeList) {
              CandidateProcessorLambda lmd = [](QuadTree*)->void {};
              startFillingTree(returnResult, nodeList, lmd);
            }*/

      /** Fill the tree structure */
      void startFillingTree(CandidateProcessorLambda& lmdProcessor);

      /**
       * Fill children of node with hits (according to bin crossing criteria)
       */
      void fillChildren(/*const std::vector<CDCLegendreTrackHit*>& hits*/);

      /** Forced filling of tree, skipping limitation on number of hits
       * Filling nodes which are parents to the current one
       */
      void fillChildrenForced();

      /** Insert hit into node */
      void insertHit(TrackHit* hit) {m_hits.push_back(hit); };

      /** Reserve memory for holding hits */
      void reserveHitsVector(int nHits) {m_hits.reserve(nHits); };

      /** Check if the node passes threshold on number of hits */
      bool checkNode() const {return m_hits.size() >= s_hitsThreshold;};

      /** Get hits from node */
      inline std::vector<TrackHit*>& getHits() {return m_hits;};

      /** Check if the node passes threshold on number of hits */
      inline int getNHits() const {return m_hits.size();};

      /** Removing used or bad hits */
      void cleanHitsInNode() ;

      //TODO: IMPORTANT! check functionality of this method and impact of uncommenting _m_level_ ! */
      /** Check whether node is leaf (lowest node in the tree) */
      bool isLeaf() const {return m_level >= s_lastLevel;};

      /** Check whether node has been processed, i.e. children nodes has been filled */
      inline bool checkFilled() const {return m_filled; };

      /** Set status of node to "filled" (children nodes has been filled) */
      void setFilled() {m_filled = true; };

      /** Get mean value of theta */
      inline double getThetaMean() const {return static_cast<double>((m_thetaMin + m_thetaMax) / 2. * boost::math::constants::pi<double>() / TrigonometricalLookupTable::Instance().getNBinsTheta());};

      /** Get mean value of r */
      inline double getRMean() const {return static_cast<double>((m_rMin + m_rMax) / 2.);};

      /** Get number of bins in "r" direction */
      inline int getRNbins() const {return m_nbins_r;};

      /** Get minimal "r" value of the node */
      inline double getRMin() const {return static_cast<double>(m_rMin);};

      /** Get maximal "r" value of the node */
      inline double getRMax() const {return static_cast<double>(m_rMax);};

      /** Get number of bins in "Theta" direction */
      inline int getThetaNbins() const {return m_nbins_theta;};

      /** Get minimal "Theta" value of the node */
      inline int getThetaMin() const {return m_thetaMin;};

      /** Get maximal "Theta" value of the node */
      inline int getThetaMax() const {return m_thetaMax;};

      /** Return pointer to the parent of the node */
      inline QuadTree* getParent() const {return m_parent;};

      /** Get child of the node by index */
      QuadTree* getChild(int t_index, int r_index) ;

      /** Add pointer to some node to list of neighbors of current node */
      void addNeighbor(QuadTree* node) {m_neighbors.push_back(node);};

      /** Get number of neighbors of the current node (used mostly for debugging purposes) */
      int getNneighbors() const {return m_neighbors.size();};

      /** Check whether neighbors of the node has been found */
      inline bool isNeighborsDefined() const {return m_neighborsDefined;};

      /** Find and store neighbors of the node */
      void findNeighbors();

      /** Get list of neighbors of the current node */
      std::vector<QuadTree*>& getNeighborsVector();

      /** Clear hits which the node holds */
      void clearNode() {m_hits.clear(); };

      /** Clear hits which the node holds and destroy all children below this node.
       * This method must only be called on the root node, for fast QuadTree reusage */
      void clearTree();

    private:

      /** apply a lambda expression to all children of this tree node */
      void applyToChildren(std::function<void(QuadTree*)>);

      /** Check if we reach limitation on dr and dtheta; returns true when reached limit */
      bool checkLimitsR();

      static unsigned int s_hitsThreshold;
      static float s_rThreshold; /**< Threshold on r variable; allows to set threshold on pt of tracks */
      static int s_lastLevel;

      float m_rMin; /**< Lower border of the node (R) */
      float m_rMax; /**< Upper border of the node (R) */
      int m_thetaMin;  /**< Lower border of the node (Theta) */
      int m_thetaMax;  /**< Upper border of the node (Theta) */
      int m_level;  /**< Level of node in the tree */

      std::vector<TrackHit*> m_hits;  /**< Vector of hits which belongs to the node */

      QuadTree* m_parent;  /**< Pointer to the parent node */

      std::vector<QuadTree*> m_neighbors; /**< 8 neighbours of each node (or 5 at borders) */

      /** Pointers to the children nodes */
      std::unique_ptr< QuadChildren > m_children;

      FloatBinTuple m_r;          /**< bins range on r */
      IntBinTuple m_thetaBin;      /**< bins range on theta */
      int m_nbins_r;        /**< number of r bins */
      int m_nbins_theta;    /**< number of theta bins */
      bool m_filled; /**< Is the node has been filled with hits */
      bool m_neighborsDefined; /**< Checks whether neighbors of current node has been defined */

    };
  }
}
