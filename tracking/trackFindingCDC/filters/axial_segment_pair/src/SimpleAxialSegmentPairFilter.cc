/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/filters/axial_segment_pair/SimpleAxialSegmentPairFilter.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

SimpleAxialSegmentPairFilter::SimpleAxialSegmentPairFilter() : m_riemannFitter()
{
  m_riemannFitter.useOnlyOrientation();
}

CellWeight SimpleAxialSegmentPairFilter::operator()(const CDCAxialSegmentPair& axialSegmentPair)
{
  const CDCAxialRecoSegment2D* ptrStartSegment = axialSegmentPair.getStart();
  const CDCAxialRecoSegment2D* ptrEndSegment = axialSegmentPair.getEnd();

  assert(ptrStartSegment);
  assert(ptrEndSegment);

  const CDCAxialRecoSegment2D& startSegment = *ptrStartSegment;
  const CDCAxialRecoSegment2D& endSegment = *ptrEndSegment;

  // Do fits
  const CDCTrajectory2D& startFit = getFittedTrajectory2D(startSegment);
  const CDCTrajectory2D& endFit = getFittedTrajectory2D(endSegment);

  // Check if segments are coaligned
  bool endSegmentIsCoaligned = startFit.getTotalPerpS(endSegment) >= 0.0;
  bool startSegmentIsCoaligned = endFit.getTotalPerpS(startSegment) >= 0.0;

  if (not endSegmentIsCoaligned or not startSegmentIsCoaligned) {
    return NOT_A_CELL;
  }

  // Check if there is a positive gap between start and end segment
  FloatType startFitGap = startFit.getPerpSGap(startSegment, endSegment);
  FloatType endFitGap = endFit.getPerpSGap(startSegment, endSegment);

  if (startFitGap < 0 or startFitGap > 100 or endFitGap < 0 or endFitGap > 100) {
    return NOT_A_CELL;
  }

  FloatType startFitFrontOffset = startFit.getPerpSFrontOffset(startSegment, endSegment);
  FloatType endFitBackOffset = endFit.getPerpSBackOffset(startSegment, endSegment);

  if (startFitFrontOffset < 0 or endFitBackOffset < 0) {
    return NOT_A_CELL;
  }

  // Momentum agreement cut
  Vector2D startMomAtCenter = startFit.getUnitMom2DAtCenter(startSegment);
  Vector2D endMomAtCenter = endFit.getUnitMom2DAtCenter(endSegment);

  Vector2D startMomAtExtrapolation = startFit.getUnitMom2DAtCenter(endSegment);
  Vector2D endMomAtExtrapolation = endFit.getUnitMom2DAtCenter(startSegment);

  FloatType momAngleAtStartCenter = startMomAtCenter.angleWith(endMomAtExtrapolation);
  FloatType momAngleAtEndCenter = endMomAtCenter.angleWith(startMomAtExtrapolation);

  if (momAngleAtEndCenter > 2 or momAngleAtStartCenter > 2) {
    return NOT_A_CELL;
  }

  // Proximity cut
  FloatType startFit_dist2DToFront_endSegment = startFit.getDist2DToFront(endSegment);
  FloatType endFit_dist2DToBack_startSegment = endFit.getDist2DToBack(startSegment);

  if (startFit_dist2DToFront_endSegment < 6 and  endFit_dist2DToBack_startSegment < 6)
    return startSegment.size() + endSegment.size();
  else {
    return NOT_A_CELL;
  }

}



const CDCTrajectory2D& SimpleAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialRecoSegment2D& segment) const
{

  CDCTrajectory2D& trajectory2D = segment.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, segment);
  }
  return trajectory2D;

}



const CDCTrajectory2D& SimpleAxialSegmentPairFilter::getFittedTrajectory2D(const CDCAxialSegmentPair&
    axialSegmentPair) const
{
  CDCTrajectory2D& trajectory2D = axialSegmentPair.getTrajectory2D();
  if (not trajectory2D.isFitted()) {
    getRiemannFitter().update(trajectory2D, axialSegmentPair);
  }
  return trajectory2D;
}
