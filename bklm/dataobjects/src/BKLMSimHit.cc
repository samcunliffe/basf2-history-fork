/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/dataobjects/BKLMSimHit.h>

using namespace Belle2;

ClassImp(BKLMSimHit)

//! empty constructor for ROOT
BKLMSimHit::BKLMSimHit() :
  SimHitBase(),
  m_ModuleID(0),
  m_Time(0.0),
  m_EDep(0.0),
  m_LocalX(0.0)
{
}

//! Constructor with initial values
BKLMSimHit::BKLMSimHit(int moduleID, double localX, double time, double eDep) :
  SimHitBase(),
  m_ModuleID(moduleID),
  m_Time(time),
  m_EDep(eDep),
  m_LocalX(localX)
{
}

//! Copy constructor
BKLMSimHit::BKLMSimHit(const BKLMSimHit& hit) :
  SimHitBase(hit),
  m_ModuleID(hit.m_ModuleID),
  m_Time(hit.m_Time),
  m_EDep(hit.m_EDep),
  m_LocalX(hit.m_LocalX)
{
}
