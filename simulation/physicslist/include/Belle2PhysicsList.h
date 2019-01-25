/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BELLE2PHYSICSLIST_H
#define BELLE2PHYSICSLIST_H

#include "G4VModularPhysicsList.hh"
#include "globals.hh"

namespace Belle2 {

  namespace Simulation {

    /**
     * Custom Geant4 physics list for Belle II with options to
     * add optical physics and high precision neutrons
     */
    class Belle2PhysicsList: public G4VModularPhysicsList {
    public:
      Belle2PhysicsList();
      ~Belle2PhysicsList();

      virtual void ConstructParticle();
      virtual void SetCuts();

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // BELLE2PHYSICSLIST_H
