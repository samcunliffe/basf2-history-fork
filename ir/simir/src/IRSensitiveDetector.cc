/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Hiroshi Nakano, Andreas Moll                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ir/simir/IRSensitiveDetector.h>
#include <ir/dataobjects/IRVolumeUserInfo.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>

// Geant4
#include <G4Types.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Step.hh>
#include <G4UserLimits.hh>

#include <string>

using namespace std;
using namespace Belle2;
using namespace Simulation;

IRSensitiveDetector::IRSensitiveDetector(G4String name) :
    SensitiveDetectorBase(name)
{
  // Tell the framework that this sensitive detector creates
  // a relation MCParticles -> IRSimHits
  addRelationCollection(DEFAULT_IRSIMHITSREL);
  StoreArray<Relation> mcPartRelation(getRelationCollectionName());
  StoreArray<IRSimHit> irArray(DEFAULT_IRSIMHITS);
}

void IRSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
{ }

IRSensitiveDetector::~IRSensitiveDetector()
{ }

G4bool IRSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  const G4Track& track  = *step->GetTrack();
  const G4int trackID = track.GetTrackID();
  const G4int partPDGCode = track.GetDefinition()->GetPDGEncoding();

  const G4VPhysicalVolume& g4Volume    = *track.GetVolume();
  string Volname = g4Volume.GetName();

  const G4double depEnergy = step->GetTotalEnergyDeposit() * Unit::MeV;

  const G4StepPoint& preStep      = *step->GetPreStepPoint();
  const G4StepPoint& postStep      = *step->GetPostStepPoint();
  const G4ThreeVector& preStepPos = preStep.GetPosition();
  const G4ThreeVector& postStepPos = postStep.GetPosition();
  const G4ThreeVector momIn(preStep.GetMomentum());
  const G4ThreeVector momOut(postStep.GetMomentum());
  const G4ThreeVector preStepPosLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(preStepPos);
  const G4ThreeVector postStepPosLocal = postStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(postStepPos);
  const G4ThreeVector momInLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformAxis(momIn);
  const G4ThreeVector momOutLocal = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformAxis(momOut);
  TVector3 posInVec(preStepPosLocal.x() * Unit::mm, preStepPosLocal.y() * Unit::mm, preStepPosLocal.z() * Unit::mm);
  TVector3 posOutVec(postStepPosLocal.x() * Unit::mm, postStepPosLocal.y() * Unit::mm, postStepPosLocal.z() * Unit::mm);
  TVector3 momInVec(momInLocal.x() * Unit::MeV, momInLocal.y() * Unit::MeV, momInLocal.z() * Unit::MeV);
  TVector3 momOutVec(momOutLocal.x() * Unit::MeV, momOutLocal.y() * Unit::MeV, momOutLocal.z() * Unit::MeV);

  // B2INFO("Step in volume: " << g4Volume.GetName())
  // check that user limits are set properly
  G4UserLimits* userLimits = g4Volume.GetLogicalVolume()->GetUserLimits();
  if (userLimits) {
    B2DEBUG(100, "Volume " << g4Volume.GetName() << ": max. allowed step set to " << userLimits->GetMaxAllowedStep(track))
  }

  StoreArray<IRSimHit> irArray(DEFAULT_IRSIMHITS);
  int hitIndex = irArray->GetLast() + 1 ;
  new(irArray->AddrAt(hitIndex)) IRSimHit(posInVec,
                                          momInVec,
                                          posOutVec,
                                          momOutVec,
                                          partPDGCode,
                                          depEnergy,
                                          Volname
                                         );

  // Add relation between the MCParticle and the hit.
  // The index of the MCParticle has to be set to the TrackID and will be
  // replaced later by the correct MCParticle index automatically.
  StoreArray<Relation> mcPartRelation(getRelationCollectionName());
  StoreArray<MCParticle> mcPartArray(DEFAULT_MCPARTICLES);
  new(mcPartRelation->AddrAt(hitIndex)) Relation(mcPartArray, irArray, trackID, hitIndex);

  return true;
}
