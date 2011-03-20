/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/simpxd/PXDSensitiveDetector.h>
#include <pxd/dataobjects/PXDVolumeUserInfo.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>
#include <framework/gearbox/Unit.h>

#include <TVector3.h>
#include <TGeoManager.h>
#include <TG4RootNavMgr.h>

// Geant4
#include <G4Types.hh>
#include <G4ThreeVector.hh>
#include <G4Track.hh>
#include <G4VPhysicalVolume.hh>
#include <G4Step.hh>
#include <G4UserLimits.hh>

#include <string>
#include <cmath>

using namespace std;
using namespace Belle2;
using namespace Simulation;

PXDSensitiveDetector::PXDSensitiveDetector(G4String name) :
    SensitiveDetectorBase(name)
{
  // Set threshold for energy deposition.
  // MPV loss of a MIP is
  const double mpvLossMIP = 388 * Unit::eV / Unit::um;
  // In a 75 um thick detector, this goes down to (lower bound)
  const double mpvLossThin = 0.6 * mpvLossMIP;
  // We set threshold to a fraction of this (note the threshold is in energy/length):
  m_thresholdPXD = 0.1 * mpvLossThin;

  //Tell the framework that this sensitive detector creates
  //a relation MCParticles -> PXDSimHits
  addRelationCollection(DEFAULT_PXDSIMHITSREL);

  StoreArray<Relation> mcPartRelation(getRelationCollectionName());
  StoreArray<PXDSimHit> pxdArray(DEFAULT_PXDSIMHITS);
}

void PXDSensitiveDetector::Initialize(G4HCofThisEvent* HCTE)
{
  // Get DetectorConstruction class to map G4 volumes to TGeo - for UserInfo.
  TG4RootNavMgr* navMgr = TG4RootNavMgr::GetInstance(gGeoManager);
  if (navMgr == NULL) {
    B2ERROR("Could not retrieve an instance of the TG4RootNavMgr !")
  } else {
    m_detMap = navMgr->GetDetConstruction();
  }
  if (m_detMap == NULL) {
    B2ERROR("Detector Construction unavailable!!!")
  };
}

PXDSensitiveDetector::~PXDSensitiveDetector()
{

}

G4bool PXDSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory*)
{
  //--------------------------------------------------------------------------------
  //               Collect information and do some checks
  //--------------------------------------------------------------------------------
  // Get step length (make sure step length is not zero)
  const G4double minStepLength = 0.01 * Unit::um; // for all practical purposes
  const G4double stepLength = step->GetStepLength() * Unit::mm ;
  if (fabs(stepLength) < minStepLength) {
    B2WARNING("Geant4 steplength below " << minStepLength << " cm in PXD!!! ")
    return false;
  }

  //Get deposited energy (make sure energy was deposited in the sensitive volume)
  const G4double depEnergy = step->GetTotalEnergyDeposit() * Unit::MeV;
  if (fabs(depEnergy) < m_thresholdPXD * stepLength) return false;

  // Get track
  const G4Track& track  = *step->GetTrack();

  // Get particle PDG code
  const G4int partPDGCode = track.GetDefinition()->GetPDGEncoding();

  //Get particle charge (only keep charged tracks and photons)
  const G4double minCharge = 0.01 * Unit::e;
  const G4double partPDGCharge = track.GetDefinition()->GetPDGCharge() * Unit::C;
  if ((fabs(partPDGCharge) < minCharge) && (partPDGCode != 22)) return false;

  // Get particle mass
  const G4double partPDGMass = track.GetDefinition()->GetPDGMass() * Unit::MeV;

  // Get track ID
  const G4int trackID = track.GetTrackID();

  //Get time (a better time info will be that from the MCParticle)
  const G4double globalTime = track.GetGlobalTime() * Unit::ns ;

  // Get step information
  const G4StepPoint& preStep      = *step->GetPreStepPoint();
  const G4StepPoint& posStep      = *step->GetPostStepPoint();

  const G4ThreeVector& preStepPos = preStep.GetPosition();
  const G4ThreeVector& posStepPos = posStep.GetPosition();

  G4ThreeVector preStepMomentum   = preStep.GetMomentum();
  G4ThreeVector posStepMomentum   = posStep.GetMomentum();

  // Get information about the active medium
  const G4VPhysicalVolume& g4Volume    = *track.GetVolume();

  // check that user limits are set properly
  G4UserLimits* userLimits = g4Volume.GetLogicalVolume()->GetUserLimits();
  if (userLimits) {
    B2INFO("Volume " << g4Volume.GetName() << ": max. allowed step set to " << userLimits->GetMaxAllowedStep(track))
  }

  const G4ThreeVector momIn(preStep.GetMomentum());
  const G4ThreeVector momInLocal =
    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformAxis(momIn);

  const G4ThreeVector preStepPosLocal =
    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(preStepPos);
  const G4ThreeVector posStepPosLocal =
    preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(posStepPos);

  G4ThreeVector trackVector(posStepPos - preStepPos);
  double theta = trackVector.theta() * Unit::rad;


  //-------------------------------------------------------
  //              Add SimHit to the DataStore
  //-------------------------------------------------------

  int layerID = -1;
  int ladderID = -1;
  int sensorID = -1;

  const TGeoVolume* tGeoVolume = m_detMap->GetVolume(g4Volume.GetLogicalVolume());
  if (tGeoVolume == NULL) {
    B2ERROR("Cannot retrieve logical volume!")
    return false;
  } else {
    PXDVolumeUserInfo* userInfo = dynamic_cast<PXDVolumeUserInfo*>(tGeoVolume->GetField());
    if (userInfo == NULL) {
      B2ERROR("Volume User Info unavailable for " << g4Volume.GetName())
      return false;
    } else {
      layerID = userInfo->getLayerID();
      ladderID = userInfo->getLadderID();
      sensorID = userInfo->getSensorID();
      B2DEBUG(100, "*** Name: " << g4Volume.GetName() << std::endl << "Layer: " << layerID << " Ladder: " << ladderID << " Sensor: " << sensorID)
    }
  }

  StoreArray<PXDSimHit> pxdArray(DEFAULT_PXDSIMHITS);

  //Create new PXDSimHit.
  TVector3 posIn(preStepPosLocal.x() * Unit::mm, preStepPosLocal.y() * Unit::mm, preStepPosLocal.z() * Unit::mm);
  TVector3 posOut(posStepPosLocal.x() * Unit::mm, posStepPosLocal.y() * Unit::mm, posStepPosLocal.z() * Unit::mm);
  TVector3 momInVec(momInLocal.x() * Unit::MeV, momInLocal.y() * Unit::MeV, momInLocal.z() * Unit::MeV);

  int hitIndex = pxdArray->GetLast() + 1 ;
  new(pxdArray->AddrAt(hitIndex)) PXDSimHit(layerID, ladderID, sensorID,
                                            posIn, posOut, theta,
                                            momInVec,
                                            partPDGCode, partPDGMass, partPDGCharge,
                                            trackID,
                                            depEnergy,
                                            stepLength,
                                            globalTime);
  //Set the SeenInDetector flag
  setSeenInDetectorFlag(step, MCParticle::c_SeenInPXD);

  //Add relation between the MCParticle and the hit.
  //The index of the MCParticle has to be set to the TrackID and will be
  //replaced later by the correct MCParticle index automatically.
  StoreArray<Relation> mcPartRelation(getRelationCollectionName());
  StoreArray<MCParticle> mcPartArray(DEFAULT_MCPARTICLES);
  new(mcPartRelation->AddrAt(hitIndex)) Relation(mcPartArray, pxdArray, trackID, hitIndex);

  return true;
}
