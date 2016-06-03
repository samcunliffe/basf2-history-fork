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

#include <tracking/trackFindingCDC/findlets/minimal/TrackCreatorSingleSegments.h>

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <vector>

namespace Belle2 {

  namespace TrackFindingCDC {
    class CDCRecoSegment2D;
    class CDCTrack;
  }

  /// Module base class to construct tracks from two dimensional segments
  class TrackFinderCDCFromSegmentsModule : public virtual TrackFinderCDCBaseModule {

  public:
    /// Default constructor initialising the filters with the default settings.
    TrackFinderCDCFromSegmentsModule();

    /// Initialize the Module before event processing.
    void initialize() override;

    /// Generates the tracks into the output argument.
    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) override final;

    /// Generates the tracks from the given segments into the output argument.
    virtual void generate(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments,
                          std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) = 0;

  private:
    // Findlets
    /// Creates tracks from left over segments
    TrackFindingCDC::TrackCreatorSingleSegments m_trackCreatorSingleSegments;

    /// Parameter: Name of the output StoreObjPtr of the segments generated by this module.
    std::string m_param_segmentsStoreObjName;
  };

}
