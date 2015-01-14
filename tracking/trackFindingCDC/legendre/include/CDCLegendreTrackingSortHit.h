#pragma once

#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackHit.h>
#include <boost/tuple/tuple.hpp>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
       * Small helper class to perform sorting of hits in the CDC pattern reco
       */

    class TrackHit;

    struct SortHits {

      SortHits(int charge) : m_charge(charge) {}
      bool operator()(TrackHit* hit1, TrackHit* hit2);
      int m_charge;
    };
  }
}
