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

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {

    class CDCTrack;

    /**
     *  Exports tracks has genfit track candidates
     *  Sets the related genfit track candidates in each exported track
     */

    class TrackExporter:
      public Findlet<CDCTrack> {

    private:
      /// Type of the base class
      typedef Findlet<CDCTrack> Super;

    public:
      /// Short description of the findlet
      virtual std::string getDescription() override;

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix = "") override final;

      /// Signal initialisation phase to register store array for export
      virtual void initialize() override;

      /// Signal new event recreate the store array
      virtual void beginEvent() override;

      /// Write give tracks into track store array
      virtual void apply(std::vector<CDCTrack>& tracks) override final;

    private:
      /// Parameter: Switch if genfit::TrackCandidates shall be generated for each track.
      bool m_param_exportTracks = true;

      /// Parameter: Name of the output StoreArray of the Genfit track candidates generated by this module.
      std::string m_param_exportTracksInto = "";

      /// Flag to use reco tracks instead of genfit tracks
      bool m_param_useRecoTracks = false;

    }; // end class
  } // end namespace TrackFindingCDC
} // end namespace Belle2
