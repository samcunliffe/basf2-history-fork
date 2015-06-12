/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segment_train/SegmentTrainFilterFactory.h>
#include <tracking/trackFindingCDC/filters/segment_train/MCSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/RecordingSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/SimpleSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_train/TMVASegmentTrainFilter.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


SegmentTrainFilterFactory::SegmentTrainFilterFactory(const std::string& defaultFilterName) :
  Super(defaultFilterName)
{
}

std::string SegmentTrainFilterFactory::getFilterPurpose() const
{
  return "Segment train filter to be used during the construction of segment trains.";
}

std::string SegmentTrainFilterFactory::getModuleParamPrefix() const
{
  return "SegmentTrain";
}

std::map<std::string, std::string>
SegmentTrainFilterFactory::getValidFilterNamesAndDescriptions() const
{
  std::map<std::string, std::string>
  filterNames = Super::getValidFilterNamesAndDescriptions();

  filterNames.insert({
    {"mc", "depricated alias for 'truth'"},
    {"truth", "monte carlo truth"},
    {"none", "no segment track combination is valid"},
    {"simple", "mc free with simple criteria"},
    {"recording", "record variables to a TTree"},
    {"tmva", "test with a tmva method"}
  });
  return filterNames;
}

std::unique_ptr<BaseSegmentTrainFilter>
SegmentTrainFilterFactory::create(const std::string& filterName) const
{
  if (filterName == string("none")) {
    return std::unique_ptr<BaseSegmentTrainFilter>(new BaseSegmentTrainFilter());
  } else if (filterName == string("truth")) {
    return std::unique_ptr<BaseSegmentTrainFilter>(new MCSegmentTrainFilter());
  } else if (filterName == string("mc")) {
    B2WARNING("Filter name 'mc' is depricated in favour of 'truth'");
    return std::unique_ptr<BaseSegmentTrainFilter>(new MCSegmentTrainFilter());
  } else if (filterName == string("simple")) {
    return std::unique_ptr<BaseSegmentTrainFilter>(new SimpleSegmentTrainFilter());
  } else if (filterName == string("recording")) {
    return std::unique_ptr<BaseSegmentTrainFilter>(new RecordingSegmentTrainFilter());
  } else if (filterName == string("tmva")) {
    return std::unique_ptr<BaseSegmentTrainFilter>(new TMVASegmentTrainFilter());
  } else {
    return Super::create(filterName);
  }
}
