/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simkernel/B4SteppingAction.h>
#include <logging/Logger.h>

#include "G4UnitsTable.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
#include "G4EmCalculator.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "G4OpticalPhoton.hh"

using namespace Belle2;

B4SteppingAction :: B4SteppingAction()
{}

B4SteppingAction :: ~B4SteppingAction()
{}

void B4SteppingAction::UserSteppingAction(const G4Step* aStep)
{

  G4Track* track = aStep->GetTrack();

  //------------------------------
  // Check for NULL world volume
  //------------------------------
  if (track->GetVolume() == NULL) {
    const G4VProcess* lastproc = track->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
    INFO("Event ID: " << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID()
         << " B4SteppingAction: Track in NULL volume, terminating!\n"
         << " step_no=" + track->GetCurrentStepNumber()
         << " type=" << track->GetDefinition()->GetParticleName()
         << "\n volume=NULL"
         << " last_process="
         << (lastproc != 0 ? lastproc->GetProcessName() : G4String("NULL"))
         << "\n position=" << G4BestUnit(track->GetPosition(), "Length")
         << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"));
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  G4ThreeVector xyz = aStep->GetPostStepPoint()->GetPosition();

  //-----------------------------------------------------
  // If the track out of tsukuba_hall (+0.01m), kill it.
  //-----------------------------------------------------
  if (fabs(xyz.x()) > 8.01*m || fabs(xyz.y()) > 8.01*m || fabs(xyz.z()) > 8.01*m) {
    INFO("Event ID: " << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID()
         << " Out of World " << G4BestUnit(xyz, "Length"));
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  //---------------------------------------
  // Check for very high number of steps.
  //---------------------------------------
  if (track->GetCurrentStepNumber() > MaxStep) {
    const G4VPhysicalVolume* pv = track->GetVolume();
    const G4VProcess* lastproc = track->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
    WARNING("Event ID: " << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID()
            << " B4SteppingAction: Too many steps for this track, terminating!\n"
            << " step_no=" << track->GetCurrentStepNumber()
            << " type=" << track->GetDefinition()->GetParticleName()
            << "\n volume=" << (pv != 0 ? pv->GetName() : G4String("NULL"))
            << " last_process="
            << (lastproc != 0 ? lastproc->GetProcessName() : G4String("NULL"))
            << "\n position=" << G4BestUnit(track->GetPosition(), "Length")
            << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy"));
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  //-----------------------------------------
  // Check for too many zero steps in a row
  //-----------------------------------------
  static G4int num_zero_steps_in_a_row = 0;
  if (aStep->GetStepLength() <= 1e-10 && track->GetCurrentStepNumber() > 1) {
    num_zero_steps_in_a_row++;
    if (num_zero_steps_in_a_row > 4) {
      const G4VPhysicalVolume* pv = track->GetVolume();
      const G4VProcess* lastproc = track->GetStep()->GetPostStepPoint()->GetProcessDefinedStep();
      G4EmCalculator c;
      G4double range = c.GetRange(track->GetKineticEnergy(),
                                  track->GetDefinition(),
                                  pv->GetLogicalVolume()->GetMaterial());
      WARNING("Event ID: " << G4RunManager::GetRunManager()->GetCurrentEvent()->GetEventID()
              << " B4SteppingAction: Too many zero steps for this track, terminating!"
              << G4BestUnit(aStep->GetStepLength(), "Length")
              << " step_no=" << track->GetCurrentStepNumber()
              << " type=" << track->GetDefinition()->GetParticleName()
              << "\n volume=" << (pv != 0 ? pv->GetName() : G4String("NULL"))
              << " last_process="
              << (lastproc != 0 ? lastproc->GetProcessName() : G4String("NULL"))
              << "\n position=" << G4BestUnit(track->GetPosition(), "Length")
              << " momentum=" << G4BestUnit(track->GetMomentum(), "Energy")
              << " kinetic energy=" << G4BestUnit(track->GetKineticEnergy(), "Energy")
              << " material=" << pv->GetLogicalVolume()->GetMaterial()->GetName()
              << " range=" << G4BestUnit(range, "Length")
              << " num_zero=" << num_zero_steps_in_a_row);
      track->SetTrackStatus(fStopAndKill);
      num_zero_steps_in_a_row = 0;
      return;
    }
  } else {
    num_zero_steps_in_a_row = 0;
  }

  return;
}
