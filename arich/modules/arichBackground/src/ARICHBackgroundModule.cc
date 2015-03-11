/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include

#include <arich/modules/arichBackground/ARICHBackgroundModule.h>
#include <framework/core/ModuleManager.h>
#include <time.h>

// Hit classes

#include <simulation/dataobjects/BeamBackHit.h>
#include <arich/dataobjects/ARICHSimHit.h>
#include <arich/dataobjects/ARICHDigit.h>
#include <top/dataobjects/TOPSimHit.h>
#include <pxd/dataobjects/PXDSimHit.h>

#include <mdst/dataobjects/MCParticle.h>
// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TVector2.h>
#include <TClonesArray.h>

using namespace std;
using namespace boost;

namespace Belle2 {
  namespace arich {
    //-----------------------------------------------------------------
    //                 Register the Module
    //-----------------------------------------------------------------

    REG_MODULE(ARICHBackground)


    //-----------------------------------------------------------------
    //                 Implementation
    //-----------------------------------------------------------------

    ARICHBackgroundModule::ARICHBackgroundModule() : Module(),
      m_arichgp(ARICHGeometryPar::Instance())
    {
      // Set description()
      setDescription("ARICHBackground module. Used to extract information relevant for ARICH background from background files");

      // Add parameters
      addParam("FileName", m_filename, "output file name", string("mytree.root"));
      addParam("BkgType", m_bkgType, "source of background (RBB_HER(LER),Touschek_HER(LER),Coulomb_HER(LER),other", string("RBB_HER"));

    }

    ARICHBackgroundModule::~ARICHBackgroundModule()
    {
      if (m_arichgp) delete m_arichgp;
    }

    void ARICHBackgroundModule::initialize()
    {
      // Print set parameters
      printModuleParams();

      ff = new TFile(m_filename.c_str(), "RECREATE");

      TrHits = new TTree("TrHits", "tree of arich background hits");
      TrHits->Branch("source", &source, "source/I");
      TrHits->Branch("phpos", "TVector3", &phpos);
      TrHits->Branch("phmom", "TVector3", &phmom);
      TrHits->Branch("phVtx", "TVector3", &phVtx);
      TrHits->Branch("phMmom", "TVector3", &phMmom);
      TrHits->Branch("phMvtx", "TVector3", &phMvtx);
      TrHits->Branch("phPvtx", "TVector3", &phPvtx);
      TrHits->Branch("phPmom", "TVector3", &phPmom);
      TrHits->Branch("phGMvtx", "TVector3", &phGMvtx);
      TrHits->Branch("phGMmom", "TVector3", &phGMmom);
      TrHits->Branch("modOrig", "TVector3", &modOrig);
      TrHits->Branch("phPDG", &phPDG, "phPDG/I");
      TrHits->Branch("phMPDG", &phMPDG, "phMPDG/I");
      TrHits->Branch("phPPDG", &phPPDG, "phPPDG/I");
      TrHits->Branch("phGMPDG", &phGMPDG, "phGMPDG/I");
      TrHits->Branch("type", &type, "type/I");
      TrHits->Branch("edep", &edep, "edep/D");
      TrHits->Branch("ttime", &ttime, "ttime/D");
      TrHits->Branch("moduleID", &moduleID, "moduleID/I");
      TrHits->Branch("phnw", &phnw, "phnw/D");
      TrHits->Branch("trlen", &trlen, "trlen/D");
      TrHits->Branch("en", &en, "en/D");


      if (m_bkgType == "RBB_HER")      source = 1;
      else if (m_bkgType == "RBB_LER")      source = 2;
      else if (m_bkgType == "Touschek_HER") source = 3;
      else if (m_bkgType == "Touschek_LER") source = 4;
      else if (m_bkgType == "Coulomb_HER")  source = 5;
      else if (m_bkgType == "Coulomb_LER")  source = 6;
      else                                source = atoi(m_bkgType.c_str());

    }

    void ARICHBackgroundModule::beginRun()
    {
      // Print run number
      B2INFO("ARICHBackground: Processing. ");

    }

