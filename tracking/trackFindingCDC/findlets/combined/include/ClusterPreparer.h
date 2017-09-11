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

#include <tracking/trackFindingCDC/filters/wireHitRelation/BridgingWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Findlet to cluster the wire hits in the CDC to form locally connected groups with two granularities.";
    class ClusterPreparer : public Findlet<CDCWireHit&, CDCWireHitCluster, CDCWireHitCluster> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCWireHit&, CDCWireHitCluster, CDCWireHitCluster>;

    public:
      /// Constructor registering the subordinary findlets to the processing signal distribution machinery
      ClusterPreparer();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParams(ParamList* paramList, const std::string& prefix) final;

      /// Generates the segment from wire hits
      void apply(std::vector<CDCWireHit>& inputWireHits,
                 std::vector<CDCWireHitCluster>& clusters,
                 std::vector<CDCWireHitCluster>& superClusters) final;

    private:
      // Findlets
      /// Composes the super clusters.
      SuperClusterCreator m_superClusterCreator;

      /// Creates the clusters from super clusters
      ClusterRefiner<BridgingWireHitRelationFilter> m_clusterRefiner;

      /// Marks the clusters as background
      ClusterBackgroundDetector m_clusterBackgroundDetector;
    };
  }
}
