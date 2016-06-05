/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/facetRelation/FacetRelationFilters.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FacetRelationFilterFactory::FacetRelationFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string FacetRelationFilterFactory::getFilterPurpose() const
{
  return "Facet relation filter to be used during the construction of the facet network.";
}

std::string FacetRelationFilterFactory::getModuleParamPrefix() const
{
  return "FacetRelation";
}

std::map<std::string, std::string>
FacetRelationFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"all", "all facet relations are valid"},
    {"truth", "facet relations from monte carlo truth"},
    {"simple", "mc free with simple criteria"},
    {"chi2", "mc free based on chi2 fitting"},
    {"tmva", "filter facets with a tmva method"},
    {"none", "no facet relation is valid, stop at facet creation."},
    {"unionrecording", "record multiple choosable variable sets"},
  });

  return filterNames;
}

std::unique_ptr<BaseFacetRelationFilter >
FacetRelationFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new BaseFacetRelationFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new AllFacetRelationFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new MCFacetRelationFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new TMVAFacetRelationFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new SimpleFacetRelationFilter());
  } else if (filterName == string("chi2")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new Chi2FacetRelationFilter());
  } else if (filterName == string("unionrecording")) {
    return std::unique_ptr<BaseFacetRelationFilter >(new UnionRecordingFacetRelationFilter());
  } else {
    return Super::create(filterName);
  }
}
