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
#include <eklm/dataobjects/EKLMSegmentID.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>

using namespace Belle2;

EKLMSegmentID::EKLMSegmentID(
  int endcap, int layer, int sector, int plane, int segment) : m_Endcap(endcap),
  m_Layer(layer), m_Sector(sector), m_Plane(plane), m_Segment(segment)
{
}

EKLMSegmentID::EKLMSegmentID(int segment)
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  elementNumbers.segmentNumberToElementNumbers(
    segment, &m_Endcap, &m_Layer, &m_Sector, &m_Plane, &m_Segment);
}

EKLMSegmentID::~EKLMSegmentID()
{
}

int EKLMSegmentID::getEndcap() const
{
  return m_Endcap;
}

int EKLMSegmentID::getLayer() const
{
  return m_Layer;
}

int EKLMSegmentID::getSector() const
{
  return m_Sector;
}

int EKLMSegmentID::getPlane() const
{
  return m_Plane;
}

int EKLMSegmentID::getSegment() const
{
  return m_Segment;
}

int EKLMSegmentID::getSegmentGlobalNumber() const
{
  static const EKLM::ElementNumbersSingleton& elementNumbers =
    EKLM::ElementNumbersSingleton::Instance();
  return elementNumbers.segmentNumber(m_Endcap, m_Layer, m_Sector, m_Plane,
                                      m_Segment);
}

