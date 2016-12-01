/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/Colors.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::vector<std::string> Colors::getList()
{
  return {
    "red",
    "blue",
    "green",
    "orange",
    "cyan",
    "olive",
    "lime",
    "maroon",
    "tomato",
    "turquoise",
    "mediumspringgreen",
    "darkgreen",
    "indigo",
  };
}

double Colors::hueToRgb(const double p, const double q, double t)
{
  while (t < 0) t += 1;
  while (t > 1) t -= 1;
  if (t < 1. / 6.) {
    return p + (q - p) * 6 * t;
  }
  if (t < 1. / 2.) {
    return q;
  }
  if (t < 2. / 3.) {
    return p + (q - p) * (2. / 3. - t) * 6;
  }
  return p;
}

std::array<double, 3> Colors::hlsToRgb(const double h, const double l, const double s)
{
  if (s == 0) {
    return {l, l, l};
  } else {
    double q = l < 0.5 ? l * (1 + s) : l + s - l * s;
    double p = 2 * l - q;
    return {
      hueToRgb(p, q, h + 1. / 3.), //r
      hueToRgb(p, q, h),           //g
      hueToRgb(p, q, h - 1. / 3.)  //b
    };
  }
}
