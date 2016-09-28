/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/FindletModules.h>

using namespace Belle2;

REG_MODULE(WireHitCreator);
REG_MODULE(WireHitTopologyFiller);
REG_MODULE(SuperClusterCreator);
REG_MODULE(ClusterRefiner);
REG_MODULE(ClusterBackgroundDetector);
REG_MODULE(FacetCreator);
REG_MODULE(SegmentCreatorFacetAutomaton);
REG_MODULE(SegmentMerger);
REG_MODULE(SegmentFitter);
REG_MODULE(SegmentOrienter);
REG_MODULE(SegmentExporter);
REG_MODULE(SegmentCreatorMCTruth);

REG_MODULE(AxialTrackCreatorSegmentHough)

REG_MODULE(AxialSegmentPairCreator)
REG_MODULE(SegmentPairCreator)
REG_MODULE(SegmentTripleCreator)

REG_MODULE(TrackCreatorSegmentPairAutomaton)
REG_MODULE(TrackCreatorSegmentTripleAutomaton)
REG_MODULE(TrackCreatorSingleSegments)

REG_MODULE(TrackMerger)
REG_MODULE(TrackOrienter)
REG_MODULE(TrackFlightTimeAdjuster)
REG_MODULE(TrackExporter)

REG_MODULE(SegmentFinderFacetAutomaton);
REG_MODULE(TrackFinderSegmentPairAutomaton);
