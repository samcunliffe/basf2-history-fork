#include "daq/slc/base/Connection.h"

using namespace Belle2;

const Connection Connection::ONLINE(10000, "ONLINE");
const Connection Connection::OFFLINE(20000, "OFFLINE");

Connection::Connection() throw() : Enum()
{

}

Connection::Connection(const Enum& st) throw()
  : Enum(st.getId(), st.getLabel())
{

}

Connection::Connection(const Connection& st) throw()
  : Enum(st._id, st._label)
{

}

Connection::Connection(int id, const char* label) throw()
  : Enum(id, label)
{

}

Connection::~Connection() throw()
{

}

const Connection& Connection::operator=(const std::string& msg) throw()
{
  if (msg == ONLINE._label) *this = ONLINE;
  else if (msg == OFFLINE._label) *this = OFFLINE;
  else *this = UNKNOWN;
  return *this;
}

const Connection& Connection::operator=(const char* msg) throw()
{
  if (msg != NULL)  *this = std::string(msg);
  else *this = UNKNOWN;
  return *this;
}

const Connection& Connection::operator=(int i) throw()
{
  if (i == ONLINE._id) *this = ONLINE;
  else if (i == OFFLINE._id) *this = OFFLINE;
  else *this = UNKNOWN;
  return *this;
}

