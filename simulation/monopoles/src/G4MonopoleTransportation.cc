/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <simulation/monopoles/G4MonopoleTransportation.h>
#include <simulation/monopoles/G4Monopole.h>

#include <G4ProductionCutsTable.hh>
#include <G4ParticleTable.hh>
#include <G4ChordFinder.hh>
#include <G4SafetyHelper.hh>
#include <G4FieldManagerStore.hh>
#include <G4TransportationProcessType.hh>
#include <CLHEP/Units/SystemOfUnits.h>
#include <framework/logging/Logger.h>

class G4VSensitiveDetector;

using namespace std;
using namespace Belle2;
using namespace Belle2::Monopoles;
using namespace CLHEP;

G4MonopoleTransportation::G4MonopoleTransportation(const G4Monopole* mpl,
                                                   G4int verb)
  : G4VProcess(G4String("MonopoleTransportation"), fTransportation),
    fParticleDef(mpl),
    fMagSetup(0),
    fLinearNavigator(0),
    fFieldPropagator(0),
    fParticleIsLooping(false),
    fPreviousSftOrigin(0., 0., 0.),
    fPreviousSafety(0.0),
    fThreshold_Trap_Energy(10 * MeV),
    fThreshold_Warning_Energy(100 * MeV),
    fThreshold_Important_Energy(250 * MeV),
    fThresholdTrials(10),
    fNoLooperTrials(0),
    fSumEnergyKilled(0.0), fMaxEnergyKilled(0.0),
    fShortStepOptimisation(false),    // Old default: true (=fast short steps)
    fpSafetyHelper(0)
{
  verboseLevel = verb;

  // set Process Sub Type
  SetProcessSubType(TRANSPORTATION);

  fMagSetup = G4MonopoleFieldSetup::GetMonopoleFieldSetup();

  G4TransportationManager* transportMgr ;

  transportMgr = G4TransportationManager::GetTransportationManager() ;

  fLinearNavigator = transportMgr->GetNavigatorForTracking() ;

  // fGlobalFieldMgr = transportMgr->GetFieldManager() ;

  fFieldPropagator = transportMgr->GetPropagatorInField() ;

  fpSafetyHelper =   transportMgr->GetSafetyHelper();  // New

  // Cannot determine whether a field exists here,
  //  because it would only work if the field manager has informed
  //  about the detector's field before this transportation process
  //  is constructed.
  // Instead later the method DoesGlobalFieldExist() is called

  static G4TouchableHandle nullTouchableHandle;  // Points to (G4VTouchable*) 0
  fCurrentTouchableHandle = nullTouchableHandle;

  fEndGlobalTimeComputed  = false;
  fCandidateEndGlobalTime = 0;
}

G4MonopoleTransportation::~G4MonopoleTransportation()
{
  if ((verboseLevel > 0) && (fSumEnergyKilled > 0.0)) {
    B2DEBUG(25, " G4MonopoleTransportation: Statistics for looping particles ");
    B2DEBUG(25, "   Sum of energy of loopers killed: " <<  fSumEnergyKilled);
    B2DEBUG(25, "   Max energy of loopers killed: " <<  fMaxEnergyKilled);
  }
}

// Responsibilities:
//    Find whether the geometry limits the Step, and to what length
//    Calculate the new value of the safety and return it.
//    Store the final time, position and momentum.

