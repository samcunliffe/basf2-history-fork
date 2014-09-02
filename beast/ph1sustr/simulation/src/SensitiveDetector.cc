/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/ph1sustr/simulation/SensitiveDetector.h>
#include <beast/ph1sustr/dataobjects/Ph1sustrSimHit.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>

#include <G4Track.hh>
#include <G4Step.hh>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the PH1SUSTR detector */
  namespace ph1sustr {

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("Ph1sustrSensitiveDetector", Const::invalidDetector)
    {
      //Make sure all collections are registered
      StoreArray<MCParticle>   mcParticles;
      StoreArray<Ph1sustrSimHit>  simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      //Register all collections we want to modify and require those we want to use
      mcParticles.registerInDataStore();
      simHits.registerInDataStore();
      relMCSimHit.registerInDataStore();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(relMCSimHit);
    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      //Get Track information
      const G4Track& track    = *step->GetTrack();
      const int trackID       = track.GetTrackID();
      const double depEnergy  = step->GetTotalEnergyDeposit() * Unit::MeV;
      //const int detNb = step->GetTrack()->GetVolume()->GetCopyNo();

      //Ignore everything below 1eV
      if (depEnergy < Unit::eV) return false;



      //Get the datastore arrays
      StoreArray<MCParticle>  mcParticles;
      StoreArray<Ph1sustrSimHit> simHits;
      RelationArray relMCSimHit(mcParticles, simHits);

      const int hitIndex = simHits.getEntries();
      StoreArray<Ph1sustrSimHit> Ph1sustrHits;
      if (!Ph1sustrHits.isValid()) Ph1sustrHits.create();
      /*Ph1sustrSimHit* hit = Ph1sustrHits.appendNew(
                              depEnergy,
                              detNb
            );*/

      //Add Relation between SimHit and MCParticle with a weight of 1. Since
      //the MCParticle index is not yet defined we use the trackID from Geant4
      relMCSimHit.add(trackID, hitIndex, 1.0);

      return true;
    }

  } //ph1sustr namespace
} //Belle2 namespace
