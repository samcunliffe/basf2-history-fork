/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/twoHitFilters/TwoHitFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.icc.h>
#include <tracking/trackFindingCDC/filters/base/NoneFilter.icc.h>
#include <tracking/trackFindingCDC/filters/base/AllFilter.icc.h>

using namespace Belle2;
using namespace TrackFindingCDC;


TwoHitFilterFactory::TwoHitFilterFactory(const std::string& defaultFilterName)
  : Super(defaultFilterName)
{
}

TwoHitFilterFactory::~TwoHitFilterFactory() = default;

std::string TwoHitFilterFactory::getIdentifier() const
{
  return "SVD Hit Pair";
}

std::string TwoHitFilterFactory::getFilterPurpose() const
{
  return "Rejects SVD hit pairs. ";
}

std::map<std::string, std::string> TwoHitFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all combinations are valid"},
    {"none", "no combination is valid"},
  };
}

std::unique_ptr<BaseTwoHitFilter>
TwoHitFilterFactory::create(const std::string& filterName) const
{
  if (filterName == "all") {
    return std::make_unique<TrackFindingCDC::AllFilter<BaseTwoHitFilter>>();
  }

  if (filterName == "none") {
    return std::make_unique<TrackFindingCDC::NoneFilter<BaseTwoHitFilter>>();
  }

  return Super::create(filterName);
}
