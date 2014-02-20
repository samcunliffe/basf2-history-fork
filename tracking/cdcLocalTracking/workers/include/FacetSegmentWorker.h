/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FACETSEGMENTWORKER_H_
#define FACETSEGMENTWORKER_H_

#include <vector>

#include <tracking/cdcLocalTracking/typedefs/UsedDataHolders.h>
#include <framework/datastore/StoreArray.h>

#include <tracking/cdcLocalTracking/algorithms/WeightedNeighborhood.h>
#include <tracking/cdcLocalTracking/algorithms/NeighborhoodBuilder.h>
#include <tracking/cdcLocalTracking/neighbor_chooser/WireHitNeighborChooser.h>

#include <tracking/cdcLocalTracking/algorithms/MultipassCellularPathFinder.h>
#include <tracking/cdcLocalTracking/algorithms/Clusterizer.h>

#include <tracking/cdcLocalTracking/creators/FacetCreator.h>
#include <tracking/cdcLocalTracking/creators/RecoSegmentCreator.h>
#include <tracking/cdcLocalTracking/creators/TangentSegmentCreator.h>

#include <tracking/cdcLocalTracking/creators/SegmentSelecter.h>
#include <tracking/cdcLocalTracking/creators/SegmentReverser.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    // Worker for building reconstructed segments form wirehits using reconstructed facets
    template<class FacetFilter, class FacetNeighborChooser>
    class FacetSegmentWorker {

    public:

      /** Constructor. */
      FacetSegmentWorker(): m_cellularPathFinder(3.0) {;}

      /** Destructor.*/
      ~FacetSegmentWorker() {;}

      void init() {

#ifdef CDCLOCALTRACKING_USE_ROOT
        StoreArray < CDCRecoTangentCollection >::registerTransient("CDCRecoTangentSegments");
        StoreArray < CDCRecoSegment2D >::registerTransient("CDCRecoHit2DSegmentsSelected");
        StoreArray < CDCWireHitCluster >::registerTransient("CDCWireHitClusters");
#endif

        m_clusters.reserve(50);

      }


      template<class CDCWireHitRange>
      inline void apply(const CDCWireHitRange& wirehits,
                        std::vector< CDCRecoSegment2D >& outputSegments) {

        m_segments2D.clear();
        outputSegments.clear();

#ifdef CDCLOCALTRACKING_USE_ROOT
        m_recoTangentSegments.clear();
#endif

        //create the neighborhood
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        m_wirehitNeighborhood.clear();
        B2DEBUG(100, "  Append clockwise neighborhood");
        m_clockwise_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append clockwise out neighborhood");
        m_clockwiseOut_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append clockwise in neighborhood");
        m_clockwiseIn_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise neighborhood");
        m_counterClockwise_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise out neighborhood");
        m_counterClockwiseOut_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);
        B2DEBUG(100, "  Append counter clockwise in neighborhood");
        m_counterClockwiseIn_neighborhoodBuilder.append(wirehits, m_wirehitNeighborhood);

        bool isSymmetric = m_wirehitNeighborhood.isSymmetric();
        B2DEBUG(100, "  Check symmetry " << isSymmetric);
        B2DEBUG(100, "  allWirehitNeighbors.size() = " << m_wirehitNeighborhood.size());
        if (not isSymmetric) { B2WARNING("  The wire neighborhood is not symmetric"); }

        //create the clusters
        B2DEBUG(100, "Creating the CDCWireHit clusters");
        m_clusters.clear();
        m_wirehitClusterizer.create(wirehits, m_wirehitNeighborhood, m_clusters);

        B2DEBUG(100, "Created " << m_clusters.size() << " CDCWireHit clusters");

        //double d;
        //std::cin >> d;

        for (CDCWireHitCluster & cluster : m_clusters) {
          //size_t nSegmentsBefore = m_segments2D.size();

          //create the facets
          B2DEBUG(100, "Creating the CDCRecoFacets");
          m_facets.clear();
          m_facetCreator.createFacets(cluster, m_wirehitNeighborhood , m_facets);
          B2DEBUG(100, "  Created " << m_facets.size()  << " CDCRecoFacets");

          //create the facet neighborhood
          B2DEBUG(100, "Creating the CDCRecoFacet neighborhood");
          m_facetsNeighborhood.clear();
          m_facetNeighborhoodBuilder.create(m_facets, m_facetsNeighborhood);
          B2DEBUG(100, "  Created " << m_facetsNeighborhood.size()  << " FacetsNeighborhoods");

          //Apply the cellular automaton in a multipass manner
          m_facetSegments.clear();
          m_cellularPathFinder.apply(m_facets, m_facetsNeighborhood, m_facetSegments);

          //save the tangents for display only
#ifdef CDCLOCALTRACKING_USE_ROOT
          B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
          m_tangentSegmentCreator.create(m_facetSegments, m_recoTangentSegments);
#endif

          // reduce the CDCRecoFacetPtrSegment directly to the selected vector
          B2DEBUG(100, "Reduce the CDCRecoFacetPtrSegment to RecoSegment2D");
          m_segments2D.reserve(m_segments2D.size() + m_facetSegments.size());
          m_recoSegmentCreator.create(m_facetSegments, m_segments2D);

          //TODO: combine matching segments here

          //size_t nSegmentsAfter = m_segments2D.size();
          B2DEBUG(100, "  Created " << m_segments2D.size()  << " selected CDCRecoSegment2Ds");
        } // end for cluster

        //make both orientations available
        B2DEBUG(100, "Reversing CDCReco2DSegments");
        m_segmentReverser.appendReversed(m_segments2D);
        B2DEBUG(100, "  Created " << m_segments2D.size()  << " selected CDCRecoSegment2Ds after reversion");

        //TODO: combine matching segments or here


        copyToDataStoreForDebug();

        outputSegments.swap(m_segments2D);
      }

    private:
      void copyToDataStoreForDebug() const {


        // IO for monitoring in python
#ifdef CDCLOCALTRACKING_USE_ROOT
        // IO wire hit clusters

        B2DEBUG(100, "  Creating the StoreArray for the CDCWireHitClusters");
        StoreArray < CDCWireHitCluster > storedClusters("CDCWireHitClusters");
        storedClusters.create();
        B2DEBUG(100, "  Do creating the CDCWireHitCluster in the StoreArray");
        for (const CDCWireHitCluster & cluster : m_clusters) {
          storedClusters.appendNew(cluster);
        }
        B2DEBUG(100, "  Created " << storedClusters.getEntries()  << " CDCWireHitClusters");

        // IO segments with tangents
        B2DEBUG(100, "  Creating the StoreArray for the CDCRecoTangentSegment");
        StoreArray < CDCRecoTangentCollection > storedTangentSegments("CDCRecoTangentSegments");
        storedTangentSegments.create();
        B2DEBUG(100, "  Do creating the CDCRecoTangentSegment in the StoreArray");
        for (const CDCRecoTangentCollection & tangentSegment : m_recoTangentSegments) {
          storedTangentSegments.appendNew(tangentSegment);
        }
        B2DEBUG(100, "  Created " << storedTangentSegments.getEntries()  << " CDCRecoTangentSegment");

        // IO selected segments without tangents
        B2DEBUG(100, "  Creating the StoreArray for the selected CDCRecoHit2DSegments");
        StoreArray < CDCRecoSegment2D > storedSegments2D("CDCRecoHit2DSegmentsSelected");
        storedSegments2D.create();
        B2DEBUG(100, "  Copying the selected CDCRecoHit2DSegments to the StoreArray");
        for (const CDCRecoSegment2D & segment2D : m_segments2D) {
          storedSegments2D.appendNew(segment2D);
        }
        B2DEBUG(100, "  Created " << storedSegments2D.getEntries()  <<
                " selected CDCRecoSegment2D");
#endif

      }


    private:
      //typedefs

    private:
      //object pools
      typedef WeightedNeighborhood<const CDCWireHit> CDCWireHitNeighborhood;
      CDCWireHitNeighborhood m_wirehitNeighborhood;

      CDCRecoFacetCollection m_facets;

      typedef WeightedNeighborhood<const CDCRecoFacet> CDCRecoFacetNeighborhood;
      CDCRecoFacetNeighborhood m_facetsNeighborhood;

      std::vector< CDCRecoFacetPtrSegment > m_facetSegments;

