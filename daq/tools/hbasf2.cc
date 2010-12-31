#include <cstring>
#include <daq/hlt/HLTFramework.h>

using namespace Belle2;

void showUsage(void);

int main(int argc, char* argv[])
{
  if (argc < 2) {
    showUsage();
    return 1;
  }

  // For Manager node
  if (!strcmp(argv[1], "manager")) {
    if (!argv[2]) {
      B2WARNING("No HLT info xml assigned");
      showUsage();
      return 1;
    } else {
      HLTFramework* hltFramework = new HLTFramework(c_ManagerNode, argv[2]);
      hltFramework->init();
      B2INFO("Manager node initialized");
      delete hltFramework;
      return 0;
    }
  }
  // For Process node
  else if (!strcmp(argv[1], "node")) {
    HLTFramework* hltFramework = new HLTFramework(c_ProcessNode);
    hltFramework->init();
    B2INFO("Process node initialized");
    delete hltFramework;
    return 0;
  }
  // For wrong node assignment
  else {
    B2WARNING("Invalid node type");
    showUsage();
    return 1;
  }

}

void showUsage(void)
{
  B2INFO("Usage: $ hbasf2 [mode] {input}");
  B2INFO("         mode:  manager / node");
  B2INFO("         input: XML node info (only for manager mode)");
}

