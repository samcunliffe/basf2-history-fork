/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/track_relation/BaseTrackRelationFilter.h>
#include <tracking/trackFindingCDC/filters/track_relation/TrackRelationFilterFactory.h>
#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    using ChooseableTrackRelationFilter = ChooseableFilter<TrackRelationFilterFactory>;
  }
}
