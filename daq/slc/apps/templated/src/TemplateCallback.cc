#include "daq/slc/apps/templated/TemplateCallback.h"

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <unistd.h>

using namespace Belle2;

TemplateCallback::TemplateCallback()
{
}

TemplateCallback::~TemplateCallback() throw()
{
}

bool TemplateCallback::initialize(const DBObject& obj) throw()
{
  obj.print();
  return true;
}

bool TemplateCallback::configure(const DBObject& obj) throw()
{
  obj.print();
  return true;
}

void TemplateCallback::timeout(NSMCommunicator&) throw()
{
}

void TemplateCallback::load(const DBObject& obj) throw(RCHandlerException)
{
  obj.print();
}

void TemplateCallback::start(int expno, int runno) throw(RCHandlerException)
{
  LogFile::debug("run # = %04d.%04d.%03d", expno, runno, 0);
}

void TemplateCallback::stop() throw(RCHandlerException)
{
}

void TemplateCallback::resume() throw(RCHandlerException)
{
}

void TemplateCallback::pause() throw(RCHandlerException)
{
}

void TemplateCallback::recover() throw(RCHandlerException)
{
  setState(RCState::READY_S);
}

void TemplateCallback::abort() throw(RCHandlerException)
{
  setState(RCState::NOTREADY_S);
}

