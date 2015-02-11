/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef MCAXIALAXIALSEGMENTPAIRFILTER_H_
#define MCAXIALAXIALSEGMENTPAIRFILTER_H_

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialAxialSegmentPair.h>
#include <tracking/trackFindingCDC/algorithms/CellWeight.h>
#include "BaseAxialAxialSegmentPairFilter.h"
#include <tracking/trackFindingCDC/rootification/IfNotCint.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    class MCAxialAxialSegmentPairFilter : public BaseAxialAxialSegmentPairFilter {

    public:
      /// Constructor
      MCAxialAxialSegmentPairFilter(bool allowReverse = true);

      /// Empty destructor
      ~MCAxialAxialSegmentPairFilter() {;}

    public:
      /// Clears all remember information from the last event
      virtual void clear() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void initialize() override final {;}

      /// Forwards the modules initialize to the filter
      virtual void terminate() override final {;}

      /// Checks if a pair of axial segments is a good combination
      virtual CellWeight isGoodAxialAxialSegmentPair(const Belle2::TrackFindingCDC::CDCAxialAxialSegmentPair& axialAxialSegmentPair) IF_NOT_CINT(override final);

    private:
      /// Switch to indicate if the reversed version of the segment pair shall also be accepted (default is true).
      bool m_allowReverse;

    }; // end class MCAxialAxialSegmentPairFilter


  } //end namespace TrackFindingCDC
} //end namespace Belle2

#endif // MCAXIALAXIALSEGMENTPAIRFILTER_H_
