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

#include <type_traits>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Short form for std::enable_if::type - replace with std::enable_if_t when ROOT understands it.
    template <bool cond, class T = void>
    using EnableIf [[deprecated]] = typename std::enable_if_t<cond, T>;
  }
}
