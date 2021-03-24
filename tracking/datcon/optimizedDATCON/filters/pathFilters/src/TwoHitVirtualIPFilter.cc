/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Wessel                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/datcon/optimizedDATCON/filters/pathFilters/TwoHitVirtualIPFilter.h>
#include <tracking/trackFindingCDC/filters/base/Filter.icc.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <framework/core/ModuleParamList.templateDetails.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;
using namespace TrackFindingCDC;

void TwoHitVirtualIPFilter::beginRun()
{
  const double bFieldZ = BFieldManager::getField(0, 0, 0).Z() / Unit::T;
  m_threeHitVariables.setBFieldZ(bFieldZ);
}


TrackFindingCDC::Weight
TwoHitVirtualIPFilter::operator()(const BasePathFilter::Object& pair)
{
  const std::vector<TrackFindingCDC::WithWeight<const HitData*>>& previousHits = pair.first;

  // Do nothing if path is too short or too long
  if (previousHits.size() != 1) {
    return NAN;
  }

  const B2Vector3D& lastHitPos    = previousHits.at(0)->getHit()->getPosition();
  const B2Vector3D& currentHitPos = pair.second->getHit()->getPosition();

  // filter expects hits from outer to inner
  m_threeHitVariables.setHits(lastHitPos, currentHitPos, m_virtualIPPosition);

  if (m_threeHitVariables.getCosAngleRZSimple() < m_cosRZCut) {
    return NAN;
  }

  const double circleDistanceIP = m_threeHitVariables.getCircleDistanceIP();

  if (circleDistanceIP > m_circleIPDistanceCut) {
    return NAN;
  }

  return fabs(1.0 / circleDistanceIP);
}

void TwoHitVirtualIPFilter::exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix)
{
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "cosRZCut"), m_cosRZCut,
                                "Cut on the absolute value of cosine between the vectors (oHit - cHit) and (cHit - iHit).",
                                m_cosRZCut);
  moduleParamList->addParameter(TrackFindingCDC::prefixed(prefix, "circleIPDistanceCut"), m_circleIPDistanceCut,
                                "Cut on the difference between circle radius and circle center to check whether the circle is compatible with passing through the IP.",
                                m_circleIPDistanceCut);
}
