/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Timofey Uglov, Kirill Chilikin                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TVector3.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMSectorHit.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::EKLMSectorHit)

EKLMSectorHit::EKLMSectorHit(int nEndcap, int nLayer, int  nSector) :
  EKLMHitBase(nEndcap, nLayer, nSector)
{
}

int EKLMSectorHit::addHit(EKLMDigit* hit)
{
  if (hit->getEndcap() != m_Endcap || hit->getLayer() != m_Layer ||
      hit->getSector() != m_Sector)
    return -1;
  m_hits[hit->getPlane() - 1].push_back(hit);
  return 0;
}

int EKLMSectorHit::getHitNumber(int plane)
{
  return m_hits[plane - 1].size();
}

EKLMDigit* EKLMSectorHit::getHit(int plane, int hit)
{
  return m_hits[plane - 1][hit];
}

