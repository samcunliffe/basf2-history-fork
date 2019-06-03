/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Simon Kurz                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

#include <framework/datastore/StoreArray.h>
#include <tracking/dataobjects/RecoTrack.h>

/**
 * Combine two collections of tracks without additional checks.
 */
namespace Belle2 {
  /// Module to combine RecoTracks.
  class TracksCombinerModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    TracksCombinerModule();

    /// Declare required StoreArray
    void initialize() override;

    /// Event processing, combine store array
    void event() override;

  private:
    /// Name of the input CDC StoreArray
    std::string m_temp1RecoTracksStoreArrayName;
    /// Name of the input VXD StoreArray
    std::string m_temp2RecoTracksStoreArrayName;
    /// Name of the output StoreArray
    std::string m_recoTracksStoreArrayName;

    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_temp1RecoTracks;
    /// Store Array of the input tracks
    StoreArray<RecoTrack> m_temp2RecoTracks;
    /// Store Array of the output tracks
    StoreArray<RecoTrack> m_recoTracks;
  };
}

