#ifdef TRACKFINDINGCDC_COMPILE_DEBUG
// Setup for debug during structural changes to the algorithm
#define TRACKFINDINGCDC_USE_ROOT_DICTIONARY
#define TRACKFINDINGCDC_USE_ROOT_BASE
#endif

#ifdef TRACKFINDINGCDC_COMPILE_OPT
#define TRACKFINDINGCDC_USE_ROOT_DICTIONARY
#endif
