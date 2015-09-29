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

#include <tracking/trackFindingCDC/utilities/Ptr.h>
#include <tracking/trackFindingCDC/numerics/ERightLeft.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class AHitPtr>
    class RLTagged : public StarToPtr<AHitPtr> {

    private:
      /// Type of the base class
      using Super = StarToPtr<AHitPtr>;

    public:
      /// Inheriting the constructor of the base class
      using Super::Super;

      /// Getter for the right left passage hypothesis
      ERightLeft getRLInfo() const
      { return m_rlInfo; }

      /// Setter for the right left passage hypothesis
      void setRLInfo(const ERightLeft rlInfo)
      { m_rlInfo = rlInfo; }

    private:
      /// Memory for the right left passage hypotheses assoziated with the hit.
      ERightLeft m_rlInfo = ERightLeft::c_Unknown;

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
