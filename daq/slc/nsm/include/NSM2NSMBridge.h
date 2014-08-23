#ifndef _Belle2_NSM2NSMBridge_hh
#define _Belle2_NSM2NSMBridge_hh

#include "daq/slc/nsm/NSMCallback.h"
#include "daq/slc/nsm/NSMNodeDaemon.h"

namespace Belle2 {

  class NSM2NSMBridge {

  public:
    NSM2NSMBridge(NSMCallback* callback1,
                  const std::string host1, int port1,
                  NSMCallback* callback2,
                  const std::string host2, int port2);
    virtual ~NSM2NSMBridge() throw() {}

  public:
    void run() throw();

  private:
    NSMCallback* m_callback[2];
    NSMNodeDaemon* m_daemon[2];

  };

}

#endif
