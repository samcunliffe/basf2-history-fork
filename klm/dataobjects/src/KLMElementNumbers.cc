/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/dataobjects/KLMElementNumbers.h>

using namespace Belle2;

KLMElementNumbers::KLMElementNumbers()
{
  m_ElementNumbersEKLM = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMElementNumbers::~KLMElementNumbers()
{
}

const KLMElementNumbers& KLMElementNumbers::Instance()
{
  static KLMElementNumbers klmElementNumbers;
  return klmElementNumbers;
}

uint16_t KLMElementNumbers::channelNumberBKLM(
  int forward, int sector, int layer, int plane, int strip) const
{
  uint16_t channel;
  channel = BKLMElementNumbers::channelNumber(
              forward, sector, layer, plane, strip);
  return channel + m_BKLMOffset;
}

uint16_t KLMElementNumbers::channelNumberBKLM(int bklmChannel) const
{
  return bklmChannel + m_BKLMOffset;
}

uint16_t KLMElementNumbers::channelNumberEKLM(
  int endcap, int sector, int layer, int plane, int strip) const
{
  uint16_t channel;
  /*
   * Note that the default order of elements is different
   * for EKLM-specific code!
   */
  channel = m_ElementNumbersEKLM->stripNumber(
              endcap, layer, sector, plane, strip);
  return channel;
}

uint16_t KLMElementNumbers::channelNumberEKLM(int eklmStrip) const
{
  return eklmStrip;
}

uint16_t KLMElementNumbers::moduleNumberBKLM(
  int forward, int sector, int layer) const
{
  uint16_t module;
  module = BKLMElementNumbers::moduleNumber(forward, sector, layer);
  return module + m_BKLMOffset;
}

uint16_t KLMElementNumbers::moduleNumberEKLM(
  int endcap, int sector, int layer) const
{
  uint16_t module;
  /*
   * Note that the default order of elements is different
   * for EKLM-specific code!
   */
  module = m_ElementNumbersEKLM->sectorNumber(endcap, layer, sector);
  return module;
}
