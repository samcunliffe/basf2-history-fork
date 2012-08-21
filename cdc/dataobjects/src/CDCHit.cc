/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;

ClassImp(CDCHit)

CDCHit::CDCHit(short driftTime, unsigned short charge,
               unsigned short iSuperLayer, unsigned short iLayer, unsigned short iWire)
{
  setDriftTime(driftTime);
  setADCCount(charge);
  setWireID(iSuperLayer, iLayer, iWire);
}
