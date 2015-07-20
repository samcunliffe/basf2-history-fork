/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/varsets/VarSet.h>
#include <tracking/trackFindingCDC/varsets/VarNames.h>

#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;

    /// Names of the variables to be generated.
    IF_NOT_CINT(constexpr)
    static char const* const cdcTrackNames[] = {
      "size",
      "pt",
      "sz_slope",
      //"fit_prob_3d",
      //"fit_prob_2d",
      //"fit_prob_sz",
      "drift_length_mean",
      "drift_length_variance",
      "drift_length_max",
      "drift_length_min",
      "drift_length_sum",

      "adc_mean",
      "adc_variance",
      "adc_max",
      "adc_min",
      "adc_sum",

      "empty_s_mean",
      "empty_s_variance",
      "empty_s_max",
      "empty_s_min",
      "empty_s_sum",

      "s_range",
    };

    /** Class that specifies the names of the variables
     *  that should be generated from a segment.
     */
    class CDCTrackVarNames : public VarNames<CDCTrack> {

    public:
      /// Number of variables to be generated.
      static const size_t nNames = 19;

      IF_NOT_CINT(constexpr)
      static char const* getName(int iName)
      {
        return cdcTrackNames[iName];
      }
    };

    /** Class that computes floating point variables from a segment.
     *  that can be forwarded to a flat TNTuple or a TMVA method
     */
    class CDCTrackVarSet : public VarSet<CDCTrackVarNames> {

    public:
      /// Construct the peeler and take an optional prefix.
      explicit CDCTrackVarSet(const std::string& prefix = "") : VarSet<CDCTrackVarNames>(prefix) { }

      /// Generate and assign the variables from the cluster
      virtual bool extract(const CDCTrack* segment) IF_NOT_CINT(override);

    };
  }
}
