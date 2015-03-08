#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/database/DBObjectLoader.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>

#include <cstring>
#include <sstream>
#include <iostream>

#include <unistd.h>

namespace Belle2 {

  class NSMVHandlerROPID : public NSMVHandlerInt {
  public:
    NSMVHandlerROPID(ROController& con, const std::string& name)
      : NSMVHandlerInt(name, true, false), m_con(con) {}
    virtual ~NSMVHandlerROPID() throw() {}
    bool handleGetInt(int& val) {
      val = m_con.getControl().getFork().get_id();
      return true;
    }
  private:
    ROController& m_con;
  };

}

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& runcontrol)
  : RCCallback(), m_runcontrol(runcontrol)
{
  system("killall basf2");
}

bool ROCallback::initialize(const DBObject& obj) throw()
{
  allocData(getNode().getName() + "_STATUS", "ronode_status",
            ronode_status_revision);
  return configure(obj);
}

bool ROCallback::configure(const DBObject& obj) throw()
{
  try {
    const DBObject& stream0(obj("stream0"));
    const DBObjectList& senders(stream0.getObjects("sender"));
    m_eb0.init(this, 0, "eb0", obj);
    add(new NSMVHandlerROPID(m_eb0, "eb0.pid"));
    m_stream1.init(this, 1, "stream1", obj);
    add(new NSMVHandlerROPID(m_stream1, "stream1.pid"));
    m_stream0 = std::vector<Stream0Controller>();
    for (size_t i = 0; i < senders.size(); i++) {
      m_stream0.push_back(Stream0Controller());
    }
    for (size_t i = 0; i < m_stream0.size(); i++) {
      std::string name = StringUtil::form("stream0_%d", (int)i);
      m_stream0[i].init(this, i + 2, name, obj);
      add(new NSMVHandlerROPID(m_stream0[i], StringUtil::form("stream0.sender[%d].pid", (int)i)));
    }
    return true;
  } catch (const std::out_of_range& e) {
    LogFile::error(e.what());
  }
  return false;
}

void ROCallback::term() throw()
{
  m_stream1.term();
  m_eb0.term();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[0].term();
  }
}

void ROCallback::load(const DBObject&) throw(RCHandlerException)
{
  if (!m_eb0.load(0)) {
    throw (RCHandlerException("Failed to boot eb0"));
  }
  LogFile::debug("Booted eb0");
  for (size_t i = 0; i < m_stream0.size(); i++) {
    if (!m_stream0[i].load(10)) {
      throw (RCHandlerException("Faield to boot stream0-%d", (int)i));
    }
    LogFile::debug("Booted %d-th stream0", i);
  }
  if (!m_stream1.load(10)) {
    throw (RCHandlerException("Faield to boot stream1"));
  }
  LogFile::debug("Booted stream1");
}

void ROCallback::start(int expno, int runno) throw(RCHandlerException)
{
  try {
    m_eb0.start(expno, runno);
  } catch (const RCHandlerException& e) {
    LogFile::warning("eb0 did not start : %s", e.what());
  }
  for (size_t i = 0; i < m_stream0.size(); i++) {
    if (!m_stream0[i].start(expno, runno)) {
      throw (RCHandlerException("Faield to start stream0-%d", (int)i));
      return;
    }
  }
  if (!m_stream1.start(expno, runno)) {
    throw (RCHandlerException("Faield to start stream1"));
  }
}

void ROCallback::stop() throw(RCHandlerException)
{
}

void ROCallback::recover() throw(RCHandlerException)
{
  abort();
}

void ROCallback::abort() throw(RCHandlerException)
{
  m_stream1.abort();
  for (size_t i = 0; i < m_stream0.size(); i++) {
    m_stream0[i].abort();
  }
  m_eb0.abort();
}

void ROCallback::timeout(NSMCommunicator&) throw()
{
  if (getNode().getState() != RCState::RUNNING_S) {
    for (size_t i = 0; i < m_stream0.size(); i++) {
      m_stream0[i].check();
    }
  }
  NSMData& data(getData());
  if (data.isAvailable()) {
    ronode_status* nsm = (ronode_status*)data.get();
    if (m_stream1.getFlow().isAvailable()) {
      ronode_status& status(m_stream1.getFlow().monitor());
      memcpy(nsm, &status, sizeof(ronode_status));
    }
    double loads[3];
    if (getloadavg(loads, 3) > 0) {
      nsm->loadavg = (float)loads[0];
    } else {
      nsm->loadavg = -1;
    }
    data.flush();
  }
}

