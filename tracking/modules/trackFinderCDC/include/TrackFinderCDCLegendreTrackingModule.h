/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth, Viktor Trusov,       *
 *               Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/basemodules/TrackFinderCDCBaseModule.h>

#include <tracking/trackFindingCDC/legendre/TrackProcessor.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCTrack;
  }

  /**
   * CDC tracking module, using Legendre transformation of the drift time circles.
   * This is a module, performing tracking in the CDC. It is based on the paper
   * "Implementation of the Legendre Transform for track segment reconstruction in drift tube chambers"
   * by T. Alexopoulus, et al. NIM A592 456-462 (2008).
   */
  class CDCLegendreTrackingModule: public TrackFinderCDCBaseModule {

  public:
    /// Create and allocate memory for variables here and add the module parameters in this method.
    CDCLegendreTrackingModule();

  private:
    /// Parameter
    int m_param_maxLevel;               /**< Maximum Level of FastHough Algorithm. */
    bool m_param_doEarlyMerging;        /**< Defines whether early track merging will be performed. */

    /// Worker
    TrackFindingCDC::TrackProcessor m_trackProcessor;  /**< Object for doing all sort of things with the tracks. */

    void generate(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);

    /// All objects in m_hitList and m_trackList are deleted and the two lists are cleared.
    void clearVectors();

    /// Do the real tree track finding.
    void findTracks();

    /// Startup code before the event starts.
    void startNewEvent();

    /// Write the objects to the store array.
    void outputObjects(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks);
  };
}


