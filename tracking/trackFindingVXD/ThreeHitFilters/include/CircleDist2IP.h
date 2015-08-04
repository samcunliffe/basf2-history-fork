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

#include <tracking/trackFindingVXD/FilterTools/SelectionVariable.h>
#include <tracking/trackFindingVXD/FilterTools/SelectionVariableHelper.h>
#include <tracking/vectorTools/B2Vector3.h>
#include <math.h>

namespace Belle2 {

  /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm
   *
   * WARNING: this filter returns 0 if no valid value could be found!
   * */
  template <typename PointType, typename VariableType >
  class CircleDist2IP : public SelectionVariable< PointType , VariableType > {
  public:

    /** calculates the distance of the point of closest approach of circle to the IP, returning unit: cm */
    static VariableType value(const PointType& outerHit, const PointType& centerHit, const PointType& innerHit)
    {
      typedef SelVarHelper<PointType, double> Helper;

      B2Vector3D cCenter = Helper::calcCircleCenter(outerHit, centerHit, innerHit);
      double circleRadius = Helper::calcRadius(outerHit, centerHit, innerHit, cCenter);

      // distance of closest approach of circle to the IP:
      return (fabs(cCenter.Perp() - circleRadius));
    } // return unit: cm
  };

}
