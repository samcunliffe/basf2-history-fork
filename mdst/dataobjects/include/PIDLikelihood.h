/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PIDLIKELIHOOD_H
#define PIDLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>
#include <framework/gearbox/Const.h>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * Class to collect log likelihoods from TOP, ARICH, dEdx, ECL and KLM
   * aimed for output to mdst
   * includes functions to return combined likelihood probability
   */

  class PIDLikelihood : public RelationsObject {

  public:

    /**
     * Default constructor: log likelihoods and flags set to 0
     */
    PIDLikelihood();

    /**
     * Set log likelihood for a given detector and particle
     * @param det detector enumerator
     * @param part charged stable particle
     * @param logl log likelihood
     */
    void setLogLikelihood(Const::EDetector det,
                          const Const::ChargedStable& part,
                          float logl);

    /**
     * Check whether PID information from a given set of detectors is available
     * @param set a set of PID detectors
     * @return true if the given set of detectors contributed to the PID information
     */
    bool isAvailable(Const::PIDDetectorSet set) const {return  m_detectors.contains(set);}

    /**
     * Return log likelihood for a given detector set and particle
     * @param part charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood
     */
    float getLogL(const Const::ChargedStable& part,
                  Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return log likelihood difference for a given detector set and particles
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param set  a set of PID detectors to use
     * @return log likelihood difference logL_p1 - logL_p2
     */
    float getDeltaLogL(const Const::ChargedStable& p1,
                       const Const::ChargedStable& p2,
                       Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const {
      return getLogL(p1, set) - getLogL(p2, set);
    }

    /**
     * Return combined likelihood probability for a particle being p1 and not p2,
     * assuming equal prior probabilities.
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param set  a set of PID detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::ChargedStable& p1,
                          const Const::ChargedStable& p2,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const {
      return probability(getLogL(p1, set), getLogL(p2, set), 1.0);
    }

    /**
     * Return combined likelihood probability for a particle being p1 and not p2
     * @param p1 charged stable particle
     * @param p2 charged stable particle
     * @param ratio ratio of prior probabilities (p1/p2)
     * @param set  a set of PID detectors to use
     * @return likelihood probability
     */
    double getProbability(const Const::ChargedStable& p1,
                          const Const::ChargedStable& p2,
                          double ratio,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const {
      return probability(getLogL(p1, set), getLogL(p2, set), ratio);
    }


    /**
     * Return combined likelihood probability for a particle according to chargedStableSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param part charged stable particle
     * @param fractions array of prior probabilities in the order e,mu,pi,K,p
     * @param set  a set of PID detectors to use
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double getProbability(const Const::ChargedStable& part,
                          const float* fractions = 0,
                          Const::PIDDetectorSet set = Const::PIDDetectorSet::set()) const;

    /**
     * Return most likely particle among chargedStableSet;
     * if prior fractions not given equal prior probabilities assumed.
     * @param fractions array of prior probabilities in the order e,mu,pi,K,p
     * @param set  a set of PID detectors to use
     * @return particle type
     */
    Const::ChargedStable getMostLikely(const float* fractions = 0,
                                       Const::PIDDetectorSet set =
                                         Const::PIDDetectorSet::set()) const;

  private:

    enum {c_PIDDetectorSetSize = 6}; /**< temporary solution for the size */

    Const::DetectorSet m_detectors;   /**< set of detectors with PID information */
    float m_logl[c_PIDDetectorSetSize][Const::ChargedStable::c_SetSize]; /**< log likelihoods, FIXME: replace hard coded value */

    /**
     * Calculate likelihood probability from log likelihood difference logl1-logl2
     * @param logl1 log likelihood
     * @param logl2 log likelihood
     * @param ratio ratio of prior probabilities (p1/p2)
     * @return likelihood probability (a value btw. 0 and 1)
     */
    double probability(float logl1, float logl2, double ratio) const;

    /**
     * Calculate likelihood probabilities
     * @param n size of arrays (must be equal to Const::chargedStableSet.size())
     * @param fractions array of prior fractions (not needed to be normalized)
     * @param probabilities array of resulting probabilities
     * @param detSet  a set of PID detectors to use
     */
    void probability(unsigned n,
                     double fractions[],
                     double probabilities[],
                     Const::PIDDetectorSet detSet) const;


    ClassDef(PIDLikelihood, 2); /**< class definition */

  };

} // end namespace Belle2

#endif
