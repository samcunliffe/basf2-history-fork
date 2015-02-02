/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef TRACKFINDERCDCAUTOMATONDEVMODULE_H_
#define TRACKFINDERCDCAUTOMATONDEVMODULE_H_

//#define LOG_NO_B2DEBUG
#include <tracking/trackFindingCDC/eventdata/CDCEventData.h>

#include <tracking/trackFindingCDC/workers/FacetSegmentWorker.h>
#include <tracking/trackFindingCDC/workers/SegmentTripleTrackingWorker.h>

#include <tracking/trackFindingCDC/workers/SegmentPairTrackingWorker.h>

// Base track finder module
#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonDevModule.h>

// Base track finder module
#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonDevFilters.h>


namespace Belle2 {

  /// Module for development of the cellular automaton tracking for the CDC
  class TrackFinderCDCAutomatonDevModule : public SegmentFinderCDCFacetAutomatonDevModule {

  public:
    /// Constructor of the module. Setting up parameters and description.
    TrackFinderCDCAutomatonDevModule();

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Processes the event and generates track candidates
    virtual void event();

    /// Terminate and free resources after last event has been processed
    virtual void terminate();

  private:
    /// Parameter: Switch in indicating if the second stage, which is the building of tracks from segments, shall be run.
    bool m_param_runSecondStage;

    /// Worker to carry out the second stage generating tracks from segments.
    TrackFindingCDC::SegmentTripleTrackingWorker <
    TrackFindingCDC::AxialAxialSegmentPairFilter,
                    TrackFindingCDC::SegmentTripleFilter,
                    TrackFindingCDC::SegmentTripleNeighborChooser
                    > m_segmentTripleTrackingWorker;

    /// Worker to carry out the second stage generating tracks from segments utilizing pairs of segments.
    TrackFindingCDC::SegmentPairTrackingWorker <
    TrackFindingCDC::AxialStereoSegmentPairFilter,
                    TrackFindingCDC::AxialStereoSegmentPairNeighborChooser
                    > m_segmentPairTrackingWorker;

  }; // end class
} // end namespace Belle2

#endif /* TRACKFINDERCDCAUTOMATONDEVMODULE_H_ */

