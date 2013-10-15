#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef USE_ROOT_IN_CDCLOCALTRACKING

#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHit;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2D;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangent;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacet;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3D;

#endif

#endif
