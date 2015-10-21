/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <framework/datastore/StoreArray.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <algorithm>
#include <framework/core/Module.h>
#include <framework/logging/Logger.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

//out type
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <genfit/TrackCand.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

TrackFinderCDCBaseModule::TrackFinderCDCBaseModule(ETrackOrientation trackOrientation) :
  Module(),
  m_param_trackOrientationString(""),
  m_param_tracksStoreObjName("CDCTrackVector"),
  m_param_tracksStoreObjNameIsInput(false),
  m_param_gfTrackCandsStoreArrayName(""),
  m_param_writeGFTrackCands(true),
  m_trackOrientation(trackOrientation)
{
  setDescription("This a base module for all track finders in the CDC");

  addParam("TrackOrientation",
           m_param_trackOrientationString,
           "Option which orientation of tracks shall be generate. Valid options are '' (default of the finder), 'none' (one orientation, algorithm dependent), 'symmetric', 'outwards', 'downwards'.",
           string(""));

  addParam("TracksStoreObjName",
           m_param_tracksStoreObjName,
           "Name of the output StoreObjPtr of the tracks generated within this module.",
           std::string("CDCTrackVector"));

  addParam("TracksStoreObjNameIsInput",
           m_param_tracksStoreObjNameIsInput,
           "Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module",
           false);

  addParam("GFTrackCandsStoreArrayName",
           m_param_gfTrackCandsStoreArrayName,
           "Name of the output StoreArray of the Genfit track candidates generated by this module.",
           string(""));

  addParam("WriteGFTrackCands",
           m_param_writeGFTrackCands,
           "Flag to output genfit tracks to store array.",
           true);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void TrackFinderCDCBaseModule::initialize()
{
  // Output StoreArray
  if (m_param_writeGFTrackCands) {
    StoreArray <genfit::TrackCand>::registerPersistent(m_param_gfTrackCandsStoreArrayName);
  }
  if (m_param_tracksStoreObjNameIsInput) {
    StoreWrappedObjPtr< std::vector<CDCTrack> >::required(m_param_tracksStoreObjName);
  } else {
    StoreWrappedObjPtr< std::vector<CDCTrack> >::registerTransient(m_param_tracksStoreObjName);
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

void TrackFinderCDCBaseModule::event()
{
  // Now aquire the store vector
  StoreWrappedObjPtr< std::vector<CDCTrack> > storedTracks(m_param_tracksStoreObjName);
  if (not m_param_tracksStoreObjNameIsInput) {
    storedTracks.create();
  }
  std::vector<CDCTrack>& outputTracks = *storedTracks;

  // Either we just have to let the generate-method fill the outputTracks,
  // or we start from scratch with a cleared outputTracks.
  // TODO: In the first case we can not use the trackOrientation in the moment (??)

  if (m_param_tracksStoreObjNameIsInput) {
    generate(outputTracks);
  } else {
    // Generate the tracks
    std::vector<CDCTrack> generatedTracks;
    generatedTracks.reserve(20);
    generate(generatedTracks);

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
        if (lastHit.getRecoPos3D().cylindricalR() < firstHit.getRecoPos3D().cylindricalR()) {
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
  }

  if (m_param_writeGFTrackCands) {
    // Create the gftracks
    StoreArray<genfit::TrackCand> storedGFTrackCands(m_param_gfTrackCandsStoreArrayName);
    storedGFTrackCands.create();
    for (CDCTrack& track : outputTracks) {
      genfit::TrackCand gfTrackCand;
      if (track.fillInto(gfTrackCand)) {
        genfit::TrackCand* storedGenfitTrackCand = storedGFTrackCands.appendNew(gfTrackCand);
        track.setRelatedGenfitTrackCandidate(storedGenfitTrackCand);
      }
    }
  }
}

void TrackFinderCDCBaseModule::generate(std::vector<CDCTrack>&)
{

}