#ifdef CDCLOCALTRACKING_USE_ROOT
      std::vector< CDCRecoTangentCollection > m_recoTangentSegments;
#endif

      std::vector<CDCRecoSegment2D> m_segments2D;

      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;
      std::vector<CDCWireHitCluster> m_clusters;

      //object creators
      FacetCreator<FacetFilter> m_facetCreator;

      //neighborhood builders
      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_OUT_NEIGHBOR> >
      m_clockwiseOut_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_NEIGHBOR> >
      m_clockwise_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CW_IN_NEIGHBOR> >
      m_clockwiseIn_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_OUT_NEIGHBOR> >
      m_counterClockwiseOut_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_NEIGHBOR> >
      m_counterClockwise_neighborhoodBuilder;

      NeighborhoodBuilder<CDCWireHit, WireHitNeighborChooser<CCW_IN_NEIGHBOR> >
      m_counterClockwiseIn_neighborhoodBuilder;

      NeighborhoodBuilder<CDCRecoFacet, FacetNeighborChooser>
      m_facetNeighborhoodBuilder;

      //cellular automat
      MultipassCellularPathFinder<CDCRecoFacet> m_cellularPathFinder;

      RecoSegmentCreator m_recoSegmentCreator;
      TangentSegmentCreator m_tangentSegmentCreator;

      SegmentSelecter m_segmentSelecter;
      SegmentReverser m_segmentReverser;

    }; // end class FacetSegmentWorker
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //FACETSEGMENTWORKER_H_
