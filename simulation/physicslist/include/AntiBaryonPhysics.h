/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ANTIBARYONPHYSICS_H
#define ANTIBARYONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;
class G4ComponentAntiNuclNuclearXS;

namespace Belle2 {

  namespace Simulation {

    /**
     * Anti-baryon hadronic physics constructor for Belle II physics list
     */
    class AntiBaryonPhysics: public G4VPhysicsConstructor {
    public:
      AntiBaryonPhysics();
      ~AntiBaryonPhysics();

      virtual void ConstructParticle() override;
      virtual void ConstructProcess() override;

    private:
      G4TheoFSGenerator* m_ftfp;
      G4FTFModel* m_stringModel;
      G4ExcitedStringDecay* m_stringDecay;
      G4LundStringFragmentation* m_fragModel;
      G4GeneratorPrecompoundInterface* m_preCompoundModel;

      G4ComponentAntiNuclNuclearXS* m_theAntiNucleonXS;
    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // ANTIBARYONPHYSICS_H
