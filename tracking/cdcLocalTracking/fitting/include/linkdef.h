#ifdef __CINT__

#include <tracking/cdcLocalTracking/mockroot/ToggleMockRoot.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef CDCLOCALTRACKING_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::CDCObservations2D+;

#pragma link C++ class Belle2::CDCLocalTracking::RiemannsMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::ExtendedRiemannsMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::KarimakisMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::OriginalKarimakisMethod+;
#pragma link C++ class Belle2::CDCLocalTracking::PortedKarimakisMethod+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<RiemannsMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<ExtendedRiemannsMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<KarimakisMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<OriginalKarimakisMethod>+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCFitter2D<PortedKarimakisMethod>+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCRiemannFitter+;
#pragma link C++ class Belle2::CDCLocalTracking::CDCKarimakiFitter+;

#pragma link C++ class Belle2::CDCLocalTracking::CDCSZFitter+;

#endif

#endif
