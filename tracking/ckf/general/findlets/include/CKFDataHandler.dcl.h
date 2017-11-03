/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Nils Braun                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/ckf/general/findlets/TrackFitterAndDeleter.h>
#include <tracking/trackFindingCDC/findlets/base/StoreArrayLoader.h>

namespace Belle2 {
  /**
   * Findlet for loading the seeds from the data store.
   * Also, the tracks are fitted and only the fittable tracks are passed on.
   */
  template <class AResult>
  class CKFDataHandler : public TrackFindingCDC::Findlet<RecoTrack*> {
    /// Parent class
    using Super = TrackFindingCDC::Findlet<RecoTrack*>;

  public:
    /// Add the subfindlets
    CKFDataHandler();

    /// Expose the parameters of the sub findlets.
    void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override;

    /// Create the store arrays
    void initialize() override;

    /// Load in the reco tracks and the hits
    void apply(std::vector<RecoTrack*>& seeds) override;

    /// Store the reco tracks and the relations
    void store(std::vector<AResult>& results);

  private:
    // Findlets
    /// Findlet for fitting the tracks
    TrackFitterAndDeleter m_trackFitter;

    // Parameters
    /// Export the tracks or not
    bool m_param_exportTracks = true;
    /// StoreArray name of the output Track Store Array
    std::string m_param_outputRecoTrackStoreArrayName = "CKFRecoTracks";
    /// StoreArray name of the input Track Store Array
    std::string m_param_inputRecoTrackStoreArrayName = "RecoTracks";
    /// Minimal pt requirement
    double m_param_minimalPtRequirement = 0.0;

    // Store Arrays
    /// Output Reco Tracks Store Array
    StoreArray<RecoTrack> m_outputRecoTracks;
    /// Input Reco Tracks Store Array
    StoreArray<RecoTrack> m_inputRecoTracks;
  };
}
