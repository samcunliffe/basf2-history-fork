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

#include <tracking/trackFindingCDC/hough/perigee/DiscretePhi0.h>
#include <tracking/trackFindingCDC/hough/perigee/DiscreteCurv.h>
#include <tracking/trackFindingCDC/hough/SweepBox.h>
#include <tracking/trackFindingCDC/hough/Box.h>
#include <tracking/trackFindingCDC/hough/SameSignChecker.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    template<class HoughBox>
    float getLowerCurv(const HoughBox& houghBoxWithOutCurv, long)
    { return 0; }

    template<class HoughBox>
    auto getLowerCurv(const HoughBox& houghBoxWithCurv, int) ->
    decltype(houghBoxWithCurv.template getLowerBound<DiscreteCurv>())
    { return houghBoxWithCurv.template getLowerBound<DiscreteCurv>(); }

    template<class HoughBox>
    float getUpperCurv(const HoughBox& houghBoxWithOutCurv, long)
    { return 0; }

    template<class HoughBox>
    auto getUpperCurv(const HoughBox& houghBoxWithCurv, int) ->
    decltype(houghBoxWithCurv.template getUpperBound<DiscreteCurv>())
    { return houghBoxWithCurv.template getUpperBound<DiscreteCurv>(); }


    /// Takes a basic object and sweeps it by some angle phi0 range a around the origin
    template<class T, class Phi0 = DiscretePhi0>
    class Phi0Sweeped : public T {

    public:
      /// Using the constructor of the base class.
      using T::T;

    public:
      /// The box to which this object correspondes.
      typedef SweepBox<Phi0, typename T::HoughBox> HoughBox;

      /// The hough box without the sweep in phi0
      typedef typename T::HoughBox SubordinaryHoughBox;

      /// Constructor taking the curling curvature.
      Phi0Sweeped(const float& curlCurv) : m_curlCurv(curlCurv) {;}

    public:
      /** Function that gives the sign of the distance from an observed drift circle to the sweeped object
       */
      inline SignType getDistanceSign(const HoughBox& houghBox,
                                      const float& x,
                                      const float& y,
                                      const float& signedDriftLength,
                                      const float& dxdz = 0,
                                      const float& dydz = 0) const
      {
        const Vector2D& lowerPhi0Vec(houghBox.template getLowerBound<Phi0>());
        const Vector2D& upperPhi0Vec(houghBox.template getUpperBound<Phi0>());
        const SubordinaryHoughBox& subordinaryHoughBox = houghBox.getSubordinaryBox();

        const float lowerX = x * lowerPhi0Vec.x() + y * lowerPhi0Vec.y();
        const float upperX = x * upperPhi0Vec.x() + y * upperPhi0Vec.y();

        const float& lowerCurv(getLowerCurv(houghBox, 0));
        const float& upperCurv(getUpperCurv(houghBox, 0));

        const bool lowerIsNonCurler = fabs(lowerCurv) < m_curlCurv;
        const bool upperIsNonCurler = fabs(upperCurv) < m_curlCurv;

        if (lowerIsNonCurler and upperIsNonCurler) {
          if (not(lowerX >= 0) and not(upperX >= 0)) return INVALID_SIGN;
        }

        const float lowerY = -x * lowerPhi0Vec.y() + y * lowerPhi0Vec.x();
        const float upperY = -x * upperPhi0Vec.y() + y * upperPhi0Vec.x();

        const float upperDXDZ = dxdz * upperPhi0Vec.x() + dydz * upperPhi0Vec.y();
        const float lowerDXDZ = dxdz * lowerPhi0Vec.x() + dydz * lowerPhi0Vec.y();


        const float lowerDYDZ = -dxdz * lowerPhi0Vec.y() + dydz * lowerPhi0Vec.x();
        const float upperDYDZ = -dxdz * upperPhi0Vec.y() + dydz * upperPhi0Vec.x();

        const SignType lowerDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                          lowerX, lowerY,
                                                          signedDriftLength,
                                                          lowerDXDZ, lowerDYDZ);

        const SignType upperDistSign = T::getDistanceSign(subordinaryHoughBox,
                                                          upperX, upperY,
                                                          signedDriftLength,
                                                          upperDXDZ, upperDYDZ);

        return SameSignChecker::commonSign(lowerDistSign, upperDistSign);
      }

    private:
      /// The curvature above which the trajectory is considered a curler.
      float m_curlCurv = NAN;

    };
  } // end namespace TrackFindingCDC
} // end namespace Belle2
