/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ExtHit.h>

#include <framework/logging/Logger.h>

using namespace Belle2;

// empty constructor for ROOT - do not use this
ExtHit::ExtHit() :
  RelationsObject(),
  m_PdgCode(0),
  m_DetectorID(Const::EDetector::invalidDetector),
  m_CopyID(0),
  m_Status(EXT_FIRST),
  m_TOF(0.0),
  m_Position(TVector3(0.0, 0.0, 0.0)),
  m_Momentum(TVector3(0.0, 0.0, 0.0)),
  m_Covariance(TMatrixDSym(6))
{
}

// Constructor with initial values
ExtHit::ExtHit(int pdgCode, Const::EDetector detectorID, int copyID, ExtHitStatus status, double tof,
               const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance) :
  RelationsObject(),
  m_PdgCode(pdgCode),
  m_DetectorID(detectorID),
  m_CopyID(copyID),
  m_Status(status),
  m_TOF(tof),
  m_Position(position),
  m_Momentum(momentum),
  m_Covariance(covariance)
{
}

// Copy constructor
ExtHit::ExtHit(const ExtHit& h) :
  RelationsObject(h),
  m_PdgCode(h.m_PdgCode),
  m_DetectorID(h.m_DetectorID),
  m_CopyID(h.m_CopyID),
  m_Status(h.m_Status),
  m_TOF(h.m_TOF),
  m_Position(h.m_Position),
  m_Momentum(h.m_Momentum),
  m_Covariance(h.m_Covariance)
{
}
