/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Belle2 headers. */
#include <eklm/dbobjects/EKLMAlignment.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

ClassImp(Belle2::EKLMAlignment);

EKLMAlignment::EKLMAlignment()
{
}

EKLMAlignment::~EKLMAlignment()
{
}

void EKLMAlignment::setAlignmentData(uint16_t segment,
                                     EKLMAlignmentData* dat)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_data.find(segment);
  if (it == m_data.end())
    m_data.insert(std::pair<uint16_t, EKLMAlignmentData>(
                    segment, *dat));
  else
    B2WARNING("Alignment data for the segment already exists.");
}

struct EKLMAlignmentData* EKLMAlignment::getAlignmentData(uint16_t segment)
{
  std::map<uint16_t, EKLMAlignmentData>::iterator it;
  it = m_data.find(segment);
  if (it == m_data.end())
    return NULL;
  return &(it->second);
}

