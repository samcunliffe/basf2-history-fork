#ifndef _B2DAQ_TemplateCallback_hh
#define _B2DAQ_TemplateCallback_hh

#include <runcontrol/RCCallback.hh>

namespace B2DAQ {

  class TemplateCallback : public RCCallback {

  public:
    TemplateCallback(NSMNode* node = NULL);
    virtual ~TemplateCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool reboot() throw();
    virtual bool load() throw();
    virtual bool reload() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  };

}

#endif
