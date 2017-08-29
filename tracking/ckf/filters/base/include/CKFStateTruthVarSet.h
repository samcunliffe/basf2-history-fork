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

#include <tracking/ckf/filters/cdcToSpacePoint/state/BaseCKFCDCToSpacePointStateObjectFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <framework/dataobjects/EventMetaData.h>
#include <tracking/mcMatcher/TrackMatchLookUp.h>
#include <framework/datastore/StoreObjPtr.h>
#include <tracking/ckf/utilities/StateAlgorithms.h>
#include <tracking/trackFindingCDC/utilities/Algorithms.h>
#include <tracking/ckf/utilities/CKFMCUtils.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const ckfStateTruthVarNames[] = {
    "truth",
    "truth_position_x",
    "truth_position_y",
    "truth_position_z",
    "truth_momentum_x",
    "truth_momentum_y",
    "truth_momentum_z",
    "event_id",
    "seed_number"
  };

  /// Vehicle class to transport the variable names
  template<class ASeedObject, class AHitObject>
  class CKFStateTruthVarNames : public
    TrackFindingCDC::VarNames<CKFStateObject<ASeedObject, AHitObject>> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(ckfStateTruthVarNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return ckfStateTruthVarNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  template<class ASeedObject, class AHitObject>
  class CKFStateTruthVarSet : public TrackFindingCDC::VarSet<CKFStateTruthVarNames<ASeedObject, AHitObject>> {
    /// The parent class
    using Super = TrackFindingCDC::VarSet<CKFStateTruthVarNames<ASeedObject, AHitObject>>;

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const CKFStateObject<ASeedObject, AHitObject>* result) override
    {
      RecoTrack* seedTrack = result->getSeedRecoTrack();
      const auto* hit = result->getHit();

      if (not seedTrack) return false;

      StoreObjPtr<EventMetaData> eventMetaData;
      Super::template var<Super::named("event_id")>() = eventMetaData->getEvent();
      Super::template var<Super::named("seed_number")>() = seedTrack->getArrayIndex();

      const std::string& seedTrackStoreArrayName = seedTrack->getArrayName();

      TrackMatchLookUp mcCDCMatchLookUp("MCRecoTracks", seedTrackStoreArrayName);
      const RecoTrack* cdcMCTrack = mcCDCMatchLookUp.getMatchedMCRecoTrack(*seedTrack);

      // Default to 0, -1 or false (depending on context)
      Super::template var<Super::named("truth_position_x")>() = 0;
      Super::template var<Super::named("truth_position_y")>() = 0;
      Super::template var<Super::named("truth_position_z")>() = 0;
      Super::template var<Super::named("truth_momentum_x")>() = 0;
      Super::template var<Super::named("truth_momentum_y")>() = 0;
      Super::template var<Super::named("truth_momentum_z")>() = 0;
      Super::template var<Super::named("truth")>() = false;

      // In case the CDC track is a fake, return false always
      if (not cdcMCTrack) {
        return true;
      }

      if (not hit) {
        // on every second layer (the overlap layers) it is fine to have no space point
        if (isOnOverlapLayer(*result)) {
          Super::template var<Super::named("truth")>() = true;
          return true;
        } else {
          // it is also fine, if the MC track does not have this layer
          const auto& svdHits = cdcMCTrack->getSVDHitList();
          const auto& pxdHits = cdcMCTrack->getPXDHitList();

          const bool hasSVDLayer = TrackFindingCDC::any(svdHits, [&result](const SVDCluster * cluster) {
            return cluster->getSensorID().getLayerNumber() == extractGeometryLayer(*result);
          });
          const bool hasPXDLayer = TrackFindingCDC::any(pxdHits, [&result](const PXDCluster * cluster) {
            return cluster->getSensorID().getLayerNumber() == extractGeometryLayer(*result);
          });

          if (hasSVDLayer or hasPXDLayer) {
            return true;
          } else {
            Super::template var<Super::named("truth")>() = true;
            return true;
          }
        }
      }

      if (not isCorrectHit(*hit, *cdcMCTrack)) {
        // Keep all variables set to false and return.
        return true;
      }

      Super::template var<Super::named("truth")>() = true;

      Super::template var<Super::named("truth_position_x")>() = cdcMCTrack->getPositionSeed().X();
      Super::template var<Super::named("truth_position_y")>() = cdcMCTrack->getPositionSeed().Y();
      Super::template var<Super::named("truth_position_z")>() = cdcMCTrack->getPositionSeed().Z();
      Super::template var<Super::named("truth_momentum_x")>() = cdcMCTrack->getMomentumSeed().X();
      Super::template var<Super::named("truth_momentum_y")>() = cdcMCTrack->getMomentumSeed().Y();
      Super::template var<Super::named("truth_momentum_z")>() = cdcMCTrack->getMomentumSeed().Z();

      return true;
    }
  };
}
