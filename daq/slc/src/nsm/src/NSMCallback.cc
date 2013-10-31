#include "NSMCallback.h"

#include "NSMCommunicator.h"
#include "NSMMessage.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>

using namespace Belle2;

NSMCallback::NSMCallback() throw()
{
  add(Command::OK);
  add(Command::ERROR);
}

bool NSMCallback::perform(NSMMessage& msg) throw(NSMHandlerException)
{
  std::string cmd_s(msg.getRequestName());
  if (cmd_s == Command::OK.getLabel()) {
    return ok();
  } else if (cmd_s == Command::ERROR.getLabel()) {
    return error();
  }
  throw (NSMHandlerException(__FILE__, __LINE__, "Wrong request"));
}

NSMMessage& NSMCallback::getMessage()
{
  return _comm->getMessage();
}
