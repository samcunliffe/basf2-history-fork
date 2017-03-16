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
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>
#include <tracking/modules/cdcToVXDExtrapolator/BaseCDCTrackSpacePointCombinationFilter.h>

namespace Belle2 {
  /// Names of the variables to be generated.
  constexpr
  static char const* const cdcTrackSpacePointCombinationTruthNames[] = {
    "truth"
  };

  /// Vehicle class to transport the variable names
  class CDCTrackSpacePointCombinationTruthVarNames : public
    TrackFindingCDC::VarNames<BaseCDCTrackSpacePointCombinationFilter::Object> {

  public:
    /// Number of variables to be generated.
    static const size_t nVars = TrackFindingCDC::size(cdcTrackSpacePointCombinationTruthNames);

    /// Get the name of the column.
    constexpr
    static char const* getName(int iName)
    {
      return cdcTrackSpacePointCombinationTruthNames[iName];
    }
  };

  /**
   * Var set used in the VXD-CDC-Merger for calculating the probability of a VXD-CDC-track match,
   * which knows the truth information if two tracks belong together or not.
   */
  class CDCTrackSpacePointCombinationTruthVarSet : public TrackFindingCDC::VarSet<CDCTrackSpacePointCombinationTruthVarNames> {

  public:
    /// Generate and assign the variables from the pair.
    virtual bool extract(const BaseCDCTrackSpacePointCombinationFilter::Object* pair) override;
  };
}
