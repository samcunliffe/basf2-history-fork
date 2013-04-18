/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 * Derived from: G4ErrorMagFieldLimitProcess.cc                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/ExtMagFieldLimitProcess.h>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4Field.hh>
#include <G4Track.hh>
#include <G4ForceCondition.hh>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

ExtMagFieldLimitProcess::ExtMagFieldLimitProcess(const G4String& processName) :
  G4VDiscreteProcess(processName)
{
  m_stepLimit = kInfinity; // user may change this with a geant4 UI command
  m_field = G4TransportationManager::GetTransportationManager()->GetFieldManager()->GetDetectorField();
  m_particleChange = new G4ParticleChange;
}

ExtMagFieldLimitProcess::~ExtMagFieldLimitProcess()
{
  delete m_particleChange;
}

G4double ExtMagFieldLimitProcess::GetMeanFreePath(const G4Track&, G4double, G4ForceCondition*)
{
  return kInfinity;
}


G4double ExtMagFieldLimitProcess::PostStepGetPhysicalInteractionLength(const G4Track& aTrack, G4double, G4ForceCondition* condition)
{
  *condition = NotForced;
  G4double stepLength = kInfinity;
  if (m_field != 0) {
    G4ThreeVector trkPosi = aTrack.GetPosition();
    G4double pos1[3] = { trkPosi.x(), trkPosi.y(), trkPosi.z() };
    G4double h1[3] = { 0.0, 0.0, 0.0 };
    m_field->GetFieldValue(pos1, h1);
    G4ThreeVector BVec(h1[0], h1[1], h1[2]);
    G4double pmag = aTrack.GetMomentum().mag();
    G4double BPerpMom = BVec.cross(aTrack.GetMomentum()).mag() / pmag;   // LEP
    stepLength = m_stepLimit * pmag / BPerpMom;
    B2DEBUG(300, "ExtMagFieldLimitProcess::PostStepGetPhysicalInteractionLength() stepLength "
            << stepLength << " B " << BPerpMom << " BVec " << BVec << " pmag " << pmag)
  }

  return stepLength;
}

// This method in G4VErrorLimitProcess has a memory leak so avoid inheriting from that class
G4VParticleChange* ExtMagFieldLimitProcess::PostStepDoIt(const G4Track& track, const G4Step&)
{
  m_particleChange->Initialize(track);
  return m_particleChange;
}

