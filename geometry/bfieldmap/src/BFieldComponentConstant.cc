/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/bfieldmap/BFieldComponentConstant.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

using namespace std;
using namespace Belle2;


BFieldComponentConstant::BFieldComponentConstant()
{

}


BFieldComponentConstant::~BFieldComponentConstant()
{

}


TVector3 BFieldComponentConstant::calculate(const TVector3& point) const
{
  double Bz = m_magneticField[2];
  if (maxRadius4BField > 0.0 && maxRadius4BField < point.Perp() / Unit::mm) Bz = 0.0;

  //  return TVector3(m_magneticField[0], m_magneticField[1], m_magneticField[2]);
  return TVector3(m_magneticField[0], m_magneticField[1], Bz);
}


void BFieldComponentConstant::setMagneticFieldValues(double x, double y, double z, double rmax)
{
  m_magneticField[0] = x;
  m_magneticField[1] = y;
  m_magneticField[2] = z;
  maxRadius4BField = rmax;  // unit [mm]
}
