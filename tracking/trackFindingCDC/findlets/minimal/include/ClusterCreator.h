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

#include <tracking/trackFindingCDC/filters/wireHitRelation/SecondaryWireHitRelationFilter.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/ca/Clusterizer.h>
#include <tracking/trackFindingCDC/ca/WeightedNeighborhood.h>

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <framework/logging/Logger.h>

#include <boost/range/adaptor/transformed.hpp>
#include <memory>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Refines the clustering of wire hits from  clusters to clusters
    template<class AWireHitRelationFilter = SecondaryWireHitRelationFilter>
    class ClusterCreator:
      public Findlet<CDCWireHit, CDCWireHitCluster> {

    private:
      /// Type of the base class
      typedef Findlet<CDCWireHit, CDCWireHitCluster> Super;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      ClusterCreator()
      {
        addProcessingSignalListener(&m_wireHitRelationFilter);
      }

      /// Short description of the findlet
      virtual std::string getDescription() override
      {
        return "Groups the wire hits into super by expanding the given wire hit relation";
      }

    public:
      /// Main algorithm applying the cluster refinement
      virtual void apply(std::vector<CDCWireHit>& inputWireHits,
                         std::vector<CDCWireHitCluster>& outputClusters) override final
      {
        // create the neighborhood
        B2DEBUG(100, "Creating the CDCWireHit neighborhood");
        m_wirehitNeighborhood.clear();
        auto ptrWireHits = inputWireHits | boost::adaptors::transformed(&std::addressof<CDCWireHit>);
        m_wirehitNeighborhood.appendUsing(m_wireHitRelationFilter, ptrWireHits);
        B2ASSERT("Expect wire hit neighborhood to be symmetric ", m_wirehitNeighborhood.isSymmetric());
        B2DEBUG(100, "  wirehitNeighborhood.size() = " << m_wirehitNeighborhood.size());

        m_wirehitClusterizer.createFromPointers(ptrWireHits, m_wirehitNeighborhood, outputClusters);
      }

    private:
      /// Instance of the hit cluster generator
      Clusterizer<CDCWireHit, CDCWireHitCluster> m_wirehitClusterizer;

      /// Memory for the wire hit neighborhood in a cluster.
      WeightedNeighborhood<CDCWireHit> m_wirehitNeighborhood;

      /// Wire hit neighborhood relation filter
      AWireHitRelationFilter m_wireHitRelationFilter;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