G4double G4MonopoleTransportation::
AlongStepGetPhysicalInteractionLength(const G4Track&  track,
                                      G4double, //  previousStepSize
                                      G4double  currentMinimumStep,
                                      G4double& currentSafety,
                                      G4GPILSelection* selection)
{
  fMagSetup->SwitchChordFinder(1);
  // change to monopole equation

  G4double geometryStepLength, newSafety ;
  fParticleIsLooping = false ;

  // Initial actions moved to  StartTrack()
  // --------------------------------------
  // Note: in case another process changes touchable handle
  //    it will be necessary to add here (for all steps)
  // fCurrentTouchableHandle = aTrack->GetTouchableHandle();

  // GPILSelection is set to defaule value of CandidateForSelection
  // It is a return value
  //
  *selection = CandidateForSelection ;

  // Get initial Energy/Momentum of the track
  //
  const G4DynamicParticle* pParticle      = track.GetDynamicParticle() ;
  G4ThreeVector startMomentumDir          = pParticle->GetMomentumDirection() ;
  G4ThreeVector startPosition             = track.GetPosition() ;

  // G4double   theTime        = track.GetGlobalTime() ;

  // The Step Point safety can be limited by other geometries and/or the
  // assumptions of any process - it's not always the geometrical safety.
  // We calculate the starting point's isotropic safety here.
  //
  G4ThreeVector OriginShift = startPosition - fPreviousSftOrigin ;
  G4double      MagSqShift  = OriginShift.mag2() ;
  if (MagSqShift >= sqr(fPreviousSafety)) {
    currentSafety = 0.0 ;
  } else {
    currentSafety = fPreviousSafety - std::sqrt(MagSqShift) ;
  }

  // Is the monopole charged ?
  //
  G4double particleMagneticCharge = fParticleDef->MagneticCharge() ;
  G4double particleElectricCharge = pParticle->GetCharge();

  fGeometryLimitedStep = false ;
  // fEndGlobalTimeComputed = false ;

  // There is no need to locate the current volume. It is Done elsewhere:
  //   On track construction
  //   By the tracking, after all AlongStepDoIts, in "Relocation"

  // Check whether the particle have an (EM) field force exerting upon it
  //
  G4FieldManager* fieldMgr = nullptr;
  G4bool          fieldExertsForce = false ;

  if ((particleMagneticCharge != 0.0)) {
    fieldMgr = fFieldPropagator->FindAndSetFieldManager(track.GetVolume());
    if (fieldMgr != 0) {
      // Message the field Manager, to configure it for this track
      fieldMgr->ConfigureForTrack(&track);
      // Moved here, in order to allow a transition
      //   from a zero-field  status (with fieldMgr->(field)0
      //   to a finite field  status

      // If the field manager has no field, there is no field !
      fieldExertsForce = (fieldMgr->GetDetectorField() != 0);
    }
  }

  // Choose the calculation of the transportation: Field or not
  //
  if (!fieldExertsForce) {
    G4double linearStepLength ;
    if (fShortStepOptimisation && (currentMinimumStep <= currentSafety)) {
      // The Step is guaranteed to be taken
      //
      geometryStepLength   = currentMinimumStep ;
      fGeometryLimitedStep = false ;
    } else {
      //  Find whether the straight path intersects a volume
      //
      linearStepLength = fLinearNavigator->ComputeStep(startPosition,
                                                       startMomentumDir,
                                                       currentMinimumStep,
                                                       newSafety) ;
      // Remember last safety origin & value.
      //
      fPreviousSftOrigin = startPosition ;
      fPreviousSafety    = newSafety ;
      // fpSafetyHelper->SetCurrentSafety( newSafety, startPosition);

      // The safety at the initial point has been re-calculated:
      //
      currentSafety = newSafety ;

      fGeometryLimitedStep = (linearStepLength <= currentMinimumStep);
      if (fGeometryLimitedStep) {
        // The geometry limits the Step size (an intersection was found.)
        geometryStepLength   = linearStepLength ;
      } else {
        // The full Step is taken.
        geometryStepLength   = currentMinimumStep ;
      }
    }
    endpointDistance = geometryStepLength ;

    // Calculate final position
    //
    fTransportEndPosition = startPosition + geometryStepLength * startMomentumDir ;

    // Momentum direction, energy and polarisation are unchanged by transport
    //
    fTransportEndMomentumDir   = startMomentumDir ;
    fTransportEndKineticEnergy = track.GetKineticEnergy() ;
    fTransportEndSpin          = track.GetPolarization();
    fParticleIsLooping         = false ;
    fMomentumChanged           = false ;
    fEndGlobalTimeComputed     = false ;
  } else { //  A field exerts force
    G4double       momentumMagnitude = pParticle->GetTotalMomentum() ;
    G4ThreeVector  EndUnitMomentum ;
    G4double       lengthAlongCurve ;
    G4double       restMass = fParticleDef->GetPDGMass() ;

    G4ChargeState chargeState(particleElectricCharge,  // The charge can change (dynamic)
                              fParticleDef->GetPDGSpin(),
                              0,   //  Magnetic moment:  pParticleDef->GetMagneticMoment(),
                              0,   //  Electric Dipole moment - not in Particle Definition
                              particleMagneticCharge);    // in units of the positron charge

    G4EquationOfMotion* equationOfMotion =
      (fFieldPropagator->GetChordFinder()->GetIntegrationDriver()->GetStepper())
      ->GetEquationOfMotion();

    equationOfMotion
    ->SetChargeMomentumMass(chargeState,
                            momentumMagnitude,
                            restMass) ;
    // SetChargeMomentumMass now passes both the electric and magnetic charge - in chargeState

    G4ThreeVector spin        = track.GetPolarization() ;
    G4FieldTrack  aFieldTrack = G4FieldTrack(startPosition,
                                             track.GetMomentumDirection(),
                                             0.0,
                                             track.GetKineticEnergy(),
                                             restMass,
                                             track.GetVelocity(),
                                             track.GetGlobalTime(), // Lab.
                                             track.GetProperTime(), // Part.
                                             &spin) ;
    if (currentMinimumStep > 0) {
      // Do the Transport in the field (non recti-linear)
      //
      lengthAlongCurve = fFieldPropagator->ComputeStep(aFieldTrack,
                                                       currentMinimumStep,
                                                       currentSafety,
                                                       track.GetVolume()) ;
      fGeometryLimitedStep = lengthAlongCurve < currentMinimumStep;
      if (fGeometryLimitedStep) {
        geometryStepLength   = lengthAlongCurve ;
      } else {
        geometryStepLength   = currentMinimumStep ;
      }
    } else {
      geometryStepLength   = lengthAlongCurve = 0.0 ;
      fGeometryLimitedStep = false ;
    }

    // Remember last safety origin & value.
    //
    fPreviousSftOrigin = startPosition ;
    fPreviousSafety    = currentSafety ;
    // fpSafetyHelper->SetCurrentSafety( newSafety, startPosition);

    // Get the End-Position and End-Momentum (Dir-ection)
    //
    fTransportEndPosition = aFieldTrack.GetPosition() ;

    // Momentum:  Magnitude and direction can be changed too now ...
    //
    fMomentumChanged         = true ;
    fTransportEndMomentumDir = aFieldTrack.GetMomentumDir() ;

    fTransportEndKineticEnergy  = aFieldTrack.GetKineticEnergy() ;

    fCandidateEndGlobalTime   = aFieldTrack.GetLabTimeOfFlight();
    fEndGlobalTimeComputed    = true;

    fTransportEndSpin = aFieldTrack.GetSpin();
    fParticleIsLooping = fFieldPropagator->IsParticleLooping() ;
    endpointDistance   = (fTransportEndPosition - startPosition).mag() ;
  }

  // If we are asked to go a step length of 0, and we are on a boundary
  // then a boundary will also limit the step -> we must flag this.
  //
  if (currentMinimumStep == 0.0) {
    if (currentSafety == 0.0)  fGeometryLimitedStep = true ;
  }

  // Update the safety starting from the end-point,
  // if it will become negative at the end-point.
  //
  if (currentSafety < endpointDistance) {
    // if( particleMagneticCharge == 0.0 )
    //    G4cout  << "  Avoiding call to ComputeSafety : charge = 0.0 " << G4endl;

    if (particleMagneticCharge != 0.0) {

      G4double endSafety =
        fLinearNavigator->ComputeSafety(fTransportEndPosition) ;
      currentSafety      = endSafety ;
      fPreviousSftOrigin = fTransportEndPosition ;
      fPreviousSafety    = currentSafety ;
      fpSafetyHelper->SetCurrentSafety(currentSafety, fTransportEndPosition);

      // Because the Stepping Manager assumes it is from the start point,
      //  add the StepLength
      //
      currentSafety     += endpointDistance ;

#ifdef G4DEBUG_TRANSPORT
      G4cout.precision(12) ;
      G4cout << "***G4MonopoleTransportation::AlongStepGPIL ** " << G4endl  ;
      G4cout << "  Called Navigator->ComputeSafety at " << fTransportEndPosition
             << "    and it returned safety= " << endSafety << G4endl ;
      G4cout << "  Adding endpoint distance " << endpointDistance
             << "   to obtain pseudo-safety= " << currentSafety << G4endl ;
#endif
    }
  }

  fParticleChange.ProposeTrueStepLength(geometryStepLength) ;

  fMagSetup->SwitchChordFinder(0);
  // change back to usual equation

  return geometryStepLength ;
}

