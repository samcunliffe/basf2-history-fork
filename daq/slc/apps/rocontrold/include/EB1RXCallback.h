#ifndef _Belle2_EB1RXCallback_h
#define _Belle2_EB1RXCallback_h

#include "daq/slc/readout/ProcessController.h"

#include <daq/slc/base/ConfigFile.h>
#include <daq/slc/nsm/NSMData.h>

#include "daq/slc/runcontrol/RCCallback.h"
#include "daq/slc/readout/FlowMonitor.h"

namespace Belle2 {

  class EB1RXCallback : public RCCallback {

  public:
    EB1RXCallback(const NSMNode& node, const std::string& conf);
    virtual ~EB1RXCallback() throw();

  public:
    virtual void init() throw();
    virtual void term() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();
    virtual void timeout() throw();

  private:
    ProcessController m_con;
    std::vector<FlowMonitor> m_flow;
    NSMData m_data;
    ConfigFile m_file;
    int m_eflag;
    int m_reserved_i[2];

  };

}

#endif
