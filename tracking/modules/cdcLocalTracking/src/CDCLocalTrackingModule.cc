/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/cdcLocalTracking/CDCLocalTrackingModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <cdc/geometry/CDCGeometryPar.h>

//in type
#include <cdc/dataobjects/CDCHit.h>

#include <tracking/cdcLocalTracking/eventtopology/CDCWireHitTopology.h>

#ifdef CDCLOCALTRACKING_USE_MC_INFORMATION
#include <mdst/dataobjects/MCParticle.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <tracking/cdcLocalTracking/mclookup/CDCMCManager.h>
#endif


//out type
#include "genfit/TrackCand.h"

#ifdef HAS_CALLGRIND
#include <valgrind/callgrind.h>
#endif

#include <time.h>

using namespace std;
using namespace Belle2;
using namespace CDCLocalTracking;


REG_MODULE(CDCLocalTracking);

CDCLocalTrackingModule::CDCLocalTrackingModule() : Module()
{
  setDescription("Performs patter recognition in the CDC based on local hit following");

  addParam("GFTrackCandColName",  m_param_gfTrackCandColName, "Name of the output collection of genfit::TrackCands ", string(""));

}

CDCLocalTrackingModule::~CDCLocalTrackingModule()
{
}

void CDCLocalTrackingModule::initialize()
{
  //output collection
  StoreArray <genfit::TrackCand>::registerPersistent(m_param_gfTrackCandColName);

  m_segmentWorker.initialize();
  //m_segmentTripleTrackingWorker.initialize();
  m_segmentPairTrackingWorker.initialize();

  //StoreArray with digitized CDCHits
  StoreArray <CDCHit>::required();

#ifdef CDCLOCALTRACKING_USE_MC_INFORMATION
  StoreArray <CDCSimHit>::required();
  StoreArray <MCParticle>::required();
#endif

  //preload geometry during initialization
  //marked as unused intentionally to avoid a compile warning
  CDC::CDCGeometryPar& cdcGeo __attribute__((unused)) = CDC::CDCGeometryPar::Instance();
  CDCWireTopology& topo __attribute__((unused)) = CDCWireTopology::getInstance();

}

void CDCLocalTrackingModule::beginRun()
{

}

void CDCLocalTrackingModule::event()
{
  B2DEBUG(100, "########## CDCLocalTracking begin ##########");



#ifdef CDCLOCALTRACKING_USE_MC_INFORMATION
  CDCMCManager::getInstance().clear();
  CDCMCManager::getInstance().fill();
#endif



#ifdef HAS_CALLGRIND
  CALLGRIND_START_INSTRUMENTATION;
#endif

  //Start callgrind recording
  //To profile start basf2 with
  //  nohup valgrind --tool=callgrind --instr-atstart=no basf2 [basf2-options] > output.txt &
  // since that takes a while.
  // Do a callgrind_control -b for an intermediate output or callgrind_control -b -e
  // Definitions need callgrind.h


  // Fetch the CDCHits from the datastore
  B2DEBUG(100, "Getting the CDCHits from the data store");
  StoreArray <CDCHit> storedCDCHits;
  B2DEBUG(100, "  storedCDCHits.getEntries() == " << storedCDCHits.getEntries());


  // Create the wirehits - translate the CDCHits and attach the geometry.
  B2DEBUG(100, "Creating all CDCWireHits");
  CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  size_t nHits = wireHitTopology.fill();
  if (nHits == 0) {
    B2WARNING("Event with no hits - skipping");
    return;
  }

  // Stage one
  // Build the segments
  m_recoSegments.clear();
  m_segmentWorker.generate(m_recoSegments);
  B2DEBUG(100, "Received " << m_recoSegments.size() << " RecoSegments from worker");


  // Stage two
  // Build the gfTracks
  StoreArray < genfit::TrackCand > storedGFTrackCands(m_param_gfTrackCandColName);
  storedGFTrackCands.create();
  //m_segmentTripleTrackingWorker.apply(m_recoSegments, storedGFTrackCands);
  m_segmentPairTrackingWorker.apply(m_recoSegments, storedGFTrackCands);



  //End callgrind recording
#ifdef HAS_CALLGRIND
  CALLGRIND_STOP_INSTRUMENTATION;
#endif

  B2DEBUG(100, "########## CDCLocalTracking end ############");

}

void CDCLocalTrackingModule::endRun()
{
}

void CDCLocalTrackingModule::terminate()
{
  m_segmentWorker.terminate();
  //m_segmentTripleTrackingWorker.terminate();
  m_segmentPairTrackingWorker.terminate();

#ifdef HAS_CALLGRIND
  CALLGRIND_DUMP_STATS;
#endif
}

