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

#include <tracking/trackFindingCDC/basemodules/SegmentFinderCDCBaseModule.h>

#include <framework/core/Module.h>

namespace Belle2 {

  /// Module to build segments from pure Monte Carlo information.
  class SegmentFinderCDCMCTruthModule : public SegmentFinderCDCBaseModule {

  public:
    SegmentFinderCDCMCTruthModule();

    virtual void initialize() override;

  public:
    /// Generates the segment from Monte Carlo information. Default orientation is the flight direction.
    virtual void generateSegments(std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D>& segments) override final;

  private:
    /// Minimum number of CDC hits per segment.
    int m_minCDCHits;

  }; // end class SegmentFinderCDCMCTruthModule

} //end namespace Belle2

