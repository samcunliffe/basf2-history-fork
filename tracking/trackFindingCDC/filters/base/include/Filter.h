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

#include "FilterBase.h"

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Class for filters on a generic object type.
    template<class Object_>
    class Filter : public FilterBase<Object_> {

    };
  }
}
