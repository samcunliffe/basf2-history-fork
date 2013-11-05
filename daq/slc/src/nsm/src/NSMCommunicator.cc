#include "nsm/NSMCommunicator.h"

#include "nsm/NSMCallback.h"

#include "base/StringUtil.h"
#include "base/Debugger.h"

extern "C" {
#include "nsm/nsmlib2.h"
#include "nsm/belle2nsm.h"
}

#include <iostream>
#include <cstdio>
#include <cstring>

using namespace Belle2;

std::vector<NSMCommunicator*> NSMCommunicator::__com_v;

NSMCommunicator* NSMCommunicator::select(int timeout) throw(NSMHandlerException)
{
  NSMcontext* nsmc = nsmlib_selectc(0, timeout);
  for (size_t i = 0; i < __com_v.size(); i++) {
    NSMCommunicator* com = __com_v[i];
    if (com == NULL) continue;
    if (nsmc == (NSMcontext*) - 1) {
      throw (NSMHandlerException(__FILE__, __LINE__, "Error during select"));
    } else if (nsmc == com->_nsmc) {
      com->_message.read(nsmc);
      b2nsm_context(nsmc);
      return com;
    }
  }
  return NULL;
}


NSMCommunicator::NSMCommunicator(NSMNode* node, const std::string& host, int port) throw()
  : _node(node), _callback(NULL)
{
  _id = -1;
  _nsmc = NULL;
  _host = host;
  _port = port;
}

void NSMCommunicator::init(const std::string& host, int port) throw(NSMHandlerException)
{
  if (_node == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "No node for NSM was registered!"));
  }
  if (host.size() > 0) _host = host;
  if (port > 0) _port = port;
  if (_host.size() == 0 || _port <= 0) {
    _nsmc = b2nsm_init(_node->getName().c_str());
  } else {
    _nsmc = nsmlib_init(_node->getName().c_str(), host.c_str(), port, port);
  }
  if (_nsmc == NULL) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__, "Error during init2"));
  }

  nsmlib_usesig(_nsmc, 0);
  //b2nsm_logging(stdout);
  _id = _nsmc->nodeid;

  b2nsm_context(_nsmc);
  if (_callback != NULL) {
    _callback->setCommunicator(this);
    NSMCallback::NSMRequestList& req_v(_callback->getRequestList());
    for (NSMCallback::NSMRequestList::iterator it = req_v.begin();
         it != req_v.end(); it++) {
      Command& command(it->cmd);
      if ((it->id = b2nsm_callback(command.getLabel(), NULL)) < 0) {
        it->id = -1;
        _id = -1;
        throw (NSMHandlerException(__FILE__, __LINE__,
                                   Belle2::form("Failed to register callback (%s)",
                                                command.getLabel())));
      }
    }
  }
  __com_v.push_back(this);
}

void NSMCommunicator::sendRequest(NSMNode* node, const Command& command,
                                  int npar, unsigned int* pars,
                                  int len, const char* datap) throw(NSMHandlerException)
{
  b2nsm_context(_nsmc);
  if (b2nsm_sendreq_data(node->getName().c_str(), command.getLabel(),
                         npar, (int*)pars, len, datap) < 0) {
    _id = -1;
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to send request"));
  }
}

void NSMCommunicator::sendRequest(NSMNode* node, const Command& cmd,
                                  int npar, unsigned int* pars,
                                  const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, npar, pars, message.size(),
              (message.size() == 0) ? NULL : message.c_str());
}

void NSMCommunicator::sendRequest(NSMNode* node, const Command& cmd,
                                  NSMMessage& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, message.getNParams(), message.getParams(),
              message.getData().size(),
              (message.getData().size() == 0) ? NULL : message.getData().c_str());
}

void NSMCommunicator::sendRequest(NSMNode* node, const Command& cmd,
                                  const std::string& message)
throw(NSMHandlerException)
{
  sendRequest(node, cmd, 0, NULL, message.size(), message.c_str());
}

void NSMCommunicator::replyOK(NSMNode*, const std::string& message)
throw(NSMHandlerException)
{
  b2nsm_context(_nsmc);
  if (b2nsm_ok(_message.getMsg(), _node->getState().getLabel(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply OK"));
  }
}

void NSMCommunicator::replyError(const std::string& message)
throw(NSMHandlerException)
{
  b2nsm_context(_nsmc);
  if (b2nsm_error(_message.getMsg(), message.c_str()) < 0) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Failed to reply error"));
  }
}

bool NSMCommunicator::wait(int sec) throw(NSMHandlerException)
{
  if (_nsmc == NULL) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Not ready for wait"));
  }
  NSMcontext* nsmc = nsmlib_selectc(0, sec);
  if (nsmc == (NSMcontext*) - 1) {
    throw (NSMHandlerException(__FILE__, __LINE__, "Error during wait"));
  } else if (nsmc == _nsmc) {
    _message.read(nsmc);
    b2nsm_context(_nsmc);
    return true;
  } else if (nsmc != NULL) {
    return true;
  }
  return false;
}

bool NSMCommunicator::performCallback() throw(NSMHandlerException)
{
  if (_callback) {
    return _callback->perform(_message);
  }
  return false;
}

int NSMCommunicator::getNodeIdByName(const std::string& name)
throw(NSMHandlerException)
{
  b2nsm_context(_nsmc);
  return b2nsm_nodeid(name.c_str());
}

int NSMCommunicator::getNodePidByName(const std::string& name)
throw(NSMHandlerException)
{
  b2nsm_context(_nsmc);
  return b2nsm_nodepid(name.c_str());
}
