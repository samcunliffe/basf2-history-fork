// +
// File : EvtMessage.cc
// Description : Low level interface to transfer serialized object
//
// Author : Soohyung Lee and Ryosuke Itoh
// Date : 24 - Jul - 2008
// Modified : 9 - Jun - 2010
//-

#include <framework/pcore/EvtMessage.h>

#include <cstdlib>
#include <string>

using namespace std;
using namespace Belle2;

EvtMessage::EvtMessage(char* data):
  m_data(data),
  m_ownsBuffer(false)
{
}


/// @brief Constructor of EvtMessage allocating new buffer
/// @param msg  data
/// @param size Length of the data (TMessage)
EvtMessage::EvtMessage(const char* sobjs, int size, RECORD_TYPE type = MSG_EVENT)
{
  m_data = new char[size + sizeof(EvtHeader)]; // Allocate new buffer
  msg(sobjs, size, type);
  m_ownsBuffer = true;
}

/// @brief Copy constructor of EvtMessage class
/// @param evtmsg Original EvtMessage object
EvtMessage::EvtMessage(const EvtMessage& evtmsg)
{
  *this = evtmsg;
}

/// @brief Destructor of EvtMessage class
EvtMessage::~EvtMessage(void)
{
  if (m_ownsBuffer)
    delete [] m_data;
}

/// @brief Overridden assign operator
/// @param obj Source object
EvtMessage& EvtMessage::operator=(const EvtMessage& obj)
{
  if (this != &obj) {
    if (m_ownsBuffer)
      delete [] m_data;
    buffer(obj.m_data); //copy m_data (m_ownsBuffer is set)
  }

  return *this;
}

/// @brief Get buffer
/// @return Buffer address

char* EvtMessage::buffer(void)
{
  return m_data;
}

// @brief Set buffer
// @param Existing buffer address

void EvtMessage::buffer(const char* bufadr)
{
  int size = *(int*)bufadr;
  m_data = new char[size];
  memcpy(m_data, bufadr, size);
  m_ownsBuffer = true;
}

// @brief size
// @return record size
int EvtMessage::size(void)
{
  return (((EvtHeader*)m_data)->size);
}

// @brief msgsize
// @return message size
int EvtMessage::msg_size(void)
{
  return (((EvtHeader*)m_data)->size - sizeof(EvtHeader));
}

// Record type
RECORD_TYPE EvtMessage::type(void)
{
  return (((EvtHeader*)m_data)->rectype);
}

void EvtMessage::type(RECORD_TYPE type)
{
  ((EvtHeader*)m_data)->rectype = type;
}

// Source of this record
int EvtMessage::src(void)
{
  return (((EvtHeader*)m_data)->src);
}

void EvtMessage::src(int src)
{
  ((EvtHeader*)m_data)->src = src;
}

// Destination of this record
int EvtMessage::dest(void)
{
  return (((EvtHeader*)m_data)->dest);
}

void EvtMessage::dest(int dest)
{
  ((EvtHeader*)m_data)->dest = dest;
}

// Time stamp
struct timeval EvtMessage::time(void) {
  return (((EvtHeader*)m_data)->timestamp);
}

void EvtMessage::time(struct timeval& tbuf)
{
  ((EvtHeader*)m_data)->timestamp = tbuf;
}

// Event Header
EvtHeader* EvtMessage::header(void)
{
  return ((EvtHeader*)m_data);
}

// Message body
char* EvtMessage::msg(void)
{
  return (m_data + sizeof(EvtHeader));
}

// set_message

void EvtMessage::msg(char const* msgin, int size, RECORD_TYPE type)
{
  EvtHeader* hdr = (EvtHeader*)m_data;
  hdr->size = size + sizeof(EvtHeader);
  hdr->rectype = type;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  hdr->timestamp = tv;
  hdr->src = -1;
  hdr->dest = -1;
  if (size > 0)
    memcpy(m_data + sizeof(EvtHeader), msgin, size);
}


