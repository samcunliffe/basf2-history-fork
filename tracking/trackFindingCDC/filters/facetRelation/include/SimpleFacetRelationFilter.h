/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/facetRelation/BaseFacetRelationFilter.h>


namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class filtering the neighborhood of facets based on simple criterions.
    class SimpleFacetRelationFilter : public Filter<Relation<CDCFacet>> {

    private:
      /// Type of the super class
      typedef Filter<Relation<CDCFacet>> Super;

    public:
      /// Importing all overloads from the super class
      using Super::operator();

    public:
      /// Constructor using default direction of flight deviation cut off.
      SimpleFacetRelationFilter() : m_param_deviationCosCut(cos(0.4))
      {}

      /// Constructor using given direction of flight deviation cut off.
      SimpleFacetRelationFilter(double deviationCosCut) :
        m_param_deviationCosCut(deviationCosCut)
      {}

    public:
      /// Expose the set of parameters of the filter to the module parameter list.
      virtual void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override;
    public:
      /** Main filter method returning the weight of the neighborhood relation.*/
      virtual Weight operator()(const CDCFacet& fromFacet,
                                const CDCFacet& toFacet) override final;

    private:
      /// Memory for the used direction of flight deviation.
      double m_param_deviationCosCut;

    }; // end class

  } //end namespace TrackFindingCDC
} //end namespace Belle2
