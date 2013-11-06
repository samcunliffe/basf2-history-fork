#ifndef _Belle2_NSMCommunicator_hh
#define _Belle2_NSMCommunicator_hh

#include "nsm/NSMHandlerException.h"
#include "nsm/NSMMessage.h"

#include "base/NSMNode.h"
#include "base/Command.h"
#include "base/LogMessage.h"

extern "C" {
#include "nsm/nsm2.h"
}

#include <vector>
#include <map>

namespace Belle2 {

  class NSMCallback;
  class NSMMessage;

  class NSMCommunicator {

  public:
    static NSMCommunicator* select(int timeout) throw(NSMHandlerException);

  private:
    static std::vector<NSMCommunicator*> __com_v;

  public:
    NSMCommunicator(NSMNode* node, const std::string& host = "",
                    int port = -1, const std::string& config_name = "slc_config") throw();
    ~NSMCommunicator() throw() {}

  public:
    void init(const std::string& host = "", int port = -1) throw(NSMHandlerException);
    bool wait(int sec) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     int npar = 0, unsigned int* pars = NULL,
                     int len = 0, const char* datap = NULL) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     int npar, unsigned int* pars,
                     const std::string& message) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     NSMMessage& message) throw(NSMHandlerException);
    void sendRequest(NSMNode* node, const Command& cmd,
                     const std::string& message) throw(NSMHandlerException);
    void replyOK(NSMNode* node, const std::string& message = "") throw(NSMHandlerException);
    void replyError(const std::string& message = "") throw(NSMHandlerException);
    void sendLog(const LogMessage& log) throw(NSMHandlerException);

  public:
    int getId() const throw() { return _id; }
    bool isOnline() const throw() { return (_id >= 0); }
    NSMNode* getNode() throw() { return _node; }
    NSMCallback* getCallback() throw() { return _callback; }
    NSMMessage& getMessage() throw() { return _message; }
    void setId(int id) throw() { _id = id; }
    void setNode(NSMNode* node) throw() { _node = node; }
    void setCallback(NSMCallback* callback) throw() { _callback = callback; }
    bool performCallback() throw(NSMHandlerException);
    int getNodeIdByName(const std::string& name) throw(NSMHandlerException);
    int getNodePidByName(const std::string& name) throw(NSMHandlerException);

  private:
    std::string _config_name;
    NSMNode* _node;
    NSMCallback* _callback;
    int _id;
    NSMMessage _message;
    NSMcontext* _nsmc;
    std::string _host;
    int _port;
    NSMNode* _logger_node;

  };

};

#endif
