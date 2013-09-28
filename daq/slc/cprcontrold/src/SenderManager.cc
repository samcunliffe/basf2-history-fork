#include "SenderManager.hh"

#include <node/DataSender.hh>
#include <node/HSLB.hh>
#include <node/Host.hh>

#include <util/Debugger.hh>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace B2DAQ;

std::string SenderManager::run_script(const std::string& cmd)
{
  FILE* file = popen(cmd.c_str(), "r");
  char str[1024];
  memset(str, '\0', 1024);
  fread(str, 1, 1024 - 1, file);
  pclose(file);
  std::string s = str;
  return s.substr(0, s.find_last_of("\n"));
}

void SenderManager::run() throw()
{
  B2DAQ::debug("Try to start basf2 script");
  if (_node != NULL) {
    const char* belle2_path = getenv("BELLE2_LOCAL_DIR");
    const char* belle2_sub = getenv("BELLE2_SUBDIR");
    if (belle2_path == NULL) {
      B2DAQ::debug("env BELLE2_LOCAL_DIR is not avaiable.");
      return ;
    }
    char path[128];
    sprintf(path, "%s/bin/%s/basf2", belle2_path, belle2_sub);
    char script_c[128];
    sprintf(script_c, "%s/daq/rawdata/examples/%s",
            belle2_path, _node->getSender()->getScript().c_str());
    char hostname_c[64];
    std::string hostname = run_script("hostname");
    sprintf(hostname_c, "%s", hostname.c_str());
    char id_c[32];
    sprintf(id_c, "%d", (int)_node->getID());
    int flag = 0;
    for (size_t slot = 0; slot < 4; slot++) {
      HSLB* hslb = _node->getHSLB(slot);
      if (hslb != NULL && hslb->isUsed()) {
        flag += 1 << slot;
      }
    }
    char slots_c[8];
    sprintf(slots_c, "%d", flag);
    char* argv[8];
    int i = 0;
    argv[i++] = path;
    argv[i++] = script_c;
    argv[i++] = hostname_c;
    argv[i++] = id_c;
    argv[i++] = slots_c;
    argv[i++] = NULL;
    B2DAQ::debug("%s %s %s %s %s",
                 argv[0], argv[1], argv[2], argv[3], argv[4]);
    if (execvp(path, argv) == -1) {
      B2DAQ::debug("Faield to start receiver basf2 script");
    }
  } else {
    B2DAQ::debug("No NSM node was registered");
  }
}

