/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Gelb                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef FSRCORRECTIONMODULE_H
#define FSRCORRECTIONMODULE_H

#include <framework/core/Module.h>

#include <analysis/VariableManager/Utility.h>
#include <analysis/dataobjects/ParticleList.h>
#include <analysis/dataobjects/Particle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <string>
//#include <vector>
//#include <tuple>
//#include <memory>


namespace Belle2 {
  class Particle;

  /**
   * Final state radiation correction module
   * This module adds the 4 Vector of the (closest) radiative gamma to the 4Vector of a lepton if the specified criterias are fulfilled.
   * It is intended to be used for electrons.
   */
  class FSRCorrectionModule : public Module {

  public:

    /**
     * Constructor
     */
    FSRCorrectionModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event processor.
     */
    virtual void event() override;


  private:

    int m_pdgCode;                /**< PDG code of the combined mother particle */

    // fixme do we need this?
    std::string m_decayString;   /**< Input DecayString specifying the decay being reconstructed */
    // fixme do we need this?
    DecayDescriptor m_decaydescriptor; /**< Decay descriptor of the lepton decay */
    DecayDescriptor m_decaydescriptorGamma; /**< Decay descriptor of the decay being reconstructed */

    std::string m_inputListName; /**< input ParticleList names */
    std::string m_gammaListName; /**< input ParticleList names */
    std::string m_outputListName; /**< output ParticleList name */
    std::string m_outputAntiListName;   /**< output anti-particle list name */

    double m_angleThres; /**< max angle to be accepted */
    double m_energyThres; /**< max energy of gamma to be accepted */

    bool m_writeOut;  /**< toggle output particle list btw. transient/writeOut */

    /**
     * This vector holds unique identifiers (vector of ints) of all particles
     * that are already included in the ParticleList. It is used to prevent
     * adding one or more copies of a Particle to the list that is already in.
     */
    std::vector<int> m_usedGammas;

    /**
     * Fills unique identifier for the input particle.
     * The unique identifier is a sequence of pairs (PDG code of the particle,
     * number of daughters if composite or mdstArrayIndex if final state particle)
     * for all particles in the decay chain.
     *
     * Example: B+ -> (anti-D0 -> K+ pi-) pi+
     * idSequence: (521, 2, -421, 2, 321, K+ mdstArrayIndex, -211, pi- mdstArrayIndex, 211, pi+ mdstArrayIndex)
     */
    void fillUniqueIdentifier(const Particle* p, std::vector<int>& idSequence);

    /**
     * Compares input idSequence to all sequences already included in the list.
     * If sequence is not found in the list a sequence is found to be unique.
     */
    bool isUnique(const std::vector<int>& idSequence);



    // fixme make an individual check (probably outside of this module



  };

} // Belle2 namespace

#endif
