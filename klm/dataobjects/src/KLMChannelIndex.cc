/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <eklm/dataobjects/EKLMElementNumbers.h>
#include <klm/dataobjects/KLMChannelIndex.h>

using namespace Belle2;

KLMChannelIndex::KLMChannelIndex(enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Subdetector(KLMElementNumbers::c_BKLM),
  m_Section(0),
  m_Sector(1),
  m_Layer(1),
  m_Plane(0),
  m_Strip(1)
{
  m_NStripsPlane = BKLMElementNumbers::getNStrips(
                     m_Section, m_Sector, m_Layer, m_Plane);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMChannelIndex::KLMChannelIndex(
  int subdetector, int section, int sector, int layer, int plane, int strip,
  enum IndexLevel indexLevel) :
  m_IndexLevel(indexLevel),
  m_Subdetector(subdetector),
  m_Section(section),
  m_Sector(sector),
  m_Layer(layer),
  m_Plane(plane),
  m_Strip(strip)
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    m_NStripsPlane = BKLMElementNumbers::getNStrips(
                       m_Section, m_Sector, m_Layer, m_Plane);
  } else {
    m_NStripsPlane = 0;
  }
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMChannelIndex::~KLMChannelIndex()
{
}

void KLMChannelIndex::setIndexLevel(enum IndexLevel indexLevel)
{
  m_IndexLevel = indexLevel;
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    if (indexLevel == c_IndexLevelStrip) {
      m_NStripsPlane = BKLMElementNumbers::getNStrips(
                         m_Section, m_Sector, m_Layer, m_Plane);
    }
  }
}

uint16_t KLMChannelIndex::getKLMChannelNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    return m_ElementNumbers->channelNumberBKLM(
             m_Section, m_Sector, m_Layer, m_Plane, m_Strip);
  } else {
    return m_ElementNumbers->channelNumberEKLM(
             m_Section, m_Sector, m_Layer, m_Plane, m_Strip);
  }
}

uint16_t KLMChannelIndex::getKLMModuleNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    return m_ElementNumbers->moduleNumberBKLM(m_Section, m_Sector, m_Layer);
  else
    return m_ElementNumbers->moduleNumberEKLM(m_Section, m_Sector, m_Layer);
}

uint16_t KLMChannelIndex::getKLMSectorNumber() const
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM)
    return m_ElementNumbers->sectorNumberBKLM(m_Section, m_Sector);
  else
    return m_ElementNumbers->sectorNumberEKLM(m_Section, m_Sector);
}

KLMChannelIndex KLMChannelIndex::beginBKLM()
{
  return KLMChannelIndex(KLMElementNumbers::c_BKLM, 0, 1, 1, 0, 1,
                         m_IndexLevel);
}

KLMChannelIndex& KLMChannelIndex::endBKLM()
{
  /*
   * The index level does not matter for end check, thus,
   * the object can be created once.
   */
  static KLMChannelIndex index(KLMElementNumbers::c_EKLM, 1, 1, 1, 1, 1);
  return index;
}

KLMChannelIndex KLMChannelIndex::beginEKLM()
{
  return KLMChannelIndex(KLMElementNumbers::c_EKLM, 1, 1, 1, 1, 1,
                         m_IndexLevel);
}

KLMChannelIndex& KLMChannelIndex::endEKLM()
{
  /*
   * The index level does not matter for end check, thus,
   * the object can be created once.
   */
  static KLMChannelIndex index(KLMElementNumbers::c_EKLM + 1, 1, 1, 1, 1, 1);
  return index;
}

