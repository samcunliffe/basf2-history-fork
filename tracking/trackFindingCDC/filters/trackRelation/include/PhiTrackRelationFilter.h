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

#include <tracking/trackFindingCDC/filters/trackRelation/BaseTrackRelationFilter.h>

#include <tracking/trackFindingCDC/numerics/Angle.h>

#include <tracking/trackFindingCDC/utilities/ParamList.icc.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Relation filter that lets only possibilities with small phi distance pass
    class PhiTrackRelationFilter : public BaseTrackRelationFilter {

    private:
      /// Type of the super class
      using Super = BaseTrackRelationFilter;

    public:
      /// Export all parameters
      void exposeParams(ParamList* paramList, const std::string& prefix) override
      {
        paramList->addParameter(prefixed(prefix, "maximalPhiDistance"), m_param_maximalPhiDistance,
                                "Maximal Phi distance below to tracks should be merged.",
                                m_param_maximalPhiDistance);
      }

      /// Implementation of the phi calculation.
      Weight operator()(const CDCTrack& fromTrack, const CDCTrack& toTrack) final {
        // Make sure we only have one relation out of A -> B and B -> A
        if (fromTrack.getStartRecoPos3D().y() < toTrack.getStartRecoPos3D().y())
        {
          return NAN;
        }

        const double lhsPhi = fromTrack.getStartTrajectory3D().getFlightDirection3DAtSupport().phi();
        const double rhsPhi = toTrack.getStartTrajectory3D().getFlightDirection3DAtSupport().phi();

        const double phiDistance = std::fabs(AngleUtil::normalised(lhsPhi - rhsPhi));

        if (phiDistance > m_param_maximalPhiDistance)
        {
          return NAN;
        } else {
          return phiDistance;
        }
      }

    private:
      double m_param_maximalPhiDistance = 0.2;
    };
  }
}
