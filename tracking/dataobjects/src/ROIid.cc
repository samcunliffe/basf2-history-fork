/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ROIid.h>


using namespace Belle2;

ROIid::ROIid() : m_minUid(0), m_maxUid(0), m_minVid(0), m_maxVid(0),
  m_sensorID()
{
}

ROIid::~ROIid()
{
}


bool
ROIid::Contains(const PXDRawHit& thePXDRawHit) const
{
  return (m_minUid <= thePXDRawHit.getUCellID() &&
          m_maxUid >= thePXDRawHit.getUCellID() &&
          m_minVid <= thePXDRawHit.getVCellID() &&
          m_maxVid >= thePXDRawHit.getVCellID() &&
          m_sensorID == thePXDRawHit.getSensorID()
         );
}


bool
ROIid::Contains(const PXDDigit& thePXDDigit) const
{
  return (m_minUid <= thePXDDigit.getUCellID() &&
          m_maxUid >= thePXDDigit.getUCellID() &&
          m_minVid <= thePXDDigit.getVCellID() &&
          m_maxVid >= thePXDDigit.getVCellID() &&
          m_sensorID == thePXDDigit.getSensorID()
         );
}

bool ROIid::Contains(const Belle2::SVDShaperDigit& theSVDDigit) const
{

  bool inside = false;

  if (theSVDDigit.isUStrip())
    inside = (m_minUid <= theSVDDigit.getCellID() && m_maxUid >= theSVDDigit.getCellID());
  else
    inside = (m_minVid <= theSVDDigit.getCellID() && m_maxVid >= theSVDDigit.getCellID());

  return (inside && m_sensorID == theSVDDigit.getSensorID());

}
