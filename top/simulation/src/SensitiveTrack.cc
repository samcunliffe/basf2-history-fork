/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitiveTrack.h>
#include <top/dataobjects/TOPBarHit.h>
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
      Simulation::SensitiveDetectorBase("TOP", SensitiveTrack::TOP),
      m_topgp(TOPGeometryPar::Instance())
    {
      // registration
      StoreArray<TOPBarHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPBarHit>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPBarHit>  barhits;
      RelationArray  relation(particles, barhits);
      registerMCParticleRelation(relation);
    }


    bool SensitiveTrack::step(G4Step* aStep, G4TouchableHistory*)
    {

      m_topgp->setGeanUnits();

      //! get particle track
      G4Track* aTrack = aStep->GetTrack();

      //! check which particle hits the bar
      G4ParticleDefinition* particle = aTrack->GetDefinition();
      int  PDG = (int)(particle->GetPDGEncoding());

      // Save all particles except optical photons
      if (PDG == 0) return false;

      //! get the preposition, a step before current position
      G4StepPoint* PrePosition =  aStep->GetPreStepPoint();

      //! Check that the hit comes from the bar boundary
      if (PrePosition->GetStepStatus() != fGeomBoundary) return false;

      //! get lab frame position of the prestep point
      G4ThreeVector worldPosition = PrePosition->GetPosition();

      //! Transform lab frame to bar frame
      G4ThreeVector localPosition = PrePosition->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(worldPosition);

      //! Check that it is not on the glue boundary or similar
      //!!!! this check is not a whole story -> to be re-written
      if (fabs(fabs(localPosition.y()) - (m_topgp->getQthickness() / 2)) > 10e-6) {
        return false ;
      }

      //! Get track ID
      int trackID = aTrack->GetTrackID();

      //! get track length and subtract step length to get the length to the boundary
      double tracklength = aTrack->GetTrackLength() - aStep->GetStepLength();

      //! get global time
      double globalTime = PrePosition->GetGlobalTime();

      //! momentum on the boundary
      G4ThreeVector momentum = PrePosition->GetMomentum();

      //! Fill three vectors that hold momentum and position
      TVector3 TPosition(worldPosition.x(), worldPosition.y(), worldPosition.z());
      TVector3 TMomentum(momentum.x(), momentum.y(), momentum.z());
      TVector3 TOrigin(aTrack->GetVertexPosition().x(),
                       aTrack->GetVertexPosition().y(),
                       aTrack->GetVertexPosition().z());

      //! convert to Basf units
      TPosition = TPosition * Unit::mm;
      TMomentum = TMomentum  * Unit::MeV;
      TOrigin = TOrigin * Unit::mm;
      tracklength = tracklength * Unit::mm;

      //! Get bar ID
      int barID = PrePosition->GetTouchableHandle()->GetReplicaNumber(2);

      //! write the hit to datastore
      StoreArray<TOPBarHit> barHits;
      if (!barHits.isValid()) barHits.create();
      new(barHits.nextFreeAddress()) TOPBarHit(barID, PDG, TOrigin, TPosition,
                                               TMomentum, globalTime, tracklength);

      //! set the relation
      StoreArray<MCParticle> mcParticles;
      RelationArray rel(mcParticles, barHits);
      int last = barHits.getEntries() - 1;
      rel.add(trackID, last);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
