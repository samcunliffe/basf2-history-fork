#ifndef _B2DAQ_COPPERCallback_hh
#define _B2DAQ_COPPERCallback_hh

#include <node/COPPERNode.hh>

#include <system/Fork.hh>

#include <runcontrol/RCCallback.hh>

#include <nsm/RunStatus.hh>

#include "HSLBController.hh"

namespace B2DAQ {

  class COPPERCallback : public RCCallback {

  public:
    COPPERCallback(COPPERNode* node = NULL, 
		   NSMData* data = NULL);
    virtual ~COPPERCallback() throw();

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

  private:
    int* openBuffer(size_t count, const char* path) throw();

  private:
    COPPERNode* _node;
    NSMData* _data;
    int* _buf_config;
    int* _buf_status;
    RunStatus* _status;
    HSLBController _hslbcon_v[4];
    Fork _fork;
    
  };

}

#endif
