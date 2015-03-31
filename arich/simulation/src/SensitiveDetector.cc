/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <arich/simulation/SensitiveDetector.h>
#include <framework/logging/Logger.h>
#include <arich/dataobjects/ARICHSimHit.h>

// geant4
#include <G4OpBoundaryProcess.hh>
#include <G4ProcessManager.hh>


#include <simulation/kernel/UserInfo.h>
#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <TVector2.h>
#include <TRandom3.h>

using namespace std;

namespace Belle2 {
  namespace arich {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("ARICH", Const::ARICH),
      m_arichgp(ARICHGeometryPar::Instance())
    {

      StoreArray<MCParticle> particles;
      StoreArray<ARICHSimHit> hits;
      RelationArray relation(particles, hits);
      registerMCParticleRelation(relation);
      hits.registerInDataStore();
      particles.registerRelationTo(hits);
    }


    G4bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {
      //Get particle ID
      G4Track& track  = *aStep->GetTrack();
      if (track.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      //Get time (check for proper global time)
      const G4double globalTime = track.GetGlobalTime();
      if (std::isnan(globalTime)) {
        B2ERROR("ARICH Sensitive Detector: global time is nan !");
        return false;
      }

      const G4StepPoint& postStep  = *aStep->GetPostStepPoint();
      const G4ThreeVector& postworldPosition = postStep.GetPosition();

      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
      const G4ThreeVector& preworldPosition = preStep.GetPosition();

      // direction of photon (+1 is into hapd)
      int dir = -1;
      if (postworldPosition.z() - preworldPosition.z() > 0) dir = +1;

      // trigger only on window geometrical boundary
      if (dir == 1 && postStep.GetStepStatus() != fGeomBoundary) { return false;}
      if (dir == -1 && preStep.GetStepStatus() != fGeomBoundary) { return false;}

      if ((track.GetNextVolume())->GetName() == "moduleWall") return false;

      // Check if photon is internally reflected in HAPD window
      G4OpBoundaryProcessStatus theStatus = Undefined;

      G4ProcessManager* OpManager = G4OpticalPhoton::OpticalPhoton()->GetProcessManager();

      if (OpManager) {
        G4int MAXofPostStepLoops =
          OpManager->GetPostStepProcessVector()->entries();
        G4ProcessVector* fPostStepDoItVector =
          OpManager->GetPostStepProcessVector(typeDoIt);
        for (G4int i = 0; i < MAXofPostStepLoops; i++) {
          G4VProcess* fCurrentProcess = (*fPostStepDoItVector)[i];
          G4OpBoundaryProcess* opProcess = dynamic_cast<G4OpBoundaryProcess*>(fCurrentProcess);
          if (opProcess) { theStatus = opProcess->GetStatus(); break;}
        }
      }

      // if photon is internally reflected and going backward, do nothing
      if (theStatus == 3 && dir < 0) return 0;



      // apply quantum efficiency if not yet done
      bool applyQE = true;
      Simulation::TrackInfo* info =
        dynamic_cast<Simulation::TrackInfo*>(track.GetUserInformation());
      if (info) applyQE = info->getStatus() < 2;
      if (applyQE) {
        double energy = track.GetKineticEnergy() * Unit::MeV / Unit::eV;
        double qeffi  = m_arichgp->QE(energy) * m_arichgp->getColEffi();
        double fraction = info->getFraction();
        //correct Q.E. for internally reflected photons
        if (theStatus == 3) qeffi *= m_arichgp->getQEScaling();
        if (gRandom->Uniform() * fraction > qeffi) {
          // apply possible absorbtion in HAPD window (for internally reflected photons only)
          if (theStatus == 3 && gRandom->Uniform() < m_arichgp->getWindowAbsorbtion()) track.SetTrackStatus(fStopAndKill);
          return false;
        }
      }

      //Get module ID number
      const G4int moduleID = dir > 0 ? postStep.GetTouchableHandle()->GetReplicaNumber(0) :
                             preStep.GetTouchableHandle()->GetReplicaNumber(1);

      //Transform to local position
      G4ThreeVector localPosition = dir > 0 ? postStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(
                                      postworldPosition) :
                                    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(preworldPosition);

      //Get photon energy
      const G4double energy = track.GetKineticEnergy() / CLHEP::eV;

      //------------------------------------------------------------
      //                Create ARICHSimHit and save it to datastore
      //------------------------------------------------------------

      TVector2 locpos(localPosition.x() / CLHEP::cm, localPosition.y() / CLHEP::cm);
      StoreArray<ARICHSimHit> arichSimHits;
      if (!arichSimHits.isValid()) arichSimHits.create();
      ARICHSimHit* simHit = arichSimHits.appendNew(moduleID, locpos, globalTime, energy);

      // add relation to MCParticle
      StoreArray<MCParticle> mcParticles;
      RelationArray arichSimHitRel(mcParticles, arichSimHits);
      arichSimHitRel.add(track.GetParentID(), simHit->getArrayIndex());

      // after detection photon track is killed
      track.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace arich
} // end of namespace Belle2
