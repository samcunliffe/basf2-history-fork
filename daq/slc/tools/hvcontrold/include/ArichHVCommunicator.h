#ifndef Belle2_ArichHVCommunicator_h
#define Belle2_ArichHVCommunicator_h

#include "ArichHVMessage.h"
#include "ArichHVCallback.h"

#include <nsm/NSMData.h>

#include <system/Mutex.h>
#include <system/TCPSocket.h>
#include <system/TCPSocketWriter.h>
#include <system/TCPSocketReader.h>

namespace Belle2 {

  class ArichHVCommunicator {

  public:
    ArichHVCommunicator(ArichHVCallback* callback,
                        const std::string& host,
                        int port, NSMData* data)
      : _crate(callback->getCrate()), _host(host),
        _port(port), _data(data) {
      callback->setHVCommunicator(this);
    }
    ~ArichHVCommunicator() throw() {}

  public:
    std::string sendRequest(ArichHVMessage& msg)
    throw(IOException);
    void run();

  private:
    HVCrateInfo* _crate;
    std::string _host;
    int _port;
    NSMData* _data;
    bool _available;
    Mutex _mutex;
    TCPSocket _socket;
    TCPSocketWriter _writer;
    TCPSocketReader _reader;

  };

}

#endif
