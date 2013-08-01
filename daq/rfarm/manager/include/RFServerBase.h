#ifndef SERVER_BASE_H
#define SERVER_BASE_H
//+
// File : RFServerBase.h
// Description : Base class to construct RFARM server applications
//               The class can be fed to RFNSM to hook up functions
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - Jul - 2013
//-

namespace Belle2 {
  class RFServerBase {
  public:
    RFServerBase() {};
    ~RFServerBase() {};

    virtual void Configure(NSMmsg*, NSMcontext*) {};
    virtual void Start(NSMmsg*, NSMcontext*) {};
    virtual void Stop(NSMmsg*, NSMcontext*) {};
    virtual void Pause(NSMmsg*, NSMcontext*) {};
    virtual void Resume(NSMmsg*, NSMcontext*) {};
    virtual void Restart(NSMmsg*, NSMcontext*) {};
    virtual void Status(NSMmsg*, NSMcontext*) {};

  public:
    static RFServerBase* s_instance;

  };
}
#endif
