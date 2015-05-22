/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderOutputCombiner/SegmentTrackCombinerDevModule.h>

#include <tracking/trackFindingCDC/filters/segment_track/RecordingSegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/TMVASegmentTrackChooser.h>
#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrackChooser.h>

#include <tracking/trackFindingCDC/filters/segment_track/RecordingSegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/TMVASegmentTrainFilter.h>
#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrainFilter.h>

#include <tracking/trackFindingCDC/filters/segment_track/MCSegmentTrackFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <framework/datastore/StoreArray.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(SegmentTrackCombinerDev);

SegmentTrackCombinerDevModule::SegmentTrackCombinerDevModule() :
  SegmentTrackCombinerImplModule<>(),
  m_param_segmentTrackChooser("simple"),
  m_param_segmentTrainFilter("simple"),
  m_param_segmentTrackFilter("simple")
{
  setDescription("Versatile module with adjustable filters for segment track combination.");

  addParam("SegmentTrackChooser",
           m_param_segmentTrackChooser, "", string("simple"));

  addParam("SegmentTrackChooserParameters",
           m_param_segmentTrackChooserParameters,
           "Key - Value pairs depending on the filter",
           map<string, string>());

  addParam("SegmentTrainFilter",
           m_param_segmentTrainFilter, "", string("simple"));

  addParam("SegmentTrainFilterParameters",
           m_param_segmentTrainFilterParameters,
           "Key - Value pairs depending on the filter",
           map<string, string>());

  addParam("SegmentTrackFilter",
           m_param_segmentTrackFilter, "", string("simple"));
}

void SegmentTrackCombinerDevModule::initialize()
{
  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrackChooser> ptrSegmentTrackChooser(new BaseSegmentTrackChooser());

  if (m_param_segmentTrackChooser == string("none")) {
    ptrSegmentTrackChooser.reset(new BaseSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("simple")) {
    ptrSegmentTrackChooser.reset(new SimpleSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("mc")) {
    ptrSegmentTrackChooser.reset(new MCSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("recording")) {
    ptrSegmentTrackChooser.reset(new RecordingSegmentTrackChooser());
  } else if (m_param_segmentTrackChooser == string("tmva")) {
    ptrSegmentTrackChooser.reset(new TMVASegmentTrackChooser());
  } else {
    B2ERROR("Unrecognised SegmentTrackChooser option " << m_param_segmentTrackChooser);
  }

  // Takes ownership
  setSegmentTrackChooser(std::move(ptrSegmentTrackChooser));
  getSegmentTrackChooser()->setParameters(m_param_segmentTrackChooserParameters);

  // Set the filters before they get initialized in the base module.
  std::unique_ptr<BaseSegmentTrainFilter> ptrSegmentTrainFilter(new BaseSegmentTrainFilter());;

  if (m_param_segmentTrainFilter == string("none")) {
    ptrSegmentTrainFilter.reset(new BaseSegmentTrainFilter());
  } else if (m_param_segmentTrainFilter == string("simple")) {
    ptrSegmentTrainFilter.reset(new SimpleSegmentTrainFilter());
  } else if (m_param_segmentTrainFilter == string("mc")) {
    ptrSegmentTrainFilter.reset(new MCSegmentTrainFilter());
  } else if (m_param_segmentTrainFilter == string("recording")) {
    ptrSegmentTrainFilter.reset(new RecordingSegmentTrainFilter());
  } else if (m_param_segmentTrainFilter == string("tmva")) {
    ptrSegmentTrainFilter.reset(new TMVASegmentTrainFilter());
  } else {
    B2ERROR("Unrecognised SegmentTrainFilter option " << m_param_segmentTrainFilter);
  }

  // Takes ownership
  setSegmentTrainFilter(std::move(ptrSegmentTrainFilter));
  getSegmentTrainFilter()->setParameters(m_param_segmentTrainFilterParameters);

  std::unique_ptr<BaseSegmentTrackFilter> ptrSegmentTrackFilter(new BaseSegmentTrackFilter());
  if (m_param_segmentTrackFilter == string("none")) {
    ptrSegmentTrackFilter.reset(new BaseSegmentTrackFilter());
  } else if (m_param_segmentTrackFilter == string("simple")) {
    ptrSegmentTrackFilter.reset(new SimpleSegmentTrackFilter());
  } else if (m_param_segmentTrackFilter == string("mc")) {
    ptrSegmentTrackFilter.reset(new MCSegmentTrackFilter());
  } else {
    B2ERROR("Unrecognised SegmentTrackFilter option " << m_param_segmentTrackFilter);
  }

  // Takes ownership
  setSegmentTrackFilter(std::move(ptrSegmentTrackFilter));

  SegmentTrackCombinerImplModule<>::initialize();

  if (getSegmentTrackChooser()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    StoreArray <CDCSimHit>::required();
    StoreArray <MCParticle>::required();
  }
}


void SegmentTrackCombinerDevModule::event()
{
  if (getSegmentTrackChooser()->needsTruthInformation() or
      getSegmentTrainFilter()->needsTruthInformation() or
      getSegmentTrackFilter()->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  SegmentTrackCombinerImplModule<>::event();
}
