#ifndef _Belle2_RCCallback_hh
#define _Belle2_RCCallback_hh

#include "daq/slc/runcontrol/RCState.h"
#include "daq/slc/runcontrol/RCCommand.h"
#include "daq/slc/runcontrol/RCConfig.h"

#include <daq/slc/nsm/NSMCallback.h>

#include <daq/slc/system/Mutex.h>
#include <daq/slc/system/Cond.h>
#include <daq/slc/system/PThread.h>

namespace Belle2 {

  class DBInterface;

  class RCCallback : public NSMCallback {

    friend class RCMonitor;

  public:
    RCCallback(const NSMNode& node) throw();
    virtual ~RCCallback() throw() {}

  public:
    const RCConfig& getConfig() const throw() { return m_config; }
    RCConfig& getConfig() throw() { return m_config; }

  public:
    virtual bool load() throw() { return true; }
    virtual bool start() throw() { return true; }
    virtual bool stop() throw() { return true; }
    virtual bool recover() throw() { return true; }
    virtual bool resume() throw() { return true; }
    virtual bool pause() throw() { return true; }
    virtual bool abort() throw() { return true; }
    virtual bool trigft() throw() { return true; }
    virtual bool stateCheck() throw() { return true; }

  public:
    void setDB(DBInterface* db) throw() { m_db = db; }
    DBInterface* getDB() throw() { return m_db; }
    const RCState& getStateDemand() const throw() { return m_state_demand; }
    void setStateDemand(const RCState& state) throw() { m_state_demand = state; }
    virtual bool perform(const NSMMessage& msg) throw();
    virtual void update() throw() {}
    void sendPause(const NSMNode& node) throw();
    void sendPause() throw();
    void setFilePath(const std::string path,
                     const std::string tablename) throw() {
      m_path = path;
      m_tablename = tablename;
    }


  protected:
    bool preload(const NSMMessage& msg) throw();
    bool execute(const RCCommand& cmd) throw();
    void execute() throw();

  private:
    RCState m_state_demand;
    RCConfig m_config;
    DBInterface* m_db;
    std::string m_path;
    std::string m_tablename;

  };

};

#endif
