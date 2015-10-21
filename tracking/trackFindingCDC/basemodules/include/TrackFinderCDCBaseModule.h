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

#include <framework/core/Module.h>
#include <string>
#include <vector>
#include <framework/core/ModuleParamList.h>

namespace Belle2 {
  namespace TrackFindingCDC { class CDCTrack; }

  /// A base class for track finders in the CDC.
  /** This module defines / handles the inputs and outputs to the DataStore and the respective StoreArray names.
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

    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCBaseModule(ETrackOrientation trackOrientation = c_None);

    ///  Initialize the Module before event processing
    void initialize() override;

    /// Processes the event and generates track candidates
    void event() override;

    /// Generates the tracks into the output argument. To be overriden in a concrete subclass.
    virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

    /** Returns true if a parameter with the given is present.
     *
     *  Scheduled to be moved to the Module base class.
     *  @param name The unique name of the parameter.
     *  @return True if the parameter is present, false otherwise.
     */
    template<class T>
    bool hasParam(const std::string& name) const;

    /** Returns the value of the parameter with the right type.
     *
     *  Scheduled to be moved to the Module base class.
     *  @param name The unique name of the parameter.
     *  @return The current value of the parameter.
     */
    template<class T>
    const T& getParamValue(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError,
                                                                ModuleParamList::ModuleParameterTypeError);

    /// Same as in base Module class but public.
    template<class T>
    void addParam(const std::string& name,
                  T& paramVariable,
                  const std::string& description,
                  const T& defaultValue)
    {
      Module::addParam(name, paramVariable, description, defaultValue);
    }

  protected:
    /** Parameter: String that states the desired track orientation.
     *  Valid orientations are "none" (as generated), "symmetric", "outwards", "downwards".*/
    std::string m_param_trackOrientationString = "none";

    /// Parameter: Name of the output StoreObjPtr of the tracks generated within this module.
    std::string m_param_tracksStoreObjName = "CDCTrackVector";

    /// Parameter: Flag to use the CDCTracks in the given StoreObjPtr as input and output of the module.
    bool m_param_tracksStoreObjNameIsInput = false;

    /// Parameter: Name of the output StoreArray of the Genfit track candidates generated by this module.
    std::string m_param_gfTrackCandsStoreArrayName = "";

    /// Parameter: Flag to output genfit tracks to store array.
    bool m_param_writeGFTrackCands = true;

    /** Encoded desired track orientation.
     *  Valid orientations are "c_None" (as generated), "c_Symmetric", "c_Outwards", "c_downwards.*/
    ETrackOrientation m_trackOrientation = ETrackOrientation::c_None;
  };

//--- Definition of template methods ----------------------------------------------------------------------------------
  template<class T>
  bool TrackFinderCDCBaseModule::hasParam(const std::string& name) const
  {
    try {
      getParam<T>(name);
    } catch (ModuleParamList::ModuleParameterNotFoundError& e) {
      return false;
    }
    return true;
  }

  template<class T>
  const T& TrackFinderCDCBaseModule::getParamValue(const std::string& name) const throw(ModuleParamList::ModuleParameterNotFoundError,
      ModuleParamList::ModuleParameterTypeError)
  {
    ModuleParam<T>& moduleParam = getParam<T>(name);
    return moduleParam.getValue();
  }
}

