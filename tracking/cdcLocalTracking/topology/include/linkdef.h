#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef USE_ROOT_IN_CDCLOCALTRACKING

#pragma link C++ class Belle2::CDCLocalTracking::CDCWire;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireLayer;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireSuperLayer;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireTopology;

#endif

#endif
