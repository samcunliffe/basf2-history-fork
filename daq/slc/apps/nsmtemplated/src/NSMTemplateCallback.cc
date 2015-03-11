#include "daq/slc/apps/nsmtemplated/NSMTemplateCallback.h"

#include <daq/slc/system/LogFile.h>

#include <stdlib.h>

namespace Belle2 {
  class NSMTemplateVHandler : public NSMVHandlerText {
  public:
    NSMTemplateVHandler(const std::string& name, const std::string& val)
      : NSMVHandlerText(name, true, true, val) {}
    bool handleGetText(std::string& val) {
      NSMVHandlerText::handleGetText(val);
      LogFile::info("%s is read : %s", getName().c_str(), val.c_str());
      return true;
    }
    bool handleSetText(const std::string& val) {
      LogFile::info("%s is written : %s", getName().c_str(), val.c_str());
      return NSMVHandlerText::handleSetText(val);
    }
  };
}

using namespace Belle2;

NSMTemplateCallback::NSMTemplateCallback(const std::string& name, int timeout)
{
  LogFile::debug("NSM nodename = %s (timeout: %d seconds)", name.c_str(), timeout);
  setNode(NSMNode(name));
  setTimeout(timeout);
}

NSMTemplateCallback::~NSMTemplateCallback() throw()
{
}

void NSMTemplateCallback::init(NSMCommunicator&) throw()
{
  add(new NSMVHandlerInt("ival", true, true, 10));
  add(new NSMVHandlerFloat("fval", true, true, 0.1));
  add(new NSMTemplateVHandler("tval", "example"));
}

void NSMTemplateCallback::timeout(NSMCommunicator&) throw()
{
  int ival = rand() % 256;
  set("ival", ival);
  LogFile::debug("ival updated: %d", ival);
}

