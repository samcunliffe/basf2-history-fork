//+
// File : RFEventServer.cc
// Description : Receive data from event builder 1 and distribute them to
//               Processing nodes
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 24 - June - 2013
//-
#include "daq/rfarm/manager/RFEventServer.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  RFConf conf(argv[1]);

  // Creation of event server instance. evs contains the instance
  //  RFEventServer& evs = RFEventServer::Create(argv[1]);

  RFEventServer* evs = new RFEventServer(argv[1]);


  RFNSM nsm(conf.getconf("distributor", "nodename"), evs);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  evs->SetNodeInfo(nsm.GetNodeInfo());

  evs->server();

}
