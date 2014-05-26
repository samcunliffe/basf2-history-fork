/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/ParticleCombiner/PCombinerList.h>

#include <vector>
#include <set>
#include <unordered_set>

#include <boost/functional/hash/extensions.hpp>

namespace std {
  /** Hash function used by unordered_set. */
  template<> struct hash<std::set<int>> {
    /** Actual work done by boost. */
    std::size_t operator()(const std::set<int>& v) const {
      return boost::hash_value(v);
    }
  };
}

namespace Belle2 {
  class Particle;

  /**
   * ListCombiner is a generator for all the combinations of the sublists (Particle,AntiParticle, SelfConjugatedParticle)
   * of a set of particle lists. Therefore it returns combinations like (for 2 Particle Lists):
   * For currentType == Particle:
   *   Particle,Particle,
   *   Particle, SelfConjugatedParticle
   *   SelfConjugatedParticle, Particle
   * For currentType == AntiParticle
   *   AntiParticle, AntiParticle
   *   AntiParticle, SelfConjugatedParticle
   *   SelfConjugatedParticle, AntiParticle
   * Fpr currentType == SelfConjugatedParticle
   *   SelfConjugatedParticle, SelfConjugatedParticle
   */
  class ListCombiner {
  public:
    /**
     * Constructor
     * @param numberOfLists Number of Particle Lists which shall be combined
     */
    ListCombiner(unsigned int numberOfLists);

    /**
     * Initialises the generator to produce the given type of sublist
     * @param _currentType the type of sublist which is produced by the combination of PCombinerLists
     */
    void init(PCombinerList::EParticleType _currentType);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns the type of the sublist of the current loaded combination
     */
    const std::vector<PCombinerList::EParticleType>& getCurrentTypes() const;

  private:

    // TODO: delete
    void print() const;

    const unsigned int numberOfLists; /**< Number of lists which are combined */
    unsigned int iCombination; /**< The current position of the combination */
    unsigned int nCombinations; /**< The total amount of combinations */
    PCombinerList::EParticleType currentType; /**< The current type of sublist which is produced by the combination */
    std::vector<PCombinerList::EParticleType> types; /**< The current types of sublist of the PCombinerLists for this combination */

  };

  /**
   * IndexCombiner is a generator for all the combinations of the particle indices stored in the particle lists.
   * It takes a number of particles in each of the input lists e.g. 2,3 and returns all the possible combinations: 00, 01, 02, 10, 11, 12
   */
  class IndexCombiner {

  public:
    /**
     * Constructor
     * @param numberOfLists Number of Particle Lists which shall be combined
     */
    IndexCombiner(unsigned int numberOfLists);

    /**
     * Initialises the generator to produce combinations with the given sizes of each particle list
     * @param sizes the sizes of the particle lists to combine
     */
    void init(const std::vector<unsigned int>& _sizes);

    /**
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns theindices of the current loaded combination
     */
    const std::vector<unsigned int>& getCurrentIndices() const;


  private:

    void print() const;

    const unsigned int numberOfLists;  /**< Number of lists which are combined */
    unsigned int iCombination;         /**< The current position of the combination */
    unsigned int nCombinations;        /**< The total amount of combinations */
    std::vector<unsigned int> indices; /**< The indices of the current loaded combination */
    std::vector<unsigned int> sizes;   /**< The sizes of the particle lists which are combined */

  };

