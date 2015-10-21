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

#include <tracking/trackFindingCDC/ca/CellWeight.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /// Filter type that accepts all items (just for reference).
    template<class AType>
    class AllFilter : public AType {
    public:
      /** Accept all items. */
      virtual CellWeight operator()(const typename AType::Object&) override final
      {
        return 1;
      }

    };
  }
}
