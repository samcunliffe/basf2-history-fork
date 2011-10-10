/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Timofey Uglov                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSENSETIVEDETECTOR_H
#define EKLMSENSETIVEDETECTOR_H

#include <eklm/eklmhit/EKLMSimHit.h>
#include <simulation/kernel/SensitiveDetectorBase.h>


namespace Belle2 {

  /** The Class for EKLM Sensitive Detector
   * In this class, every variables defined in EKLMSimHit will be calculated.
   * EKLMSimHits are saved into hits collection.
   */

  class EKLMSensitiveDetector : public Simulation::SensitiveDetectorBase  {

  public:

    //! Constructor
    EKLMSensitiveDetector(G4String name, G4double thresholdEnergyDeposit,
                          G4double thresholdKineticEnergy);

    //! Destructor
    ~EKLMSensitiveDetector() {};

    //! Register EKLM hits collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent *eventHC);

    //! Process each step and calculate variables defined in EKLMHit
    bool step(G4Step *aStep, G4TouchableHistory *history);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

    //! Add random background To be implemented later
    //void AddbgOne(bool doit);

  private:
    //! all hits with energies less than m_ThresholdEnergyDeposit
    //! will be dropped
    G4double m_ThresholdEnergyDeposit;

    //! all hits with kinetic energies less than m_ThresholdEnergyDeposit
    //! will be dropped
    G4double m_ThresholdKineticEnergy;

    //! sim hits collection
    //    EKLMSimHitsCollection *m_HitCollection;

    //! hit colleation ID
    //    G4int m_HCID;

    //! Hit number within one event
    int m_HitNumber;

    /**
     * Detector mode.
     */
    int m_mode;


    /**
     * getter for detector mode
     */
    int getMode(G4Step *aStep);


  };

} // end of namespace Belle2

#endif

