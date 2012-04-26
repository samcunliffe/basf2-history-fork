#ifndef EVTSOCKET_H
#define EVTSOCKET_H
//+
// File : EvtSocket.h
// Description : Socket I/O interface for EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Aug - 2011
//-

#include <string>

#include <daq/dataflow/SocketLib.h>
#include <framework/pcore/EvtMessage.h>

#define MAXEVTSIZE 8000000

using namespace Belle2;

class EvtSocketSend {
public:
  EvtSocketSend(std::string hostname, int port);
  ~EvtSocketSend();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

private:
  SocketSend* m_sock;
  char* m_recbuf;
};

class EvtSocketRecv {
public:
  EvtSocketRecv(int port);
  ~EvtSocketRecv();

  int status();

  int send(EvtMessage* msg);
  EvtMessage* recv(void);

private:
  SocketRecv* m_sock;
  char* m_recbuf;
};
#endif


