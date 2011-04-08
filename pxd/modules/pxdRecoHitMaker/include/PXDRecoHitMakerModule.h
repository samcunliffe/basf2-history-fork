/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDRECOHITMAKERMODULE_H
#define PXDRECOHITMAKERMODULE_H

#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * PXDRecoHit maker module.
   *
   * This simple module takes a DataStore array of PXDHits as input and converts
   * it to an array of PXDRecoHits - enriched hits containing also detector plane
   * information.
   */

  class PXDRecoHitMakerModule : public Module {

  public:

    /** Constructor.*/
    PXDRecoHitMakerModule();

    /** Destructor.*/
    virtual ~PXDRecoHitMakerModule();

    /**
     * Initialize the Module.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     *
     * Refresh SiGeoCache.
     */
    virtual void beginRun();

    /**
     * Event processor.
     *
     * Convert PXDHits of the event to PXDRecoHits.
     */
    virtual void event();

    /**
     * End-of-run action.
     *
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     *
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  protected:

    /** Print module parameters.*/
    void printModuleParams() const;


  private:

    std::string m_inColName;         /**< Input collection name */
    std::string m_outColName;        /**< Output collection name */
    std::string m_relColName;        /**< Relation collection name (MC particles to RecoHits. */

    double m_timeCPU;                /**< CPU time.     */
    int    m_nRun;                   /**< Run number.   */
    int    m_nEvent;                 /**< Event number. */
  };

} // Belle2 namespace

#endif // PXDDIGIMODULE_H
