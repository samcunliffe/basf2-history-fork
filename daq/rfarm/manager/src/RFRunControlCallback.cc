#include "daq/rfarm/manager/RFRunControlCallback.h"

#include "daq/rfarm/manager/RFCommand.h"
#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFMasterCallback.h"

#include <daq/slc/nsm/NSMCommunicator.h>
#include <daq/slc/base/StringUtil.h>

#include <daq/slc/system/LogFile.h>

#include <unistd.h>

using namespace Belle2;

RFRunControlCallback::RFRunControlCallback(const NSMNode& node,
                                           RFMaster* master,
                                           RFMasterCallback* callback)
  : RCCallback(node), m_master(master), m_callback(callback)
{
  callback->setCallback(this);
}

RFRunControlCallback::~RFRunControlCallback() throw()
{

}

void RFRunControlCallback::init() throw()
{
  m_data = NSMData(getNode().getName() + "_STATUS", "rfunitinfo", 2);
  m_data.allocate(getCommunicator());
}

bool RFRunControlCallback::load() throw()
{
  NSMMessage& msg(getMessage());
  msg.setRequestName(RFCommand::RF_CONFIGURE);
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}

bool RFRunControlCallback::start() throw()
{
  NSMMessage& msg(getMessage());
  msg.setRequestName(RFCommand::RF_START);
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}

bool RFRunControlCallback::stop() throw()
{
  NSMMessage& msg(getMessage());
  msg.setRequestName(RFCommand::RF_STOP);
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}

bool RFRunControlCallback::recover() throw()
{
  NSMMessage& msg(getMessage());
  msg.setRequestName(RFCommand::RF_RESTART);
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}

bool RFRunControlCallback::abort() throw()
{
  NSMMessage& msg(getMessage());
  msg.setRequestName(RFCommand::RF_UNCONFIGURE);
  m_callback->setMessage(msg);
  return m_callback->perform(msg);
}
