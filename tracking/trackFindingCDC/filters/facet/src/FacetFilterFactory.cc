/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/FacetFilterFactory.h"
#include "../include/FacetFilters.h"

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


FacetFilterFactory::FacetFilterFactory(const std::string& defaultFilterName) :
  FilterFactory<BaseFacetFilter>(defaultFilterName)
{
}

std::string FacetFilterFactory::getFilterPurpose() const
{
  return "Facet filter to be used during the construction of facets.";
}

std::string FacetFilterFactory::getModuleParamPrefix() const
{
  return "Facet";
}

std::map<std::string, std::string> FacetFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all facets are valid"},
    {"fitless", "only checking the feasability of right left passage information"},
    {"mc", "monte carlo truth"},
    {"none", "no facet is valid, stop at cluster generation."},
    {"realistic", "mc free with more realistic criteria"},
    {"recording", "record the encountered instances of facets"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseFacetFilter> FacetFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseFacetFilter>(new BaseFacetFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseFacetFilter>(new AllFacetFilter());
  } else if (filterName == string("mc")) {
    return std::unique_ptr<BaseFacetFilter>(new MCFacetFilter());
  } else if (filterName == string("fitless")) {
    return std::unique_ptr<BaseFacetFilter>(new FitlessFacetFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseFacetFilter>(new SimpleFacetFilter());
  } else if (filterName == string("realistic")) {
    return std::unique_ptr<BaseFacetFilter>(new RealisticFacetFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseFacetFilter>(new RecordingFacetFilter());
  } else {
    return std::unique_ptr<BaseFacetFilter>(nullptr);
  }
}
