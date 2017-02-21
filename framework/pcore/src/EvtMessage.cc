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
#include <cstring>

#include <sys/time.h>

using namespace std;
using namespace Belle2;

namespace {
  /** Returns smallest value >= bytes that is divisible by sizeof(int). */
  int roundToNearestInt(int bytes)
  {
    return sizeof(int) * ((bytes + sizeof(int) - 1) / sizeof(int));
  }
}

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
  int fullsize = size + sizeof(EvtHeader);
  int bufsize = roundToNearestInt(fullsize);
  m_data = new char[bufsize]; // Allocate new buffer
  setMsg(sobjs, size, type);
  for (int i = bufsize - fullsize; i > 0; i--)
    m_data[bufsize - i] = '\0'; //zero extra bytes
  m_ownsBuffer = true;
}

/// @brief Copy constructor of EvtMessage class
/// @param evtmsg Original EvtMessage object
EvtMessage::EvtMessage(const EvtMessage& evtmsg)
{
  *this = evtmsg;
}

/// @brief Destructor of EvtMessage class
EvtMessage::~EvtMessage()
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
    buffer(obj.m_data); //copy m_data
    m_ownsBuffer = true; //already set by buffer(), but let's be explicit
  }

  return *this;
}

char* EvtMessage::buffer()
{
  return m_data;
}

void EvtMessage::buffer(const char* bufadr)
{
  UInt_t size = *(UInt_t*)bufadr;
  int bufsize = roundToNearestInt(size);
  m_data = new char[bufsize];
  memcpy(m_data, bufadr, size);
  for (int i = bufsize - size; i > 0; i--)
    m_data[bufsize - i] = '\0'; //zero extra bytes
  m_ownsBuffer = true;
}

// @brief size
// @return record size
int EvtMessage::size() const
{
  return ((reinterpret_cast<EvtHeader*>(m_data))->size);
}

int EvtMessage::paddedSize() const
{
  const int sizeBytes = size();
  //round up to next int boundary
  return roundToNearestInt(sizeBytes) / sizeof(int);
}

// @brief msgsize
// @return message size
int EvtMessage::msg_size() const
{
  return ((reinterpret_cast<EvtHeader*>(m_data))->size - sizeof(EvtHeader));
}

// Record type
RECORD_TYPE EvtMessage::type() const
{
  return ((reinterpret_cast<EvtHeader*>(m_data))->rectype);
}

void EvtMessage::type(RECORD_TYPE type)
{
  (reinterpret_cast<EvtHeader*>(m_data))->rectype = type;
}

// Source of this record
int EvtMessage::src() const
{
  return ((reinterpret_cast<EvtHeader*>(m_data))->src);
}

void EvtMessage::src(int src)
{
  (reinterpret_cast<EvtHeader*>(m_data))->src = src;
}

// Destination of this record
int EvtMessage::dest() const
{
  return ((reinterpret_cast<EvtHeader*>(m_data))->dest);
}

void EvtMessage::dest(int dest)
{
  (reinterpret_cast<EvtHeader*>(m_data))->dest = dest;
}

// Time stamp
struct timeval EvtMessage::time() const {
  struct timeval tv;
  tv.tv_sec = (reinterpret_cast<EvtHeader*>(m_data))->time_sec;
  tv.tv_usec = (reinterpret_cast<EvtHeader*>(m_data))->time_usec;
  return tv;
}

void EvtMessage::setTime(const struct timeval& tbuf)
{
  (reinterpret_cast<EvtHeader*>(m_data))->time_sec = tbuf.tv_sec;
  (reinterpret_cast<EvtHeader*>(m_data))->time_usec = tbuf.tv_usec;
}

// Event Header
EvtHeader* EvtMessage::header()
{
  return (reinterpret_cast<EvtHeader*>(m_data));
}

// Message body
char* EvtMessage::msg()
{
  return (m_data + sizeof(EvtHeader));
}

// set_message

void EvtMessage::setMsg(char const* msgin, int size, RECORD_TYPE type)
{
  EvtHeader* hdr = header();
  hdr->size = size + sizeof(EvtHeader);
  hdr->rectype = type;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  this->setTime(tv);
  hdr->src = -1;
  hdr->dest = -1;
  //TODO: set durability, nobjs, narrays here?
  if (size > 0)
    memcpy(m_data + sizeof(EvtHeader), msgin, size);
}


