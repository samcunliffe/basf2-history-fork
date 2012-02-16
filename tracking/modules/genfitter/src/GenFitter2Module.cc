/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/genfitter/GenFitter2Module.h>



#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <framework/logging/Logger.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/dataobjects/SVDRecoHit2D.h>
#include <pxd/dataobjects/PXDRecoHit.h>

#include <pxd/dataobjects/PXDTrueHit.h> //delete when not needed any more
#include <svd/dataobjects/SVDTrueHit.h>

#include <tracking/gfbfield/GFGeant4Field.h>

#include <GFTrack.h>
//#include <GFKalman2.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFAbsTrackRep.h>
#include <RKTrackRep.h>
#include <GFConstField.h>
#include <GFFieldManager.h>
#include <GFRecoHitProducer.h>
#include <GFRecoHitFactory.h>
#include <GFMaterialEffects.h>
#include <GFDetPlane.h>
#include <GFTools.h>

#include <cstdlib>
#include <iomanip>
#include <string>

#include <iostream>

#include <boost/foreach.hpp>



using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
  Module()
{
  setDescription("Simplified trackfit module for testing and debugging");
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("filter", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer", false);
  addParam("filterIterations", m_nGFIter, "number of Genfit iterations", 1);
  addParam("probCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", 0.001);
  addParam("energyLossBetheBloch", m_energyLossBetheBloch, "activate the material effect: EnergyLossBetheBloch", true);
  addParam("noiseBetheBloch", m_noiseBetheBloch, "activate the material effect: NoiseBetheBloch", true);
  addParam("noiseCoulomb", m_noiseCoulomb, "activate the material effect: NoiseCoulomb", true);
  addParam("energyLossBrems", m_energyLossBrems, "activate the material effect: EnergyLossBrems", true);
  addParam("noiseBrems", m_noiseBrems, "activate the material effect: NoiseBrems", true);
  addParam("noEffects", m_noEffects, "switch off all material effects in genfit. This overwrites all individual material effects switches", false);
}

GenFitter2Module::~GenFitter2Module()
{
}

void GenFitter2Module::initialize()
{
  // convert the geant4 geometry to a TGeo geometry
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
  //pass the magnetic field to genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());
  // activate / deactivate material effects in genfit
  if (m_noEffects == true) {
    //GFMaterialEffects::getInstance()->setNoEffects(true); //not yet possible in current basd2 genfit version (but already possible upstream)
  } else {
    GFMaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    GFMaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    GFMaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
  }
  StoreArray<GFTrack> fittedTracks(""); //initialization of the the output container of this module

  //set options for fitting algorithms
  m_kalmanFilter.setNumIterations(m_nGFIter);
  m_kalmanFilter.setBlowUpFactor(m_blowUpFactor);
  m_daf.setProbCut(m_probCut);

}

void GenFitter2Module::beginRun()
{
  m_notPerfectCounter = 0;
  m_failedFitCounter = 0;
  m_fitCounter = 0;
}

void GenFitter2Module::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  B2INFO("**********   GenFitter2Module  processing event number: " << eventCounter << " ************");
  StoreArray<GFTrackCand> trackCandidates("");
  int nTrackCandidates = trackCandidates.getEntries();
  if (nTrackCandidates not_eq 0) {  // only try to access a track candidate if there is one

    if (nTrackCandidates == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<GFTrackCand> is empty!");
    }
    StoreArray<SVDTrueHit> svdTrueHits("");
    int nSvdTrueHits = svdTrueHits.getEntries();
    if (nSvdTrueHits == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<SVDTrueHit> is empty!");
    }
    StoreArray<PXDTrueHit> pxdTrueHits("");
    int nPxdTrueHits = pxdTrueHits.getEntries();
    if (nPxdTrueHits == 0) {
      B2DEBUG(100, "GenFitter2: GFTrackCandidatesCollection is empty!");
    }
    StoreArray<CDCHit> cdcHits("");
    int nCdcHits = cdcHits.getEntries();
    if (nCdcHits == 0) {
      B2DEBUG(100, "GenFitter2: StoreArray<CDCHit> is empty!");
    }


    //filter
    bool filterEvent = false;
    if (m_filter == true) {
      // for the filter function to get only tracks that hits specific layers
      int nHitsL1 = 0; int nHitsL2 = 0; int nHitsL3 = 0; int nHitsL4 = 0; int nHitsL5 = 0; int nHitsL6 = 0;
      for (int i = 0; i not_eq nPxdTrueHits; ++i) {
        int layerId = pxdTrueHits[i]->getSensorID().getLayer();
        if (layerId == 1) {
          ++nHitsL1;
        } else {
          ++nHitsL2;
        }
      }
      for (int i = 0; i not_eq nSvdTrueHits; ++i) {
        int layerId = svdTrueHits[i]->getSensorID().getLayer();
        if (layerId == 3) {
          ++nHitsL3;
        } else if (layerId == 4) {
          ++nHitsL4;
        } else if (layerId == 5) {
          ++nHitsL5;
        } else {
          ++nHitsL6;
        }
      }
      if (nHitsL1 not_eq 1 or nHitsL2 not_eq 1 or nHitsL3 not_eq 1 or nHitsL4 not_eq 1 or nHitsL5 not_eq 1 or nHitsL6 not_eq 1) {
        filterEvent = true;
        B2INFO("Not exacly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
        ++m_notPerfectCounter;
      }
    }

    if (filterEvent == false) { // fit the track

      StoreArray<GFTrack> fittedTracks(""); //holds the output of this module in the form of Genfit track objects

      GFTrackCand* aTrackCandPointer = trackCandidates[0];
      //get fit starting values from the MCParticle
      TVector3 vertex = aTrackCandPointer->getPosSeed();
      TVector3 vertexSigma = aTrackCandPointer->getPosError();
      TVector3 momentum = aTrackCandPointer->getDirSeed() * abs(1.0 / aTrackCandPointer->getQoverPseed());
      TVector3 dirSigma = aTrackCandPointer->getDirError();
      //int pdg = aMcParticleArray[0]->getPDG();



      //B2INFO("MCIndex: "<<mcindex);
      B2INFO("Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
      B2INFO("Start values: direction std: " << dirSigma.x() << "  " << dirSigma.y() << "  " << dirSigma.z());
      B2INFO("Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
      B2INFO("Start values: vertex std:   " << vertexSigma.x() << "  " << vertexSigma.y() << "  " << vertexSigma.z());
      B2INFO("Start values: pdg:      " << aTrackCandPointer->getPdgCode());
      GFAbsTrackRep* trackRep;
      //Now create a GenFit track with this representation

      //trackRep = new RKTrackRep(vertex, momentum, poserr, momerr, pdg);
      trackRep = new RKTrackRep(aTrackCandPointer);
      GFTrack track(trackRep, true);

      GFRecoHitFactory factory;

      //create RecoHitProducers for PXD, SVD and CDC
      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
      PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;
      SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdTrueHits);
      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;
      CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);

      //add producers to the factory with correct detector Id
      factory.addProducer(0, PXDProducer);
      factory.addProducer(1, SVDProducer);
      factory.addProducer(2, CDCProducer);

      vector <GFAbsRecoHit*> factoryHits;
      //use the factory to create RecoHits for all Hits stored in the track candidate
      factoryHits = factory.createMany(*trackCandidates[0]);

      //add created hits to the track
      track.addHitVector(factoryHits);
      track.setCandidate(*trackCandidates[0]);

      B2INFO("Total Nr of Hits assigned to the Track: " << track.getNumHits());

      //process track (fit them!)
      if (m_useDaf == false) {
        m_kalmanFilter.processTrack(&track);
      } else {
        m_daf.processTrack(&track);
      }

      int genfitStatusFlag = trackRep->getStatusFlag();
      B2INFO("----> Status of fit: " << genfitStatusFlag);
      B2INFO("-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
      B2INFO("-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
      B2INFO("----> Chi2 of the fit: " << track.getChiSqu());
      B2INFO("----> NDF of the fit: " << track.getNDF());
      /*    track.Print();
          for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
            track.getHit(iHit)->Print();
          }
          */
      if (genfitStatusFlag == 0) {
        new(fittedTracks->AddrAt(0)) GFTrack(track);
        ++m_fitCounter;
      } else {
        B2WARNING("Genfit returned an error (with status flag " << genfitStatusFlag << ") during the fit of one track in event " << eventCounter);
        ++m_failedFitCounter;
      }

    }
  }
}

void GenFitter2Module::endRun()
{
  if (m_notPerfectCounter != 0) {
    B2WARNING(m_notPerfectCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks had not exactly on hit in every layer and were not fitted");
  }
  if (m_failedFitCounter != 0) {
    B2WARNING(m_failedFitCounter << " of " << m_fitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }

}

void GenFitter2Module::terminate()
{

}
