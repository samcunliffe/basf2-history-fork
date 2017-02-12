/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentRelation/BasicSegmentRelationVarSet.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

using namespace Belle2;
using namespace TrackFindingCDC;

bool BasicSegmentRelationVarSet::extract(const Relation<const CDCSegment2D>* ptrSegmentRelation)
{
  if (not ptrSegmentRelation) return false;

  const Relation<const CDCSegment2D>& segmentPair = *ptrSegmentRelation;

  const CDCSegment2D* ptrFromSegment = segmentPair.getFrom();
  const CDCSegment2D* ptrToSegment = segmentPair.getTo();

  const CDCSegment2D& fromSegment = *ptrFromSegment;
  const CDCSegment2D& toSegment = *ptrToSegment;

  var<named("stereo_kind")>() = static_cast<float>(fromSegment.getStereoKind());
  var<named("sl_id")>() = fromSegment.getISuperLayer();

  var<named("from_size")>() = fromSegment.size();
  var<named("to_size")>() = toSegment.size();
  return true;
}
