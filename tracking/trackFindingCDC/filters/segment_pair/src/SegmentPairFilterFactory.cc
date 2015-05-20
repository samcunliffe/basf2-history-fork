/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include "../include/SegmentPairFilterFactory.h"
#include "../include/SegmentPairFilters.h"

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


SegmentPairFilterFactory::SegmentPairFilterFactory(const std::string& defaultFilterName) :
  FilterFactory<BaseSegmentPairFilter>(defaultFilterName)
{
}

std::string SegmentPairFilterFactory::getFilterPurpose() const
{
  return "Segment pair filter to be used during the construction of segment pairs.";
}

std::string SegmentPairFilterFactory::getModuleParamPrefix() const
{
  return "SegmentPair";
}

std::map<std::string, std::string>
SegmentPairFilterFactory::getValidFilterNamesAndDescriptions() const
{
  return {
    {"all", "all segment pairs are valid"},
    {"mc", "monte carlo truth"},
    {"none", "no segment pair is valid"},
    {"recording", "record the encountered instances of segment pairs"},
    {"simple", "mc free with simple criteria"},
  };
}

std::unique_ptr<BaseSegmentPairFilter>
SegmentPairFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentPairFilter>(new BaseSegmentPairFilter());
  } else if (filterName == string("all")) {
    return std::unique_ptr<BaseSegmentPairFilter>(new AllSegmentPairFilter());
  } else if (filterName == string("mc")) {
    return std::unique_ptr<BaseSegmentPairFilter>(new MCSegmentPairFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentPairFilter>(new SimpleSegmentPairFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentPairFilter>(new RecordingSegmentPairFilter());
  } else {
    return std::unique_ptr<BaseSegmentPairFilter>(nullptr);
  }
}
