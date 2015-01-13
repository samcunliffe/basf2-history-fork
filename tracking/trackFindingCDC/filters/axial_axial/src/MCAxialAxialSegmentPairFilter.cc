/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCAxialAxialSegmentPairFilter.h"

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCAxialAxialSegmentPairFilter::MCAxialAxialSegmentPairFilter()
{


}

MCAxialAxialSegmentPairFilter::~MCAxialAxialSegmentPairFilter()
{



}



void MCAxialAxialSegmentPairFilter::clear()
{
}



void MCAxialAxialSegmentPairFilter::initialize()
{
}



void MCAxialAxialSegmentPairFilter::terminate()
{
}



CellWeight MCAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair(const CDCAxialAxialSegmentPair& axialAxialSegmentPair, bool allowBackward) const
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialAxialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialAxialSegmentPair.getEnd();

  if (ptrStartSegment == nullptr) {
    B2ERROR("MCAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as start segment");
    return NOT_A_CELL;
  }

  if (ptrEndSegment == nullptr) {
    B2ERROR("MCAxialAxialSegmentPairFilter::isGoodAxialAxialSegmentPair invoked with nullptr as end segment");
    return NOT_A_CELL;
  }

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();


  // Check if the segments are aligned correctly along the Monte Carlo track
  ForwardBackwardInfo pairFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrEndSegment);
  if (pairFBInfo == INVALID_INFO) return NOT_A_CELL;

  if (pairFBInfo == FORWARD or (allowBackward and pairFBInfo == BACKWARD)) {
    // Final check for the distance between the segment
    Index startNPassedSuperLayers = mcSegmentLookUp.getLastNPassedSuperLayers(ptrStartSegment);
    if (startNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    Index endNPassedSuperLayers = mcSegmentLookUp.getFirstNPassedSuperLayers(ptrEndSegment);
    if (endNPassedSuperLayers == INVALID_INDEX) return NOT_A_CELL;

    if (abs(startNPassedSuperLayers - endNPassedSuperLayers) > 2) return NOT_A_CELL;

    //do fits?
    return startSegment.size() + endSegment.size();
  }

  return NOT_A_CELL;

}
