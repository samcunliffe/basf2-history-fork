//+
// File : Rb2Sock.h
// Description : Pick an event from RingBuffer and send it to EvtSocket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 10 - Dec - 2002, Orignal for RFARM
//        26 - Apr - 2012, for Belle II RFARM
//-

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include <vector>
#include <string>

#include "framework/pcore/RingBuffer.h"
#include "daq/dataflow/REvtSocket.h"

#define RBUFSIZE 100000000

class RevRb2Sock {
public:
  /*! Constuctor and Destructor */
  RevRb2Sock(std::string rbuf, int port);
  ~RevRb2Sock(void);

  /*! Event function */
  int SendEvent(void);

private:
  RingBuffer* m_rbuf;
  REvtSocketSend* m_sock;
  char* m_evtbuf;

};