void KLMChannelIndex::increment(enum IndexLevel indexLevel)
{
  if (m_Subdetector == KLMElementNumbers::c_BKLM) {
    switch (indexLevel) {
      case c_IndexLevelStrip:
        m_Strip++;
        if (m_Strip > m_NStripsPlane) {
          m_Strip = 1;
          increment(c_IndexLevelPlane);
          m_NStripsPlane = BKLMElementNumbers::getNStrips(
                             m_Section, m_Sector, m_Layer, m_Plane);
        }
        break;
      case c_IndexLevelPlane:
        m_Plane++;
        if (m_Plane > BKLMElementNumbers::getMaximalPlaneNumber()) {
          m_Plane = 0;
          increment(c_IndexLevelLayer);
        }
        break;
      case c_IndexLevelLayer:
        m_Layer++;
        if (m_Layer > BKLMElementNumbers::getMaximalLayerNumber()) {
          m_Layer = 1;
          increment(c_IndexLevelSector);
        }
        break;
      case c_IndexLevelSector:
        m_Sector++;
        if (m_Sector > BKLMElementNumbers::getMaximalSectorNumber()) {
          m_Sector = 1;
          increment(c_IndexLevelSection);
        }
        break;
      case c_IndexLevelSection:
        m_Section++;
        if (m_Section > BKLMElementNumbers::getMaximalForwardNumber()) {
          m_Section = 0;
          increment(c_IndexLevelSubdetector);
        }
        break;
      case c_IndexLevelSubdetector:
        *this = beginEKLM();
        break;
    }
  } else {
    switch (indexLevel) {
      case c_IndexLevelStrip:
        m_Strip++;
        if (m_Strip > EKLMElementNumbers::getMaximalStripNumber()) {
          m_Strip = 1;
          increment(c_IndexLevelPlane);
        }
        break;
      case c_IndexLevelPlane:
        m_Plane++;
        if (m_Plane > EKLMElementNumbers::getMaximalPlaneNumber()) {
          m_Plane = 1;
          increment(c_IndexLevelLayer);
        }
        break;
      case c_IndexLevelLayer:
        m_Layer++;
        if (m_Layer > m_ElementNumbersEKLM->getMaximalDetectorLayerNumber(m_Section)) {
          m_Layer = 1;
          increment(c_IndexLevelSector);
        }
        break;
      case c_IndexLevelSector:
        m_Sector++;
        if (m_Sector > EKLMElementNumbers::getMaximalSectorNumber()) {
          m_Sector = 1;
          increment(c_IndexLevelSection);
        }
        break;
      case c_IndexLevelSection:
        m_Section++;
        if (m_Section > EKLMElementNumbers::getMaximalEndcapNumber()) {
          m_Section = 1;
          increment(c_IndexLevelSubdetector);
        }
        break;
      case c_IndexLevelSubdetector:
        m_Subdetector++;
        break;
    }
  }
}

KLMChannelIndex& KLMChannelIndex::operator++()
{
  increment(m_IndexLevel);
  return *this;
}

KLMChannelIndex& KLMChannelIndex::increment()
{
  increment(m_IndexLevel);
  return *this;
}

bool KLMChannelIndex::operator==(KLMChannelIndex& index)
{
  switch (m_IndexLevel) {
    case c_IndexLevelStrip:
      if (m_Strip != index.getStrip())
        return false;
      [[fallthrough]];
    case c_IndexLevelPlane:
      if (m_Plane != index.getPlane())
        return false;
      [[fallthrough]];
    case c_IndexLevelLayer:
      if (m_Layer != index.getLayer())
        return false;
      [[fallthrough]];
    case c_IndexLevelSector:
      if (m_Sector != index.getSector())
        return false;
      [[fallthrough]];
    case c_IndexLevelSection:
      if (m_Section != index.getSection())
        return false;
      [[fallthrough]];
    case c_IndexLevelSubdetector:
      if (m_Subdetector != index.getSubdetector())
        return false;
  }
  return true;
}

bool KLMChannelIndex::operator!=(KLMChannelIndex& index)
{
  switch (m_IndexLevel) {
    case c_IndexLevelStrip:
      if (m_Strip != index.getStrip())
        return true;
      [[fallthrough]];
    case c_IndexLevelPlane:
      if (m_Plane != index.getPlane())
        return true;
      [[fallthrough]];
    case c_IndexLevelLayer:
      if (m_Layer != index.getLayer())
        return true;
      [[fallthrough]];
    case c_IndexLevelSector:
      if (m_Sector != index.getSector())
        return true;
      [[fallthrough]];
    case c_IndexLevelSection:
      if (m_Section != index.getSection())
        return true;
      [[fallthrough]];
    case c_IndexLevelSubdetector:
      if (m_Subdetector != index.getSubdetector())
        return true;
  }
  return false;
}

KLMChannelIndex& KLMChannelIndex::operator*()
{
  return *this;
}
