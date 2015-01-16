/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROOTIFICATIONBASE_H_
#define ROOTIFICATIONBASE_H_


#include <framework/datastore/RelationsObject.h>
#include <tracking/trackFindingCDC/config/CompileConfiguration.h>


namespace Belle2 {

  namespace TrackFindingCDC {

#ifdef TRACKFINDINGCDC_USE_ROOT_DICTIONARY
    const bool ROOTIFICATION_DICTIONARY_IS_ACTIVE = true;
#else
    const bool ROOTIFICATION_DICTIONARY_IS_ACTIVE = false;
#endif

#if defined TRACKFINDINGCDC_USE_ROOT_DICTIONARY && defined TRACKFINDINGCDC_USE_ROOT_BASE
    const bool ROOTIFICATION_BASE_IS_ACTIVE = true;
#else
    const bool ROOTIFICATION_BASE_IS_ACTIVE = false;
#endif

    /** Base class for the ROOT inheritance.
     *  This class unconditionally inherits from ROOT,
     *  so it can always be ask if it is the active base class from Python.
     */
    class RootificationBase : public RelationsObject {

    public:
      /// Returns true, if this class is the active base and all other objects are accessable from Python.
      static bool getBaseIsActive() { return ROOTIFICATION_BASE_IS_ACTIVE; }

      /// Returns true, if the tracking objects have dictionary created by ROOT CINT.
      static bool getDictionaryIsActive() { return ROOTIFICATION_DICTIONARY_IS_ACTIVE; }

    private:
      static const bool s_isBaseActive = ROOTIFICATION_BASE_IS_ACTIVE;
      static const bool s_isDictionaryActive = ROOTIFICATION_DICTIONARY_IS_ACTIVE;

    private:
      /// ROOT Macro to unconditionally make RootificationBase a ROOT class.
      ClassDef(RootificationBase, 1);

    };

  } //end namespace TrackFindingCDC

} //end namespace Belle2


#endif // ROOTIFICATIONBASE_H_  
