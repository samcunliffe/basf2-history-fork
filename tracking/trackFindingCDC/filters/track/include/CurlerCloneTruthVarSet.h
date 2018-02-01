/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Michael Eliachevitch                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr static char const* const curlerCloneTruthVarNames[] = {
      "weight", // if CDCTrack has minimalMatchPurity (50%)
      "track_is_fake", // less than 80% purity in CDC
      "track_is_matched", // not fake
      "truth_matched_hits",
      "track_is_curler_clone_truth", // track is clone
      "truth", // CDC track is not clone
      "truth_first_nloops",
      "truth_event_id",
      "truth_MCTrackID",
    };

    /// Vehicle class to transport the variable names
    struct CurlerCloneTruthVarNames : public VarNames<CDCTrack> {

      /// Number of variables to be generated
      static const size_t nVars = size(curlerCloneTruthVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return curlerCloneTruthVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class CurlerCloneTruthVarSet : public VarSet<CurlerCloneTruthVarNames> {

    public:
      /// Require the Monte Carlo truth information at initialisation
      void initialize() final;

      /// Prepare the Monte Carlo truth information at start of the event
      void beginEvent() final;

      /// Generate and assign the contained variables
      bool extract(const CDCTrack* ptrCDCTrack) override;

    private:
      /// Type of the base class
      using Super = VarSet<CurlerCloneTruthVarNames>;

      StoreObjPtr<EventMetaData> m_eventMetaData;
    };
  }
}
