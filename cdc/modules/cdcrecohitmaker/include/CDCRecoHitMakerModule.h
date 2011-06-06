/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef RECOHITMAKERMODULE_H_
#define RECOHITMAKERMODULE_H_

#include <framework/core/Module.h>
#include <vector>


namespace Belle2 {

  /** Module for making CDCRecoHits of CDCHits.
   *
   */
  class CDCRecoHitMakerModule : public Module {

  public:

    /** Constructor.
     */
    CDCRecoHitMakerModule();

    /** Destructor.
     */
    ~CDCRecoHitMakerModule();

    /** Creating the actual RecoHits.
     */
    void event();


  private:
    /** Availability of relations.
     *
     *  Usually will be there in case of MonteCarlo.
     */
    bool        m_mc;

    /** Name of collection of CDCHits.
     */
    std::string m_cdcHitCollectionName;

    /** Name of collection of relations between SimHits and Hits.
     *
     *  This variable can only exist in MonteCarlo.
     */
    std::string m_simHitToCDCHitCollectionName;

    /** Name of collection of relations between MCParticles and SimHits.
      *
      *  This variable can only exist in MonteCarlo.
      */
    std::string m_mcPartToCDCSimHitsCollectionName;

    /** Name of MCParticle collection (needed for relation creation).
      */
    std::string m_mcParticlesCollectionName;

    /** Name of output collection of this module.
      */
    std::string m_cdcRecoHitCollectionName;

    /** Name of output relation collection of this module.
     */
    std::string m_mcParticleToCDCRecoHits;


  };
}

#endif /* EVTMETAINFO_H_ */
