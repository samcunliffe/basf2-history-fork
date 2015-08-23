/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/eventdata/entities/CDCFacet.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;


CDCFacet::CDCFacet() :
  CDCRLWireHitTriple(nullptr, nullptr, nullptr),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  B2ERROR("CDCFacet initialized with nullptr for all oriented wire hit");
}



CDCFacet::CDCFacet(const CDCRLWireHit* startRLWireHit,
                   const CDCRLWireHit* middleRLWireHit,
                   const CDCRLWireHit* endRLWireHit) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(),
  m_startToEnd(),
  m_middleToEnd(),
  m_automatonCell()
{
  if (startRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as first oriented wire hit");
  if (middleRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as second oriented wire hit");
  if (endRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as third oriented wire hit");
  adjustLines();
}



CDCFacet::CDCFacet(const CDCRLWireHit* startRLWireHit,
                   const CDCRLWireHit* middleRLWireHit,
                   const CDCRLWireHit* endRLWireHit,
                   const ParameterLine2D& startToMiddle,
                   const ParameterLine2D& startToEnd,
                   const ParameterLine2D& middleToEnd) :
  CDCRLWireHitTriple(startRLWireHit, middleRLWireHit, endRLWireHit),
  m_startToMiddle(startToMiddle),
  m_startToEnd(startToEnd),
  m_middleToEnd(middleToEnd),
  m_automatonCell()
{
  if (startRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as first oriented wire hit");
  if (middleRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as second oriented wire hit");
  if (endRLWireHit == nullptr) B2ERROR("CDCFacet initialized with nullptr as third oriented wire hit");
}



CDCFacet CDCFacet::reversed() const
{
  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();

  return CDCFacet(wireHitTopology.getReverseOf(getEndRLWireHit()),
                  wireHitTopology.getReverseOf(getMiddleRLWireHit()),
                  wireHitTopology.getReverseOf(getStartRLWireHit()));

}

void CDCFacet::adjustLines() const
{
  m_startToMiddle =
    CDCTangent::constructTouchingLine(getStartWireHit().getRefPos2D(),
                                      getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                                      getMiddleWireHit().getRefPos2D(),
                                      getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength());

  m_startToEnd =
    CDCTangent::constructTouchingLine(getStartWireHit().getRefPos2D(),
                                      getStartRLInfo() * getStartWireHit().getRefDriftLength() ,
                                      getEndWireHit().getRefPos2D(),
                                      getEndRLInfo() * getEndWireHit().getRefDriftLength());

  m_middleToEnd =
    CDCTangent::constructTouchingLine(getMiddleWireHit().getRefPos2D(),
                                      getMiddleRLInfo() * getMiddleWireHit().getRefDriftLength() ,
                                      getEndWireHit().getRefPos2D(),
                                      getEndRLInfo() * getEndWireHit().getRefDriftLength());
}
