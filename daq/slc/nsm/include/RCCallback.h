#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "daq/slc/nsm/NSMCallback.h"

namespace Belle2 {

  class RCCallback : public NSMCallback {

  public:
    RCCallback(NSMNode* node) throw();
    virtual ~RCCallback() throw() {}

  public:
    void download();

  public:
    virtual bool boot()   throw() { return true; }
    virtual bool load()   throw() { return true; }
    virtual bool start()  throw() { return true; }
    virtual bool stop()   throw() { return true; }
    virtual bool recover() throw() { return true; }
    virtual bool resume() throw() { return true; }
    virtual bool pause()  throw() { return true; }
    virtual bool abort()  throw() { return true; }
    virtual bool trigft()  throw() { return true; }

  public:
    virtual bool perform(const Command& cmd, NSMMessage& msg)
    throw(NSMHandlerException);

  };

};

#endif
