//+
// File : EvtSocket.cc
// Description : Socket interface to transver EvtMessage
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 5 - Aug - 2011
//-

#include "dqm/EvtSocket.h"

using namespace Belle2;

// EvtSocketSend

EvtSocketSend::EvtSocketSend(std::string host, int port)
{
  m_sock = new SocketSend(host.c_str(), (u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
}

EvtSocketSend::EvtSocketSend(const EvtSocketSend& es)
{
  m_sock = new SocketSend((es.m_sock)->node(), (u_short)(es.m_sock)->port());
  m_recbuf = new char[MAXEVTSIZE];
}

EvtSocketSend::~EvtSocketSend()
{
  delete m_sock;
  delete[] m_recbuf;
}

EvtSocketSend& EvtSocketSend::operator=(EvtSocketSend& other)
{
  if (this != &other) {
    if (m_sock != NULL) delete m_sock;
    if (m_recbuf != NULL) delete m_recbuf;
    m_sock = new SocketSend((other.m_sock)->node(), (u_short)(other.m_sock)->port());
    m_recbuf = new char[MAXEVTSIZE];
  }
  return *this;
}

int EvtSocketSend::send(EvtMessage* msg)
{
  //  printf ( "EvtSocketSend : sending = %d\n", msg->size() );
  return m_sock->put((char*)msg->buffer(), msg->size());
}

EvtMessage* EvtSocketSend::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int EvtSocketSend::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int EvtSocketSend::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}

int EvtSocketSend::recv_pxd_buffer(char* buf)
{
  int stat = m_sock->get_pxd(buf, MAXEVTSIZE);
  return stat;
}

SocketSend* EvtSocketSend::sock(void)
{
  return m_sock;
}


// EvtSocketRecv

EvtSocketRecv::EvtSocketRecv(int port, bool accept_at_init)
{
  m_sock = new SocketRecv((u_short)port);
  m_recbuf = new char[MAXEVTSIZE];
  if (accept_at_init)
    m_sock->accept();
}

EvtSocketRecv::~EvtSocketRecv()
{
  delete m_sock;
  delete[] m_recbuf;
}

EvtMessage* EvtSocketRecv::recv()
{
  int stat = m_sock->get(m_recbuf, MAXEVTSIZE);
  if (stat <= 0) return NULL;
  EvtMessage* evt = new EvtMessage(m_recbuf);
  //  delete [] evtbuf;
  return evt;
}

int EvtSocketRecv::send(EvtMessage* msg)
{
  return m_sock->put((char*)msg->buffer(), msg->size());
}

int EvtSocketRecv::send_buffer(int nbytes, char* buf)
{
  return m_sock->put(buf, nbytes);
}

int EvtSocketRecv::recv_buffer(char* buf)
{
  int stat = m_sock->get(buf, MAXEVTSIZE);
  return stat;
}

SocketRecv* EvtSocketRecv::sock(void)
{
  return m_sock;
}

