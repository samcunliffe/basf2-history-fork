/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck & Oksana Brovchenko                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/dataobjects/Track.h>
#include <tracking/dataobjects/ParticleCode.h>


using namespace Belle2;

ClassImp(Track);

Track::Track() : m_trackFitResultsName("")
{
}

Track::Track(const GFTrack& gftrack)
{
  // Take over of GFTrack information
}


TrackFitResult* Track::getTrackFitResult(const ParticleCode& particleCode)
{

  return m_trackFitResults[m_trackFitIndices[particleCode.getIContinuous()]];
}



