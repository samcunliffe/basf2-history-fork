/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Glina Pakhlova, Timofey Uglov                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSD_H
#define EKLMSD_H

#include <eklm/simeklm/EKLMHit.h>
#include "G4VSensitiveDetector.hh"

namespace Belle2 {

  //! The Class for EKLM Sensitive Detector
  /*! In this class, every variable defined in EKLMHit will be calculated.
      And save EKLMHit into hits collection.
  */

//class EKLMSD: public B4SensitiveDetectorBase
  class EKLMSD: public G4VSensitiveDetector {

  public:

    //! Constructor
    EKLMSD(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy);

    //! Destructor
    ~EKLMSD() {};

    //! Register EKLM hits collection into G4HCofThisEvent
    void Initialize(G4HCofThisEvent *eventHC);

    //! Process each step and calculate variables defined in EKLMHit
    G4bool ProcessHits(G4Step *aStep, G4TouchableHistory *history);

    //! Do what you want to do at the end of each event
    void EndOfEvent(G4HCofThisEvent *eventHC);

    //void CreateCollection();

    //void AddbgOne(bool doit);

  private:
    G4double m_ThresholdEnergyDeposit;
    G4double m_ThresholdKineticEnergy;
    EKLMHitsCollection *m_HitCollection;
    G4int m_HCID;

  };

} // end of namespace Belle2

#endif
