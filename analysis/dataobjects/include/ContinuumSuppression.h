/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Pablo Goldenzweig                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CONTINUUMSUPPRESSION_H
#define CONTINUUMSUPPRESSION_H

#include <framework/datastore/RelationsObject.h>

#include <vector>
#include <set>

namespace Belle2 {

  /** \addtogroup dataobjects
   * @{
   */

  /**
   * This is a class for collecting variables used in continuum suppression. These variables
   * include thrust related quantities and the Super Fox Wolfram (KSFW) moments. The moments
   * are stored in separate vectors depending on whether the use_final_state_for_sig==0 (FS0)
   * or ==1 (FS1). For FS0 the moments are calculated using the B daughters, while for FS1
   * they are calculated using the B final state daughters.
   *
   * The ContinuumSuppression object is created for given existing Particle object by the
   * ContinuumSuppressionBuilder module and are related between each other with a BASF2 relation.
   *
   * Internally, the ContinuumSuppression class holds only floats and vectors of floats.
   */

  class ContinuumSuppression : public RelationsObject {

  public:

    /**
     * Default constructor.
     * All private members are set to 0 (all vectors are empty).
     */
    ContinuumSuppression() : m_thrustBm(0.0), m_thrustOm(0.0), m_cosTBTO(0.0), m_cosTBz(0.0) {};

    // setters
    /**
     * Add magnitude of B thrust axis.
     *
     * @param Float magnitute of B thrust axis
     */
    void addThrustBm(float thrustBm);

    /**
     * Add magnitude of ROE thrust axis.
     *
     * @param Float magnitute of ROE thrust axis
     */
    void addThrustOm(float thrustOm);

    /**
     * Add cosine of the angle between the thrust axis of the B and the thrust axis of the ROE.
     *
     * @param Float cosine of the angle between the thrust axis of the B and the thrust axis of the ROE
     */
    void addCosTBTO(float cosTBTO);

    /**
     * Add cosine of the angle between the thrust axis of the B and the z-axis.
     *
     * @param Float cosine of the angle between the thrust axis of the B and the z-axis
     */
    void addCosTBz(float cosTBz);

    /**
     * Add vector of KSFW moments, Et, and mm2 for final state = 0.
     *
     * @param vector of KSFW moments, Et, and mm2 for final state = 0
     */
    void addKsfwFS0(std::vector<float> ksfwFS0);

    /**
     * Add vector of KSFW moments, Et, and mm2 for final state = 1.
     *
     * @param vector of KSFW moments, Et, and mm2 for final state = 1
     */
    void addKsfwFS1(std::vector<float> ksfwFS1);


    // getters
    /**
     * Get magnitude of B thrust axis.
     *
     * @return Float magnitute of B thrust axis
     */
    float getThrustBm(void) const {
      return m_thrustBm;
    }

    /**
     * Get magnitude of ROE thrust axis.
     *
     * @return Float magnitute of ROE thrust axis
     */
    float getThrustOm(void) const {
      return m_thrustOm;
    }

    /**
     * Get cosine of the angle between the thrust axis of the B and the thrust axis of the ROE.
     *
     * @return Float cosine of the angle between the thrust axis of the B and the thrust axis of the ROE
     */
    float getCosTBTO(void) const {
      return m_cosTBTO;
    }

    /**
     * Get cosine of the angle between the thrust axis of the B and the z-axis.
     *
     * @return Float cosine of the angle between the thrust axis of the B and the z-axis
     */
    float getCosTBz(void) const {
      return m_cosTBz;
    }

    /**
     * Get vector of KSFW moments, Et, and mm2 for final state = 0.
     *
     * @return vector of KSFW moments, Et, and mm2 for final state = 0
     */
    std::vector<float> getKsfwFS0(void) const {
      return m_ksfwFS0;
    }

    /**
     * Get vector of KSFW moments, Et, and mm2 for final state = 1.
     *
     * @return vector of KSFW moments, Et, and mm2 for final state = 1
     */
    std::vector<float> getKsfwFS1(void) const {
      return m_ksfwFS1;
    }


  private:

    // persistent data members
    float m_thrustBm;  /**< Float of magnitude of B thrust axis */
    float m_thrustOm;  /**< Float of magnitude of ROE thrust axis */
    float m_cosTBTO;   /**< cosine of the angle between the thrust axis of the B and the thrust axis of the ROE */
    float m_cosTBz;    /**< cosine of the angle between the thrust axis of the B and the z-axis */

    std::vector<float> m_ksfwFS0;  /**< vector of KSFW moments, Et, and mm2 for final state = 0 */
    std::vector<float> m_ksfwFS1;  /**< vector of KSFW moments, Et, and mm2 for final state = 1 */

    ClassDef(ContinuumSuppression, 1) /**< class definition */

  };

  /** @}*/

} // end namespace Belle2

#endif
