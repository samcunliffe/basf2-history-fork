/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/hough/z0_tanLambda/Z0TanLambdaBox.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a z0 tan lambda hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    class HitInZ0TanLambdaBox {
    public:
      /** Checks if the wire hit is contained in a z0 tan lambda hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const CDCRecoHit3D* recoHit,
                               const Z0TanLambdaBox* z0TanLambdaBox)
      {
        const float& lowerZ0 = z0TanLambdaBox->getLowerZ0();
        const float& upperZ0 = z0TanLambdaBox->getUpperZ0();

        const float& perpS = recoHit->getArcLength2D();
        const float& reconstructedZ = recoHit->getRecoZ();

        const float& lowerTanLambda = z0TanLambdaBox->getLowerTanLambda();
        const float& upperTanLambda = z0TanLambdaBox->getUpperTanLambda();

        float dist[2][2];
        dist[0][0] = perpS * lowerTanLambda - reconstructedZ + lowerZ0;
        dist[0][1] = perpS * lowerTanLambda - reconstructedZ + upperZ0;
        dist[1][0] = perpS * upperTanLambda - reconstructedZ + lowerZ0;
        dist[1][1] = perpS * upperTanLambda - reconstructedZ + upperZ0;

        if (not SameSignChecker::sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) {
          return 1.0;
        } else {
          return NAN;
        }
      }
    };
  }
}
