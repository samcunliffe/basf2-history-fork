/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PARTICLECOMBINERMODULE_H
#define PARTICLECOMBINERMODULE_H

#include <framework/core/Module.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <analysis/dataobjects/ParticleList.h>

#include <string>
#include <vector>
#include <unordered_set>
#include <tuple>

//Hack: allow access to m_bits to define hash function
#define BOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS
#include <boost/dynamic_bitset.hpp>
#include <boost/functional/hash.hpp>

namespace std {
  /** Define hash for dynamic_bitset. */
  template<> struct hash<boost::dynamic_bitset<> > {
    /** Create hash by accessing the raw bits (m_bits). */
    std::size_t operator()(const boost::dynamic_bitset<>& bs) const {
      return boost::hash_value(bs.m_bits);
    }
  };
}

namespace Belle2 {
  class Particle;

  /**
   * particle combiner module
   */
  class ParticleCombinerModule : public Module {

  public:

    /**
     * Constructor
     */
    ParticleCombinerModule();

    /**
     * Destructor
     */
    virtual ~ParticleCombinerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    /**
     * Checks if given daughter particles are self conjugated
     * @param plists refernce to vector of input particle lists StoreObjPtr's
     */
    bool isDecaySelfConjugated(std::vector<StoreObjPtr<ParticleList> >& plists);

    /**
     * Checks if the particle itself is self conjugated
     * @param outputList reference to output particle list StoreObjPtr
     */
    bool isParticleSelfConjugated(StoreObjPtr<ParticleList>& outputList);

    /**
     * Main routine for making combinations
     * @param plist reference to output particle list StoreObjPtr
     * @param plists refernce to vector of input particle lists StoreObjPtr's
     * @param chargeState charge state (0=unflavored or 1=flavored)
     */
    void combination(StoreObjPtr<ParticleList>& plist,
                     std::vector<StoreObjPtr<ParticleList> >& plists,
                     const std::vector<bool>& useSelfConjugatedDaughter,
                     ParticleList::EParticleType particleType);

    /**
     * Check that all FS particles of a combination differ
     * @param stack stack for pointers to Particle
     * @param Particles reference to StoreArray holding Particles
     * @return true if all FS particles of a combination differ
     */
    bool differentSources(std::vector<Particle*> stack,
                          const StoreArray<Particle>& Particles);

    /**
     * Check that the combination is unique
     * @param indexStack reference to stack of combination indices
     * @param indices combination indices to check
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool uniqueCombination(std::unordered_set<boost::dynamic_bitset<> >& indexStack,
                           const std::vector<int>& indices);

    /**
     * Check if the particleStack pass all cut requirements
     * @param particleStack Reference to stack of pointers to Particle
     * @return True if all requirements are passed, false otherwise
     */
    bool checkCuts(const std::vector<Particle*>& particleStack);

    int m_pdg;                /**< PDG code of combined particles */
    std::string m_listName;   /**< output particle list name */
    std::vector<std::string> m_inputListNames; /**< input particle list names */
    std::map<std::string, std::tuple<double, double> > m_productCuts; /**< variables -> low/high cut on their product. */
    std::map<std::string, std::tuple<double, double> > m_sumCuts; /**< variables -> low/high cut on their sum. */
    std::tuple<double, double> m_massCut; /**< lower and upper mass cut */
    bool m_persistent;  /**< toggle output particle list btw. transient/persistent */

  };

} // Belle2 namespace

#endif
