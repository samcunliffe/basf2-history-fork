#include "daq/slc/runcontrol/RCCallback.h"

#include <daq/slc/database/DBInterface.h>
#include <daq/slc/database/DBObjectLoader.h>

#include <daq/slc/system/LogFile.h>
#include <daq/slc/system/TCPSocket.h>
#include <daq/slc/system/TCPSocketReader.h>
#include <daq/slc/system/TCPSocketWriter.h>

#include <daq/slc/nsm/NSMCommunicator.h>

#include <cstring>
#include <cstdlib>

using namespace Belle2;

RCCallback::RCCallback(int timeout) throw()
  : NSMCallback(timeout)
{
  reg(RCCommand::CONFIGURE);
  reg(RCCommand::LOAD);
  reg(RCCommand::START);
  reg(RCCommand::STOP);
  reg(RCCommand::RECOVER);
  reg(RCCommand::RESUME);
  reg(RCCommand::PAUSE);
  reg(RCCommand::ABORT);
  m_auto = true;
  m_db = NULL;
  m_showall = true;
}

void RCCallback::init(NSMCommunicator&) throw()
{
  NSMNode& node(getNode());
  node.setState(RCState::NOTREADY_S);
  reset();
  add(new NSMVHandlerText("rcstate", true, false, node.getState().getLabel()));
  add(new NSMVHandlerText("rcrequest", true, false, ""));
  if (m_table.size() == 0 || m_rcconfig.size() == 0) {
    //LogFile::notice("dbtable or rcconfig is empty (dbtable='%s', runtype='%s')",
    //        m_table.c_str(), m_rcconfig.c_str());
  } else {
    const std::string config = node.getName() + "@RC:" + m_rcconfig;
    if (getDB()) {
      DBInterface& db(*getDB());
      m_obj = DBObjectLoader::load(db, m_table, config, m_showall);
      db.close();
      m_obj.print(m_showall);
    } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
      TCPSocket socket(m_provider_host, m_provider_port);
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeString(m_table + "/" + config);
        TCPSocketReader reader(socket);
        m_obj.readObject(reader);
        m_obj.print(m_showall);
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
      socket.close();
    } else {
      //m_obj = DBObjectLoader::load("dbconf/" + m_table + "/" + config);
    }
  }
  add(new NSMVHandlerText("rcconfig", true, false, m_obj.getName()));
  add(m_obj);
  try {
    initialize(m_obj);
  } catch (const RCHandlerException& e) {
    LogFile::fatal("Failed to initialize. %s. terminating process", e.what());
    term();
    exit(1);
  }
}

bool RCCallback::perform(NSMCommunicator& com) throw()
{
  NSMMessage msg(com.getMessage());
  const RCCommand cmd = msg.getRequestName();
  RCState state(getNode().getState());
  if (cmd == NSMCommand::VSET &&
      (state != RCState::NOTREADY_S && state != RCState::READY_S)) {
    return true;
  }
  if (NSMCallback::perform(com)) return true;
  if (cmd.isAvailable(state) ==  NSMCommand::DISABLED) {
    return true;
  }
  LogFile::debug("%s >> %s (state = %s)", msg.getNodeName(),
                 cmd.getLabel(), state.getLabel());
  addNode(NSMNode(msg.getNodeName()));
  try {
    set("rcrequest", msg.getRequestName());
  } catch (const std::exception& e) {
    LogFile::error(e.what());
  }
  RCState tstate(cmd.nextTState());
  try {
    if (tstate != Enum::UNKNOWN) {
      setState(tstate);
      if (cmd == RCCommand::CONFIGURE) {
        try {
          abort();
          dbload(com);
        } catch (const IOException& e) {
          throw (RCHandlerException(e.what()));
        }
        set("rcconfig", m_obj.getName());
        configure(m_obj);
        setState(state);
        reply(NSMMessage(NSMCommand::OK, state.getLabel()));
      } else if (cmd == RCCommand::LOAD) {
        get(m_obj);
        load(m_obj);
      } else if (cmd == RCCommand::START) {
        int expno = (msg.getNParams() > 0) ? msg.getParam(0) : 0;
        int runno = (msg.getNParams() > 1) ? msg.getParam(1) : 0;
        start(expno, runno);
      } else if (cmd == RCCommand::STOP) {
        stop();
      } else if (cmd == RCCommand::RECOVER) {
        recover(m_obj);
      } else if (cmd == RCCommand::RESUME) {
        resume(msg.getParam(0));
      } else if (cmd == RCCommand::PAUSE) {
        pause();
      } else if (cmd == RCCommand::ABORT) {
        abort();
      }
    }
    RCState state = cmd.nextState();
    if (state != Enum::UNKNOWN && m_auto) {
      setState(state);
      reply(NSMMessage(NSMCommand::OK, state.getLabel()));
    }
  } catch (const RCHandlerException& e) {
    std::string emsg = StringUtil::form("%s. aborting", e.what());
    LogFile::error(emsg);
    reply(NSMMessage(NSMCommand::ERROR, emsg));
    try {
      setState(RCState::ABORTING_RS);
      abort();
      setState(RCState::NOTREADY_S);
    } catch (const RCHandlerException& e) {
      std::string emsg = StringUtil::form("Failed to abort : %s", e.what());
      LogFile::fatal(emsg);
    }
  } catch (const std::exception& e) {
    LogFile::fatal("Unknown exception: %s. terminating process", e.what());
  }
  return true;
}

void RCCallback::setState(const RCState& state) throw()
{
  if (getNode().getState() != state) {
    LogFile::debug("state transit : %s >> %s",
                   getNode().getState().getLabel(), state.getLabel());
    getNode().setState(state);
    try {
      set("rcstate", state.getLabel());
    } catch (const std::exception& e) {
      LogFile::error(e.what());
    }
  }
}

void RCCallback::dbload(NSMCommunicator& com)
throw(IOException)
{
  remove(m_obj);
  const NSMMessage& msg(com.getMessage());
  if (msg.getLength() > 0 && strlen(msg.getData()) > 0) {
    StringList s = StringUtil::split(msg.getData(), '/');
    if (s.size() == 0) {
      throw (DBHandlerException("Bad config name was selected : %s", msg.getData()));
    }
    const std::string table = (s.size() > 1) ? s[0] : m_table;
    std::string config = (s.size() > 1) ? s[1] : s[0];
    if (table.size() == 0) {
      throw (DBHandlerException("Empty DB table name"));
    }
    if (config.size() == 0) {
      throw (DBHandlerException("Empty config name"));
    }
    if (!StringUtil::find(config, "@")) {
      config = getNode().getName() + "@" + config;
    }
    if (getDB()) {
      DBInterface& db(*getDB());
      try {
        m_obj = DBObjectLoader::load(db, table, config, m_showall);
        db.close();
        m_obj.print(m_showall);
      } catch (const DBHandlerException& e) {
        db.close();
        throw (e);
      }
    } else if (m_provider_host.size() > 0 && m_provider_port > 0) {
      TCPSocket socket(m_provider_host, m_provider_port);
      try {
        socket.connect();
        TCPSocketWriter writer(socket);
        writer.writeString(table + "/" + config);
        TCPSocketReader reader(socket);
        m_obj.readObject(reader);
      } catch (const IOException& e) {
        socket.close();
        throw (IOException("Socket connection error : %s ", e.what()));
      }
      socket.close();
    }
    m_obj.print(m_showall);
  } else {
    LogFile::warning("No DB objects was loaded");
  }
  add(m_obj);
}
