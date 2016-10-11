/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2014 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Jakob Lettenbichler (jakob.lettenbichler@oeaw.ac.at)     *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <tracking/trackFindingVXD/sectorMap/filterFramework/SelectionVariable.h>
#include <tracking/trackFindingVXD/filterTools/SelectionVariableHelper.h>
#include <framework/geometry/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the angle between the hits/vectors (RZ),
   * returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead).
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType >
  class AngleRZSimple : public SelectionVariable< PointType , double > {
  public:
    /** return name of the Class */
    static const std::string name(void) {return "AngleRZSimple"; };


    /** calculates the angle between the hits/vectors (RZ), returning unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)  */
    static double value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D outerVector(outerHit.X() - centerHit.X(), outerHit.Y() - centerHit.Y(), outerHit.Z() - centerHit.Z());
      B2Vector3D innerVector(centerHit.X() - innerHit.X(), centerHit.Y() - innerHit.Y(), centerHit.Z() - innerHit.Z());

      B2Vector3D rzVecAB(outerVector.Perp(), outerVector[2], 0.);
      B2Vector3D rzVecBC(innerVector.Perp(), innerVector[2], 0.);

      // TODO: is the result not just theta_1 - theta_2; with theta angle wrt the z-axis?
      return Helper::calcAngle2D(rzVecAB, rzVecBC);
    } // return unit: none (calculation for degrees is incomplete, if you want readable numbers, use AngleRZFull instead)
  };

}
