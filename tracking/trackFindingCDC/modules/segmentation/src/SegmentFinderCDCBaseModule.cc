/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/modules/segmentation/SegmentFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCTrackStore.h>
#include <tracking/trackFindingCDC/mclookup/CDCSimHitLookUp.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

SegmentFinderCDCBaseModule::SegmentFinderCDCBaseModule() :
  m_param_segmentStoreObjName("CDCRecoSegment2DVector")
{
  addParam("SegmentStoreObjName",  m_param_segmentStoreObjName, "Name of the output StoreObjPtr of the segments generated by this module.", string("CDCRecoSegment2DVector"));

  addParam("Orientation",
           m_param_orientationString,
           "Option which orientation of segments shall be generate. Valid options are 'none' (one orientation, algorithm dependent), 'symmetric', 'outwards', 'downwards'.",
           string("none"));

  addParam("CreateGFTrackCands", m_param_createGFTrackCands, "Switch for the creation of one genfit::TrackCand for each segment.", false);
  addParam("GFTrackCandsStoreArrayName",  m_param_gfTrackCandsStoreArrayName, "Name of the output StoreArray of the Genfit track candidates generated by this module.", string(""));

}

void SegmentFinderCDCBaseModule::initialize()
{
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> >::registerTransient(m_param_segmentStoreObjName);

  if (m_param_orientationString == string("none")) {
    m_orientation = c_None;
  } else if (m_param_orientationString == string("symmetric")) {
    m_orientation = c_Symmetric;
  } else if (m_param_orientationString == string("outwards")) {
    m_orientation = c_Outwards;
  } else if (m_param_orientationString == string("downwards")) {
    m_orientation = c_Downwards;
  } else {
    B2WARNING("Unexpected 'Orientation' parameter of segment finder module : '" << m_param_orientationString << "'. Default to none");
    m_orientation = c_None;
  }

  if (m_param_createGFTrackCands) {
    StoreArray <genfit::TrackCand>::registerPersistent(m_param_gfTrackCandsStoreArrayName);
  }

}

void SegmentFinderCDCBaseModule::event()
{
  // Generate the segments
  std::vector<CDCRecoSegment2D> generatedSegments;
  generatedSegments.reserve(90);
  generate(generatedSegments);

  // Now aquire the store vectore
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments("CDCRecoSegment2DVector");
  storedRecoSegments.create();
  std::vector<CDCRecoSegment2D>& outputSegments = *storedRecoSegments;

  outputSegments.reserve(generatedSegments.size() * (m_orientation == c_Symmetric ? 2 : 1));

  /// Copy segments to output fixing their orientation
  for (const CDCRecoSegment2D & segment : generatedSegments) {
    if (m_orientation == c_None) {
      outputSegments.push_back(std::move(segment));
      // Nothing to do here
    } else if (m_orientation == c_Symmetric) {
      outputSegments.push_back(segment.reversed());
      // ^ Save because we reserved the memory beforehand.
      outputSegments.push_back(std::move(segment));
    } else if (m_orientation == c_Outwards) {
      outputSegments.push_back(std::move(segment));
      CDCRecoSegment2D& lastSegment = outputSegments.back();
      const CDCRecoHit2D& firstHit = lastSegment.front();
      const CDCRecoHit2D& lastHit = lastSegment.back();
      if (lastHit.getRecoPos2D().polarR() > firstHit.getRecoPos2D().polarR()) {
        lastSegment.reverse();
      }
    } else if (m_orientation == c_Downwards) {
      outputSegments.push_back(std::move(segment));
      CDCRecoSegment2D& lastSegment = outputSegments.back();
      const CDCRecoHit2D& firstHit = lastSegment.front();
      const CDCRecoHit2D& lastHit = lastSegment.back();
      if (lastHit.getRecoPos2D().y() > firstHit.getRecoPos2D().y()) {
        lastSegment.reverse();
      }
    } else {
      B2WARNING("Unexpected 'Orientation' parameter of segment finder module : '" << m_orientation << "'. No segments generated.");
    }
  }

  // Put code to generate gf track cands here if requested.


}


