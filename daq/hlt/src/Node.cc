/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/Node.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

Node::Node(ENodeType nodeType, std::vector<std::string> dest)
{
  m_nodeType = nodeType;

  m_dest.clear();
  m_dest = dest;
}

EStatus Node::init()
{
  if (m_signalMan != NULL) {
    B2ERROR("SignalMan exists!");
    return c_InitFailed;
  } else {
    if (m_dest.size() == 0) {
      B2ERROR("Destinations are not assigned");
      return c_InitFailed;
    } else {
      B2DEBUG("SignalMan initialized with destination");
      m_signalMan = new SignalMan(c_DataInPort, c_DataOutPort, m_dest);
      m_signalMan->init();

      return c_Success;
    }
  }
}

