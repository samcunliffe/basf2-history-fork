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

#include <tracking/ckf/filters/cdcToSpacePoint/result/BaseCDCVXDTrackCombinationFilter.h>
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcVXDTrackCombinationTruthNames[] = {
    "truth",
    "truth_number_of_correct_hits",
    "truth_number_of_mc_hits",
    "truth_event_number",
    "truth_cdc_number",
    "truth_teacher"
  };

  /// Vehicle class to transport the variable names
  class CDCVXDTrackCombinationTruthVarNames : public
    TrackFindingCDC::VarNames<BaseCDCVXDTrackCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcVXDTrackCombinationTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcVXDTrackCombinationTruthNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCVXDTrackCombinationTruthVarSet : public TrackFindingCDC::VarSet<CDCVXDTrackCombinationTruthVarNames> {

  public:
    /// Generate and assign the variables from the object.
    virtual bool extract(const BaseCDCVXDTrackCombinationFilter::Object* object) override;
  };
}
