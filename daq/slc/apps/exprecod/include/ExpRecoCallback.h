#ifndef _Belle2_ExpRecoCallback_hh
#define _Belle2_ExpRecoCallback_hh

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#include <daq/slc/nsm/RCCallback.h>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFSharedMem.h>
#include <daq/rfarm/manager/RFLogManager.h>

#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  class ExpRecoCallback : public RCCallback {

  public:
    ExpRecoCallback(NSMNode* node = NULL);
    virtual ~ExpRecoCallback() throw();

  public:
    virtual bool boot() throw();
    virtual bool load() throw();
    virtual bool start() throw();
    virtual bool stop() throw();
    virtual bool recover() throw();
    virtual bool trigft() throw();

    void ExpRecoLogger();

  private:
    RFConf* m_conf;
    RFProcessManager* m_proc;
    RFLogManager* m_log;
    char* m_nodename;
    RFSharedMem* m_shm;
    RingBuffer* m_rbuf;
    int m_pid_basf2;
    int m_pid_receiver;
    int m_pid_evs;
    pthread_t m_logthread;

  };

}

#endif
