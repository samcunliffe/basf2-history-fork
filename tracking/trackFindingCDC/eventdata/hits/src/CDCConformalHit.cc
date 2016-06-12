/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/eventdata/hits/CDCConformalHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace TrackFindingCDC;

CDCConformalHit::CDCConformalHit(const CDCWireHit* wireHit)
  : m_wireHit(wireHit)
{
  assert(wireHit);
  std::tie(m_conformalPos2D, m_conformalDriftLength) = performConformalTransformWithRespectToPoint(Vector2D(0, 0));
}

std::tuple<Vector2D, double> CDCConformalHit::performConformalTransformWithRespectToPoint(const Vector2D& pos2D) const
{
  Circle2D conformalDriftCircle = m_wireHit->conformalTransformed(pos2D);

  // TODO : Resolve sad mismatch between the legendre conformal transformation and the one defined in the reset of the CDC tracking.
  return std::make_tuple(conformalDriftCircle.center() * 2,
                         conformalDriftCircle.radius() * 2);
}


bool CDCConformalHit::checkHitDriftLength() const
{
  //Get the position of the hit wire from CDCGeometryParameters
  CDCGeometryPar& cdcg = CDCGeometryPar::Instance();

  Vector3D wireBegin(cdcg.wireForwardPosition(m_wireHit->getWireID().getICLayer(), m_wireHit->getWireID().getIWire()));

  Vector3D wireBeginNeighbor;

  if (m_wireHit->getWireID().getIWire() != 0) {
    wireBeginNeighbor = cdcg.wireForwardPosition(m_wireHit->getWireID().getICLayer(), m_wireHit->getWireID().getIWire() - 1);
  } else {
    wireBeginNeighbor = cdcg.wireForwardPosition(m_wireHit->getWireID().getICLayer(), m_wireHit->getWireID().getIWire() + 1);
  }

  double delta = fabs(Vector3D(wireBegin - wireBeginNeighbor).xy().norm());

  double coef = 1.;

  if (m_wireHit->isAxial()) coef = 0.8;
  else coef = 0.9;


  if (m_wireHit->getRefDriftLength() > delta * coef) {
    return false;
  }

  return true;
}
