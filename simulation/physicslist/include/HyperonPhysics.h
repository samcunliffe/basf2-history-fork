/*************************************************************************o
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(c) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Dennis Wright (SLAC)                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HYPERONPHYSICS_H
#define HYPERONPHYSICS_H

#include "G4VPhysicsConstructor.hh"

class G4TheoFSGenerator;
class G4FTFModel;
class G4ExcitedStringDecay;
class G4LundStringFragmentation;
class G4GeneratorPrecompoundInterface;

namespace Belle2 {

  namespace Simulation {

    /**
     * Hyperon hadronic physics constructor for Belle II physics list
     */
    class HyperonPhysics: public G4VPhysicsConstructor {
    public:
      HyperonPhysics();
      ~HyperonPhysics();

      virtual void ConstructParticle() override;
      virtual void ConstructProcess() override;

    private:
      G4TheoFSGenerator* ftfp;
      G4FTFModel* stringModel;
      G4ExcitedStringDecay* stringDecay;
      G4LundStringFragmentation* fragModel;
      G4GeneratorPrecompoundInterface* preCompoundModel;

    };

  } // end of namespace Simulation

} // end of namespace Belle2

#endif // HYPERONPHYSICS_H
