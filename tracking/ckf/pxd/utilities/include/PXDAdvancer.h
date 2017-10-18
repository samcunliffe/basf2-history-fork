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

#include <tracking/ckf/general/utilities/Advance.h>

namespace Belle2 {
  class SpacePoint;

  class PXDAdvancer : public Advancer {
  public:
    double extrapolateToPlane(genfit::MeasuredStateOnPlane& measuredStateOnPlane,
                              const SpacePoint& spacePoint, double direction = 1);

    using Advancer::extrapolateToPlane;
  };
}