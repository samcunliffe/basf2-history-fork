
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CRYINPUTMODULE_H
#define CRYINPUTMODULE_H

#include <framework/core/Module.h>

#include <generators/cry/CRY.h>

#include <mdst/dataobjects/MCParticleGraph.h>

namespace Belle2 {

  /**
   * The CRY Generator module.
   * Generates cosmic events using the CRY generator.
   */
  class CRYInputModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the module parameters.
     */
    CRYInputModule();

    /** Destructor. */
    virtual ~CRYInputModule() = default;

    /** Initializes the module. */
    virtual void initialize() override;

    /** Method is called for each event. */
    virtual void event() override;

    /** Method is called at the end of the event processing. */
    virtual void terminate() override;

  protected:
    std::string m_cosmicdatadir; /**< cosmic data (used by CRY for interpolation). */
    std::vector<double> m_acceptance; /**< Shape parameters for the acceptance box */
    double m_kineticEnergyThreshold;  /**< kinetic energy threshold. */
    double m_timeOffset;  /**< time offset. */
    int m_maxTrials;  /**< maximum number of trials. */

    CRY m_generator;   /**< The CRY generator. */
    MCParticleGraph m_mcGraph; /**< The MCParticle graph object. */
  };


} // end namespace Belle2

#endif /*CRYINPUTMODULE_H */
