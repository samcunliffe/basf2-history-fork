/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Findlet to exports CDCTracks as RecoTracks
    class TrackExporter : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack&>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Signal initialisation phase to register store array for export
      void initialize() final;

      /// Write give tracks into track store array
      void apply(std::vector<CDCTrack>& tracks) final;

    private:
      /// Parameter: Switch if a RecoTrack be generated for each track
      bool m_param_exportTracks = true;

      /// Parameter: Name of the output StoreArray of the RecoTracks generated by this module.
      std::string m_param_exportTracksInto = "";
    };
  }
}
