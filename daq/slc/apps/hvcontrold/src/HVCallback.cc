#include "daq/slc/apps/hvcontrold/HVCallback.h"

#include "daq/slc/apps/hvcontrold/HVCommand.h"
#include "daq/slc/apps/hvcontrold/HVState.h"

#include "daq/slc/nsm/NSMCommunicator.h"

#include "daq/slc/system/TCPSocketWriter.h"
#include "daq/slc/system/TCPSocketReader.h"

#include "daq/slc/base/Debugger.h"
#include "daq/slc/base/ConfigFile.h"

using namespace Belle2;

void HVCallback::getInfo() throw()
{
  TCPSocket socket;
  try {
    ConfigFile config;
    socket.connect(config.get("HV_GLOBAL_HOST"),
                   config.getInt("HV_GLOBAL_PORT"));
    TCPSocketWriter writer(socket);
    writer.writeString(_node->getName() + ":R");
    TCPSocketReader reader(socket);
    m_crate->readInfo(reader);
  } catch (const IOException& e) {
    Belle2::debug("Socket Erorr: %s", e.what());
  }
  socket.close();
}

void HVCallback::sendStatus() throw()
{
  TCPSocket socket;
  try {
    ConfigFile config;
    socket.connect(config.get("HV_GLOBAL_HOST"),
                   config.getInt("HV_GLOBAL_PORT"));
    TCPSocketWriter writer(socket);
    writer.writeString(_node->getName() + ":S");
    m_crate->writeStatus(writer);
  } catch (const IOException& e) {
    Belle2::debug("Socket Erorr: %s", e.what());
  }
  socket.close();
}

void HVCallback::init() throw()
{
  getInfo();
}

bool HVCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  HVCommand cmd(msg.getRequestName());
  setReply("");
  if (cmd == Command::OK) {
    return ok();
  } else if (cmd == Command::ERROR) {
    return error();
  }
  HVState state(_node->getState());
  bool result = false;
  NSMCommunicator* com = getCommunicator();
  if (cmd == HVCommand::LOAD) {
    getInfo();
    result = load();
  } else if (cmd == HVCommand::SWITCHON) {
    result = switchOn();
  } else if (cmd == HVCommand::SWITCHOFF) {
    result = switchOff();
  } else if (cmd == Command::STATECHECK) {
    com->replyOK(_node, "");
    return true;
  }
  if (result) {
    com->replyOK(_node, _reply);
    return true;
  } else {
    _node->setState(State::ERROR_ES);
    com->replyError(_reply);
  }
  return false;
}

HVCallback::HVCallback(NSMNode* node) throw()
  : NSMCallback(node)
{
  m_crate = new HVCrateInfo();
  add(HVCommand::LOAD);
  add(HVCommand::SWITCHON);
  add(HVCommand::SWITCHOFF);
  add(Command::STATECHECK);
}
