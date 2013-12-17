#ifndef ERECO_EVENT_SERVER_H
#define ERECO_EVENT_SERVER_H
//+
// File : ERecoEventServer.h
// Description : Event server to provide sampled events to outside
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 17 - Dec - 2013
//-

#include <string>

#include <daq/dataflow/SocketLib.h>
#include <daq/dataflow/EvtSocket.h>
#include <daq/dataflow/EvtSocketManager.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>

#define MAXBUFSIZE 80000000

namespace Belle2 {

  class ERecoEventServer {
  public:
    ERecoEventServer(std::string rbufname, int port);
    ~ERecoEventServer();

    int server();  // Loop

  private:
    EvtSocketRecv* m_sock;
    EvtSocketManager* m_man;
    RingBuffer* m_rbuf;
    int m_port;
    int m_force_exit;
  };
}
#endif
