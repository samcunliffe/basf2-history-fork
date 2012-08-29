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


//framework stuff
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
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <pxd/reconstruction/PXDRecoHit.h>

#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/dataobjects/VXDTrueHit.h>
#include <vxd/dataobjects/VXDSimpleDigiHit.h>
#include <tracking/gfbfield/GFGeant4Field.h>
//genfit stuff
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
// c++ stl stuff
#include <cstdlib>
#include <iomanip>
#include <string>
#include <iostream>
#include <algorithm>
#include <cmath>
//boost stuff
#include <boost/foreach.hpp>
//root stuff
#include <TVector3.h>


using namespace std;
using namespace Belle2;

REG_MODULE(GenFitter2)

GenFitter2Module::GenFitter2Module() :
  Module()
{
  setDescription("Simplified trackfit module for debugging and testing new features before they go into the official GenFitter");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);
  addParam("useDaf", m_useDaf, "use the DAF instead of the std. Kalman filter", false);
  addParam("blowUpFactor", m_blowUpFactor, "factor multiplied with the cov of the Kalman filter when backward filter starts", 500.0);
  addParam("only6", m_filter, "throw away tracks with do not have exactly 1 hit in every Si layer (so 6 hits altogether)", false);
  addParam("filterIterations", m_nGFIter, "number of Genfit iterations", 1);
  addParam("probCut", m_probCut, "Probability cut for the DAF (0.001, 0.005, 0.01)", 0.001);
  addParam("energyLossBetheBloch", m_energyLossBetheBloch, "activate the material effect: EnergyLossBetheBloch", true);
  addParam("noiseBetheBloch", m_noiseBetheBloch, "activate the material effect: NoiseBetheBloch", true);
  addParam("noiseCoulomb", m_noiseCoulomb, "activate the material effect: NoiseCoulomb", true);
  addParam("energyLossBrems", m_energyLossBrems, "activate the material effect: EnergyLossBrems", true);
  addParam("noiseBrems", m_noiseBrems, "activate the material effect: NoiseBrems", true);
  addParam("noEffects", m_noEffects, "switch off all material effects in Genfit. This overwrites all individual material effects switches", false);
  addParam("angleCut", m_angleCut, "only process tracks with scattering angles smaller then angleCut (The angles are calculated from TrueHits). If negative value given no selection will take place", -1.0);
  addParam("mscModel", m_mscModel, "select the MSC model in Genfit", string("Highland"));
  addParam("hitType", m_hitType, "select what kind of hits are feeded to Genfit. Current Options \"TrueHit\", \"Cluster\" or \"VXDSimpleDigiHit\"", string("TrueHit"));

  addParam("dafTemperatures", m_dafTemperatures, "set the annealing scheme (temperatures) for the DAF. Length of vector will determine DAF iterations", vector<double>(1, -999.0));
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
    GFMaterialEffects::getInstance()->setNoEffects(true); //not yet possible in current basd2 genfit version (but already possible upstream)
  } else {
    GFMaterialEffects::getInstance()->setEnergyLossBetheBloch(m_energyLossBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseBetheBloch(m_noiseBetheBloch);
    GFMaterialEffects::getInstance()->setNoiseCoulomb(m_noiseCoulomb);
    GFMaterialEffects::getInstance()->setEnergyLossBrems(m_energyLossBrems);
    GFMaterialEffects::getInstance()->setNoiseBrems(m_noiseBrems);
  }
  GFMaterialEffects::getInstance()->setMscModel(m_mscModel);
  StoreArray<GFTrack> fittedTracks(""); //initialization of the the output container of this module

  //set options for fitting algorithms
  m_kalmanFilter.setNumIterations(m_nGFIter);
  m_kalmanFilter.setBlowUpFactor(m_blowUpFactor);
  m_daf.setProbCut(m_probCut);
  int nDafTemps = m_dafTemperatures.size();
  if (nDafTemps == 1 && m_dafTemperatures[0] < 0.0) { // user did not set an annealing scheme. Set the default one.
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
  } else if (nDafTemps <= 10 && nDafTemps >= 1) {
    m_dafTemperatures.resize(10, -1.0);
    m_daf.setBetas(m_dafTemperatures[0], m_dafTemperatures[1], m_dafTemperatures[2], m_dafTemperatures[3], m_dafTemperatures[4], m_dafTemperatures[5], m_dafTemperatures[6], m_dafTemperatures[7], m_dafTemperatures[8], m_dafTemperatures[9]);
//    cout << "m_dafTemperatures: ";
//    for ( int i = 0; i not_eq m_dafTemperatures.size(); ++i){
//      cout << m_dafTemperatures[i] << " | ";
//    }
//    cout << endl;
  } else {
    m_daf.setBetas(81, 8, 4, 1, 1, 1);
    B2ERROR("You either set 0 DAF temperatures or more than 10. This is not supported. The default scheme (81,8,4,1,1,1) was selected instead.");
  }

  //interpred the hittype option
  if (m_hitType == "TrueHit") {
    m_hitTypeId = 0;
  } else if (m_hitType == "VXDSimpleDigiHit") {
    m_hitTypeId = 1;
  } else if (m_hitType == "Cluster") {
    m_hitTypeId = 2;
  }
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
  B2DEBUG(100, "**********   GenFitter2Module  processing event number: " << eventCounter << " ************");
  StoreArray<GFTrackCand> trackCandidates("");
  int nTrackCandidates = trackCandidates.getEntries();
  if (nTrackCandidates == 0) {
    B2DEBUG(100, "GenFitter2: StoreArray<GFTrackCand> is empty!");
  }

  StoreArray<GFTrack> fittedTracks; //holds the output of this module in the form of Genfit track objects
  fittedTracks.create();

  for (int iTrackCand = 0; iTrackCand not_eq nTrackCandidates; ++iTrackCand) {


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

    StoreArray<VXDSimpleDigiHit> pxdSimpleDigiHits("pxdSimpleDigiHits");
    StoreArray<VXDSimpleDigiHit> svdSimpleDigiHits("svdSimpleDigiHits");
    B2DEBUG(100, "pxdSimpleDigiHits.getEntries() " << pxdSimpleDigiHits.getEntries());
    B2DEBUG(100, "svdSimpleDigiHits.getEntries() " << svdSimpleDigiHits.getEntries());

    //PXD clusters
    StoreArray<PXDCluster> pxdClusters("");
    int nPXDClusters = pxdClusters.getEntries();
    B2DEBUG(149, "GenFitter2: Number of PXDClusters: " << nPXDClusters);
    if (nPXDClusters == 0) {B2DEBUG(100, "GenFitter2: PXDClustersCollection is empty!");}

    //SVD clusters
    StoreArray<SVDCluster> svdClusters("");
    int nSVDClusters = svdClusters.getEntries();
    B2DEBUG(149, "GenFitter2: Number of SVDClusters: " << nSVDClusters);
    if (nSVDClusters == 0) {B2DEBUG(100, "GenFitter2: SVDClustersCollection is empty!");}


    GFTrackCand* aTrackCandPointer = trackCandidates[iTrackCand];
    int nTrackCandHits = aTrackCandPointer->getNHits();
    B2DEBUG(100, "nTrackCandHits " << nTrackCandHits);
    // if option is set ignore every track that does not have exactly 1 hit in every Si layer
    bool filterTrack = false;
    if (m_filter == true) {
      if (nTrackCandHits not_eq 6) {
        filterTrack = true;
        B2DEBUG(100, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
        ++m_notPerfectCounter;
      } else {
        vector<int> layerIds(nTrackCandHits);
        for (int i = 0; i not_eq nTrackCandHits; ++i) {
          unsigned int detId = UINT_MAX;
          unsigned int hitId = UINT_MAX;
          aTrackCandPointer->getHit(i, detId, hitId);
          int layerId = -1;
          if (detId == 0) {
            if (m_hitTypeId == 1) {
              layerId = pxdSimpleDigiHits[hitId]->getSensorID().getLayerNumber();
            } else if (m_hitTypeId == 0) {
              layerId = pxdTrueHits[hitId]->getSensorID().getLayerNumber();
            }
          }
          if (detId == 1) {
            if (m_hitTypeId == 1) {
              layerId = svdSimpleDigiHits[hitId]->getSensorID().getLayerNumber();
            } else if (m_hitTypeId == 0) {
              layerId = svdTrueHits[hitId]->getSensorID().getLayerNumber();
            }
          }
          layerIds[i] = layerId;
        }
        sort(layerIds.begin(), layerIds.end());
        for (int l = 0; l not_eq nTrackCandHits; ++l) {
          if (l + 1 not_eq layerIds[l]) {
            filterTrack = true;
            B2DEBUG(100, "Not exactly one hit in very Si layer. Track "  << eventCounter << " will not be reconstructed");
            ++m_notPerfectCounter;
            break;
          }
        }

      }
    }
    //find out if a track has a too large scattering angel and if yes ignore it ( only check if event was not filter out by the six hit only filter)
    if (m_angleCut > 0.0 and filterTrack == false) {
      // class to convert global and local coordinates into each other
      //VXD::GeoCache& aGeoCach = VXD::GeoCache::getInstance();
      for (int i = 0; i not_eq nTrackCandHits; ++i) {
        unsigned int detId = UINT_MAX;
        unsigned int hitId = UINT_MAX;
        aTrackCandPointer->getHit(i, detId, hitId);
        VXDTrueHit const* aVxdTrueHitPtr = NULL;
        if (detId == 0) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(pxdTrueHits[hitId]);
        }
        if (detId == 1) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(svdTrueHits[hitId]);
        }
        TVector3 pTrueIn = aVxdTrueHitPtr->getEntryMomentum();
        TVector3 pTrueOut = aVxdTrueHitPtr->getExitMomentum();
        //        const VXD::SensorInfoBase& aCoordTrans = aGeoCach.getSensorInfo(aVxdTrueHitPtr->getSensorID());
        //        TVector3 pTrueInGlobal = aCoordTrans.vectorToGlobal(pTrueIn);
        //        TVector3 pTrueOutGlobal = aCoordTrans.vectorToGlobal(pTrueOut);
        if (abs(pTrueIn.Angle(pTrueOut)) > m_angleCut) {
          filterTrack = true;
          ++m_largeAngleCounter;
          B2INFO("Scattering angle larger than "  << m_angleCut << ". Track " << eventCounter << " will not be reconstructed");
          break;
        }

      }

    }


    if (filterTrack == false) { // fit the track



      //get fit starting values from the MCParticle
      TVector3 vertex = aTrackCandPointer->getPosSeed();
      TVector3 vertexSigma = aTrackCandPointer->getPosError();
      TVector3 momentum = aTrackCandPointer->getDirSeed() * abs(1.0 / aTrackCandPointer->getQoverPseed());
      TVector3 dirSigma = aTrackCandPointer->getDirError();


      //B2DEBUG(100,"MCIndex: "<<mcindex);
      B2DEBUG(100, "Start values: momentum: " << momentum.x() << "  " << momentum.y() << "  " << momentum.z() << " " << momentum.Mag());
      B2DEBUG(100, "Start values: momentum std: " << dirSigma.x() << "  " << dirSigma.y() << "  " << dirSigma.z());
      B2DEBUG(100, "Start values: vertex:   " << vertex.x() << "  " << vertex.y() << "  " << vertex.z());
      B2DEBUG(100, "Start values: vertex std:   " << vertexSigma.x() << "  " << vertexSigma.y() << "  " << vertexSigma.z());
      B2DEBUG(100, "Start values: pdg:      " << aTrackCandPointer->getPdgCode());
      RKTrackRep* trackRep;
      //Now create a GenFit track with this representation

      trackRep = new RKTrackRep(aTrackCandPointer);
      trackRep->setPropDir(1); // setting the prop dir disables the automatic selection of the direction (but it still will automatically change when switching between forward and backward filter
      GFTrack track(trackRep, true);

      GFRecoHitFactory factory;


      GFRecoHitProducer <PXDTrueHit, PXDRecoHit> * PXDProducer;
      GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> * SVDProducer;
      GFRecoHitProducer <CDCHit, CDCRecoHit> * CDCProducer;

      GFRecoHitProducer <VXDSimpleDigiHit, PXDRecoHit> * pxdSimpleDigiHitProducer;
      GFRecoHitProducer <VXDSimpleDigiHit, SVDRecoHit2D> * svdSimpleDigiHitProducer;

      GFRecoHitProducer <PXDCluster, PXDRecoHit> * pxdClusterProducer;
      GFRecoHitProducer <SVDCluster, SVDRecoHit> * svdClusterProducer;

      //create RecoHitProducers for PXD, SVD and CDC
      if (m_hitTypeId == 0) { // use the trueHits
        PXDProducer =  new GFRecoHitProducer <PXDTrueHit, PXDRecoHit> (&*pxdTrueHits);
        SVDProducer =  new GFRecoHitProducer <SVDTrueHit, SVDRecoHit2D> (&*svdTrueHits);
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);
      } else if (m_hitTypeId == 1) {
        pxdSimpleDigiHitProducer =  new GFRecoHitProducer <VXDSimpleDigiHit, PXDRecoHit> (&*pxdSimpleDigiHits);
        svdSimpleDigiHitProducer =  new GFRecoHitProducer <VXDSimpleDigiHit, SVDRecoHit2D> (&*svdSimpleDigiHits);
      } else if (m_hitTypeId == 2) {
        pxdClusterProducer =  new GFRecoHitProducer <PXDCluster, PXDRecoHit> (&*pxdClusters);
        svdClusterProducer =  new GFRecoHitProducer <SVDCluster, SVDRecoHit> (&*svdClusters);
        CDCProducer =  new GFRecoHitProducer <CDCHit, CDCRecoHit> (&*cdcHits);
      }


      //add producers to the factory with correct detector Id

      if (m_hitTypeId == 0) { // use the trueHits
        factory.addProducer(0, PXDProducer);
        factory.addProducer(1, SVDProducer);
        factory.addProducer(2, CDCProducer);
      } else if (m_hitTypeId == 1) {
        factory.addProducer(0, pxdSimpleDigiHitProducer);
        factory.addProducer(1, svdSimpleDigiHitProducer);
      } else if (m_hitTypeId == 2) {
        factory.addProducer(0, pxdClusterProducer);
        factory.addProducer(1, svdClusterProducer);
        factory.addProducer(2, CDCProducer);
      }



      vector <GFAbsRecoHit*> factoryHits;
      //use the factory to create RecoHits for all Hits stored in the track candidate
      factoryHits = factory.createMany(*aTrackCandPointer);

      //add created hits to the track
      track.addHitVector(factoryHits);
      track.setCandidate(*aTrackCandPointer);

      B2DEBUG(100, "Total Nr of Hits assigned to the Track: " << track.getNumHits());
      /*for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
        GFAbsRecoHit* const aGFAbsRecoHitPtr = track.getHit(iHit);

        PXDRecoHit const* const aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const * const>(aGFAbsRecoHitPtr);
        SVDRecoHit2D const* const aSvdRecoHitPtr =  dynamic_cast<SVDRecoHit2D const * const>(aGFAbsRecoHitPtr);
        VXDTrueHit const*  aTrueHitPtr = NULL;

          if (aPxdRecoHitPtr not_eq NULL) {
            cout << "this is a pxd hit" << endl;
            aTrueHitPtr = static_cast<VXDTrueHit const*>(aPxdRecoHitPtr->getTrueHit());
          } else {
            cout << "this is a svd hit" << endl;
            aTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHitPtr->getTrueHit());
          }

          //cout << "aVxdTrueHitPtr->getU() " << aTrueHitPtr->getU() << endl;
          //cout << "aVxdTrueHitPtr->getGlobalTime() " << aTrueHitPtr->getGlobalTime()<< endl;
          cout << "layerId " << aTrueHitPtr->getSensorID().getLayer()<< endl;
      }*/
      //process track (fit them!)
      if (m_useDaf == false) {
        m_kalmanFilter.processTrack(&track);
      } else {
        m_daf.processTrack(&track);
      }

      int genfitStatusFlag = trackRep->getStatusFlag();
      B2DEBUG(100, "----> Status of fit: " << genfitStatusFlag);
      B2DEBUG(100, "-----> Fit Result: momentum: " << track.getMom().x() << "  " << track.getMom().y() << "  " << track.getMom().z() << " " << track.getMom().Mag());
      B2DEBUG(100, "-----> Fit Result: current position: " << track.getPos().x() << "  " << track.getPos().y() << "  " << track.getPos().z());
      B2DEBUG(100, "----> Chi2 of the fit: " << track.getChiSqu());
      B2DEBUG(100, "----> NDF of the fit: " << track.getNDF());
      //          track.Print();
      //          for ( int iHit = 0; iHit not_eq track.getNumHits(); ++iHit){
      //            track.getHit(iHit)->Print();
      //          }

      if (genfitStatusFlag == 0) {
        fittedTracks.appendNew(track);
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
    B2INFO(m_notPerfectCounter << " of " << m_fitCounter + m_failedFitCounter + m_notPerfectCounter << " tracks had not exactly on hit in every layer and were not fitted");
  }
  if (m_largeAngleCounter != 0) {
    B2WARNING(m_largeAngleCounter << " of " << m_fitCounter + m_failedFitCounter + m_largeAngleCounter << " had a scattering larger than " << m_angleCut <<  " rad and were not fitted");
  }
  if (m_failedFitCounter != 0) {
    B2WARNING(m_failedFitCounter << " of " << m_fitCounter + m_failedFitCounter << " tracks could not be fitted in this run");
  }

}

void GenFitter2Module::terminate()
{

}
