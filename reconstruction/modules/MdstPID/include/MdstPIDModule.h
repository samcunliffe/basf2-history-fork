/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MDSTPIDMODULE
#define MDSTPIDMODULE

#include <framework/core/Module.h>
#include <string>

namespace Belle2 {

  class PIDLikelihood;
  class TOPLikelihood;
  class ARICHLikelihood;
  class DedxLikelihood;
  class ECLPidLikelihood;
  class Muid;

  /**
   * a module to fill PIDLikelihoods
   */

  class MdstPIDModule : public Module {
  public:


    /**
     * Constructor
     */
    MdstPIDModule();

    /**
     * Destructor
     */
    virtual ~MdstPIDModule();

    /**
     * Initialize the module.
     * data store registration of PIDLikelihoods and relations to Tracks
     */
    virtual void initialize();

    /**
     * Called when a new run is started.
     */
    virtual void beginRun();

    /**
     * Called for each event.
     * loop over Tracks, collect likelihoods and fill PIDLikelihoods
     */
    virtual void event();

    /**
     * Called when run ended.
     */
    virtual void endRun();

    /**
     * Terminates the module.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    /**
     * Set TOP log likelihoods and corresponding reconstruction flag
     * @param logl TOPLikelihood pointer
     */
    void setLikelihoods(const TOPLikelihood* logl);

    /**
     * Set ARICH log likelihoods and corresponding reconstruction flag
     * @param logl ARICHLikelihood pointer
     */
    void setLikelihoods(const ARICHLikelihood* logl);

    /**
     * Set Dedx log likelihoods and corresponding reconstruction flag
     * @param logl DedxLikelihood pointer
     */
    void setLikelihoods(const DedxLikelihood* logl);

    /**
     * Set ECL log likelihoods and corresponding reconstruction flag
     * @param logl ECLPidLikelihood pointer
     */
    void setLikelihoods(const ECLPidLikelihood* logl);

    /**
     * Set KLM log likelihoods and corresponding reconstruction flag
     * @param muid Muid pointer
     */
    void setLikelihoods(const Muid* muid);

    PIDLikelihood* m_pid; /**< pointer to the object to be filled */

  };

} // Belle2 namespace

#endif




