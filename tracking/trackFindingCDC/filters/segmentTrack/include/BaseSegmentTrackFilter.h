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

#include <tracking/trackFindingCDC/filters/base/Filter.dcl.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCTrack;
    template class Filter<WeightedRelation<CDCTrack, const CDCSegment2D>>;
    using BaseSegmentTrackFilter = Filter<WeightedRelation<CDCTrack, const CDCSegment2D>>;
  }
}
