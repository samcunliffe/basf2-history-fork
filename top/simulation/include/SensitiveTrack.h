/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TOPSENSITIVETRACK_H
#define TOPSENSITIVETRACK_H

#include <simulation/kernel/SensitiveDetectorBase.h>
#include <top/geometry/TOPGeometryPar.h>


namespace Belle2 {
  namespace TOP {
    //! This is optional (temporary) class that provides information on track parameters on aerogel plane, until tracking is not prepared.

    class SensitiveTrack : public Simulation::SensitiveDetectorBase {

    public:


      /**
       * Constructor.
       * @param name Name of the sensitive detector. Do we still need that?
       */
      SensitiveTrack();

      /**
       * Process each step and calculate variables defined in TOPTrack.
       * @param aStep Current Geant4 step in the sensitive medium.
       * @result true if a hit was stored, o.w. false.
       */
      bool step(G4Step* aStep, G4TouchableHistory*);

    protected:
      //! used for reading parameters from the xml
      TOPGeometryPar* m_topgp;
    };

  } // end of namespace top
} // end of namespace Belle2

#endif /* TOPSENSITIVEQUARTZ_H */
