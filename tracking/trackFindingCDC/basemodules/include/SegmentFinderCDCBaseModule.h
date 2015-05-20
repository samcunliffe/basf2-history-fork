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
#ifndef SEGMENTFINDERCDCBASEMODULE_H_
#define SEGMENTFINDERCDCBASEMODULE_H_

// Base track finder module
#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>


namespace Belle2 {
  /// Worker for building reconstructed segments form wirehits using facets
  class SegmentFinderCDCBaseModule : public virtual TrackFinderCDCBaseModule {

  public:
    /// Constructor of the module overwritting the default desired orientation of the Segments.
    SegmentFinderCDCBaseModule(ETrackOrientation segmentOrientation = c_None);

    /// Destructor of the module.
    virtual ~SegmentFinderCDCBaseModule() {;}

    ///  Initialize the Module before event processing
    virtual void initialize();

    /// Processes the event and generates track candidates
    virtual void event();

  public:
    /** Implementation specific method that generates segments and
     *  writes them to the output vector not carry about orientation.
     */
    virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>&) {;}

  protected:
    /// Parameter: Name of the output StoreObjPtr of the segments generated by this module.
    std::string m_param_segmentsStoreObjName;

    /** Parameter: String that states the desired segment orientation
     *  Valid orientations are "none" (as generated), "symmetric", "outwards", "downwards".
     */
    std::string m_param_segmentOrientationString;

    /// Parameter: Switch if the segments shall be fitted after the generation
    bool m_param_fitSegments;

    /// Parameter: Switch if genfit::TrackCandidates shall be generated for each segment.
    bool  m_param_createGFTrackCands;

    /** Encoded desired segment orientation.
     *  Valid orientations are "c_None" (as generated), "c_Symmetric", "c_Outwards", "c_Downwards.
     */
    ETrackOrientation m_segmentOrientation;

  }; // end class SegmentFinderCDCBaseModule

} //end namespace Belle2

#endif // SEGMENTFINDERCDCBASEMODULE_H_
