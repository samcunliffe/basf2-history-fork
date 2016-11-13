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
#include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <vector>
#include <string>
#include <cassert>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Forward declaration of the CDCFacet.
    class CDCFacet;

    /// Names of the variables to be generated.
    constexpr
    static char const* const basicFacetVarNames[] = {
      "superlayer_id",

      "start_layer_id",
      "start_drift_length",
      "start_drift_length_sigma",

      "middle_layer_id",
      "middle_drift_length",
      "middle_drift_length_sigma",

      "end_layer_id",
      "end_drift_length",
      "end_drift_length_sigma",

      "oclock_delta",
      "twist",
      "cell_extend",
      "n_crossing",
    };

    /**
     *  Class that specifies the names of the variables
     *  that should be generated from a facet
     */
    class BasicFacetVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(basicFacetVarNames);

      /// Getter for the name a the given index
      constexpr
      static char const* getName(int iName)
      {
        return basicFacetVarNames[iName];
      }
    };

    /**
     *  Class that computes floating point variables from a facet.
     *  that can be forwarded to a flat TNtuple or a TMVA method
     */
    class BasicFacetVarSet : public VarSet<BasicFacetVarNames> {

    public:
      /// Generate and assign the variables from the cluster
      bool extract(const CDCFacet* ptrFacet) final;
    };
  }
}
