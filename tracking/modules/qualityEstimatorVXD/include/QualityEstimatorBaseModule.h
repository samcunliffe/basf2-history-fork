/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner, Sebastian Racs                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <tracking/spacePointCreation/SpacePointTrackCand.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorBase.h>

#include <framework/datastore/StoreArray.h>
#include <framework/core/Module.h>

#include <memory>
#include <string>
#include <vector>


namespace Belle2 {

  /** Base Module of all quality estimation type module using SpacePointTrackCandidates.
   *  */
  class QualityEstimatorBaseModule : public Module {

  public:

    /** Constructor of the module. */
    QualityEstimatorBaseModule();

    /** Initializes the Module. */
    virtual void initialize();

    virtual void beginRun();

    /** Applies the selected quality estimation method for a given set of TCs */
    virtual void event();

    virtual void singleSPTCevent(SpacePointTrackCand&) = 0;


  protected:

    // module parameters

    /** Identifier which estimation method to use. Valid identifiers are:
     * mcInfo
     * circleFit
     * tripletFit
     * helixFit
     * random
     */
    std::string m_EstimationMethod;

    /** sets the name of the expected StoreArray containing SpacePointTrackCands */
    std::string m_SpacePointTrackCandsStoreArrayName;

    /** sets the name of the expected StoreArray containing MCRecoTracks. Only required for MCInfo method */
    std::string m_MCRecoTracksStoreArrayName;
    /** Only required for MCInfo method */
    bool m_MCStrictQualityEstimator;

    // member variables

    /** the storeArray for SpacePointTrackCands as member, is faster than recreating link for each event */
    StoreArray<SpacePointTrackCand> m_spacePointTrackCands;

    /** pointer to the selected QualityEstimator */
    std::unique_ptr<QualityEstimatorBase> m_estimator;

  };
}
