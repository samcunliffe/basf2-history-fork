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

#include "BaseFacetNeighborChooser.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    ///Class filtering the neighborhood of facets with monte carlo information
    class AllFacetNeighborChooser : public Filter<Relation<CDCRecoFacet>> {

    public:
      /** Main filter method returning the weight of the neighborhood relation.
       *  Return -2 for all valid combinations to accepting all facets,
       *  but compensating for overlap.
       */
      virtual NeighborWeight operator()(const CDCRecoFacet& fromFacet,
                                        const CDCRecoFacet& toFacet) override final
      {
        // The last wire of the neighbor should not be the same as the start wire of the facet
        // The  default weight must be -2 because the overlap of the facets is two points
        // so the amount of two facets is 4 points hence the cellular automat
        // must calculate 3 + (-2) + 3 = 4 as cellstate
        // this can of course be adjusted for a more realistic information measure
        // ( together with the facet creator filter)
        return fromFacet.getStartWire() == toFacet.getEndWire() ? NOT_A_NEIGHBOR : -2;
      }

    }; // end class
  } //end namespace TrackFindingCDC
} //end namespace Belle2
