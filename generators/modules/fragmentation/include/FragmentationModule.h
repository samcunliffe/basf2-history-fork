/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ami Rostomyan, Torben Ferber                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <Pythia8/Pythia.h>
#include <generators/modules/fragmentation/EvtGenDecays.h>

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
    double flat(); /**< flat random generator. */

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
    virtual ~FragmentationModule();

    virtual void event() override;
    virtual void initialize() override;
    virtual void terminate() override;

  protected:

    Pythia8::Pythia* pythia; /**< PYTHIA generator. */
    Pythia8::Event* PythiaEvent; /**< PYTHIA event. */
    EvtGenDecays* evtgen; /**< EvtGen decay engine inside PYTHIA8 */

    /** Module parameters */
    std::string m_parameterfile; /**< PYTHIA input parameter file. */
    int m_listEvent;  /**< list event generated by PYTHIA. */
    int m_useEvtGen;  /**< use EvtGen for some decays. */
    std::string m_DecFile;  /**< EvtGen decay file */
    std::string m_UserDecFile;  /**< User EvtGen decay file */
    int m_useEvtGenParticleData; /**< Override PYTHIA parameters */

    MCParticleGraph mcParticleGraph; /**< An instance of the MCParticle graph. */

    int nAll;   /**< number of events created. */
    int nGood;   /**< number of events with successful fragmentation. */

    int nAdded;  /**< number of added particles. */
    int addParticleToPYTHIA(MCParticle& mcParticle);   /**< picks particles and adds them to a pythia event. */
    int nQuarks; /**< number of quarks. */
    int nVpho; /**< number of virtual exchange particles. */

    std::string m_particleList; /**< The name of the MCParticle collection. */
    StoreArray<MCParticle> m_mcparticles; /**< store array for the MCParticles */
  };

} // end namespace Belle2
