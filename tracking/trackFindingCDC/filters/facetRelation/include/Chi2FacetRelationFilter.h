/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>

#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCFacet;

    /// Class filtering the neighborhood of facets based on chi2 from a fit both facets.
    class Chi2FacetRelationFilter : public BaseFacetRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseFacetRelationFilter;

    public:
      /// Constructor with the default chi2 cut value and width parameter
      Chi2FacetRelationFilter() = default;

      /// Constructor using given chi2 cut value and width parameter
      explicit Chi2FacetRelationFilter(double chi2Cut, double penaltyWidth);

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    public:
      /// Main filter method returning the weight of the neighborhood relation.
      Weight operator()(const CDCFacet& fromFacet, const CDCFacet& toFacet) final;

    private:
      /// Memory for the chi2 cut value
      double m_param_chi2Cut = 130; // first guess was 350;

      /// Memory for the width parameter to translate the chi2 value to a weight penatlity
      double m_param_penaltyWidth = 200; // first guess was 800;
    };
  }
}
