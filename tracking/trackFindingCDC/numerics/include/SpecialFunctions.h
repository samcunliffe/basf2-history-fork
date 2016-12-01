/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <boost/math/tools/precision.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <cmath>

namespace Belle2 {
  namespace TrackFindingCDC {

    /**
     *  An implementation of the function asin x / x which safely evaluates near x=0
     *
     *  Naming asinc (arc sine cardinalis) in analogy to the sine cardinalis function.
     */
    inline double asinc(double x)
    {
      // Implementation inspired by BOOST's sinc
      BOOST_MATH_STD_USING;

      using namespace boost::math;
      double const taylor_n_bound = tools::forth_root_epsilon<double>();

      if (abs(x) >= taylor_n_bound) {
        return  asin(x) / x;
      } else {
        // approximation by taylor series in x at 0 up to order 0
        double result = 1.0;

        double const taylor_0_bound = tools::epsilon<double>();
        if (abs(x) >= taylor_0_bound) {
          double x2 = x * x;
          // approximation by taylor series in x at 0 up to order 2
          result += x2 / 6.0;

          double const taylor_2_bound = tools::root_epsilon<double>();
          if (abs(x) >= taylor_2_bound) {
            // approximation by taylor series in x at 0 up to order 4
            result += x2 * x2 * (3.0 / 40.0);

          }
        }
        return result;
      }
    }
  }
}
