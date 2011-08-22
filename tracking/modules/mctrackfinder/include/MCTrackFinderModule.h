/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko & Moritz Nadler          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCTRACKFIDNERMODULE_H_
#define MCTRACKFIDNERMODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>
#include <framework/datastore/RelationArray.h>


namespace Belle2 {

  /** This module use MC true Relations to define which hits belong to which particles and writes track candidates filled with necessary information into the DataStore.
   *
   *  The Relations MCParticles -> Hits for PXD, SVD and CDC are used.
   *  At the moment track candidates are created only for primary particles.
   *
   *  The created TrackCandidates can be fitted with GenFitterModule.
   *
   *  @todo: same procedure for CDC and VTX, check hit ordering and planeIds when adding hits to GFTrackCand, maybe create track candidates not only for primary particles
   */
  class MCTrackFinderModule : public Module {

  public:

    /**
     * Constructor of the module.
     *
     * Sets the description of the module.
     */
    MCTrackFinderModule();

    /** Destructor of the module. */
    ~MCTrackFinderModule();

    /** Initialize the Module.
     * This method is called only once before the actual event processing starts.
     */
    void initialize();

    /** Called when entering a new run.
     */
    void beginRun();

    /** This method is the core of the module.
     * This method is called for each event. All processing of the event has to take place in this method.
     */
    void event();
    /** This method is called if the current run ends.
     */
    void endRun();

    /** End of the event processing. */
    void terminate();


  private:

    std::string m_mcParticlesColName;                           /**< MCParticles collection name */

    std::string m_pxdHitColName;                                /**< PXDHits collection name */
    std::string m_mcParticleToPXDHits;                          /**< MCParticles <-> PXDHits relation name */

    std::string m_svdHitColName;                                /**< SVDHits collection name */
    std::string m_mcParticleToSVDHits;                          /**< MCParticles <-> SVDHits relation name */

    std::string m_cdcHitColName;                                /**< CDCHits collection name */
    std::string m_cdcSimHitColName;                             /**< CDCSimHits collection name */
    std::string m_mcParticleToCDCSimHits;                       /**< MCParticles <-> CDCSimHits relation name */
    std::string m_cdcSimHitsToCDCHits;                          /**< CDCSimHits <-> CDCHits relation name */

    bool m_usePXDHits;                                          /**< Boolean to select if PXDHits should be used*/
    bool m_useSVDHits;                                          /**< Boolean to select if SVDHits should be used*/
    bool m_useCDCHits;                                          /**< Boolean to select if CDCHits should be used*/
    bool m_onlyAxial;                                           /**< Boolean to select if only axial CDCHits should be used*/

    int m_smearing;                                             /**< Smearing of MCMomentum in % */

    std::string m_gfTrackCandsColName;                          /**< TrackCandidates collection name */
    std::string m_gfTrackCandToMCParticleColName;               /**< TrackCandidates to MCParticles relation name */


  };
}

#endif /* MCTRACKFINDERMODULE_H_ */
