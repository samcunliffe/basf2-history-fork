/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCFACETFILTER_H_
#define MCFACETFILTER_H_

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {


    /// Filter for the constuction of good facets based on monte carlo information
    class MCFacetFilter : public BaseFacetFilter {

    public:
      /// Constructor also setting the switch if the reversed version of a facet (in comparision to MC truth) shall be accepted.
      MCFacetFilter(bool allowReverse = true) : m_allowReverse(allowReverse) {;}

    public:
      /// Main filter method returning the weight of the facet. Returns NOT_A_CELL if the cell shall be rejected.
      virtual CellWeight isGoodFacet(const CDCRecoFacet& facet) IF_NOT_CINT(override final);

    private:
      /// Indicated if the oriented triple is a correct hypotheses
      bool isCorrect(const CDCRLWireHitTriple& rlWireHit, int inTrackHitDistanceTolerance = 99999) const;

    private:
      /// Switch to indicate if the reversed version of the facet shall also be accepted (default is true).
      bool m_allowReverse;

    }; // end class MCFacetFilter
  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif //MCFACETFILTER_H_
