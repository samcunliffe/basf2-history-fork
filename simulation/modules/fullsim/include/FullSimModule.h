/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FULLSIMMODULE_H_
#define FULLSIMMODULE_H_

#include <framework/core/Module.h>
#include <generators/dataobjects/MCParticleGraph.h>

#include <string>

namespace Belle2 {

  /** The full Geant4 simulation module.
   *
   * This module provides the full Geant4 simulation for the framework.
   * It initializes Geant4, calls the converter to translate the ROOT TGeo
   * volumes/materials to native Geant4 volumes/materials, initializes the
   * physics processes and user actions.
   *
   * This module requires a valid geometry in memory (gGeoManager). Therefore,
   * a geometry building module should have been executed before this module is called.
   *
   */
  class FullSimModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    FullSimModule();

    /** Destructor of the module. */
    virtual ~FullSimModule();

    /**
     * Initialize the Module.
     *
     * Initializes Geant4, calls the geometry converter, creates the physics processes and
     * create the user actions.
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     *
     * Initializes the Geant4 run manager and sets the run number in Geant4.
     */
    virtual void beginRun();

    /**
     * Performs the full Geant4 simulation.
     */
    virtual void event();

    /**
     * Called when run has ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();


  protected:

    MCParticleGraph m_mcParticleGraph;     /**< The MCParticle Graph used to manage the MCParticles before and after the simulation.*/

    std::string m_mcParticleInputColName;  /**< The parameter variable for the name of the input MCParticle collection. */
    std::string m_mcParticleOutputColName; /**< The parameter variable for the name of the output MCParticle collection. */
    std::string m_relationOutputColName;   /**< The parameter variable for the name of the output Relation (Hit -> MCParticle) collection. */
    double m_thresholdImportantEnergy;     /**< A particle which got 'stuck' and has less than this energy will be killed after m_thresholdTrials trials. */
    int m_thresholdTrials;                 /**< Geant4 will try m_thresholdTrials times to move a particle which got 'stuck' and has an energy less than m_thresholdImportantEnergy. */
    int m_trackingVerbosity;               /**< Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info. */
    bool m_createRelations;                /**< Set to true to create relations between hits and MCParticles. */
    std::string m_physicsList;             /**< The name of the physics list which is used for the simulation. */
    bool m_optics;                         /*!< If set to true, registers the optical physics list. */
    double m_productionCut;                /*!< Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance. */
    int m_maxNumberSteps;                  /*!< The maximum number of steps before the track transportation is stopped and the track is killed. */
    double m_photonFraction;               /**< The fraction of Cerenkov photons which will be kept and propagated. */


  private:

  };
}

#endif /* FULLSIMMODULE_H_ */
