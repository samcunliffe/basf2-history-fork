/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentMerger.h>

#include <tracking/trackFindingCDC/findlets/minimal/SegmentFitter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentOrienter.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentExporter.h>

#include <tracking/trackFindingCDC/findlets/base/StoreVectorSwapper.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Legacy : Findlet implementing the segment finding part of the full track finder
    template<class AClusterFilter,
             class AFacetFilter,
             class AFacetRelationFilter,
             class ASegmentRelationFilter>
    class SegmentFinderFacetAutomaton :
      public Findlet<CDCWireHit, class CDCRecoSegment2D> {

      private:
        /// Type of the base class
        typedef Findlet<CDCWireHit, class CDCRecoSegment2D> Super;

      public:
        /// Constructor registering the subordinary findlets to the processing signal distribution machinery
        SegmentFinderFacetAutomaton()
        {
          addProcessingSignalListener(&m_superClusterCreator);
          addProcessingSignalListener(&m_clusterRefiner);
          addProcessingSignalListener(&m_clusterBackgroundDetector);
          addProcessingSignalListener(&m_facetCreator);
          addProcessingSignalListener(&m_segmentCreator);
          addProcessingSignalListener(&m_segmentMerger);
          addProcessingSignalListener(&m_segmentOrienter);

          addProcessingSignalListener(&m_segmentFitter);
          addProcessingSignalListener(&m_segmentOrienter);
          addProcessingSignalListener(&m_segmentExporter);

          addProcessingSignalListener(&m_superClusterSwapper);
          addProcessingSignalListener(&m_clusterSwapper);
          addProcessingSignalListener(&m_facetSwapper);
        }

        /// Short description of the findlet
        virtual std::string getDescription() override
        {
          return "Generates segments from hits using a cellular automaton build from hit triples (facets).";
        }

        /// Expose the parameters of the cluster filter to a module
        virtual void exposeParameters(ModuleParamList* moduleParamList) override
        {
          m_superClusterCreator.exposeParameters(moduleParamList);
          m_clusterRefiner.exposeParameters(moduleParamList);
          m_clusterBackgroundDetector.exposeParameters(moduleParamList);
          m_facetCreator.exposeParameters(moduleParamList);
          m_segmentCreator.exposeParameters(moduleParamList);
          m_segmentMerger.exposeParameters(moduleParamList);

          m_segmentFitter.exposeParameters(moduleParamList);
          m_segmentOrienter.exposeParameters(moduleParamList);
          m_segmentExporter.exposeParameters(moduleParamList);

          m_superClusterSwapper.exposeParameters(moduleParamList);
          m_clusterSwapper.exposeParameters(moduleParamList);
          m_facetSwapper.exposeParameters(moduleParamList);
        }

        /// Processes the current event
        void beginEvent() override
        {
          m_superClusters.clear();
          m_clusters.clear();
          m_facets.clear();
          m_segments.clear();
          m_mergedSegments.clear();
          Super::beginEvent();
        }

        /// Generates the segment.
        virtual void apply(std::vector<CDCWireHit>& inputWireHits,
                           std::vector<CDCRecoSegment2D>& outputSegments) override final;

      private:
        // Findlets
        /// Composes the super clusters.
        SuperClusterCreator m_superClusterCreator;

        /// Creates the clusters from super clusters
        ClusterRefiner<PrimaryWireHitRelationFilter> m_clusterRefiner;

        /// Marks the clusters as background
        ClusterBackgroundDetector<AClusterFilter> m_clusterBackgroundDetector;

        /// Find the segments by composition of facets path from a cellular automaton
        FacetCreator<AFacetFilter> m_facetCreator;

        /// Find the segments by composition of facets path from a cellular automaton
        SegmentCreatorFacetAutomaton<AFacetRelationFilter> m_segmentCreator;

        /// Merges segments with closeby segments of the same super cluster
        SegmentMerger<ASegmentRelationFilter> m_segmentMerger;

        /// Fits the generated segments
        SegmentFitter m_segmentFitter;

        /// Adjustes the orientation of the generated segments to a prefered direction of flight
        SegmentOrienter m_segmentOrienter;

        /// Writes out copies of the segments as track candidates.
        SegmentExporter m_segmentExporter;

        /// Puts the internal super clusters on the DataStore
        StoreVectorSwapper<CDCWireHitCluster> m_superClusterSwapper{
          "CDCWireHitSuperClusterVector",
          "superCluster",
          "wire hit super cluster"
        };

        /// Puts the internal clusters on the DataStore
        StoreVectorSwapper<CDCWireHitCluster> m_clusterSwapper{"CDCWireHitClusterVector"};

        /// Puts the internal clusters on the DataStore
        StoreVectorSwapper<CDCFacet> m_facetSwapper{"CDCFacetVector"};


        /// Memory for the wire hit clusters
        std::vector<CDCWireHitCluster> m_clusters;

        /// Memory for the wire hit super cluster
        std::vector<CDCWireHitCluster> m_superClusters;

        /// Memory for the reconstructed segments
        std::vector<CDCFacet> m_facets;

        /// Memory for the reconstructed segments
        std::vector<CDCRecoSegment2D> m_segments;

        /// Memory for the reconstructed segments
        std::vector<CDCRecoSegment2D> m_mergedSegments;

      }; // end class SegmentFinderFacetAutomaton

    template<class AClusterFilter,
             class AFacetFilter,
             class AFacetRelationFilter,
             class ASegmentRelationFilter>
    void SegmentFinderFacetAutomaton<AClusterFilter,
         AFacetFilter,
         AFacetRelationFilter,
         ASegmentRelationFilter>
         ::apply(std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCRecoSegment2D>& outputSegments)
    {
      m_superClusters.reserve(100);
      m_clusters.reserve(200);
      m_facets.reserve(800);
      m_segments.reserve(200);
      m_mergedSegments.reserve(200);
      outputSegments.reserve(outputSegments.size() + 200);

      m_superClusterCreator.apply(inputWireHits, m_superClusters);
      m_clusterRefiner.apply(m_superClusters, m_clusters);
      m_clusterBackgroundDetector.apply(m_clusters);
      m_facetCreator.apply(m_clusters, m_facets);
      m_segmentCreator.apply(m_facets, m_segments);
      m_segmentMerger.apply(m_segments, m_mergedSegments);
      m_segmentFitter.apply(m_mergedSegments);
      m_segmentOrienter.apply(m_mergedSegments, outputSegments);
      m_segmentExporter.apply(outputSegments);

      // Move superclusters to the DataStore
      m_superClusterSwapper.apply(m_superClusters);
      // Move clusters to the DataStore
      m_clusterSwapper.apply(m_clusters);
      // Move facets to the DataStore
      m_facetSwapper.apply(m_facets);
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
