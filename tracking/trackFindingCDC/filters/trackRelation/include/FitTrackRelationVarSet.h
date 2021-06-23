/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/utilities/Relation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated
    constexpr
    static char const* const fitTrackRelationVarNames[] = {
      "is_fitted",
      "curv",
      "curv_var",
      "z0",
      "z0_var",
      "tanl",
      "tanl_var",
      "chi2",
      "chi2_per_ndf",
      "ndf",
      "p_value",
      "p_value_cut",
    };

    /// Vehicle class to transport the variable names
    struct FitTrackRelationVarNames : public VarNames<Relation<const CDCTrack>> {

      /// Number of variables to be generated
      // we shouldn't use public member variables but we do want to rewrite all related code using setters/getters
      // at least tell cppcheck that everything is fine
      // cppcheck-suppress duplInheritedMember
      static const size_t nVars = size(fitTrackRelationVarNames);

      /// Getter for the name at the given index
      static constexpr char const* getName(int iName)
      {
        return fitTrackRelationVarNames[iName];
      }
    };

    /**
     *  Class to compute floating point variables from a track relation
     *  which can be recorded as a flat TNtuple or serve as input to a MVA method
     */
    class FitTrackRelationVarSet : public VarSet<FitTrackRelationVarNames> {

    public:
      /// Generate and assign the contained variables
      bool extract(const Relation<const CDCTrack>* ptrTrackRelation) final;
    };
  }
}
