/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKFINDERCDCBASEMODULE_H_
#define TRACKFINDERCDCBASEMODULE_H_

// Basf2 module
#include <framework/core/Module.h>

namespace Belle2 {

  /// A base class for track finders in the CDC.
  /** This module defines / handles
   *  * the inputs and outputs to the DataStore and the respective StoreArray names.
   *  * unpacking of the CDCHits and attaching of the geometry
   *  * Preparation of the Monte Carlo information.
   */
  class TrackFinderCDCBaseModule : public Module {

  public:
    // Constant for the orientation of segments to be generated.
    enum ETrackOrientation {
      c_None,
      c_Symmetric,
      c_Outwards,
      c_Downwards
    };

  public:

    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCBaseModule();

    /// Destructor of the module.
    virtual ~TrackFinderCDCBaseModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Called when entering a new run.
    virtual void beginRun();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Called at the end of a run.
    virtual void endRun();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  protected:
    /// Parameter: Name of the input StoreArray of the CDCHits.
    std::string m_param_cdcHitsStoreArrayName;

    /// Parameter: Name of the output StoreArray of the Genfit track candidates generated by this module.
    std::string m_param_gfTrackCandsStoreArrayName;

  }; // end class
} // end namespace Belle2

#endif /* TRACKFINDERCDCBASEMODULE_H_ */

