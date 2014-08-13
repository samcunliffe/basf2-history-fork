//+
// File : flow_monitor.cc
// Description : Readout flow rate monitor
//
// Author : Tomoyuki Konno, Tokyo Metroplitan University
// Date : 25 - Sep - 2013
//-

#include <daq/slc/nsm/NSMData.h>
#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/system/LogFile.h>

#include <daq/slc/base/StringUtil.h>

#include <daq/slc/readout/ronode_status.h>

#include <fstream>
#include <unistd.h>
#include <cstdlib>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    LogFile::debug("%s storage monitor", argv[0]);
    return 1;
  }
  const std::string stornode = argv[1];
  const std::string node = argv[2];
  NSMCommunicator* comm = new NSMCommunicator();
  comm->init(NSMNode(node));
  NSMData data(stornode + "_STATUS", "ronode_status", ronode_status_revision);
  ronode_status* info = (ronode_status*)data.open(comm);

  while (true) {
    sleep(2);
    fputs("\033[2J\033[0;0H", stdout);
    rewind(stdout);
    ftruncate(1, 0);
    //printf(" exp = %04u run = %04u\n", info->expno, info->runno);
    //printf(" rxqueue from eb0    : %4.1f [kB]\n", (float)(info->io[0].nqueue / 1024.));
    printf(" txqueue to eb0    : %4.1f [kB]\n", (float)(info->io[1].nqueue / 1024.));
    printf("\n");
    printf(" %17s |      count | freq [kHz] | rate [MB/s] | evtsize [kB]\n", "socket");
    for (int i = 0; i < 2; i++) {
      std::string name = "";
      int state = 0;
      if (i == 0) {
        name = "hslb   --> copper";
        state = ((info->eflag >> 8) & 0xF) == 0;
      } else if (i == 1) {
        name = "copper --> eb0   ";
        state = info->io[i].state;
      }
      if (state != 1) {
        name = "\x1b[49m\x1b[31m" +
               StringUtil::replace(name, "-->", "-x->");
      } else {
        name = "\x1b[49m\x1b[32m" + name;
      }
      printf(" %-15s | %s | %10s | %11s | %12s\x1b[49m\x1b[39m\n",
             name.c_str(),
             StringUtil::form("%10u", info->io[i].count).c_str(),
             StringUtil::form("%02.2f", info->io[i].freq).c_str(),
             StringUtil::form("%04.2f", info->io[i].rate).c_str(),
             StringUtil::form("%03.2f", info->io[i].evtsize).c_str());
    }
  }
  return 0;
}

