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

#include <tracking/trackFindingCDC/filters/facet/BaseFacetFilter.h>
#include <tracking/trackFindingCDC/filters/base/RecordingFilter.h>
#include <tracking/trackFindingCDC/filters/facet/CDCFacetTruthVarSet.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Records the encountered CDCWireHitFacets.
    class RecordingFacetFilter: public RecordingFilter<CDCFacetTruthVarSet> {

    public:
      /// Constructor initialising the RecordingFilter with standard root file name for this filter.
      RecordingFacetFilter() :
        RecordingFilter<CDCFacetTruthVarSet>("CDCFacetTruthRecords.root")
      {;}

    };
  }
}
