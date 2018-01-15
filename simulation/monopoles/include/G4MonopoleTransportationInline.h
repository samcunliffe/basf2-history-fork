/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// modified from GEANT4 exoticphysics/monopole/*

#include <CLHEP/Units/SystemOfUnits.h>

namespace Belle2 {

  namespace Monopoles {

    inline void
    G4MonopoleTransportation::SetPropagatorInField(G4PropagatorInField* pFieldPropagator)
    {
      fFieldPropagator = pFieldPropagator;
    }

    inline G4PropagatorInField* G4MonopoleTransportation::GetPropagatorInField()
    {
      return fFieldPropagator;
    }

    inline G4bool G4MonopoleTransportation::DoesGlobalFieldExist()
    {
      G4TransportationManager* transportMgr;
      transportMgr = G4TransportationManager::GetTransportationManager();

      // fFieldExists= transportMgr->GetFieldManager()->DoesFieldExist();
      // return fFieldExists;
      return transportMgr->GetFieldManager()->DoesFieldExist();
    }

    inline G4double G4MonopoleTransportation::GetThresholdWarningEnergy() const
    {
      return fThreshold_Warning_Energy;
    }

    inline G4double G4MonopoleTransportation::GetThresholdImportantEnergy() const
    {
      return fThreshold_Important_Energy;
    }

    inline G4int G4MonopoleTransportation::GetThresholdTrials() const
    {
      return fThresholdTrials;
    }

    inline void G4MonopoleTransportation::SetThresholdWarningEnergy(G4double newEnWarn)
    {
      fThreshold_Warning_Energy = newEnWarn;
    }

    inline void G4MonopoleTransportation::SetThresholdImportantEnergy(G4double newEnImp)
    {
      fThreshold_Important_Energy = newEnImp;
    }

    inline void G4MonopoleTransportation::SetThresholdTrials(G4int newMaxTrials)
    {
      fThresholdTrials = newMaxTrials;
    }

    // Get/Set parameters for killing loopers:
    //   Above 'important' energy a 'looping' particle in field will
    //   *NOT* be abandoned, except after fThresholdTrials attempts.
    // Below Warning energy, no verbosity for looping particles is issued

    inline G4double G4MonopoleTransportation::GetMaxEnergyKilled() const
    {
      return fMaxEnergyKilled;
    }

    inline G4double G4MonopoleTransportation::GetSumEnergyKilled() const
    {
      return fSumEnergyKilled;
    }

    inline void G4MonopoleTransportation::ResetKilledStatistics(G4int report)
    {
      if (report) {
        G4cout << " G4MonopoleTransportation: Statistics for looping particles " << G4endl; //TODO move to B2INFO
        G4cout << "   Sum of energy of loopers killed: " <<  fSumEnergyKilled << G4endl;
        G4cout << "   Max energy of loopers killed: " <<  fMaxEnergyKilled << G4endl;
      }

      fSumEnergyKilled = 0;
      fMaxEnergyKilled = -1.0 * CLHEP::GeV;
    }
    // Statistics for tracks killed (currently due to looping in field)

    inline void G4MonopoleTransportation::EnableShortStepOptimisation(G4bool optimiseShortStep)
    {
      fShortStepOptimisation = optimiseShortStep;
    }

  }

}
