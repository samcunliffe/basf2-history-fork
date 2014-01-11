#ifndef _Belle2_RunInfoBuffer_h
#define _Belle2_RunInfoBuffer_h

#include "daq/slc/readout/SharedMemory.h"
#include "daq/slc/readout/MMutex.h"
#include "daq/slc/readout/MCond.h"

namespace Belle2 {

  class RunInfoBuffer {

  public:
    static const unsigned int NOTREADY = 0;
    static const unsigned int READY = 1;
    static const unsigned int RUNNING = 2;
    static const unsigned int ERROR = 3;

  public:
    RunInfoBuffer() {}
    ~RunInfoBuffer() {}

  public:
    size_t size() throw();
    bool open(const std::string& nodename, bool recreate = false);
    bool init();
    bool close();
    bool unlink();
    bool lock() throw();
    bool unlock() throw();
    bool wait() throw();
    bool wait(int time) throw();
    bool notify() throw();
    void clear();

  public:
    const std::string getPath() const throw() { return _path; }
    unsigned int* getParams() throw() { return _info; }
    unsigned int getState() const throw() { return _info[0]; }
    unsigned int getExpNumber() const throw() { return _info[1]; }
    unsigned int getColdNumber() const throw() { return _info[2]; }
    unsigned int getHotNumber() const throw() { return _info[3]; }
    unsigned int getNodeId() const throw() { return _info[4]; }
    void setState(unsigned int state) { _info[0] = state; }
    void setExpNumber(unsigned int number) { _info[1] = number; }
    void setColdNumber(unsigned int number) { _info[2] = number; }
    void setHotNumber(unsigned int number) { _info[3] = number; }
    void setNodeId(unsigned int id) { _info[4] = id; }
    bool waitRunning(int timeout);
    bool reportRunning();
    bool reportError();
    bool reportReady();
    bool reportNotReady();

  private:
    std::string _path;
    SharedMemory _memory;
    MMutex _mutex;
    MCond _cond;
    unsigned int* _info;

  };

}

#endif
