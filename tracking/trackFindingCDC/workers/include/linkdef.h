#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY

#ifdef TRACKFINDINGCDC_USE_ROOT_BASE

#pragma link C++ class Belle2::TrackFindingCDC::AllFacetWorker+;

#pragma link C++ class Belle2::TrackFindingCDC::MCSegmentWorker+;

#else

#pragma link C++ class Belle2::TrackFindingCDC::AllFacetWorker-;

#pragma link C++ class Belle2::TrackFindingCDC::MCSegmentWorker-;

#endif

#endif

#endif