//
//   Initialize ParticleChange  (by setting all its members equal
//                               to corresponding members in G4Track)

G4VParticleChange* G4MonopoleTransportation::AlongStepDoIt(const G4Track& track,
                                                           const G4Step&  stepData)
{
  static G4int noCalls = 0;
  static const G4ParticleDefinition* fOpticalPhoton =
    G4ParticleTable::GetParticleTable()->FindParticle("opticalphoton");

  noCalls++;

  fParticleChange.Initialize(track) ;

  //  Code for specific process
  //
  fParticleChange.ProposePosition(fTransportEndPosition) ;
  fParticleChange.ProposeMomentumDirection(fTransportEndMomentumDir) ;
  fParticleChange.ProposeEnergy(fTransportEndKineticEnergy) ;
  fParticleChange.SetMomentumChanged(fMomentumChanged) ;

  fParticleChange.ProposePolarization(fTransportEndSpin);

  G4double deltaTime = 0.0 ;

  // Calculate  Lab Time of Flight (ONLY if field Equations used it!)
  // G4double endTime   = fCandidateEndGlobalTime;
  // G4double delta_time = endTime - startTime;

  G4double startTime = track.GetGlobalTime() ;

  if (!fEndGlobalTimeComputed) {
    // The time was not integrated .. make the best estimate possible
    //
    G4double finalVelocity   = track.GetVelocity() ;
    G4double initialVelocity = stepData.GetPreStepPoint()->GetVelocity() ;
    G4double stepLength      = track.GetStepLength() ;

    deltaTime = 0.0; // in case initialVelocity = 0
    const G4DynamicParticle* fpDynamicParticle = track.GetDynamicParticle();
    if (fpDynamicParticle->GetDefinition() == fOpticalPhoton) {
      //  A photon is in the medium of the final point
      //  during the step, so it has the final velocity.
      deltaTime = stepLength / finalVelocity ;
    } else if (finalVelocity > 0.0) {
      G4double meanInverseVelocity ;
      // deltaTime = stepLength/finalVelocity ;
      meanInverseVelocity = 0.5
                            * (1.0 / initialVelocity + 1.0 / finalVelocity) ;
      deltaTime = stepLength * meanInverseVelocity ;
    } else if (initialVelocity > 0.0) {
      deltaTime = stepLength / initialVelocity ;
    }
    fCandidateEndGlobalTime   = startTime + deltaTime ;
  } else {
    deltaTime = fCandidateEndGlobalTime - startTime ;
  }

  fParticleChange.ProposeGlobalTime(fCandidateEndGlobalTime) ;

  // Now Correct by Lorentz factor to get "proper" deltaTime

  G4double  restMass       = track.GetDynamicParticle()->GetMass() ;
  G4double deltaProperTime = deltaTime * (restMass / track.GetTotalEnergy()) ;

  fParticleChange.ProposeProperTime(track.GetProperTime() + deltaProperTime) ;
  //fParticleChange. ProposeTrueStepLength( track.GetStepLength() ) ;

  // If the particle is caught looping or is stuck (in very difficult
  // boundaries) in a magnetic field (doing many steps)
  //   THEN this kills it ...
  //
  if (fParticleIsLooping) {
    G4double endEnergy = fTransportEndKineticEnergy;

    if ((endEnergy < fThreshold_Important_Energy)
        || (fNoLooperTrials >= fThresholdTrials)) {
      // Kill the looping particle
      //
      fParticleChange.ProposeTrackStatus(fStopAndKill)  ;

      // 'Bare' statistics
      fSumEnergyKilled += endEnergy;
      if (endEnergy > fMaxEnergyKilled) { fMaxEnergyKilled = endEnergy; }

#ifdef G4VERBOSE
      if ((verboseLevel > 1) ||
          (endEnergy > fThreshold_Warning_Energy)) {
        B2DEBUG(25, " G4MonopoleTransportation is killing track that is looping or stuck ");
        B2DEBUG(25, "   This track has " << track.GetKineticEnergy() / MeV << " MeV energy.");
        B2DEBUG(25, "   Number of trials = " << fNoLooperTrials
                << "   No of calls to AlongStepDoIt = " << noCalls);
      }
#endif
      fNoLooperTrials = 0;
    } else {
      fNoLooperTrials ++;
#ifdef G4VERBOSE
      if ((verboseLevel > 2)) {
        B2DEBUG(25, "   G4MonopoleTransportation::AlongStepDoIt(): Particle looping -  "
                << "   Number of trials = " << fNoLooperTrials
                << "   No of calls to  = " << noCalls);
      }
#endif
    }
  } else {
    fNoLooperTrials = 0;
  }

  // Another (sometimes better way) is to use a user-limit maximum Step size
  // to alleviate this problem ..

  // Introduce smooth curved trajectories to particle-change
  //
  fParticleChange.SetPointerToVectorOfAuxiliaryPoints
  (fFieldPropagator->GimmeTrajectoryVectorAndForgetIt());

  //Monopole bounding
  if (fTransportEndKineticEnergy < fThreshold_Trap_Energy) {
    fParticleChange.ProposeTrackStatus(fStopAndKill);
    B2DEBUG(150, "Monopole bound in " << fCurrentTouchableHandle->GetVolume()->GetName());
    B2DEBUG(150, "with energy " << fTransportEndKineticEnergy << " MeV");
  }

  return &fParticleChange ;
}

