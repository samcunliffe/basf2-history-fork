#pragma once

#include <daq/slc/runcontrol/RCCallback.h>

#include <daq/rfarm/manager/RFConf.h>
#include <daq/rfarm/manager/RFProcessManager.h>
#include <daq/rfarm/manager/RFSharedMem.h>
#include <daq/rfarm/manager/RFLogManager.h>
#include <atomic>

namespace Belle2 {

  class RoiSenderCallback : public RCCallback {

  public:
    RoiSenderCallback();

  private:
    /** overloaded functions from base class */
    void load(const DBObject&, const std::string&) override;
    void start(int /*expno*/, int /*runno*/) override;
    void stop(void) override;
    void recover(const DBObject&, const std::string&) override;
    void abort(void) override;

  public:
    void server();

  private:
    RFConf* m_conf{};
    RFProcessManager* m_proc{};
    RFLogManager* m_log{};
    char* m_nodename{};
    RFSharedMem* m_shm{};
    std::atomic_int m_pid_merger{};
    pthread_t m_logthread{};

  };

}

