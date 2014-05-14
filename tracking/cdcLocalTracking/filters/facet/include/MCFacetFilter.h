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

#include <tracking/cdcLocalTracking/eventdata/entities/CDCRecoFacet.h>

namespace Belle2 {
  namespace CDCLocalTracking {


    /// Filter for the constuction of good facets based on monte carlo information
    class MCFacetFilter {

    public:

      /** Constructor. */
      MCFacetFilter();

      /** Destructor.*/
      ~MCFacetFilter();

      CellState isGoodFacet(const CDCRecoFacet& facet) const;

    public:
      /// Clears all remember information from the last event
      void clear() const;

      /// Forwards the modules initialize to the filter
      void initialize();

      /// Forwards the modules initialize to the filter
      void terminate();


    private:
      /// Indicated if the oriented triple is a correct hypotheses
      bool isCorrect(const CDCRLWireHitTriple& rlWireHit, int inTrackHitDistanceTolerance = 99999) const;

    private:
      // const CDCMCLookUp& m_mcLookUp;




    }; // end class SimpleFacetFilter
  } //end namespace CDCLocalTracking
} //end namespace Belle2

#endif //MCFACETFILTER_H_
