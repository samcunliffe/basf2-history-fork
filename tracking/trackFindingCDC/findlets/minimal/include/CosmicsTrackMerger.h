/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/findlets/minimal/WeightedRelationCreator.h>
#include <tracking/trackFindingCDC/filters/trackRelation/ChooseableTrackRelationFilter.h>

#include <vector>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;

    /// Links cosmics tracks based on filter criterion
    class CosmicsTrackMerger : public Findlet<const CDCTrack, CDCTrack> {
      /// Type of the base class
      using Super = Findlet<const CDCTrack, CDCTrack>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      CosmicsTrackMerger();

      /// Short description of the findlet
      std::string getDescription() final;

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Main algorithm
      void apply(const std::vector<CDCTrack>& inputTracks, std::vector<CDCTrack>& outputTracks) final;

    private:
      /// Subfindlet for creating relations of needed
      ChooseableTrackRelationFilter m_trackRelationFilter;

      /// Memory for the relations between tracks to be followed on linking
      std::vector<WeightedRelation<const CDCTrack> > m_trackRelations;
    };
  }
}
