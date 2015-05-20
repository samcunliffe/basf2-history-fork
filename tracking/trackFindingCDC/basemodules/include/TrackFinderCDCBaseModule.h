/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#ifndef TRACKFINDERCDCBASEMODULE_H_
#define TRACKFINDERCDCBASEMODULE_H_


#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

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
    TrackFinderCDCBaseModule(ETrackOrientation trackOrientation = c_None);

    /// Destructor of the module.
    virtual ~TrackFinderCDCBaseModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Called when entering a new run.
    virtual void beginRun();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Generates the tracks into the output argument. To be overriden in a concrete subclass.
    size_t prepareHits();

    /** Copies the CDCHits that are not blocked at the end of the processing
     *  to a StoreArray if requested by the RemainingCDCHitsStoreArray parameter.
     *  @returns The number of remaining CDCHits.
     */
    size_t copyRemainingHits();

    /// Generates the tracks into the output argument. To be overriden in a concrete subclass.
    virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

    /// Called at the end of a run.
    virtual void endRun();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  public:
    /**
     * Returns true if a parameter with the given is present.
     *
     * Scheduled to be moved to the Module base class.
     *
     * @param name The unique name of the parameter.
     * @return True if the parameter is present, false otherwise
     */
    template<typename T>
    bool hasParam(const std::string& name) const;

    /**
     * Returns the value of the parameter with the right type.
     *
     * Scheduled to be moved to the Module base class.
     *
     * @param name The unique name of the parameter.
     * @return The current value of the parameter.
     */
    template<typename T>
    const T& getParamValue(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError,
                                                                ModuleParamList::ModuleParameterTypeError);

    /**
       Same as in base Module class but public
    */
    template<typename T>
    void addParam(const std::string& name,
                  T& paramVariable,
                  const std::string& description,
                  const T& defaultValue)
    {
      Module::addParam(name, paramVariable, description, defaultValue);
    }

  protected:

    /// Parameter: Full name of a StoreArray that has a Relation to the CDCHits StoreArray. Only CDCHits that have a relation will be used in this track finder.
    std::string m_param_useOnlyCDCHitsRelatedFromStoreArrayName;

    /// Parameter: Full name of a StoreArray that has a Relation to the CDCHits StoreArray. CDCHits that have a relation will be blocked in this track finder.
    std::string m_param_dontUseCDCHitsRelatedFromStoreArrayName;

    /// Parameter: String that states the desired track orientation. Valid orientations are "none" (as generated), "symmetric", "outwards", "downwards".
    std::string m_param_trackOrientationString;

    /// Parameter: Name of the output StoreObjPtr of the tracks generated within this module.
    std::string m_param_tracksStoreObjName;

    /// Parameter: Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module
    bool m_param_tracksStoreObjNameIsInput;

    /// Parameter: Name of the output StoreArray of the Genfit track candidates generated by this module.
    std::string m_param_gfTrackCandsStoreArrayName;

    /// Parameter: Flag to output genfit tracks to store array
    bool m_param_writeGFTrackCands;

    /// Parameter: Name of the output StoreArray for copies of the original CDCHits that are still unblocked at the end of the module.
    std::string m_param_remainingCDCHitsStoreArrayName;

    /// Encoded desired track orientation. Valid orientations are "c_None" (as generated), "c_Symmetric", "c_Outwards", "c_downwards.
    ETrackOrientation m_trackOrientation;

    /// Parameter: Flag to skip wire hits generation, because other modules have done so before
    bool m_param_skipHitsPreparation;

  }; // end class


  template<typename T>
  bool TrackFinderCDCBaseModule::hasParam(const std::string& name) const
  {
    try {
      getParam<T>(name);
    } catch (ModuleParamList::ModuleParameterNotFoundError& e) {
      return false;
    }
    return true;
  }

  template<typename T>
  const T& TrackFinderCDCBaseModule::getParamValue(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError,
      ModuleParamList::ModuleParameterTypeError)
  {
    ModuleParam<T>& moduleParam = getParam<T>(name);
    return moduleParam.getValue();
  }


} // end namespace Belle2

#endif /* TRACKFINDERCDCBASEMODULE_H_ */

