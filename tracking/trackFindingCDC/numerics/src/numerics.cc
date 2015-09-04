/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <cmath>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <boost/math/tools/precision.hpp>


using namespace boost::math;
using namespace Belle2;
using namespace TrackFindingCDC;

SignType Belle2::TrackFindingCDC::sign(double x)
{
  return std::isnan(x) ? INVALID_SIGN : (std::signbit(x) ? MINUS : PLUS);
}
