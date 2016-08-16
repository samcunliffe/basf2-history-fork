/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <tracking/trackFindingCDC/eventdata/hits/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/boxes/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/baseelements/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
      * Predicate class to check for the containment of hits in a z0 tan lambda hough space part using a different algorithm than normal.
      * Instead of calculating of the hit belongs to the box, it constructs a trajectory out if the box parameters (using its taln lambda and z0)
      * and checks the distance from the hit to the trajectory. It returns e^{-distance}.
      * Note this part this code defines the performance of
      * the search in the hough plain quite significantly and there is probably room for improvement.
      */
    class HitInZ0TanLambdaBoxUsingZ {
    public:

      /// Use a Z0TanLambdaBox
      typedef Z0TanLambdaBox HoughBox;

      /**
       * Return exp{-distance} with distance = distance between the trajectory constructed from the box parameters
       * and the hit in the xy-plane.
       * Note that this is not a binary decision and must be used with some sort of cutoff (because otherwise all hits belong in all boxes).
       */
      inline Weight operator()(const CDCRecoHit3D& recoHit,
                               const HoughBox* z0TanLambdaBox)
      {
        const CDCWire& wire = recoHit.getWire();
        const WireLine& wireLine = wire.getWireLine();

        const float& lowerZ0 = z0TanLambdaBox->getLowerZ0();
        const float& upperZ0 = z0TanLambdaBox->getUpperZ0();
        const float centerZ0 = 0.5 * (lowerZ0 + upperZ0);

        const float& lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        const float& upperTanLambda = z0TanLambdaBox->getUpperTanLambda();
        const float centerTanLambda = 0.5 * (lowerTanLambda + upperTanLambda);

        const float& perpS = recoHit.getArcLength2D();
        const Vector2D& recoPosition = recoHit.getRecoPos2D();

        const float& hitZ = centerTanLambda * perpS + centerZ0;

        const Vector3D& pos3D = wireLine.pos3DAtZ(hitZ);

        const float& distanceToRecoPosition = (pos3D.xy() - recoPosition).norm();

        return exp(-distanceToRecoPosition);
      }
    };
  }
}
