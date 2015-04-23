/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include "BaseAxialAxialSegmentPairFilter.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialAxialSegmentPairFilter : public Filter<CDCAxialAxialSegmentPair> {

    private:
      /// Type of the super class
      typedef Filter<CDCAxialAxialSegmentPair> Super;

    public:
      /// Constructor
      MCAxialAxialSegmentPairFilter(bool allowReverse = true);

      /** Set the parameter with key to value.
       *
       *  Parameters are:
       *  symmetric -  Accept the axial axial segment pair if the reverse axial axial segment pair
       *               is correct preserving the progagation reversal symmetry
       *               on this level of detail.
       *               Allowed values "true", "false". Default is "true".
       */
      virtual
      void setParameter(const std::string& key, const std::string& value) IF_NOT_CINT(override);

      /** Returns a map of keys to descriptions describing the individual parameters of the filter.
       */
      virtual
      std::map<std::string, std::string> getParameterDescription() IF_NOT_CINT(override);

      /// Indicates that the filter requires Monte Carlo information.
      virtual bool needsTruthInformation() IF_NOT_CINT(override final);

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight operator()(const Belle2::TrackFindingCDC::CDCAxialAxialSegmentPair& axialAxialSegmentPair) IF_NOT_CINT(
        override final);

    public:
      /// Setter for the allow reverse parameter
      void setAllowReverse(bool allowReverse)
      { m_param_allowReverse = allowReverse; }

      /// Getter for the allow reverse parameter
      bool getAllowReverse() const
      { return m_param_allowReverse; }

    private:
      /// Switch to indicate if the reversed version of the segment pair shall also be accepted (default is true).
      bool m_param_allowReverse;

    }; // end class MCAxialAxialSegmentPairFilter

  } //end namespace TrackFindingCDC
} //end namespace Belle2
