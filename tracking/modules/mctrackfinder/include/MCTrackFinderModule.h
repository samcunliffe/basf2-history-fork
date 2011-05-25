/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCTRACKFIDNERMODULE_H_
#define MCTRACKFIDNERMODULE_H_

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>


namespace Belle2 {

  /** This module tries to use Relations to define which hits belong to which particles and writes track candidates into the DataStore.
   *
   *  The Relations between MCParticles <-> SimHits <-> Hits <-> RecoHits have to be used for this purpose.
   *
   *  @author <a href="mailto:martin.heck@kit.edu?subject=MCTrackFinder">Martin Heck</a>
   *
   *  @todo   Currently only the CDC is used in this module. This is mainly for testing purposes. Once everything is defined nicely, SVD and PXD will be added.
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

    void initialize();

    /** Here the actual work is done. */
    void event();

    /** End of the event processing. */
    void terminate();


  private:

    /** Returns the list of 'from' indices for the given 'to' index for the given relation.
     */
    std::list<int> getFromForTo(const std::string& relationName, const unsigned short int& toIndex);

    /** Returns the 'to' index for the given 'from' index for the given relation.
     */
    int getToForFrom(const std::string relationName, int fromIndex) {
      StoreArray<Relation> relations(relationName);
      for (int ii = 0; ii < relations->GetEntriesFast(); ii++) {
        if (relations[ii]->getFromIndex() == fromIndex) {
          return (relations[ii]->getToIndex());
        }
      }
      return (-999);
    }

    std::string m_mcParticlesCollectionName;                          /**< MCParticles collection name */
    std::string m_mcPartToCDCSimHitsCollectionName;                   /**< MCParticles to CDCSimHits relation name */
    std::string m_cdcRecoHitsCollectionName;                          /**< CDCRecoHits collection name */
    std::string m_cdcSimHitToCDCHitCollectioName;                     /**< CDCSimHits to CDCHits relation name */

    std::string m_gfTrackCandsCollectionName;                         /**< TrackCandidates collection name */
    std::string m_gfTrackCandToMCParticleCollectionName;              /**< TrackCandidates to MCParticles relation name */
    std::string m_gfTrackCandToCDCRecoHitsCollectionName;             /**< TrackCandidates to CDCRecoHits relation name */
  };
}

#endif /* MCTRACKFINDERMODULE_H_ */
