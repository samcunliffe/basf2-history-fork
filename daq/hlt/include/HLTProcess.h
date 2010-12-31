/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTPROCESS_H
#define HLTPROCESS_H

#include <string>
#include <vector>

#include <daq/hlt/NodeInfo.h>
#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/SignalMan.h>

namespace Belle2 {
  class HLTProcess {
  public:
    HLTProcess(NodeInfo* nodeInfo);
    ~HLTProcess(void);

    EStatus init(void);

  private:
    NodeInfo* m_nodeInfo;

    SignalMan* m_signalMan;
  };
}

#endif
