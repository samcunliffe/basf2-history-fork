#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CDCLocalTrackingConfig.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma link C++ class Belle2::CDCLocalTracking::CDCMCHitLookUp+;

#ifdef CDCLOCALTRACKING_USE_ROOT
#pragma link C++ class Belle2::CDCLocalTracking::CDCMCSegmentLookUp+;
#endif

#endif
