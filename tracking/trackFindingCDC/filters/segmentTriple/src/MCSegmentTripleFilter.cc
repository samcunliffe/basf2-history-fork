/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/segmentTriple/MCSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory3D.h>


#include <framework/logging/Logger.h>

#include <TDatabasePDG.h>

using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleFilter::MCSegmentTripleFilter(bool allowReverse) :
  Super(allowReverse),
  m_mcAxialSegmentPairFilter(allowReverse)
{
}


void MCSegmentTripleFilter::beginEvent()
{
  m_mcAxialSegmentPairFilter.beginEvent();
  Super::beginEvent();
}



void MCSegmentTripleFilter::initialize()
{
  Super::initialize();
  m_mcAxialSegmentPairFilter.initialize();
}



void MCSegmentTripleFilter::terminate()
{
  m_mcAxialSegmentPairFilter.terminate();
  Super::terminate();
}



Weight MCSegmentTripleFilter::operator()(const CDCSegmentTriple& segmentTriple)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  const CDCStereoRecoSegment2D* ptrMiddleSegment = segmentTriple.getMiddleSegment();
  const CDCAxialRecoSegment2D* ptrEndSegment = segmentTriple.getEndSegment();

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& middleSegment = *ptrMiddleSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  /// Recheck the axial axial compatability
  Weight pairWeight =
    m_mcAxialSegmentPairFilter(CDCAxialSegmentPair(ptrStartSegment, ptrEndSegment));

  if (std::isnan(pairWeight)) return NAN;

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  // Check if the segments are aligned correctly along the Monte Carlo track
  EForwardBackward startToMiddleFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrStartSegment, ptrMiddleSegment);
  if (startToMiddleFBInfo == EForwardBackward::c_Invalid) return NAN;

  EForwardBackward middleToEndFBInfo = mcSegmentLookUp.areAlignedInMCTrack(ptrMiddleSegment, ptrEndSegment);
  if (middleToEndFBInfo == EForwardBackward::c_Invalid) return NAN;


  if (startToMiddleFBInfo != middleToEndFBInfo) return NAN;


  if ((startToMiddleFBInfo == EForwardBackward::c_Forward and middleToEndFBInfo == EForwardBackward::c_Forward) or
      (getAllowReverse() and startToMiddleFBInfo == EForwardBackward::c_Backward and middleToEndFBInfo == EForwardBackward::c_Backward)) {

    // Do fits
    setTrajectoryOf(segmentTriple);

    CellState cellWeight = startSegment.size() + middleSegment.size() + endSegment.size();
    return cellWeight;

  }

  return NAN;
}



void MCSegmentTripleFilter::setTrajectoryOf(const CDCSegmentTriple& segmentTriple) const
{
  if (segmentTriple.getTrajectorySZ().isFitted()) {
    // SZ trajectory has been fitted before. Skipping
    // A fit sz trajectory implies a 2d trajectory to be fitted, but not the other way around
    return;
  }

  const CDCAxialRecoSegment2D* ptrStartSegment = segmentTriple.getStartSegment();
  if (not ptrStartSegment) {
    B2WARNING("Start segment of segmentTriple is nullptr. Could not set fits.");
    return;
  }

  const CDCMCSegmentLookUp& mcSegmentLookUp = CDCMCSegmentLookUp::getInstance();

  CDCTrajectory3D trajectory3D = mcSegmentLookUp.getTrajectory3D(ptrStartSegment);
  segmentTriple.setTrajectory3D(trajectory3D);
}
