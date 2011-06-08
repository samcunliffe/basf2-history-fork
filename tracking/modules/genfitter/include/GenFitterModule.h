/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef GENFITTERMODULE_H
#define GENFITTERMODULE_H

#include <framework/core/Module.h>
#include <fstream>

namespace Belle2 {


  class GenFitterModule : public Module {

  public:

    /** Constructor .
     */
    GenFitterModule();

    /** Destructor.
     */
    virtual ~GenFitterModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    virtual void initialize();

    /** Called when entering a new run.
     */
    virtual void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    virtual void event();

    /** This method is called if the current run ends.
     */
    virtual void endRun();

    /** This method is called at the end of the event processing.
     */
    virtual void terminate();


  protected:


  private:

    bool m_useCdcHits;                              /**< True if CDCHits should be used in the fit. */
    bool m_useSvdHits;                              /**< True if SVDHits should be used in the fit. */
    bool m_usePxdHits;                              /**< True if PXDHits should be used in the fit. */


    std::string m_cdcRecoHitsColName;                /**< CDCRecoHits collection name */
    std::string m_svdRecoHitsColName;                /**< SVDRecoHits collection name */
    std::string m_pxdRecoHitsColName;                /**< PXDRecoHits collection name */
    std::string m_gfTrackCandsColName;               /**< GFTrackCandidates collection name */

    std::string m_tracksColName;                     /**< Tracks collection name */
    std::string m_gfTracksColName;                   /**< GFTracks collection name */

    int m_failedFitCounter;                          /**< Number of failed fits. */
    int m_successfulFitCounter;                      /**< Number of successfully fitted tracks. */


  };
} // end namespace Belle2


#endif /* GENFITTERMODULE_H */
