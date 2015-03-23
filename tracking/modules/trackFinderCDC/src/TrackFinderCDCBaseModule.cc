/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCBaseModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <cdc/geometry/CDCGeometryPar.h>

//in type
#include <cdc/dataobjects/CDCHit.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

//out type
#include "genfit/TrackCand.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderCDCBaseModule::TrackFinderCDCBaseModule(ETrackOrientation trackOrientation) :
  Module(),
  m_param_useOnlyCDCHitsRelatedFromStoreArrayName(""),
  m_param_dontUseCDCHitsRelatedFromStoreArrayName(""),
  m_param_trackOrientationString(""),
  m_param_tracksStoreObjName("CDCTrackVector"),
  m_param_gfTrackCandsStoreArrayName(""),
  m_param_remainingCDCHitsStoreArrayName(""),
  m_trackOrientation(trackOrientation)
{
  setDescription("This a base module for all track finders in the CDC");
  addParam("UseOnlyCDCHitsRelatedFrom",
           m_param_useOnlyCDCHitsRelatedFromStoreArrayName,
           "Full name of a StoreArray that has a Relation to the CDCHits StoreArray. "
           "Only CDCHits that have a relation will be used in this track finder.",
           string(""));

  addParam("DontUseCDCHitsRelatedFrom",
           m_param_dontUseCDCHitsRelatedFromStoreArrayName,
           "Full name of a StoreArray that has a Relation to the CDCHits StoreArray. "
           "CDCHits that have a relation will be blocked in this track finder.",
           string(""));

  addParam("TrackOrientation",
           m_param_trackOrientationString,
           "Option which orientation of tracks shall be generate. Valid options are '' (default of the finder), 'none' (one orientation, algorithm dependent), 'symmetric', 'outwards', 'downwards'.",
           string(""));

  addParam("TracksStoreObjName",
           m_param_tracksStoreObjName,
           "Name of the output StoreObjPtr of the tracks generated within this module.",
           std::string("CDCTrackVector"));

  addParam("GFTrackCandsStoreArrayName",
           m_param_gfTrackCandsStoreArrayName,
           "Name of the output StoreArray of the Genfit track candidates generated by this module.",
           string(""));

  addParam("RemainingCDCHitsStoreArrayName",
           m_param_remainingCDCHitsStoreArrayName,
           "Name of the StoreArray of CDCHits that are still unblocked at the end of the module. "
           "A copy for each CDCHit is created into the new StoreArray and a relation to the main CDCHits"
           " is constructed. The default value '' means no StoreArray and no copies are created.",
           std::string(""));
}

TrackFinderCDCBaseModule::~TrackFinderCDCBaseModule()
{
}

void TrackFinderCDCBaseModule::initialize()
{
  // Preload geometry during initialization
  // Marked as unused intentionally to avoid a compile warning
  CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
  CDCWireTopology& wireTopology  __attribute__((unused)) = CDCWireTopology::getInstance();

  // Input StoreArray of CDCHits
  CDCWireHitTopology::getInstance().initialize();

  // Output StoreArray
  StoreArray <genfit::TrackCand>::registerPersistent(m_param_gfTrackCandsStoreArrayName);
  StoreWrappedObjPtr< std::vector<CDCTrack> >::registerTransient(m_param_tracksStoreObjName);

  // Output StoreArray for the remaining CDCHits
  if (m_param_remainingCDCHitsStoreArrayName != "") {
    StoreArray<CDCHit> storedRemainingCDCHits(m_param_remainingCDCHitsStoreArrayName);
    storedRemainingCDCHits.registerInDataStore();

    StoreArray<CDCHit> storedCDCHits("");
    storedRemainingCDCHits.registerRelationTo(storedCDCHits);
  }

  if (m_param_trackOrientationString == string("")) {
    // Keep the default value in this case, if the user did not specify anything.
  } else if (m_param_trackOrientationString == string("none")) {
    m_trackOrientation = c_None;
  } else if (m_param_trackOrientationString == string("symmetric")) {
    m_trackOrientation = c_Symmetric;
  } else if (m_param_trackOrientationString == string("outwards")) {
    m_trackOrientation = c_Outwards;
  } else if (m_param_trackOrientationString == string("downwards")) {
    m_trackOrientation = c_Downwards;
  } else {
    B2WARNING("Unexpected 'TrackOrientation' parameter of track finder module : '" << m_param_trackOrientationString <<
              "'. Default to none");
    m_trackOrientation = c_None;
  }
}

void TrackFinderCDCBaseModule::beginRun()
{
}


