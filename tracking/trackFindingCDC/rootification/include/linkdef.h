
#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclasses;

#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/segments/CDCWireHitCluster.h>;

#pragma extra_include <tracking/trackFindingCDC/eventdata/hits/CDCFacet.h>;

#pragma extra_include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>;

#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>;

#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>;
#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>;

#pragma extra_include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>;

#pragma extra_include <tracking/trackFindingCDC/ca/WeightedRelation.h>;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCWireHit> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<Belle2::TrackFindingCDC::CDCWireHit> > >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCWireHitCluster> >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCFacet> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCFacet> > >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCRecoSegment2D> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCRecoSegment2D> > >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentPair> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentPair> > >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCAxialSegmentPair> >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCSegmentTriple> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCSegmentTriple> > >+;

#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::CDCTrack> >+;
#pragma link C++ class Belle2::TrackFindingCDC::StoreWrapper<std::vector<Belle2::TrackFindingCDC::WeightedRelation<const Belle2::TrackFindingCDC::CDCTrack> > >+;
