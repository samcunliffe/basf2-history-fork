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

#include "CDCFacetFitVarSet.h"
#include "MCFacetFilter.h"

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const facetTruthNames[] = {
      "truth"
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a facet.
     */
    class CDCFacetTruthVarNames : public VarNames<CDCFacet> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = size(facetTruthNames);

      /// Getter for the name a the given index
      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return facetTruthNames[iName];
      }
    };

    /** Class that computes floating point variables from facet.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class  CDCFacetTruthVarSet : public VarSet<CDCFacetTruthVarNames> {

    public:
      /// Construct the varset and take an optional prefix.
      CDCFacetTruthVarSet(const std::string& prefix = "");

      /// Generate and assign the variables from the facet
      virtual bool extract(const CDCFacet* facet) IF_NOT_CINT(override final);

      /// Initialize the varset before event processing
      virtual void initialize() IF_NOT_CINT(override final);

      /// Initialize the varset before event processing
      virtual void terminate() IF_NOT_CINT(override final);

    public:
      /// Facet filter that gives if the facet is a true facet.
      MCFacetFilter m_mcFacetFilter;

    };
  }
}
