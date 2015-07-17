/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>
#include <tracking/trackFindingCDC/legendre/TrackHit.h>

#include <tracking/trackFindingCDC/hough/phi0_curv/Phi0CurvBox.h>
#include <tracking/trackFindingCDC/hough/phi0_curv/RLTagged.h>

#include <tracking/trackFindingCDC/numerics/numerics.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /** Predicate class to check for the containment of hits in a phi0 curv hough space part.
     *  Note this part this code defines the performance of
     *  the search in the hough plain quite significantly and there is probably room for improvement.
     */
    template<bool refined = false>
    class HitInPhi0CurvBox {

    public:
      /// Check if four values have the same sign.
      static inline bool sameSign(float n1, float n2, float n3, float n4)
      {
        return ((n1 > 0 and n2 > 0 and n3 > 0 and n4 > 0) or
                (n1 < 0 and n2 < 0 and n3 < 0 and n4 < 0));
      }

      /// Check if two values have the same sign.
      static inline bool sameSign(float n1, float n2)
      {
        return ((n1 > 0 and n2 > 0) or (n1 < 0 and n2 < 0));
      }
    public:
      /** Checks if the track hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       */
      inline Weight operator()(const TrackHit* trackHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const CDCWireHit* wireHit = trackHit->getUnderlayingCDCWireHit();
        return operator()(wireHit, phi0CurvBox);
      }

      /** Checks if the wire hit is contained in a phi0 curv hough space.
       *  If the wire hit was already determined to be right or left of all tracks
       *  in a wider hough box up the hierarchy only evaluate for that orientation.
       *  If one of the right left passage hypothesis can be ruled out in this box
       *  signal so by tagging it.
       *  Note the that the RLTagged<WireHit*> is obtained as non-const reference to
       *  be able to write back the new right left passage hypothesis.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(RLTagged<const TrackHit*>& rlTaggedTrackHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const RightLeftInfo& rlInfo = rlTaggedTrackHit.getRLInfo();

        const TrackHit* trackHit = rlTaggedTrackHit;
        const CDCWireHit* wireHit = trackHit->getUnderlayingCDCWireHit();

        const FloatType driftLength = wireHit->getRefDriftLength();
        const Vector2D& pos2D =  wireHit->getRefPos2D();
        const FloatType r = wireHit->getRefCylindricalR();

        RightLeftInfo newRLInfo =
          isRightOrLeftObservationIn(r, pos2D, driftLength, phi0CurvBox, rlInfo);

        rlTaggedTrackHit.setRLInfo(newRLInfo);
        return isValidInfo(newRLInfo) ? 1 : NAN;
      }


      /** Checks if the wire hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(const CDCWireHit* wireHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType driftLength = wireHit->getRefDriftLength();
        const Vector2D& pos2D =  wireHit->getRefPos2D();
        const FloatType r = wireHit->getRefCylindricalR();

        bool rightIn = this->isObservationIn(r, pos2D, driftLength, phi0CurvBox);
        bool leftIn = this->isObservationIn(r, pos2D, -driftLength, phi0CurvBox);
        return (rightIn or leftIn) ? 1 : NAN;
      }

      /** Checks if the wire hit is contained in a phi0 curv hough space.
       *  If the wire hit was already determined to be right or left of all tracks
       *  in a wider hough box up the hierarchy only evaluate for that orientation.
       *  If one of the right left passage hypothesis can be ruled out in this box
       *  signal so by tagging it.
       *  Note the that the RLTagged<WireHit*> is obtained as non-const reference to
       *  be able to write back the new right left passage hypothesis.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(RLTagged<const CDCWireHit*>& rlTaggedWireHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const RightLeftInfo& rlInfo = rlTaggedWireHit.getRLInfo();
        const CDCWireHit* wireHit = rlTaggedWireHit;

        const FloatType driftLength = wireHit->getRefDriftLength();
        const Vector2D& pos2D =  wireHit->getRefPos2D();
        const FloatType r = wireHit->getRefCylindricalR();

        RightLeftInfo newRLInfo =
          isRightOrLeftObservationIn(r, pos2D, driftLength, phi0CurvBox, rlInfo);

        rlTaggedWireHit.setRLInfo(newRLInfo);
        return isValidInfo(newRLInfo) ? 1 : NAN;
      }


      /** Checks if the track hit is contained in a phi0 curv hough space.
       *  Returns 1.0 if it is contained, returns NAN if it is not contained.
       *  Accepts if either the right passage hypothesis or the left passage hypothesis
       *  is in the box.
       */
      inline Weight operator()(const CDCRLWireHit* rlWireHit,
                               const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType signedDriftLength = rlWireHit-> getSignedRefDriftLength();
        const Vector2D& pos2D =  rlWireHit->getRefPos2D();
        const FloatType r = rlWireHit->getRefCylindricalR();

        bool in = this->isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
        return in ? 1 : NAN;
      }

      /** Checks if an observation (in the CDC)
       *  can be assoziated as left or right of a trajectory in the phi0 curvature hough space box.
       *  The evaluation can be limited by the optional rlInfo parameter.
       *
       *  @param r           (Precomputed) Cylindrical radius of the observed position
       *  @param pos2D       Position of the observation
       *  @param dirftLength Drift radius around position of the observation
       *  @param phi0CurvBox The phi0 curvature hough space region.
       *  @param rlInfo      If rlInfo is RIGHT or LEFT the observations is only checked
       *                     for this hypothesis. All other values lead to checks of both
       *                     orientations.
       *  @returns
       *      * UNKNOWN if both right and left are still possible.
       *      * LEFT if only left is still possible.
       *      * RIGHT if only right is still possible.
       *      * INVALID_INFO if non of the orientations is possible.
       */
      inline RightLeftInfo isRightOrLeftObservationIn(const FloatType& r,
                                                      const Vector2D& pos2D,
                                                      const FloatType& driftLength,
                                                      const Phi0CurvBox* phi0CurvBox,
                                                      const RightLeftInfo rlInfo = UNKNOWN)
      {
        bool rightIn = rlInfo != LEFT and this->isObservationIn(r, pos2D, driftLength, phi0CurvBox);
        bool leftIn = rlInfo != RIGHT and this->isObservationIn(r, pos2D, -driftLength, phi0CurvBox);

        if (rightIn and leftIn) {
          return UNKNOWN;
        } else if (rightIn) {
          return RIGHT;
        } else if (leftIn) {
          return LEFT;
        } else {
          return INVALID_INFO;
        }
      }

      /** Checks if an observation (in the CDC)
       *  can be assoziated to a trajectory in the phi0 curvature hough space box.
       *
       *  @param pos2D             Position of the observation
       *  @param signedDirftLength Drift radius around position of the observation
       *  @param phi0CurvBox       The phi0 curvature hough space region.
       *  @returns                 True if the observation can be linked to a trajectory in the box.
       */
      inline bool isObservationIn(const Vector2D& pos2D,
                                  const FloatType& signedDriftLength,
                                  const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType r = pos2D.norm();
        return isObservationIn(r, pos2D, signedDriftLength, phi0CurvBox);
      }


      /** Checks if an observation (in the CDC)
       *  can be assoziated to a trajectory in the phi0 curvature hough space box.
       *
       *  @param r                 (Precomputed) Cylindrical radius of the observed position
       *  @param pos2D             Position of the observation
       *  @param signedDirftLength Drift radius around position of the observation
       *  @param phi0CurvBox       The phi0 curvature hough space region.
       *  @returns                 True if the observation can be linked to a trajectory in the box.
       */
      inline bool isObservationIn(const FloatType& r,
                                  const Vector2D& pos2D,
                                  const FloatType& signedDriftLength,
                                  const Phi0CurvBox* phi0CurvBox)
      {
        const FloatType rReducedSquared = (r + signedDriftLength) * (r - signedDriftLength);

        const Vector2D& lowerPhi0Vec = phi0CurvBox->getLowerPhi0Vec();
        const Vector2D& upperPhi0Vec = phi0CurvBox->getUpperPhi0Vec();

        const FloatType orthoToPhi0[2] = { pos2D.cross(lowerPhi0Vec), pos2D.cross(upperPhi0Vec) };

        const float& lowerCurv = phi0CurvBox->getLowerCurv();
        const float& upperCurv = phi0CurvBox->getUpperCurv();

        const FloatType rSquareTimesHalfCurv[2] = {
          rReducedSquared* (lowerCurv / 2),
          rReducedSquared* (upperCurv / 2)
        };

        float dist[2][2];
        dist[0][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[0] - signedDriftLength;
        dist[0][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[0] - signedDriftLength;
        dist[1][0] = rSquareTimesHalfCurv[0] + orthoToPhi0[1] - signedDriftLength;
        dist[1][1] = rSquareTimesHalfCurv[1] + orthoToPhi0[1] - signedDriftLength;

        // Sinogram separates at least on of the edges from the others.
        if (not sameSign(dist[0][0], dist[0][1], dist[1][0], dist[1][1])) return true;
        if (not refined) return false;

        // Continue to check if the extrema of the sinogram are in the box,
        // while only touching only one boundary of the box
        // (but not crossing two what the check above actually means).
        // Currently only checking the positive curvature branch correctly
        FloatType extremR = r - signedDriftLength;
        bool extremRInCurvRange = not sameSign(extremR * lowerCurv - 1, extremR * upperCurv - 1);
        if (not extremRInCurvRange) return false;

        Vector2D extremPhi0Vec = pos2D.orthogonal(CCW); // Not normalised but does not matter.
        bool extremPhi0VecInPhi0Range = extremPhi0Vec.isBetween(lowerPhi0Vec, upperPhi0Vec);
        return extremRInCurvRange and extremPhi0VecInPhi0Range;
      }

    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
