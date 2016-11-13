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

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <vector>
#include <tuple>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    class CDCTrack;

    /**
     *  Findlet to adjust the flight time of tracks relative to a trigger point.
     *
     *  The flight time of tracks is only adjusted in the start trajectories of each given track
     *  and the time which is need to reach the trigger point with the speed of flight
     *  on the curved trajectory is set as the flight time.
     */
    class TrackFlightTimeAdjuster : public Findlet<CDCTrack&> {

    private:
      /// Type of the base class
      using Super = Findlet<CDCTrack>;

    public:
      /// Short description of the findlet
      std::string getDescription() final;

      /** Add the parameters of the filter to the module */
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final;

      /// Initialize at the start of the event processing
      void initialize() final;

      /// Write give tracks into track store array
      void apply(std::vector<CDCTrack>& tracks) final;

    private:
      /// Parameter: Point relative to which the flight time should be estimated.
      std::tuple<double, double, double> m_param_triggerPoint{0.0, 0.0, 0.0};

      /// Point relative to which the flight time should be estimated.
      Vector3D m_triggerPoint{0.0, 0.0, 0.0};
    };
  }
}
