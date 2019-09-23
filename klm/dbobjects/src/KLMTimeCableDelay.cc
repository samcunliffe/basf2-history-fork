/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jincheng Mei                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <klm/dbobjects/KLMTimeCableDelay.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

KLMTimeCableDelay::KLMTimeCableDelay()
{
  m_effLightSpeed = 0;
  m_effLightSpeedRPC = 0;
  m_ampTimeConstant = 0;
  m_ampTimeConstantRPC = 0;
}

KLMTimeCableDelay::~KLMTimeCableDelay()
{
}

void KLMTimeCableDelay::setTimeShift(uint16_t strip, double par)
{
  std::map<uint16_t, double>::iterator it;
  it = m_timeShift.find(strip);
  if (it == m_timeShift.end())
    m_timeShift.insert(std::pair<uint16_t, double>(strip, par));
  else
    B2WARNING("TimeCalibration para for the strip already exists.");
}

double KLMTimeCableDelay::getTimeShift(uint16_t strip) const
{
  std::map<uint16_t, double>::const_iterator it;
  it = m_timeShift.find(strip);
  if (it == m_timeShift.end())
    return 0.0;
  return it->second;
}

void KLMTimeCableDelay::cleanTimeShift()
{
  m_timeShift.clear();
}

double KLMTimeCableDelay::getEffLightSpeed() const
{
  return m_effLightSpeed;
}

void KLMTimeCableDelay::setEffLightSpeed(double lightSpeed)
{
  m_effLightSpeed = lightSpeed;
}

double KLMTimeCableDelay::getAmpTimeConstant() const
{
  return m_ampTimeConstant;
}

void KLMTimeCableDelay::setAmpTimeConstant(double amplitudeTimeConstant)
{
  m_ampTimeConstant = amplitudeTimeConstant;
}

double KLMTimeCableDelay::getEffLightSpeedRPC() const
{
  return m_effLightSpeedRPC;
}

void KLMTimeCableDelay::setEffLightSpeedRPC(double lightSpeed)
{
  m_effLightSpeedRPC = lightSpeed;
}

double KLMTimeCableDelay::getAmpTimeConstantRPC() const
{
  return m_ampTimeConstantRPC;
}

void KLMTimeCableDelay::setAmpTimeConstantRPC(double amplitudeTimeConstant)
{
  m_ampTimeConstantRPC = amplitudeTimeConstant;
}