    void ARICHBackgroundModule::event()
    {

      StoreArray<MCParticle> mcParticles;

      StoreArray<ARICHSimHit>  arichSimHits;

      StoreArray<BeamBackHit> beamBackHits;

      StoreArray<ARICHDigit> arichDigits;

      int nHits = beamBackHits.getEntries();

      for (int iHit = 0; iHit < nHits; ++iHit) {

        BeamBackHit* arichhit = beamBackHits[iHit];
        int subdet = arichhit->getSubDet();
        if (subdet != 4) continue;
        type = 0;
        if (arichhit->getIdentifier() == 1) type = 1;
        edep = arichhit->getEnergyDeposit();
        ttime = arichhit->getTime();
        phPDG = arichhit->getPDG();
        phpos = arichhit->getPosition();
        phmom = arichhit->getMomentum();
        moduleID = m_arichgp->getCopyNo(phpos);
        modOrig = m_arichgp->getOrigin(moduleID);
        en = arichhit->getEnergy();

        if (phPDG == 2112) {
          phnw = arichhit->getNeutronWeight();
          trlen = arichhit->getTrackLength();
        }
        phVtx = TVector3(0, 0, 0); phMvtx = TVector3(0, 0, 0);  phMmom = TVector3(0, 0, 0);
        phMPDG = -1; phPPDG = -1; phGMPDG = -1;
        phPvtx = TVector3(0, 0, 0); phPmom = TVector3(0, 0, 0); phGMvtx = TVector3(0, 0, 0); phGMmom = TVector3(0, 0, 0);

        RelationIndex<MCParticle, BeamBackHit> relBeamBackHitToMCParticle(mcParticles, beamBackHits);
        if (relBeamBackHitToMCParticle.getFirstElementTo(arichhit)) {
          const MCParticle* currParticle = relBeamBackHitToMCParticle.getFirstElementTo(arichhit)->from;
          phVtx = currParticle->getVertex();
          const MCParticle* mother = currParticle->getMother();
          int mm = 0;
          while (mother) {
            if (mm == 0) {
              phMPDG = mother->getPDG();
              phMvtx = mother->getVertex();
              phMmom = mother->getMomentum();
            }
            if (mm == 1) {
              phGMPDG = mother->getPDG();
              phGMvtx = mother->getVertex();
              phGMmom = mother->getMomentum();
            }
            const MCParticle* pommother = mother->getMother();
            if (!pommother) {
              phPPDG = mother->getPDG();
              phPvtx = mother->getVertex();
              phPmom = mother->getMomentum();
            }
            mother = pommother;
            mm++;
          }

        }
        TrHits->Fill();
      }

      nHits = arichSimHits.getEntries();

      for (int iHit = 0; iHit < nHits; ++iHit) {
        ARICHSimHit* simHit = arichSimHits[iHit];
        TVector2 locpos(simHit->getLocalPosition().X(), simHit->getLocalPosition().Y());
        int channelID = m_arichgp->getChannelID(locpos);

        if (channelID < 0) continue;
        moduleID = simHit->getModuleID();
        modOrig = m_arichgp->getOrigin(moduleID);
        phpos = m_arichgp->getChannelCenterGlob(moduleID, channelID);
        type = 2;
        phPDG = 0;
        edep = 0;
        ttime = simHit->getGlobalTime();

        phVtx = TVector3(0, 0, 0); phMvtx = TVector3(0, 0, 0);  phMmom = TVector3(0, 0, 0);
        phMPDG = -1; phPPDG = -1; phGMPDG = -1; phmom = TVector3(0, 0, 0);
        phPvtx = TVector3(0, 0, 0); phPmom = TVector3(0, 0, 0); phGMvtx = TVector3(0, 0, 0); phGMmom = TVector3(0, 0, 0);
        RelationIndex<MCParticle, ARICHSimHit> relSimHitToMCParticle(mcParticles, arichSimHits);
        if (relSimHitToMCParticle.getFirstElementTo(simHit)) {
          const MCParticle* currParticle = relSimHitToMCParticle.getFirstElementTo(simHit)->from;
          phVtx = currParticle->getVertex();
          const MCParticle* mother = currParticle->getMother();
          int mm = 0;
          while (mother) {
            if (mm == 0) {
              phMPDG = mother->getPDG();
              phMvtx = mother->getVertex();
              phMmom = mother->getMomentum();
            }
            if (mm == 1) {
              phGMPDG = mother->getPDG();
              phGMvtx = mother->getVertex();
              phGMmom = mother->getMomentum();
            }
            const MCParticle* pommother = mother->getMother();
            if (!pommother) {
              phPPDG = mother->getPDG();
              phPvtx = mother->getVertex();
              phPmom = mother->getMomentum();
            }
            mother = pommother;
            mm++;
          }

        }
        TrHits->Fill();
      }
    }

    void ARICHBackgroundModule::endRun()
    {
    }

    void ARICHBackgroundModule::terminate()
    {
      // CPU time end

      // Announce
      B2INFO("ARICHBackground finished.");

      //TrHits->Write();
      ff->Write();
      ff->Close();

    }

    void ARICHBackgroundModule::printModuleParams() const
    {

    }


  } // end arich namespace
} // end Belle2 namespace
