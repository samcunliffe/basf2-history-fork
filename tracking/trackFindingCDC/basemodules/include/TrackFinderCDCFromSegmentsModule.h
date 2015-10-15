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

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <vector>
#include <map>

namespace Belle2 {

  class TrackFinderCDCFromSegmentsModule : public virtual TrackFinderCDCBaseModule {

  public:
    /// Default constructor initialising the filters with the default settings.
    TrackFinderCDCFromSegmentsModule(ETrackOrientation trackOrientation = c_None);

    /// Initialize the Module before event processing.
    void initialize() override;

    /// Generates the tracks into the output argument.
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

    /// Generates the tracks from the given segments into the output argument.
    virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                          std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) = 0;

  private:
    /// Parameter: Name of the output StoreObjPtr of the segments generated by this module.
    std::string m_param_segmentsStoreObjName;

    /** Parameter: Map of super layer ids to minimum hit number
     *  for which left over segments shall be forwarded as tracks, if they exceed the minimal hit requirement.
     *
     *  Default empty.
     */
    std::map<TrackFindingCDC::ISuperLayerType, size_t> m_minimalHitsForSingleSegmentTrackBySuperLayerId;
  };
}
