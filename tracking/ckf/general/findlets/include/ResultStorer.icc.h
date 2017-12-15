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

#include <tracking/ckf/general/findlets/ResultStorer.dcl.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/dataobjects/RecoTrack.h>

#include <framework/core/ModuleParamList.icc.h>

namespace Belle2 {
  /// Expose the parameters of the sub findlets.
  template <class AResult>
  void ResultStorer<AResult>::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
  {
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "exportTracks"), m_param_exportTracks,
                                  "Export the result tracks into a StoreArray.",
                                  m_param_exportTracks);

    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRecoTrackStoreArrayName"),
                                  m_param_outputRecoTrackStoreArrayName,
                                  "StoreArray name of the output Track Store Array.");
    moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "outputRelationRecoTrackStoreArrayName"),
                                  m_param_outputRelationRecoTrackStoreArrayName,
                                  "StoreArray name of the tracks, the output reco tracks should be related to.");
  }

  /// Create the store arrays
  template <class AResult>
  void ResultStorer<AResult>::initialize()
  {
    Super::initialize();

    if (not m_param_exportTracks) {
      return;
    }

    m_outputRecoTracks.registerInDataStore(m_param_outputRecoTrackStoreArrayName);
    RecoTrack::registerRequiredRelations(m_outputRecoTracks);

    StoreArray<RecoTrack> relationRecoTracks(m_param_outputRelationRecoTrackStoreArrayName);
    relationRecoTracks.registerRelationTo(m_outputRecoTracks);
  }

  /// Store the reco tracks and the relations
  template <class AResult>
  void ResultStorer<AResult>::apply(std::vector<AResult>& results)
  {
    if (not m_param_exportTracks) {
      return;
    }

    for (const auto& result : results) {

      const TVector3& trackPosition = result.getPosition();
      const TVector3& trackMomentum = result.getMomentum();
      const short& trackCharge = result.getCharge();

      RecoTrack* newRecoTrack = m_outputRecoTracks.appendNew(trackPosition, trackMomentum, trackCharge);
      result.addToRecoTrack(*newRecoTrack);

      const RecoTrack* seed = result.getSeed();
      if (not seed) {
        continue;
      }
      seed->addRelationTo(newRecoTrack);
    }
  }
}
