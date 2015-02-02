/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ALLFACETFILTER_H_
#define ALLFACETFILTER_H_

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of good facets based on simple criterions.
    class AllFacetFilter : public BaseFacetFilter {

    public:
      /// Main filter method returning the weight of the facet. Returns 3 to accept all facets.
      virtual CellWeight isGoodFacet(const CDCRecoFacet&) override final { return 3.0; }

    }; //end class AllFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // ALLFACETFILTER_H_
