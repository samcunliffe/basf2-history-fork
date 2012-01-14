/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveTrack.h>
#include <top/dataobjects/TOPTrack.h>
#include <top/geometry/TOPGeometryPar.h>

#include <G4Step.hh>
#include <G4Track.hh>
#include <G4UnitsTable.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitiveTrack::SensitiveTrack():
      Simulation::SensitiveDetectorBase("TOP", SensitiveTrack::TOP), m_topgp(TOPGeometryPar::Instance())
    {
      //! MCPacrticle store array needed for creation of relations
      StoreArray<MCParticle> mcParticles;
      //! TOPTracks into which the tracks will be stored
      StoreArray<TOPTrack>  topTracks;

      //! The relation array between MCParticle and TOPTrack
      RelationArray  relTOPTrackToMCParticle(topTracks, mcParticles);
      //! Registraction of the relation array
      registerMCParticleRelation(relTOPTrackToMCParticle);

    }


    bool SensitiveTrack::step(G4Step* aStep, G4TouchableHistory*)
    {

      //! get particle track
      G4Track* aTrack = aStep->GetTrack();

      //! check which particle did hit the bar
      G4ParticleDefinition* particle = aTrack->GetDefinition();

      //! query for it's PDG number
      int  PDG = (int)(particle->GetPDGEncoding());

      // Save all tracks excluding opticalphotons
      if (PDG == 0) return false;

      //! get the preposition, a step before current position
      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();

      //! get lab frame position of the prestep point
      G4ThreeVector worldPosition = PrePosition->GetPosition();

      //! Check that the hit come from the boundary
      if (PrePosition->GetStepStatus() != fGeomBoundary) return false;

      //! Transform lab frame to bar frame
      G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //! Check that it is on the outside boundary not on the glue boundary
      if (fabs(fabs(localPosition.y()) - (m_topgp->getQthickness() / 2.0)) > 10e-6) return false ;

      //! This few lines are for debugging
      /*
       B2INFO("SensitiveTrack: " << aTrack->GetDefinition()->GetParticleName()
       << " " << aTrack->GetTrackID()
       << " " << aTrack->GetParentID()
       << " " << G4BestUnit(localPosition, "Length")
       << " " << G4BestUnit(worldPosition, "Length")
       << " " << G4BestUnit(aTrack->GetMomentum(), "Energy")
       << " " << G4BestUnit(aTrack->GetGlobalTime(), "Time")
       << " Edep is " << G4BestUnit(aStep->GetTotalEnergyDeposit(), "Energy"));*/

      //! Get track ID
      int trackID = aTrack->GetTrackID();

      //! get track length and subtract step length to get the length to the boundary
      double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();

      //! get the global time
      const G4double globalTime = PrePosition->GetGlobalTime();

      //! get the local time
      const G4double localTime = PrePosition->GetLocalTime();

      //! the momentum on the boundary
      G4ThreeVector momentum = PrePosition->GetMomentum();

      //! calculate momentum at vertex position
      double vmomentum = sqrt(aTrack->GetVertexKineticEnergy() * aTrack->GetVertexKineticEnergy() + 2 * aTrack->GetVertexKineticEnergy() * particle->GetPDGMass());

      //! Fill three vectors that hold momentum and position
      TVector3 TPosition(worldPosition.x() , worldPosition.y() , worldPosition.z());
      TVector3 TMomentum(momentum.x() , momentum.y()  , momentum.z());

      TVector3 TVPosition(aTrack->GetVertexPosition().x() , aTrack->GetVertexPosition().y() , aTrack->GetVertexPosition().z());
      TVector3 TVMomentum(vmomentum * aTrack->GetVertexMomentumDirection().x(), vmomentum * aTrack->GetVertexMomentumDirection().y() , vmomentum * aTrack->GetVertexMomentumDirection().z());

      //! Get the ID of the bar that was hit
      int barID = PrePosition->GetTouchableHandle()->GetReplicaNumber(2);

      //!Get the charge of the particle
      int PDGCharge = (int)particle->GetPDGCharge();


      /*!------------------------------------------------------------
       *                Create TOPTrack and save it to datastore
       * ------------------------------------------------------------
       */

      //! Define TOPTrack array to which the hit will be stored
      StoreArray<TOPTrack> topTracks;

      //! get the number of already stored topTracks
      G4int nentr = topTracks->GetEntries();

      //! Store hit
      new(topTracks->AddrAt(nentr)) TOPTrack(trackID, PDG, PDGCharge, TPosition, TVPosition, TMomentum, TVMomentum, barID, tracklength, globalTime, localTime);


      /*!--------------------------------------------------------------------------
       *                Make relation between TOPTracks and MCParticle
       * --------------------------------------------------------------------------
       */

      //! Define the MCParticle class to be used for relation definition
      StoreArray<MCParticle> mcParticles;

      //! Define the relation array
      RelationArray relTOPTrackToMCParticle(topTracks, mcParticles);
      //! add the relation
      relTOPTrackToMCParticle.add(nentr, trackID);

      //! everything done successfully
      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
