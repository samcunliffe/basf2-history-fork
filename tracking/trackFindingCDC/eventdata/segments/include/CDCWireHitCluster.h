/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCWireHit;

    /// An aggregation of CDCWireHits.
    class CDCWireHitCluster : public std::vector<const Belle2::TrackFindingCDC::CDCWireHit*> {

      /// Return if this was set as background
      bool getBackgroundFlag() const
      { return m_backgroundFlag; }

      /// Set whether this cluster is consider as made of background
      void setBackgroundFlag(bool backgroundFlag = true)
      { m_backgroundFlag = backgroundFlag; }

    private:
      /// Memory for the flag marking this cluster as background
      bool m_backgroundFlag;

    }; //end class CDCWireHitCluster

  }
}
