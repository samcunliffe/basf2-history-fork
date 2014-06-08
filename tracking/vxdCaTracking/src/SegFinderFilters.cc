/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/vxdCaTracking/SegFinderFilters.h"
#include "tracking/vxdCaTracking/Cutoff.h"
#include <TRandom.h>

using namespace std;
using namespace Belle2;

bool SegFinderFilters::checkDist3D(int nameDist3D)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDist3D();
    if (aResult > pCutoff->getMinValue() and aResult < pCutoff->getMaxValue()) {   // min & max
      m_dist3DCtr.first++;
      return true;
    }
  }
  m_dist3DCtr.second++;
  return false;
}

bool SegFinderFilters::checkDistXY(int nameDistXY)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDistXY, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistXY();
    if (aResult > pCutoff->getMinValue() and aResult < pCutoff->getMaxValue()) {   // min & max
      m_distXYCtr.first++;
      return true;
    }
  }
  m_distXYCtr.second++;
  return false;
}

bool SegFinderFilters::checkDistZ(int nameDistZ)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameDistZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcDistZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {   // min & max
      m_distZCtr.first++;
      return true;
    }
  }
  m_distZCtr.second++;
  return false;
}

bool SegFinderFilters::checkNormedDist3D(int nameNormDist3D)    // max only
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameNormDist3D, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcNormedDist3D();
    if (aResult < pCutoff->getMaxValue()) {
      m_normDist3DCtr.first++;
      return true;
    }
  }
  m_normDist3DCtr.second++;
  return false;
}

bool SegFinderFilters::checkSlopeRZ(int nameSlopeRZ)    // min & max
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(nameSlopeRZ, m_friendID);
  if (pCutoff != NULL) {
    double aResult = TwoHitFilters::calcSlopeRZ();
    if (aResult > pCutoff->getMinValue() && aResult < pCutoff->getMaxValue()) {
      m_slopeRZCtr.first++;
      return true;
    }
  }
  m_slopeRZCtr.second++;
  return false;
}


pair <double, double> SegFinderFilters::getCutoffs(int aFilter)
{
  const Cutoff* pCutoff = m_thisSector->getCutoff(aFilter, m_friendID);
  if (pCutoff != NULL) {
//     pair <double, double> aPair = make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
    return make_pair(pCutoff->getMinValue(), pCutoff->getMaxValue());
  }
  return make_pair(0., 0.);
}


bool SegFinderFilters::checkAlwaysTrue2Hit(int nameAlwaysTrue2Hit)
{
  if (std::isnan(nameAlwaysTrue2Hit) == true) { return false; }
  return true;
}


bool SegFinderFilters::checkAlwaysFalse2Hit(int nameAlwaysFalse2Hit)
{
  if (std::isnan(nameAlwaysFalse2Hit) == true) { return true; }
  return false;
}


bool SegFinderFilters::checkRandom2Hit(int nameRandom2Hit)
{
  if (std::isnan(nameRandom2Hit) == true) { return false; }
  double randomVal = gRandom->Uniform(0, 1);
  if (randomVal > 0.5) { return true; } // 50% chance of getting true
  return false;
}


double SegFinderFilters::calcRandom2Hit() { return gRandom->Uniform(0, 1); }