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

#include <tracking/trackFindingCDC/filters/cluster/BaseClusterFilter.h>
#include <tracking/trackFindingCDC/filters/cluster/AllClusterFilter.h>
#include <tracking/trackFindingCDC/filters/facet/RealisticFacetFilter.h>
#include <tracking/trackFindingCDC/filters/facet_relation/SimpleFacetRelationFilter.h>
#include <tracking/trackFindingCDC/filters/segment_relation/BaseSegmentRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/findlets/minimal/SuperClusterCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterBackgroundDetector.h>
#include <tracking/trackFindingCDC/findlets/minimal/ClusterRefiner.h>
#include <tracking/trackFindingCDC/findlets/minimal/FacetCreator.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentCreatorFacetAutomaton.h>
#include <tracking/trackFindingCDC/findlets/minimal/SegmentMerger.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Forward declaration of the module implementing the segment generation by cellular automaton on facets using specific filter instances.
    template<class ClusterFilter = BaseClusterFilter,
             class FacetFilter = BaseFacetFilter,
             class FacetRelationFilter = BaseFacetRelationFilter,
             class SegmentRelationFilter = BaseSegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule;
  }

  /// Module specialisation using the default Monte Carlo free filters. To be used in production.
  typedef TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule <
  TrackFindingCDC::AllClusterFilter,
                  TrackFindingCDC::RealisticFacetFilter,
                  TrackFindingCDC::SimpleFacetRelationFilter >
                  SegmentFinderCDCFacetAutomatonModule;

  namespace TrackFindingCDC {
    template<class ClusterFilter,
             class FacetFilter,
             class FacetRelationFilter,
             class SegmentRelationFilter>
    class SegmentFinderCDCFacetAutomatonImplModule :
      public SegmentFinderCDCBaseModule {

    private:
      /// Type of the base class
      typedef SegmentFinderCDCBaseModule Super;

    public:
      /// Default constructor initialising the filters with the default settings
      SegmentFinderCDCFacetAutomatonImplModule()
      {

        this->setDescription("Generates segments from hits using a cellular automaton build from hit triples (facets).");

        this->addParam("WriteSuperClusters",
                       m_param_writeSuperClusters,
                       "Switch if superclusters shall be written to the DataStore",
                       false);

        this->addParam("SuperClustersStoreObjName",
                       m_param_superClustersStoreObjName,
                       "Name of the output StoreObjPtr of the super clusters generated within this module.",
                       std::string("CDCWireHitSuperClusterVector"));

        this->addParam("WriteClusters",
                       m_param_writeClusters,
                       "Switch if clusters shall be written to the DataStore",
                       false);

        this->addParam("ClustersStoreObjName",
                       m_param_clustersStoreObjName,
                       "Name of the output StoreObjPtr of the clusters generated within this module.",
                       std::string("CDCWireHitClusterVector"));

        this->addParam("WriteFacets",
                       m_param_writeFacets,
                       "Switch if facets shall be written to the DataStore",
                       m_param_writeFacets);

        this->addParam("FacetsStoreObjName",
                       m_param_facetsStoreObjName,
                       "Name of the output StoreObjPtr of the facets generated within this module.",
                       m_param_facetsStoreObjName);

        ModuleParamList moduleParamList = this->getParamList();
        m_superClusterCreator.exposeParameters(&moduleParamList);
        m_clusterRefiner.exposeParameters(&moduleParamList);
        m_clusterBackgroundDetector.exposeParameters(&moduleParamList);
        m_facetCreator.exposeParameters(&moduleParamList);
        m_segmentCreator.exposeParameters(&moduleParamList);
        m_segmentMerger.exposeParameters(&moduleParamList);
        this->setParamList(moduleParamList);
      }

      /// Initialize the Module before event processing
      virtual void initialize() override
      {
        Super::initialize();
        m_superClusterCreator.initialize();
        m_clusterRefiner.initialize();
        m_clusterBackgroundDetector.initialize();
        m_facetCreator.initialize();
        m_segmentCreator.initialize();
        m_segmentMerger.initialize();

        if (m_param_writeSuperClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_superClustersStoreObjName);
        }

        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >::registerTransient(m_param_clustersStoreObjName);
        }

        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> >::registerTransient(m_param_facetsStoreObjName);
        }
      }

      /// Processes the current event
      void event() override
      {
        m_superClusterCreator.beginEvent();
        m_clusterRefiner.beginEvent();
        m_clusterBackgroundDetector.beginEvent();
        m_facetCreator.beginEvent();
        m_segmentCreator.beginEvent();
        m_segmentMerger.beginEvent();

        m_superClusters.clear();
        m_clusters.clear();
        m_facets.clear();
        m_segments.clear();
        m_mergedSegments.clear();

        /// Attain super cluster vector on the DataStore if needed.
        if (m_param_writeSuperClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedSuperClusters(m_param_superClustersStoreObjName);
          storedSuperClusters.create();
        }

        /// Attain cluster vector on the DataStore if needed.
        if (m_param_writeClusters) {
          StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
          storedClusters.create();
        }

        /// Attain facet vector on the DataStore if needed.
        if (m_param_writeFacets) {
          StoreWrappedObjPtr< std::vector<CDCFacet> > storedFacets(m_param_facetsStoreObjName);
          storedFacets.create();
        }

        Super::event();
      }

      /// Generates the segment.
      virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;

      // implementation below
      virtual void terminate() override
      {
        m_segmentMerger.terminate();
        m_segmentCreator.terminate();
        m_facetCreator.terminate();
        m_clusterBackgroundDetector.terminate();
        m_clusterRefiner.terminate();
        m_superClusterCreator.terminate();
        Super::terminate();
      }

    private:
      /// Parameter: Switch if superclusters shall be written to the DataStore
      bool m_param_writeSuperClusters = false;

      /// Parameter: Name of the output StoreObjPtr of the superclusters generated within this module.
      std::string m_param_superClustersStoreObjName = "CDCWireHitSuperClusterVector";

      /// Parameter: Switch if clusters shall be written to the DataStore
      bool m_param_writeClusters = false;

      /// Parameter: Name of the output StoreObjPtr of the clusters generated within this module.
      std::string m_param_clustersStoreObjName = "CDCWireHitClusterVector";

      /// Parameter: Switch if facets shall be written to the DataStore
      bool m_param_writeFacets = false;

      /// Parameter: Name of the output StoreObjPtr of the facets generated within this module.
      std::string m_param_facetsStoreObjName = "CDCFacetVector";

    private:
      // Findlets
      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Creates the clusters from super clusters
      ClusterRefiner<PrimaryWireHitRelationFilter> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector<ClusterFilter> m_clusterBackgroundDetector;

      /// Find the segments by composition of facets path from a cellular automaton
      FacetCreator<FacetFilter> m_facetCreator;

      /// Find the segments by composition of facets path from a cellular automaton
      SegmentCreatorFacetAutomaton<FacetRelationFilter> m_segmentCreator;

      /// Merges segments with closeby segments of the same super cluster
      SegmentMerger<SegmentRelationFilter> m_segmentMerger;


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

    }; // end class SegmentFinderCDCFacetAutomatonImplModule


    template<class ClusterFilter,
             class FacetFilter,
             class FacetRelationFilter,
             class SegmentRelationFilter>
    void
    SegmentFinderCDCFacetAutomatonImplModule <ClusterFilter,
                                             FacetFilter,
                                             FacetRelationFilter,
                                             SegmentRelationFilter>
                                             ::generateSegments(std::vector<CDCRecoSegment2D>& outputSegments)
    {
      CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
      std::vector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();

      m_superClusterCreator.apply(wireHits, m_superClusters);
      m_clusterRefiner.apply(m_superClusters, m_clusters);
      m_clusterBackgroundDetector.apply(m_clusters);
      m_facetCreator.apply(m_clusters, m_facets);
      m_segmentCreator.apply(m_facets, m_segments);
      m_segmentMerger.apply(m_segments, m_mergedSegments);

      // Move superclusters to the DataStore
      if (m_param_writeSuperClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> >
        storedSuperClusters(m_param_superClustersStoreObjName);
        std::vector<CDCWireHitCluster>& superClusters = *storedSuperClusters;
        superClusters.swap(m_superClusters);
      }

      // Move clusters to the DataStore
      if (m_param_writeClusters) {
        StoreWrappedObjPtr< std::vector<CDCWireHitCluster> > storedClusters(m_param_clustersStoreObjName);
        std::vector<CDCWireHitCluster>& clusters = *storedClusters;
        clusters.swap(m_clusters);
      }

      // Move facets to the DataStore
      if (m_param_writeFacets) {
        StoreWrappedObjPtr< std::vector<CDCFacet> > storedFacets(m_param_facetsStoreObjName);
        std::vector<CDCFacet>& facets = *storedFacets;
        facets.swap(m_facets);
      }

      outputSegments.swap(m_mergedSegments);
    }

  } //end namespace TrackFindingCDC
} //end namespace Belle2
