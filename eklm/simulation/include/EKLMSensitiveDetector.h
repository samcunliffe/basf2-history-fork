/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMSENSETIVEDETECTOR_H
#define EKLMSENSETIVEDETECTOR_H

/* Belle2 headers. */
#include <simulation/kernel/SensitiveDetectorBase.h>
#include <eklm/dataobjects/EKLMStepHit.h>
#include <eklm/geometry/GeoEKLMTypes.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * The Class for EKLM Sensitive Detector.
     * @details
     * In this class, every variables defined in EKLMStepHit will be calculated.
     * EKLMStepHits are saved into hits collection.
     */

    class EKLMSensitiveDetector : public Simulation::SensitiveDetectorBase  {

    public:

      /**
       * Constructor.
       */
      EKLMSensitiveDetector(G4String name, enum EKLMSensitiveType type);

      /**
       * Destructor.
       */
      ~EKLMSensitiveDetector() {};

      /**
       * Process each step and calculate variables for EKLMStepHit
       * store EKLMStepHit.
       */
      bool step(G4Step* aStep, G4TouchableHistory* history);

    private:
      /**
       * All hits with energies less than m_ThresholdEnergyDeposit
       * will be dropped.
       */
      G4double m_ThresholdEnergyDeposit;

      /**
       * All hits with time large  than m_ThresholdHitTime
       * will be dropped.
       */
      G4double m_ThresholdHitTime;

      /**
       * Sensitive detector type.
       */
      enum EKLMSensitiveType m_type;

      /**
       * Sensitive detector operation mode.
       */
      enum EKLM::EKLMDetectorMode m_mode;

    };

  }

}

#endif

