/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace Belle2::TrackFindingCDC;

const CDCRiemannFitter& CDCRiemannFitter::getFitter()
{
  static CDCRiemannFitter fitter;
  return fitter;
}

const CDCRiemannFitter& CDCRiemannFitter::getLineFitter()
{
  static CDCRiemannFitter lineFitter;
  lineFitter.setLineConstrained();
  return lineFitter;
}

const CDCRiemannFitter& CDCRiemannFitter::getOriginCircleFitter()
{
  static CDCRiemannFitter originCircleFitter;
  originCircleFitter.setOriginConstrained();
  return originCircleFitter;
}
