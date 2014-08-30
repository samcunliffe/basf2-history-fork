#include "daq/slc/apps/rocontrold/ROCallback.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/readout/ronode_info.h"
#include "daq/slc/readout/ronode_status.h"

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/Time.h>

#include <daq/slc/base/StringUtil.h>
#include <daq/slc/base/ConfigFile.h>

#include <cstring>

using namespace Belle2;

ROCallback::ROCallback(const NSMNode& node)
  : RCCallback(node), m_con(this)
{
  system("killall basf2");
}

ROCallback::~ROCallback() throw()
{
}

void ROCallback::init() throw()
{
  m_con.init("ropcbasf2_" + getNode().getName(), 1);
  m_data = NSMData(getNode().getName() + "_STATUS",
                   "ronode_status",
                   ronode_status_revision);
  m_data.allocate(getCommunicator());
}

void ROCallback::term() throw()
{
  m_con.abort();
  m_con.getInfo().unlink();
}

bool ROCallback::load() throw()
{
  if (m_con.isAlive()) return true;
  const DBObject& obj(getConfig().getObject());
  m_con.setExecutable("basf2");
  m_con.clearArguments();
  m_con.addArgument(StringUtil::form("%s/%s", getenv("BELLE2_LOCAL_DIR"),
                                     obj.getText("ropc_script").c_str()));
  m_con.addArgument("1");
  m_con.addArgument(StringUtil::form("%d", obj.getInt("port_from")));
  m_con.addArgument("ropcbasf2_" + getNode().getName());
  if (m_con.load(10)) {
    LogFile::debug("load succeded");
    m_flow.open(&(m_con.getInfo()));
    return true;
  } else {
    LogFile::error("load timeout");
  }
  return false;
}

bool ROCallback::start() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = Time().getSecond();
  return m_con.start();
}

bool ROCallback::stop() throw()
{
  ronode_status* status = (ronode_status*)m_data.get();
  status->stime = 0;
  return true;
}

bool ROCallback::resume() throw()
{
  return true;
}

bool ROCallback::pause() throw()
{
  return true;
}

bool ROCallback::recover() throw()
{
  if (abort() && load()) {
    getNode().setState(RCState::READY_S);
    return true;
  }
  return false;
}

bool ROCallback::abort() throw()
{
  m_con.abort();
  getNode().setState(RCState::NOTREADY_S);
  return true;
}

void ROCallback::timeout() throw()
{
  if (m_data.isAvailable()) {
    ronode_status* nsm = (ronode_status*)m_data.get();
    if (m_flow.isAvailable()) {
      ronode_status& status(m_flow.monitor());
      uint32 stime = nsm->stime;
      memcpy(nsm, &status, sizeof(ronode_status));
      nsm->stime = stime;
    }
    double loads[3];
    if (getloadavg(loads, 3) > 0) {
      nsm->loadavg = (float)loads[0];
    } else {
      nsm->loadavg = -1;
    }
  }
  int eflag = m_con.getInfo().getErrorFlag();
  if (eflag > 0) {
    if (eflag == RunInfoBuffer::PROCESS_DOWN) {
      if (getNode().getState() == RCState::RUNNING_S) {
        abort();
        NSMCommunicator& com(*getCommunicator());
        if (load()) {
          start();
          getNode().setState(RCState::RUNNING_S);
          com.replyOK(getNode());
        } else {
          com.replyError(RunInfoBuffer::PROCESS_DOWN,
                         "Process recover failed " +
                         m_con.getExecutable());
        }
      }
    }
  }
}

