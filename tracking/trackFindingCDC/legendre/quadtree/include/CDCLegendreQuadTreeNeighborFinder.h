/*
 * CDCLegendreQuadTreeNeighborFinder.h
 *
 *  Created on: Apr 15, 2014
 *      Author: vtrusov
 */

#pragma once

#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>

#include <framework/logging/Logger.h>
#include <algorithm>


namespace Belle2 {
  namespace TrackFindingCDC {

    template<typename typeX, typename typeY, class typeData>
    class QuadTreeTemplate;

    class QuadTreeNeighborFinder {
    public:
      typedef QuadTreeTemplate<int, double, TrackHit> QuadTreeLegendre;

      QuadTreeNeighborFinder() {};
      ~QuadTreeNeighborFinder() {};

      /** Get static instance of the class */
      static QuadTreeNeighborFinder& Instance();

      /** Controls process of neighbors finding */
      void controller(QuadTreeLegendre*, QuadTreeLegendre*, QuadTreeLegendre*);

      /** One level up in the tree */
      void levelUp(QuadTreeLegendre*, QuadTreeLegendre*);

      /** One level down in the tree */
      void levelDown(QuadTreeLegendre*, QuadTreeLegendre*);

      /** Trying to find neighbors to node */
      void findNeighbors(QuadTreeLegendre*, QuadTreeLegendre*, QuadTreeLegendre*);

      /** Checks if nodes are neighbors */
      bool compareNodes(QuadTreeLegendre*, QuadTreeLegendre*);

    private:

      static QuadTreeNeighborFinder* s_cdcLegendreQuadTreeNeighborFinder; /**< Static instance of the class */


    };
  }
}
