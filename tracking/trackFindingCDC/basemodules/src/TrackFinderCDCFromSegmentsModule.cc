/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCFromSegmentsModule.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


TrackFinderCDCFromSegmentsModule::TrackFinderCDCFromSegmentsModule() :
  TrackFinderCDCBaseModule(),
  m_param_segmentsStoreObjName("CDCRecoSegment2DVector")
{
  if (not hasParam<std::string>("SegmentsStoreObjName")) {
    addParam("SegmentsStoreObjName",
             m_param_segmentsStoreObjName,
             "Name of the output StoreObjPtr of the segments generated by this module.",
             string("CDCRecoSegment2DVector"));
  }

  ModuleParamList moduleParamList = this->getParamList();
  m_trackCreatorSingleSegments.exposeParameters(&moduleParamList);
  this->setParamList(moduleParamList);
}



void TrackFinderCDCFromSegmentsModule::initialize()
{
  TrackFinderCDCBaseModule::initialize();

  // Synchronise a possible external definition of SegmentsStoreObjName with the local value.
  // Does nothing in the normal case.
  m_param_segmentsStoreObjName = getParamValue<std::string>("SegmentsStoreObjName");

  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> >::required(m_param_segmentsStoreObjName);
  m_trackCreatorSingleSegments.initialize();
}



void TrackFinderCDCFromSegmentsModule::generate(std::vector<CDCTrack>& tracks)
{
  m_trackCreatorSingleSegments.beginEvent();

  // Aquire the store vector
  StoreWrappedObjPtr< std::vector<CDCRecoSegment2D> > storedRecoSegments(m_param_segmentsStoreObjName);
  if (not storedRecoSegments) {
    B2WARNING("The segments have not been created.");
    return;
  }

  std::vector<CDCRecoSegment2D>& segments = *storedRecoSegments;

  generate(segments, tracks);
  m_trackCreatorSingleSegments.apply(segments, tracks);
}