//  This ensures that the PostStep action is always called,
//  so that it can do the relocation if it is needed.
//

G4double G4MonopoleTransportation::
PostStepGetPhysicalInteractionLength(const G4Track&,
                                     G4double, // previousStepSize
                                     G4ForceCondition* pForceCond)
{
  *pForceCond = Forced ;
  return DBL_MAX ;  // was kInfinity ; but convention now is DBL_MAX
}

G4VParticleChange* G4MonopoleTransportation::PostStepDoIt(const G4Track& track,
                                                          const G4Step&)
{
  G4TouchableHandle retCurrentTouchable ;   // The one to return

  // Initialize ParticleChange  (by setting all its members equal
  //                             to corresponding members in G4Track)
  // fParticleChange.Initialize(track) ;  // To initialise TouchableChange

  fParticleChange.ProposeTrackStatus(track.GetTrackStatus()) ;

  // If the Step was determined by the volume boundary,
  // logically relocate the particle

  if (fGeometryLimitedStep) {
    // fCurrentTouchable will now become the previous touchable,
    // and what was the previous will be freed.
    // (Needed because the preStepPoint can point to the previous touchable)

    fLinearNavigator->SetGeometricallyLimitedStep() ;
    fLinearNavigator->
    LocateGlobalPointAndUpdateTouchableHandle(track.GetPosition(),
                                              track.GetMomentumDirection(),
                                              fCurrentTouchableHandle,
                                              true) ;
    // Check whether the particle is out of the world volume
    // If so it has exited and must be killed.
    //
    if (fCurrentTouchableHandle->GetVolume() == 0) {
      fParticleChange.ProposeTrackStatus(fStopAndKill) ;
    }
    retCurrentTouchable = fCurrentTouchableHandle ;
    fParticleChange.SetTouchableHandle(fCurrentTouchableHandle) ;
  } else {             // fGeometryLimitedStep  is false
    // This serves only to move the Navigator's location
    //
    fLinearNavigator->LocateGlobalPointWithinVolume(track.GetPosition()) ;

    // The value of the track's current Touchable is retained.
    // (and it must be correct because we must use it below to
    // overwrite the (unset) one in particle change)
    //  It must be fCurrentTouchable too ??
    //
    fParticleChange.SetTouchableHandle(track.GetTouchableHandle()) ;
    retCurrentTouchable = track.GetTouchableHandle() ;
  }         // endif ( fGeometryLimitedStep )

  const G4VPhysicalVolume* pNewVol = retCurrentTouchable->GetVolume() ;
  G4Material* pNewMaterial   = nullptr ;
  G4VSensitiveDetector* pNewSensitiveDetector   = nullptr ;

  if (pNewVol != 0) {
    pNewMaterial = pNewVol->GetLogicalVolume()->GetMaterial();
    pNewSensitiveDetector = pNewVol->GetLogicalVolume()->GetSensitiveDetector();
  }

  fParticleChange.SetMaterialInTouchable(
    pNewMaterial) ;
  fParticleChange.SetSensitiveDetectorInTouchable(
    pNewSensitiveDetector) ;

  const G4MaterialCutsCouple* pNewMaterialCutsCouple = 0;
  if (pNewVol != 0) {
    pNewMaterialCutsCouple = pNewVol->GetLogicalVolume()->GetMaterialCutsCouple();
  }

  if (pNewVol != 0 && pNewMaterialCutsCouple != 0 &&
      pNewMaterialCutsCouple->GetMaterial() != pNewMaterial) {
    // for parametrized volume
    //
    pNewMaterialCutsCouple =
      G4ProductionCutsTable::GetProductionCutsTable()
      ->GetMaterialCutsCouple(pNewMaterial,
                              pNewMaterialCutsCouple->GetProductionCuts());
  }
  fParticleChange.SetMaterialCutsCoupleInTouchable(pNewMaterialCutsCouple);

  // temporarily until Get/Set Material of ParticleChange,
  // and StepPoint can be made const.
  // Set the touchable in ParticleChange
  // this must always be done because the particle change always
  // uses this value to overwrite the current touchable pointer.
  //
  fParticleChange.SetTouchableHandle(retCurrentTouchable) ;

  return &fParticleChange ;
}

