/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/numerics/ESign.h>
#include <vector>
#include <functional>
#include <utility>
#include <cmath>
#include <assert.h>

namespace Belle2 {

  namespace TrackFindingCDC {
    /// The famous number
    const double PI = 3.141592653589793238462;

    ///Checks if an integer is even
    template<class AInt>
    inline bool isEven(const AInt& x)
    { return (x % 2) == 0; }

    ///Checks if an integer is odd
    template<class AInt>
    inline bool isOdd(const AInt& x)
    { return (x % 2) != 0; }

    /// Computes the square of an input value, and allows for operations like
    /// TVector3 * TVector3, which results in a double return type
    template<class ANumber>
    inline auto square(ANumber t) -> decltype(t* t) {return t * t;}

    ///Returns the sign of an integer number
    inline int sign(int x)
    { return (x > 0) - (x < 0); }

    ///Returns the sign of a floating point number.
    /** Essentially return the signbit of the float.
     *  This means 0.0 has sign ESign::c_Plus while -0.0 has sign ESign::c_Minus
     *  NAN is treat specially and returns an ESign::c_Invalid
     */
    ESign sign(double x);

    /// Returns the two roots in pq formula
    /** Calculates the two roots of the parabola x*x + p*x + q = 0 in a rather stable manner \n
     *  The first value of the returned pair is the solution with the bigger absolut value. \n
     *  If you ask for the roots of a*x*x + b*x + c = 0, do not your formula to the pq from but use \n
     *  the better behaved solveQuadraticABC */
    inline std::pair<double, double> solveQuadraticPQ(double p, double q)
    {
      //solves the quadratic equation x*x + p*x + q == 0 in a rather stable manner (see book numerical recipes in c++)
      //the result has only float precision though, if the discriminant phalf*phalf - q is small
      //use always double precision in intermediate steps
      const double phalf = p / 2;
      const double discriminant = phalf * phalf - q;

      // calculate bigger solution first !!!
      const double solutionOne = (p > 0) ?  -phalf  - sqrt(discriminant)  : -phalf  + sqrt(discriminant) ;

      //second solution by vieta identity
      return std::pair<double, double>(solutionOne, q / solutionOne);
    }

    /// Returns the two roots of abc formula
    /** Calculates the two roots of the parabola a*x*x + b*x + c = 0 in stable manner. \n
     *  The first value of the returned pair is the solution with the bigger absolut value. \n
     *  The formula also handels the case a == 0 correctly. Here to only root of the linear \n
     *  form b*x + c = 0 is returned in the .second  of the pair, while the bigger 'solution' \n
     *  .first is infinity */
    inline std::pair<double, double> solveQuadraticABC(const double a, const double b, const double c)
    {

      std::pair<double, double> result;

      double& bigSolution = result.first;
      double& smallSolution = result.second;

      /*if ( a == 0 ){
        //this distinction must not be made since the else block gives the same result
        //we may save some computation steps here but maybe that is not worth while
        //since we most often ask for solutions with a != 0

        //essentially this is the zero position of a line
        smallSolution = - c / b
        bigSolution = std::numeric_limits<double>::infinity() ;
      return result;
      }
      */

      const double discriminant = ((double)b) * b - 4 * a * c;
      const double root = sqrt(discriminant);

      const double bigSum = (b > 0) ? -b - root : -b + root;

      bigSolution = bigSum / 2 / a;
      smallSolution = 2 * c / bigSum;

      return result;
    }

    /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
    template<class AResultType = double>
    std::vector<AResultType> linspace(const double start, const double final, const size_t n,
                                      const std::function<AResultType(double)>& map)
    {
      std::vector<AResultType> result;
      result.reserve(n);
      result.push_back(map(start));
      for (size_t i = 1; i < n - 1; ++i) {
        result.push_back(map((start * (n - 1 - i) + final * i) / (n - 1)));
      }
      result.push_back(map(final));
      assert(result.size() == n);
      return result;
    }

    /** Returns n evenly spaced samples, calculated over the closed interval [start, stop ].*/
    template<class AResultType = double>
    std::vector<AResultType> linspace(const double start, const double final, const size_t n)
    {
      auto map = [](const double in) -> AResultType {return AResultType(in);};
      return linspace<AResultType>(start, final, n, map);
    }
  } // end namespace TrackFindingCDC
} // end namespace Belle2

