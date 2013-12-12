#ifndef _Belle2_ROCallback_hh
#define _Belle2_ROCallback_hh

#include "daq/slc/readout/ProcessController.h"

#include "daq/slc/nsm/RCCallback.h"

namespace Belle2 {

  class ROCallback : public RCCallback {

  public:
    ROCallback(NSMNode* node);
    virtual ~ROCallback() throw();

  public:
    virtual void init() throw();
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool resume() throw();
    virtual bool pause() throw();
    virtual bool recover() throw();
    virtual bool abort() throw();

  private:
    ProcessController _con;

  };

}

#endif
