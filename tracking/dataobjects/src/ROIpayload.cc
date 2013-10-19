/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/dataobjects/ROIpayload.h>
#include <arpa/inet.h>
#include <boost/crc.hpp>
#include <assert.h>
#include <stdio.h>

using namespace Belle2;
using boost::crc_optimal;

ROIpayload::ROIpayload(int length)
{

  init(length);

};



void ROIpayload::init(int length)
{
  m_index = 0;
  m_length = length;
  m_rootdata = new int[length];
  m_data32 = (unsigned int*)m_rootdata;
  m_data64 = (ROIrawID::baseType*)(m_data32 + 3);
}

void ROIpayload::setPayloadLength(int length)
{

  m_data32[0] = htobe32(length);

  m_packetLengthByte = length + sizeof(int);
}

void ROIpayload::setHeader()
{

  m_data32[1] = htobe32(0xCAFE8000);

};

void ROIpayload::setTriggerNumber(unsigned long int triggerNumber)
{

  m_data32[2] = htobe32(triggerNumber);

};

void ROIpayload::addROIraw(ROIrawID roiraw)
{

  assert((int*)(m_data64 + m_index) < m_rootdata + m_length);

  m_data64[m_index++] = roiraw.getBigEndian();

}

void ROIpayload::setCRC()
{

  assert((int*)(m_data64 + m_index) ==  m_rootdata + m_length - 1);

  crc_optimal<32, 0x04C11DB7, 0, 0, false, false> dhh_crc_32;

  dhh_crc_32.process_bytes((void*)(m_rootdata + 1), 2 * sizeof(unsigned int) + m_index * sizeof(unsigned long int));

  m_data32[m_index * 2 + 3] = htobe32(dhh_crc_32.checksum()) ;


}

ClassImp(ROIpayload);