// New method takes over the responsibility to reset the state
// of G4MonopoleTransportation object at the start of a new track
// or the resumption of a suspended track.

void
G4MonopoleTransportation::StartTracking(G4Track* aTrack)
{
  G4VProcess::StartTracking(aTrack);

// The actions here are those that were taken in AlongStepGPIL
//   when track.GetCurrentStepNumber()==1

  // reset safety value and center
  //
  fPreviousSafety    = 0.0 ;
  fPreviousSftOrigin = G4ThreeVector(0., 0., 0.) ;

  // reset looping counter -- for motion in field
  fNoLooperTrials = 0;
  // Must clear this state .. else it depends on last track's value
  //  --> a better solution would set this from state of suspended track TODO ?
  // Was if( aTrack->GetCurrentStepNumber()==1 ) { .. }

  // ChordFinder reset internal state
  //
  if (DoesGlobalFieldExist()) {
    fFieldPropagator->ClearPropagatorState();
    // Resets all state of field propagator class (ONLY)
    //  including safety values (in case of overlaps and to wipe for first track).

    // G4ChordFinder* chordF= fFieldPropagator->GetChordFinder();
    // if( chordF ) chordF->ResetStepEstimate();
  }

  // Make sure to clear the chord finders of all fields (ie managers)
  static G4FieldManagerStore* fieldMgrStore = G4FieldManagerStore::GetInstance();
  fieldMgrStore->ClearAllChordFindersState();

  // Update the current touchable handle  (from the track's)
  //
  fCurrentTouchableHandle = aTrack->GetTouchableHandle();
}
