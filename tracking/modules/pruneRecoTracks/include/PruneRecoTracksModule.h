/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Hauth                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/core/Module.h>

/**
 * This module calls RecoTrack::prune to remove hits before
 * the track gets stored to disk to save storage space.
 * By default, this module will remove all but the first and last hits of
 * the track.
 */

namespace Belle2 {

  /// Depricated alias for TrackFinderCDCAutomatonModule
  class PruneRecoTracksModule : public Module {

  public:
    /// Constructor of the module. Setting up parameters and description.
    PruneRecoTracksModule();

    // Declare required StoreArray
    virtual void initialize() override;

    /// Event processing, prunes the RecoTracks contained in each event
    virtual void event() override;

  private:

    // flags used for pruning, See genfit::Track class for possible settings
    std::string m_pruneFlags;

    // name of the StoreArray to prune
    std::string m_storeArrayName;

  }; // end class
} // end namespace Belle2