size_t TrackFinderCDCBaseModule::prepareHits()
{
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  // Create the wirehits - translate the CDCHits and attach the geometry.
  B2DEBUG(100, "Creating all CDCWireHits");
  size_t nHits = wireHitTopology.event();
  size_t useNHits = nHits;

  if (m_param_useOnlyCDCHitsRelatedFromStoreArrayName != std::string("")) {
    B2DEBUG(100, "  Use only CDCHits related from " << m_param_useOnlyCDCHitsRelatedFromStoreArrayName);
    useNHits = wireHitTopology.useOnlyRelatedFrom(m_param_useOnlyCDCHitsRelatedFromStoreArrayName);
  } else {
    useNHits = wireHitTopology.useAll();
  }

  if (m_param_dontUseCDCHitsRelatedFromStoreArrayName != std::string("")) {
    B2DEBUG(100, "  Don't use CDCHits related from " << m_param_dontUseCDCHitsRelatedFromStoreArrayName);
    useNHits -= wireHitTopology.dontUseRelatedFrom(m_param_dontUseCDCHitsRelatedFromStoreArrayName);
  }

  B2DEBUG(100, "  Created number of CDCWireHits == " << nHits);
  B2DEBUG(100, "  Number of usable CDCWireHits == " << useNHits);

  if (nHits == 0) {
    B2WARNING("Event with no hits");
  }

  return nHits;
}


size_t TrackFinderCDCBaseModule::copyRemainingHits()
{
  if (m_param_remainingCDCHitsStoreArrayName == "") return 0;

  B2INFO("Copying remaining hits");

  StoreArray<CDCHit> storedRemainingCDCHits(m_param_remainingCDCHitsStoreArrayName);
  storedRemainingCDCHits.create();

  for (const CDCWireHit& wireHit : CDCWireHitTopology::getInstance().getWireHits()) {
    const AutomatonCell& automatonCell = wireHit.getAutomatonCell();
    if (not automatonCell.hasDoNotUseFlag()) {
      const CDCHit* hit = wireHit.getHit();
      B2INFO("Copy hit" << hit);
      if (hit) {
        const CDCHit* newAddedCDCHit = storedRemainingCDCHits.appendNew(*hit);
        newAddedCDCHit->addRelationTo(hit);
      }
    }
  }
  return storedRemainingCDCHits.getEntries();
}


void TrackFinderCDCBaseModule::event()
{
  prepareHits();

  // Generate the tracks
  std::vector<CDCTrack> generatedTracks;
  generatedTracks.reserve(20);
  generate(generatedTracks);

  // Now aquire the store vector
  StoreWrappedObjPtr< std::vector<CDCTrack> > storedTracks(m_param_tracksStoreObjName);
  storedTracks.create();
  std::vector<CDCTrack>& outputTracks = *storedTracks;
  outputTracks.clear();

  // Copy Tracks to output fixing their orientation
  if (m_trackOrientation == c_None) {
    std::swap(generatedTracks, outputTracks);
  } else if (m_trackOrientation == c_Symmetric) {
    outputTracks.reserve(2 * generatedTracks.size());
    for (const CDCTrack& track : generatedTracks) {
      outputTracks.push_back(track.reversed());
      outputTracks.push_back(std::move(track));
    }
  } else if (m_trackOrientation == c_Outwards) {
    outputTracks.reserve(generatedTracks.size());
    for (const CDCTrack& track : generatedTracks) {
      outputTracks.push_back(std::move(track));
      CDCTrack& lastTrack = outputTracks.back();
      const CDCRecoHit3D& firstHit = lastTrack.front();
      const CDCRecoHit3D& lastHit = lastTrack.back();
      if (lastHit.getRecoPos3D().polarR() > firstHit.getRecoPos3D().polarR()) {
        lastTrack.reverse();
      }
    }
  } else if (m_trackOrientation == c_Downwards) {
    outputTracks.reserve(generatedTracks.size());
    for (const CDCTrack& track : generatedTracks) {
      outputTracks.push_back(std::move(track));
      CDCTrack& lastTrack = outputTracks.back();
      const CDCRecoHit3D& firstHit = lastTrack.front();
      const CDCRecoHit3D& lastHit = lastTrack.back();
      if (lastHit.getRecoPos3D().y() > firstHit.getRecoPos3D().y()) {
        lastTrack.reverse();
      }
    }
  } else {
    B2WARNING("Unexpected 'TrackOrientation' parameter of track finder module : '" << m_trackOrientation << "'. No tracks generated.");
  }

  // Create the gftracks
  StoreArray<genfit::TrackCand> storedGFTrackCands(m_param_gfTrackCandsStoreArrayName);
  storedGFTrackCands.create();
  for (const CDCTrack& track : outputTracks) {
    genfit::TrackCand gfTrackCand;
    if (track.fillInto(gfTrackCand)) {
      storedGFTrackCands.appendNew(gfTrackCand);
    }
  }

  copyRemainingHits();
}

void TrackFinderCDCBaseModule::generate(std::vector<CDCTrack>&)
{

}

void TrackFinderCDCBaseModule::endRun()
{
}

void TrackFinderCDCBaseModule::terminate()
{
}
