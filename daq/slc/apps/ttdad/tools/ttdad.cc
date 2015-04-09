#include "daq/slc/apps/ttdad/TTDACallback.h"

#include <daq/slc/runcontrol/RCNodeDaemon.h>

#include <daq/slc/base/ConfigFile.h>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (Daemon::start(argv[1], argc, argv, 1, "<config>")) {
    ConfigFile config("slowcontrol", argv[1]);
    RCNode ttd(config.get("ttd"));
    TTDACallback callback(ttd);
    callback.setFTSWID(config.getInt("ftsw"));
    RCNodeDaemon(config, &callback).run();
  }
  return 0;
}
