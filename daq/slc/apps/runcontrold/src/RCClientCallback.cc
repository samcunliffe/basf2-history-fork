#include "daq/slc/apps/runcontrold/RCClientCallback.h"

#include "daq/slc/apps/runcontrold/RCMaster.h"
#include "daq/slc/apps/runcontrold/RCCommunicator.h"
#include "daq/slc/apps/runcontrold/RCNSMCommunicator.h"
#include "daq/slc/apps/runcontrold/RCSequencer.h"
#include "daq/slc/apps/runcontrold/RunControlMessage.h"

#include <daq/slc/base/Debugger.h>
#include <daq/slc/base/StringUtil.h>

using namespace Belle2;

RCClientCallback::RCClientCallback(NSMNode* node, RCMaster* master)
  : RCCallback(node), _master(master)
{

}

RCClientCallback::~RCClientCallback() throw()
{

}

void RCClientCallback::init() throw()
{
  RCNSMCommunicator* com = new RCNSMCommunicator(getNode(),
                                                 getCommunicator());
  _master->lock();
  _master->setClientCommunicator(com);
  _master->unlock();
}

bool RCClientCallback::ok() throw()
{
  NSMMessage& nsm(getMessage());
  int id = nsm.getNodeId();
  _master->lock();
  NSMNode* node = _master->findNode(id, nsm);
  if (node == NULL) {
    Belle2::debug("Got OK from unknown node (id=%d) ", id);
    return true;
  }
  node->setState(State(nsm.getData()));
  if (node->getConnection() != Connection::ONLINE) {
    RCCommunicator* comm = _master->getClientCommunicator();
    comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::INFO,
                            Belle2::form("Node %s connected.", node->getName().c_str())));
    node->setConnection(Connection::ONLINE);
  }
  RCCommunicator* comm = _master->getMasterCommunicator();
  //Belle2::debug("[DEBUG] node=%s >> OK", node->getName().c_str());
  RCSequencer::notify();
  bool synchronized = true;
  State state_org = _master->getNode()->getState();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    if ((*it)->isUsed() && (*it)->getState() != node->getState()) {
      synchronized = false;
      break;
    }
  }
  if (synchronized) {
    State& state(node->getState());
    _master->getNode()->setState(state);
    if (state == State::RUNNING_S) {
      comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::INFO,
                              Belle2::form("New run %d was started.", _master->getStatus()->getColdNumber())));
    }
  }
  bool result = (comm != NULL) ? comm->sendState(node) : true;
  if (comm != NULL) comm->sendState(_master->getNode());
  _master->getStatus()->update();
  _master->signal();
  _master->unlock();
  return result;
}

bool RCClientCallback::error() throw()
{
  NSMMessage& nsm(getMessage());
  int id = nsm.getNodeId();
  _master->unlock();
  NSMNode* node = _master->findNode(id, nsm);
  if (node == NULL) {
    Belle2::debug("Got OK from unknown node (id=%d) ", id);
    return true;
  }
  node->setState(State::ERROR_ES);
  node->setConnection(Connection::ONLINE);
  _master->getStatus()->update();
  RCCommunicator* comm = _master->getMasterCommunicator();
  RunControlMessage msg(node, Command::ERROR, nsm);
  bool result = (comm != NULL) ? comm->sendMessage(msg) : true;
  _master->signal();
  _master->unlock();
  Belle2::debug("[ERROR] %s got error (message = %s)", node->getName().c_str(), nsm.getData().c_str());
  return result;
}

void RCClientCallback::selfCheck() throw(NSMHandlerException)
{
  _master->lock();
  RCCommunicator* comm = _master->getClientCommunicator();
  RunControlMessage msg(NULL, Command::STATECHECK);
  RCCommunicator* master_comm = _master->getMasterCommunicator();
  for (RCMaster::NSMNodeList::iterator it = _master->getNSMNodes().begin();
       it != _master->getNSMNodes().end(); it++) {
    NSMNode* node = *it;
    if (!node->isUsed()) continue;
    msg.setNode(node);
    try {
      if (!comm->isOnline(node)) {
        if (node->getConnection() == Connection::ONLINE) {
          node->setState(State::ERROR_ES);
          node->setConnection(Connection::OFFLINE);
          comm->sendLog(SystemLog(getCommunicator()->getNode()->getName(), SystemLog::ERROR,
                                  Belle2::form("Node %s got down.", node->getName().c_str())));
          _master->getNode()->setState(State::ERROR_ES);
          if (master_comm != NULL) {
            master_comm->sendState(node);
            master_comm->sendState(_master->getNode());
          }
        }
      }
      State& state(node->getState());
      if (state != State::RUNNING_S &&
          !state.isTransaction() && state.isRecovering()) {
        if (!comm->sendMessage(msg)) {
          _master->getNode()->setState(State::ERROR_ES);
          if (master_comm != NULL) {
            master_comm->sendState(node);
            master_comm->sendState(_master->getNode());
          }
        }
      }
    } catch (const IOException& e) {
      Belle2::debug("%s:%d %s", __FILE__, __LINE__, e.what());
      _master->getNode()->setState(State::ERROR_ES);
      if (master_comm != NULL)
        master_comm->sendState(_master->getNode());
      _master->unlock();
      return;
    }
  }
  if (master_comm != NULL) {
    master_comm->sendState(_master->getNode());
  }
  _master->unlock();
}
