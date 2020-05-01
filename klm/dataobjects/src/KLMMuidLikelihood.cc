/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen, Giacomo De Pietro                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/dataobjects/KLMMuidLikelihood.h>

using namespace std;
using namespace Belle2;

KLMMuidLikelihood::KLMMuidLikelihood() :
  m_PDGCode(0),
  m_JunkPDFValue(false),
  m_ChiSquared(0.0),
  m_DegreesOfFreedom(0),
  m_Outcome(0),
  m_IsForward(true),
  m_BarrelExtLayer(-1),
  m_EndcapExtLayer(-1),
  m_ExtLayer(-1),
  m_BarrelHitLayer(-1),
  m_EndcapHitLayer(-1),
  m_HitLayer(-1),
  m_ExtLayerPattern(0),
  m_HitLayerPattern(0)
{
  for (const auto& particle : Const::chargedStableSet) {
    m_PDFValue[particle.getIndex()] = 0.0;
    m_LogL[particle.getIndex()] = -1.0E20;
  }
  for (int i = 0; i < BKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtBKLMEfficiencyValue[i] = 1.0;
  }
  for (int i = 0; i < EKLMElementNumbers::getMaximalLayerNumber(); ++i) {
    m_ExtEKLMEfficiencyValue[i] = 1.0;
  }
}

KLMMuidLikelihood::~KLMMuidLikelihood()
{
}

int KLMMuidLikelihood::getCharge() const
{
  int charge = 1;
  int pdg = KLMMuidLikelihood::getPDGCode();
  if (pdg < 0)
    charge = -charge;
  if ((std::abs(pdg) == Const::muon.getPDGCode()) || (std::abs(pdg) == Const::electron.getPDGCode()))
    charge = -charge;
  return charge;
}

unsigned int KLMMuidLikelihood::getTotalBarrelHits() const
{
  unsigned int hits = 0;
  unsigned int pattern = m_HitLayerPattern;
  for (int bit = 0; bit < BKLMElementNumbers::getMaximalLayerNumber(); ++bit) {
    if (pattern & (1 << bit)) {
      hits++;
    }
  }
  return hits;
}

unsigned int KLMMuidLikelihood::getTotalEndcapHits() const
{
  unsigned int hits = 0;
  unsigned int pattern = m_HitLayerPattern;
  for (int bit = BKLMElementNumbers::getMaximalLayerNumber();
       bit < BKLMElementNumbers::getMaximalLayerNumber() + EKLMElementNumbers::getMaximalLayerNumber(); ++bit) {
    if (pattern & (1 << bit)) {
      hits++;
    }
  }
  return hits;
}
