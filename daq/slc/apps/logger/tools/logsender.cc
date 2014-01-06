#include <daq/slc/nsm/NSMCommunicator.h>

#include <daq/slc/base/Debugger.h>

#include <unistd.h>
#include <cstring>

using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 3) {
    printf("Usage : ./logsender <name> <priority> <message>\n");
    return 1;
  }
  const char* nodename = argv[1];
  const char* priority_s = argv[2];
  const char* message = argv[3];
  NSMCommunicator* comm = new NSMCommunicator(new NSMNode(nodename));
  comm->init();
  SystemLog::Priority priority = SystemLog::DEBUG;
  if (strcmp(priority_s, "INFO") == 0) {
    priority = SystemLog::INFO;
  } else if (strcmp(priority_s, "NOTICE") == 0) {
    priority = SystemLog::NOTICE;
  } else if (strcmp(priority_s, "WARNING") == 0) {
    priority = SystemLog::WARNING;
  } else if (strcmp(priority_s, "ERROR") == 0) {
    priority = SystemLog::ERROR;
  } else if (strcmp(priority_s, "FATAL") == 0) {
    priority = SystemLog::FATAL;
  }
  comm->sendLog(SystemLog(nodename, priority, message));
  return 0;
}
