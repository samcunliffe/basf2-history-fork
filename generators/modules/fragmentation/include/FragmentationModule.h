/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ami Rostomyan, Torben Ferber                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FRAGMENTATIONMODULE_H
#define FRAGMENTATIONMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <Pythia8/Pythia.h>

#include <string>
#include <vector>

namespace Belle2 {

  /** Minimal class for external random generator to be used in PYTHIA.
  */
  class FragmentationRndm : public Pythia8::RndmEngine {

  public:
    // Constructor
    FragmentationRndm();

    // in PYTHIA, there is one pure virtual method in RndmEngine,
    // to generate one random number flat in the range between 0 and 1:
    double flat();

  private:

  };


  /** The Fragmentation module.
  */
  class FragmentationModule : public Module {

  public:
    /**
    * Constructor.
    * Sets the module parameters.
    */
    FragmentationModule();

    /** Destructor. */
    virtual ~FragmentationModule() {}

    virtual void event();

  protected:

    Pythia8::Pythia pythia;  //  Declare generator
    Pythia8::Event& PythiaEvent = pythia.event;  //  Pythia event

    /** Module parameters */
    std::string m_parameterfile; /**< PYTHIA input parameter file. */
    int m_listEvent;  /**< list event generated by PYTHIA. */

    MCParticleGraph mcParticleGraph;        /**< An instance of the MCParticle graph. */

    void initialize();

    int nAddedQuarks;  /**< number of added quarks. */
    int addQuarkToPYTHIA(MCParticle& mcParticle);   /**< picks quark/antiquark and adds them to a pythia event. */

    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
  };

} // end namespace Belle2

#endif // FRAGMENTATIONMODULE_H
