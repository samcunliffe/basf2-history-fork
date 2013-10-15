#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef USE_ROOT_IN_CDCLOCALTRACKING

#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitPtrSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCWireHitVector;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacetSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoFacetVector;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangentSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoTangentVector;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2DSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit2DVector;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3DSet;
#pragma link C++ class Belle2::CDCLocalTracking::CDCRecoHit3DVector;

#endif

#endif