  /**
   * ParticleCombiner combines PCombinerLists to a new PCombinerList using the ListCombiner and IndexCombiner.
   * 1. The particle combiner loops over the three types of the output particle. The output particle can
   *    be a particle, anti-particle or a self-conjugated particle
   * 2. For each output types there are a number of possible combinations of the input sublists. These combinations
   *    are produced via the ListCombiner: E.g. for 2 Particles
   *    Particle + Particle -> Particle
   *    Particle + SelfConjugatedParticle -> Particle
   *    SelfConjugatedParticle + Particle -> Particle
   *    AntiParticle + AntiParticle -> AntiParticle
   *    AntiParticle + SelfConjugatedParticle -> AntiParticle
   *    SelfConjugatedParticle + AntiParticle -> AntiParticle
   *    SelfConjugatedParticle + SelfConjugatedParticle -> SelfConjugatedParticle
   * 3. For each combination of lists the IndexCombiner produces the combination of the particles
   *    inside the lists.
   * 4. The ParticleCombiner takes the current OutputType, ListCombination and IndexCombination
   *    and fills a vector: with the StoreArray indices of the current combination (getCurrentIndices), with the
   *    pointers to the particle objects of the current combination (getCurrentParticles). In addition the ParticleCombiner
   *    can return the output particle (combined from the current combination) directly (getCurrentParticle)
   *
   * \note This class retains state, so create a new ParticleCombiner object for each event.
   */
  class ParticleCombiner {

  public:
    /**
     * Constructor
     * @param inputLists input PCombinerLists
     * @param isCombinedParticleSelfConjugated If the combined particle is self conjugated the produced PCombinerList by this combiner is also SelfConjugated (so only the sublist SelfConjugatedParticle is filled)
     */
    ParticleCombiner(const std::vector<PCombinerList>& inputLists, bool isCombinedParticleSelfConjugated);

    /*
     * Loads the next combination. Returns false if there is no next combination
     */
    bool loadNext();

    /**
     * Returns the current loaded combination of particles as pointers to the particle objects
     */
    const std::vector<Particle*>& getCurrentParticles() const { return m_particles; }

    /**
     * Returns the current loaded combination of particles as indices which are found in the given PCombinerLists
     */
    const std::vector<int>& getCurrentIndices() const { return m_indices; }

    /**
     * Returns the current particle created from the current combination
     * @param pdg PDG of the Particle
     * @param pdgbar PDG of the AntiParticle
     */
    Particle getCurrentParticle(int pdg, int pdgbar) const;

    /**
     * Return the ParticleType of the current combination. If the Particle which is combined
     * is selfConjugated itself, all combinations have to be PCombinerList::c_SelfConjugatedParticle.
     */
    PCombinerList::EParticleType getCurrentType() const;

  private:
    /**
     * Returns the type of sublist which is currently produced, only difference to getCurrentType()
     * is that this function doesn't consider the isCombinedParticleSelfConjugated flag given in the constructor
     */
    PCombinerList::EParticleType getCurrentCombinationType() const;

    /**
     * Checks if given daughter particles are self conjugated
     * @param plists reference to vector of input PCombinerLists
     */
    bool isDecaySelfConjugated(std::vector<PCombinerList>& plists);

    /**
     * Loads the next combination provided by the IndexCombiner
     */

    bool loadNextCombination();

    /**
     * Loads the next type of output sublist
     */
    bool loadNextType();

    /**
     * Check that all FS particles of a combination differ
     * @return true if all FS particles of a combination differ
     */
    bool currentCombinationHasDifferentSources();

    /**
     * Check that the combination is unique
     * @return true if indices not found in the stack; if true indices pushed to stack
     */
    bool currentCombinationIsUnique();

  private:

    // TODO: delete
    void printCombiner();

    ListCombiner listCombiner;   /**< ListCombiner makes the combinations of the types of sublists of the PCombinerLists */
    IndexCombiner indexCombiner; /**< IndexCombiner makes the combinations of indices stored in the sublists of the PCombinerLists */

    std::vector<PCombinerList> plists;  /**< PCombinerLists which should be combined */
    std::vector<Particle*> m_particles; /**< Pointers to the particle objects of the current combination */
    std::vector<int> m_indices;         /**< Indices stored in the PCombinerLists of the current combination */
    std::unordered_set<std::set<int>> m_usedCombinations; /**< already used combinations (as sets of indices). */

    unsigned int numberOfLists; /**< Number of lists which are combined */

    int nTypes; /**< Total amount of types of sublists. So... 3! */
    int iType; /**< Current type of sublist which is produced */

    bool m_isCombinedParticleSelfConjugated; /**< True if the combined particle is self-conjugated */


  };

}
