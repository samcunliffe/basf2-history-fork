/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <top/simulation/SensitivePMT.h>
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPSimPhoton.h>

#include <G4Track.hh>
#include <G4Step.hh>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <TVector3.h>

using namespace std;

namespace Belle2 {
  namespace TOP {

    SensitivePMT::SensitivePMT():
      Simulation::SensitiveDetectorBase("TOP", SensitivePMT::TOP)
    {
      // registration
      StoreArray<TOPSimHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, TOPSimHit>();
      StoreArray<TOPSimPhoton>::registerTransient();
      RelationArray::registerTransient<TOPSimHit, TOPSimPhoton>();

      // additional registration of MCParticle relation (required for correct relations)
      StoreArray<MCParticle> particles;
      StoreArray<TOPSimHit>  hits;
      RelationArray  relation(particles, hits);
      registerMCParticleRelation(relation);
    }


    G4bool SensitivePMT::step(G4Step* aStep, G4TouchableHistory*)
    {
      // photon track
      G4Track& photon  = *aStep->GetTrack();

      // check if it is realy an optical photon
      if (photon.GetDefinition()->GetParticleName() != "opticalphoton") return false;

      // pmt and bar ID
      int pmtID = photon.GetTouchableHandle()->GetReplicaNumber(1);
      int barID = photon.GetTouchableHandle()->GetReplicaNumber(4);

      // photon at detection
      const G4ThreeVector& g_detPoint = photon.GetPosition();
      const G4ThreeVector& g_detMomDir = photon.GetMomentumDirection();
      TVector3 detPoint(g_detPoint.x(), g_detPoint.y(), g_detPoint.z());
      TVector3 detMomDir(g_detMomDir.x(), g_detMomDir.y(), g_detMomDir.z());
      double detTime = photon.GetGlobalTime();
      double energy = photon.GetKineticEnergy();
      double length = photon.GetTrackLength();

      // photon at emission
      const G4ThreeVector& g_emiPoint = photon.GetVertexPosition();
      const G4ThreeVector& g_emiMomDir = photon.GetVertexMomentumDirection();
      TVector3 emiPoint(g_emiPoint.x(), g_emiPoint.y(), g_emiPoint.z());
      TVector3 emiMomDir(g_emiMomDir.x(), g_emiMomDir.y(), g_emiMomDir.z());
      double emiTime = photon.GetGlobalTime() - photon.GetLocalTime();

      // convert to Basf units (photon energy in [eV]!)
      emiPoint = emiPoint * Unit::mm;
      detPoint = detPoint * Unit::mm;
      energy = energy * Unit::MeV / Unit::eV;
      length = length * Unit::mm;

      // hit position in local frame, converted to Basf units
      G4ThreeVector localPosition = photon.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(g_detPoint);
      double xLocal = localPosition.x() * Unit::mm;
      double yLocal = localPosition.y() * Unit::mm;

      // write to store arrays; add relations

      StoreArray<TOPSimHit> hits;
      if (!hits.isValid()) hits.create();
      new(hits.nextFreeAddress()) TOPSimHit(barID, pmtID, xLocal, yLocal,
                                            detTime, energy);

      StoreArray<MCParticle> particles;
      RelationArray relParticleHit(particles, hits);
      int lastHit = hits.getEntries() - 1;
      int parentID = photon.GetParentID();
      relParticleHit.add(parentID, lastHit);

      StoreArray<TOPSimPhoton> photons;
      if (!photons.isValid()) photons.create();
      new(photons.nextFreeAddress()) TOPSimPhoton(barID, emiPoint, emiMomDir, emiTime,
                                                  detPoint, detMomDir, detTime,
                                                  length, energy);

      RelationArray relHitPhot(hits, photons);
      int lastPhot = photons.getEntries() - 1;
      relHitPhot.add(lastHit, lastPhot);

      // kill photon after detection
      photon.SetTrackStatus(fStopAndKill);

      return true;
    }


  } // end of namespace top
} // end of namespace Belle2
