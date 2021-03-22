/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2021 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Wessel                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/
#pragma once

// #include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVariables.h>
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/ThreeHitVariables.h>
#include <tracking/spacePointCreation/SpacePoint.h>
#include <framework/geometry/B2Vector3.h>

#include <cmath>

namespace Belle2 {

  class FourHitVariables {
  public:
    FourHitVariables() : m_oHit(0., 0., 0.), m_ocHit(0., 0., 0.), m_icHit(0., 0., 0.), m_iHit(0., 0., 0.)
    {};

    FourHitVariables(const B2Vector3D& oHit, const B2Vector3D& ocHit, const B2Vector3D& icHit, const B2Vector3D& iHit) :
      m_oHit(oHit), m_ocHit(ocHit), m_icHit(icHit), m_iHit(iHit)
    {
      m_outerThreeHitVariables = ThreeHitVariables(oHit, ocHit, icHit);
      m_innerThreeHitVariables = ThreeHitVariables(ocHit, icHit, iHit);
    };

    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: cm */
    double getCircleRadiusDifference()
    {
//       B2Vector3D outerCircleCenter = getCircleCenterXY(m_oHit, m_ocHit, m_icHit);
//       if (outerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }
//       double outerCircleRadius = calcAvgDistanceXY(m_oHit, m_ocHit, m_icHit, outerCircleCenter);
//
//       B2Vector3D innerCircleCenter = getCircleCenterXY(m_ocHit, m_icHit, m_iHit);
//       if (innerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }
//       double innerCircleRadius = calcAvgDistanceXY(m_ocHit, m_icHit, m_iHit, innerCircleCenter);

      B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
      B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
      if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
        return NAN;
      }
      double outerCircleRadius = m_outerThreeHitVariables.calcAvgDistanceXY(outerCircleCenter);
      double innerCircleRadius = m_innerThreeHitVariables.calcAvgDistanceXY(innerCircleCenter);

      return outerCircleRadius - innerCircleRadius;
    } // return unit: cm


    /** calculates the distance between the estimated circle centers (using 2 subsets of given hits) in the xy-plane, returning unit: cm */
    double getCircleCenterPositionDifference()
    {
//       B2Vector3D outerCircleCenter = getCircleCenterXY(m_oHit, m_ocHit, m_icHit);
//       if (outerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }
//       B2Vector3D innerCircleCenter = getCircleCenterXY(m_ocHit, m_icHit, m_iHit);
//       if (innerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }

      B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
      B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
      if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
        return NAN;
      }

      return fabs(outerCircleCenter.Perp() - innerCircleCenter.Perp());
    } // return unit: GeV/c


    /** calculates dpt-value (dpt= difference in transverse momentum of 2 subsets of the hits), returning unit: GeV/c */
    double getDeltaPT()
    {
//       B2Vector3D outerCircleCenter = getCircleCenterXY(m_oHit, m_ocHit, m_icHit);
//       if (outerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }
//       double outerCircleRadius = calcAvgDistanceXY(m_oHit, m_ocHit, m_icHit, outerCircleCenter);
//
//       B2Vector3D innerCircleCenter = getCircleCenterXY(m_ocHit, m_icHit, m_iHit);
//       if (innerCircleCenter.Perp2() > 1e30) {
//         return NAN;
//       }
//       double innerCircleRadius = calcAvgDistanceXY(m_ocHit, m_icHit, m_iHit, innerCircleCenter);

      B2Vector3D outerCircleCenter = m_outerThreeHitVariables.getCircleCenterXY();
      B2Vector3D innerCircleCenter = m_innerThreeHitVariables.getCircleCenterXY();
      if (outerCircleCenter.Perp2() > 1e30 or innerCircleCenter.Perp2() > 1e30) {
        return NAN;
      }
      double outerCircleRadius = m_outerThreeHitVariables.calcAvgDistanceXY(outerCircleCenter);
      double innerCircleRadius = m_innerThreeHitVariables.calcAvgDistanceXY(innerCircleCenter);

      return fabs(0.00299792458 * m_BFieldZ * (outerCircleRadius - innerCircleRadius));
    } // return unit: GeV/c

    void setBFieldZ(const double bfieldZ = 1.5) { m_BFieldZ = bfieldZ; }

  private:
    double m_BFieldZ = 1.5;
    B2Vector3D m_oHit;
    B2Vector3D m_ocHit;
    B2Vector3D m_icHit;
    B2Vector3D m_iHit;
    ThreeHitVariables m_outerThreeHitVariables;
    ThreeHitVariables m_innerThreeHitVariables;

  };

}
