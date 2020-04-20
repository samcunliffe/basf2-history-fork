/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Tracking DQM                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackingDQM/TrackDQMModule.h>
#include <tracking/modules/trackingDQM/TrackingEventProcessor.h>
#include <tracking/dqmUtils/THFFactory.h>

#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <tracking/dataobjects/RecoHitInformation.h>
#include <vxd/geometry/GeoTools.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <TDirectory.h>
#include <TVectorD.h>

using namespace Belle2;
using namespace std;
using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackDQMModule::TrackDQMModule() : BaseDQMHistogramModule()
{
  /*
  //Set module properties
  setDescription("DQM of finding tracks, their momentum, "
                 "Number of hits in tracks, "
                 "Number of tracks. "
                );
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("TracksStoreArrayName", m_TracksStoreArrayName, "StoreArray name where the merged Tracks are written.", m_TracksStoreArrayName);
  addParam("RecoTracksStoreArrayName", m_RecoTracksStoreArrayName, "StoreArray name where the merged RecoTracks are written.", m_RecoTracksStoreArrayName);
  */
}

TrackDQMModule::~TrackDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void TrackDQMModule::initialize()
{
  StoreArray<RecoTrack> recoTracks(m_RecoTracksStoreArrayName);
  if (!recoTracks.isOptional()) {
    B2WARNING("Missing recoTracks array, Track-DQM is skipped.");
    return;
  }

  StoreArray<Track> Tracks(m_TracksStoreArrayName);
  if (!Tracks.isOptional()) {
    B2WARNING("Missing Tracks array, Track-DQM is skipped.");
    return;
  }

  // eventLevelTrackingInfo is currently only set by VXDTF2, if VXDTF2 is not in path the StoreObject is not there
  m_eventLevelTrackingInfo.isOptional();

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void TrackDQMModule::defineHisto()
{
  if (VXD::GeoCache::getInstance().getGeoTools()->getNumberOfLayers() == 0)
    B2WARNING("Missing geometry for VXD.");

  // Create a separate histogram directories and cd into it.
  TDirectory* originalDirectory = gDirectory;

  // There might be problems with nullptr if the directory with the same name already exists (but I am not sure because there isn't anything like that in AlignmentDQM)
  TDirectory* TracksDQM = originalDirectory->GetDirectory("TracksDQM");
  if (!TracksDQM)
    TracksDQM = originalDirectory->mkdir("TracksDQM");

  TDirectory* TracksDQMAlignment = originalDirectory->GetDirectory("TracksDQMAlignment");
  if (!TracksDQMAlignment)
    TracksDQMAlignment = originalDirectory->mkdir("TracksDQMAlignment");

  TracksDQM->cd();

  DefineGeneral();
  DefineUBResiduals();
  DefineHelixParameters();
  DefineMomentum();
  DefineHits();
  DefineTracks();
  DefineHalfShells();
  DefineFlags();

  DefineClusters();

  TracksDQMAlignment->cd();
  DefineSensors();

  originalDirectory->cd();
}

void TrackDQMModule::event()
{
  TrackingEventProcessor eventProcessor = TrackingEventProcessor(this, m_RecoTracksStoreArrayName, m_TracksStoreArrayName);

  eventProcessor.Run();

  if (m_eventLevelTrackingInfo.isValid())
    m_trackingErrorFlags->Fill((double)m_eventLevelTrackingInfo->hasAnErrorFlag());
  else
    m_trackingErrorFlags->Fill(0.0);
}

void TrackDQMModule::DefineFlags()
{
  // only monitor if any flag was set so only 2 bins needed
  const char* flagTitle =
    "Tracking error summary. Mean = errors/event (should be 0 or very close to 0); Error occured yes or no; Number of events";

  m_trackingErrorFlags = Create("NumberTrackingErrorFlags", flagTitle, 2, -0.5, 1.5, "", "");

  m_trackingErrorFlags->GetXaxis()->SetBinLabel(1, "No Error");
  m_trackingErrorFlags->GetXaxis()->SetBinLabel(2, "Error occured");
}