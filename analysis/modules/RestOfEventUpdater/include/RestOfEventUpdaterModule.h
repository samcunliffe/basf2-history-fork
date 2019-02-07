/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej, Sviatoslav Bilokin                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <analysis/dataobjects/Particle.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>

#include <string>
#include <vector>
#include <map>
#include <tuple>

namespace Belle2 {

  /**
   * Updates an existing mask (map of boolean values) for tracks or eclClusters in RestOfEvent with an available property (e.g. after performing training)
   */
  class RestOfEventUpdaterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    RestOfEventUpdaterModule();

    /** Overriden initialize method */
    virtual void initialize() override;

    /** Overriden event method */
    virtual void event() override;


  private:
    //Data members:
    typedef std::map<unsigned int, bool> intAndBoolMap; /**< Type definition helper for a map of integer and boolean */
    typedef std::map<unsigned int, unsigned int> intAndIntMap; /**< Type definition helper for a map of integer and integer */

    std::string m_inputListName; /**< Name of the ParticleList which contains information that will be used for updating */
    StoreObjPtr<ParticleList> m_inputList; /**< ParticleList which contains information that will be used for updating */

    std::vector<std::string> m_maskNamesForUpdating; /**< Container for all mask names which will be updated */

    std::string m_selection; /**< Cut string which will be used for updating masks */
    std::shared_ptr<Variable::Cut> m_cut; /**< Cut object which performs the cuts */
    bool m_discard; /**< Update the ROE mask by passing or discarding particles in the provided particle list. Default is to pass. */

    std::vector<double>
    m_fractions; /**< A priori fractions used for newly added masks. Fractions of existing masks will not be changed (default: pion always) */

    // Methods:
    /**
     * Get the ParticleType of the input particle list
     * @returns ParticleType, which is used in RestOfEvent methods
     */
    Particle::EParticleType getListType();
    /**
     * Update ROE masks by excluding or keeping particles
     * @param Pointer to RestOfEvent object for update
     * @param Reference to particle collection
     * @param ParticleType of the collection
     */
    void updateMasksWithParticles(StoreObjPtr<RestOfEvent> roe, std::vector<const Particle*>& particlesToUpdate,
                                  Particle::EParticleType listType);
    /**
     * Update ROE masks with provided composite particle collection
     * @param Pointer to RestOfEvent object for update
     * @param Reference to composite particle collection
     */
    void updateMasksWithV0(StoreObjPtr<RestOfEvent> roe, std::vector<const Particle*>& particlesToUpdate);
  };
}

