/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTProcess.h>

using namespace Belle2;

/* @brief HLTProcess constructor
 * @param nodeInfo NodeInfo object
*/
HLTProcess::HLTProcess(NodeInfo* nodeInfo)
{
}

/* @brief HLTProcess destructor
*/
HLTProcess::~HLTProcess()
{
}

/* @brief Initializing HLTProcess
 * @return c_Success Initializing success
 * @return c_InitFailed Initializing failed
*/
EStatus HLTProcess::init()
{
  if (m_signalMan != NULL) {
    B2ERROR("SignalMan exists!");
    return c_InitFailed;
  } else {
    if (m_nodeInfo->targetIP().size() == 0) {
      //if (m_dest.size() == 0) {
      B2ERROR("Destinations are not assigned");
      return c_InitFailed;
    } else {
      B2INFO("SignalMan initialized with destination");
      m_signalMan = new SignalMan(c_DataInPort, c_DataOutPort, m_nodeInfo->targetIP()[0]);
      //m_signalMan = new SignalMan(c_DataInPort, c_DataOutPort, m_dest);
      m_signalMan->init();

      return c_Success;
    }
  }
}

