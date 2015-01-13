#ifdef __CINT__

#include <tracking/trackFindingCDC/config/CompileConfiguration.h>

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#ifdef TRACKFINDINGCDC_USE_ROOT

#pragma link C++ class Belle2::CDCLocalTracking::Vector2D+;
#pragma link C++ class Belle2::CDCLocalTracking::Vector3D+;

#pragma link C++ class Belle2::CDCLocalTracking::Circle2D+;

#pragma link C++ class Belle2::CDCLocalTracking::Line2D+;
#pragma link C++ class Belle2::CDCLocalTracking::ParameterLine2D+;

#pragma link C++ class Belle2::CDCLocalTracking::GeneralizedCircle+;

#pragma link C++ class Belle2::CDCLocalTracking::PerigeeCircle+;
#pragma link C++ class Belle2::CDCLocalTracking::PerigeeCovariance+;
#pragma link C++ class Belle2::CDCLocalTracking::UncertainPerigeeCircle+;

#pragma link C++ class Belle2::CDCLocalTracking::SZCovariance+;
#pragma link C++ class Belle2::CDCLocalTracking::UncertainSZLine+;

#pragma link C++ class Belle2::CDCLocalTracking::Helix+;
#pragma link C++ class Belle2::CDCLocalTracking::HelixCovariance+;
#pragma link C++ class Belle2::CDCLocalTracking::UncertainHelix+;

#pragma link C++ class Belle2::CDCLocalTracking::BoundSkewLine+;

#endif

#endif
