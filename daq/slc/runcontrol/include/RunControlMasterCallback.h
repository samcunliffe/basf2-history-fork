#ifndef _Belle2_RunControlMasterCallback_h
#define _Belle2_RunControlMasterCallback_h

#include "daq/slc/runcontrol/RCCallback.h"

#include "daq/slc/database/RunNumberInfoTable.h"
#include "daq/slc/runcontrol/RunSetting.h"
#include "daq/slc/runcontrol/RunSummary.h"

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMNode.h>

#include <vector>
#include <map>

namespace Belle2 {

  class RunControlMasterCallback : public RCCallback {

    typedef std::vector<NSMNode> NSMNodeList;
    typedef NSMNodeList::iterator NSMNodeIterator;
    typedef std::map<std::string, NSMNode> NSMNodeMap;
    typedef std::vector<NSMData> NSMDataList;

  public:
    RunControlMasterCallback(const NSMNode& node);
    virtual ~RunControlMasterCallback() throw() {}

  public:
    virtual void init() throw();
    virtual void timeout() throw();
    virtual bool send(const NSMMessage msg) throw();
    virtual bool ok() throw();
    virtual bool error() throw();

  public:
    virtual bool boot() throw() { return send(getMessage()); }
    virtual bool load() throw() { return send(getMessage()); }
    virtual bool start() throw() { return send(getMessage()); }
    virtual bool stop() throw() { return send(getMessage()); }
    virtual bool recover() throw() { return send(getMessage()); }
    virtual bool resume() throw() { return send(getMessage()); }
    virtual bool pause() throw() { return send(getMessage()); }
    virtual bool abort() throw() { return send(getMessage()); }
    virtual bool trigft() throw() { return send(getMessage()); }
    virtual bool stateCheck() throw() { return true; }

  protected:
    void prepareRun(NSMMessage& msg) throw();
    void postRun(NSMMessage& msg) throw();
    NSMNodeIterator find(const std::string& nodename) throw();
    NSMNodeIterator synchronize(NSMNode& node) throw();
    virtual bool isManual() { return true; }

  protected:
    RunSetting m_setting;
    NSMNodeList m_node_v;
    NSMDataList m_data_v;
    NSMMessage m_msg_tmp;
    RunNumberInfo m_info;
    NSMData m_data;

  };

}

#endif

