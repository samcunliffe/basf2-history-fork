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

#include <tracking/trackFindingCDC/hough/perigee/SimpleHitBasedHoughTree.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLTaggedWireHit.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A simple hough tree taking rl tagable wire hits as smallest units.
    template<class InBox, size_t ... divisions>
    using SimpleRLTaggedWireHitHoughTree =
      SimpleHitBasedHoughTree<CDCRLTaggedWireHit, InBox, divisions ... >;

  }
}
