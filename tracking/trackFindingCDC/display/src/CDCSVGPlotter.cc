/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: dschneider, Oliver Frost                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/display/CDCSVGPlotter.h>
#include <tracking/trackFindingCDC/display/SVGPrimitivePlotter.h>
#include <tracking/trackFindingCDC/display/Styling.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>

#include <tracking/trackFindingCDC/filters/axialSegmentPair/MCAxialSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentPair/MCSegmentPairFilter.h>
#include <tracking/trackFindingCDC/filters/segmentTriple/MCSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCSegmentLookUp.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

#include <tracking/trackFindingCDC/geometry/Vector3D.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>

#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <framework/datastore/StoreArray.h>

#include <cmath>

using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  template<bool a_drawTrajectory, class AObject>
  struct DrawTrajectoryHelper;

  template<class AObject>
  struct DrawTrajectoryHelper<true, AObject> {
  public:
    static void draw(EventDataPlotter& plotter, AObject& object, const AttributeMap& attributeMap)
    {
      plotter.drawTrajectory(object, attributeMap);
    }
  };

  template<class AObject>
  struct DrawTrajectoryHelper<false, AObject> {
  public:
    static void draw(EventDataPlotter& plotter, AObject& object, const AttributeMap& attributeMap)
    {
      plotter.draw(object, attributeMap);
    }
  };
}

namespace {
  /// used by drawCDCSimHitsConnectByToF as comparison tool
  class FlightTimeOrder {
  public:
    /// Comparision function that sorts CDCSimHits for their flight time.
    bool operator()(const CDCSimHit* x, const CDCSimHit* y) const
    {
      return (x->getFlightTime() < y->getFlightTime());
    }
  };
}

static void printDataStoreContent()
{
  B2INFO("Current content of the DataStore:");
  B2INFO("StoreArrays:");
  for (auto n : DataStore::Instance().getListOfArrays(TObject::Class(), DataStore::EDurability(0))) {
    B2INFO(n);
  }
  B2INFO("");
  B2INFO("StoreObjPtr:");
  for (auto n : DataStore::Instance().getListOfObjects(TObject::Class(), DataStore::EDurability(0))) {
    B2INFO(n);
  }
}

const AttributeMap c_defaultSVGAttributes({
  {"stroke", "orange"},
  {"stroke-width", "0.55"},
  {"fill", "none"},
  {"transform", "translate(0, 1120) scale(1,-1)"}
});

CDCSVGPlotter::CDCSVGPlotter(bool animate)
  : m_animate(animate)
  , m_eventdataPlotter(makeUnique<SVGPrimitivePlotter>(c_defaultSVGAttributes), animate)
{
  int top = -112;
  int left = -112;
  int right = 112;
  int bottom = 112;

  TrackFindingCDC::BoundingBox default_bound(left, bottom, right, top);
  int default_width = 1120;
  int default_height = 1120;

  m_eventdataPlotter.setBoundingBox(default_bound);
  m_eventdataPlotter.setCanvasHeight(default_height);
  m_eventdataPlotter.setCanvasWidth(default_width);
}

CDCSVGPlotter* CDCSVGPlotter::clone()
{
  return (new CDCSVGPlotter(*this));
}

void CDCSVGPlotter::drawInteractionPoint()
{
  m_eventdataPlotter.drawInteractionPoint();
}

void CDCSVGPlotter::drawSuperLayerBoundaries(const std::string& stroke)
{
  AttributeMap attributeMap;
  attributeMap.emplace("stroke", stroke);
  m_eventdataPlotter.drawSuperLayerBoundaries(attributeMap);
}

void CDCSVGPlotter::drawOuterCDCWall(const std::string& stroke)
{
  AttributeMap attributeMap;
  attributeMap.emplace("stroke", stroke);
  m_eventdataPlotter.drawOuterCDCWall(attributeMap);
}

void CDCSVGPlotter::drawInnerCDCWall(const std::string& stroke)
{
  AttributeMap attributeMap;
  attributeMap.emplace("stroke", stroke);
  m_eventdataPlotter.drawInnerCDCWall(attributeMap);
}

void CDCSVGPlotter::drawWires(const CDCWireTopology& cdcWireTopology)
{
  m_eventdataPlotter.draw(cdcWireTopology);
}

void CDCSVGPlotter::drawHits(const std::string& storeArrayName,
                             const std::string& stroke,
                             const std::string& strokeWidth)
{
  ChooseableHitStyling styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreArray<const CDCHit>(storeArrayName, styling);
}

void CDCSVGPlotter::drawSimHits(const std::string& storeArrayName,
                                const std::string& stroke,
                                const std::string& strokeWidth)
{
  B2INFO("Drawing simulated hits");
  StoreArray<CDCHit> storeArray(storeArrayName);
  if (not storeArray) {
    B2WARNING("StoreArray " << storeArrayName << " not present");
    printDataStoreContent();
    return;
  }
  std::vector<CDCSimHit> simHitsRelatedToHits;
  for (const CDCHit& hit : storeArray) {
    simHitsRelatedToHits.push_back(*hit.getRelated<CDCSimHit>("CDCSimHits"));
  }
  B2INFO("#CDCSimHits: " << storeArray.getEntries());
  FixedStyling<const CDCSimHit> styling;
  if (stroke != "") {
    styling.setStroke("orange");
  } else {
    styling.setStroke(stroke);
  }
  styling.setStrokeWidth(strokeWidth);
  drawIterable(simHitsRelatedToHits, styling);
}

void CDCSVGPlotter::drawClusters(const std::string& storeObjName,
                                 const std::string& stroke,
                                 const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCWireHitCluster> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCWireHitCluster>(storeObjName, styling);
}

void CDCSVGPlotter::drawSegments(const std::string& storeObjName,
                                 const std::string& stroke,
                                 const std::string& strokeWidth)
{
  ChooseableSegmentStyling styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCRecoSegment2D>(storeObjName, styling);
}

void CDCSVGPlotter::drawSegmentTrajectories(const std::string& storeObjName,
                                            const std::string& stroke,
                                            const std::string& strokeWidth)
{
  FixedStyling<const CDCRecoSegment2D> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  const bool drawTrajectories = true;
  drawStoreVector<const CDCRecoSegment2D, drawTrajectories>(storeObjName, styling);
}

void CDCSVGPlotter::drawAxialSegmentPairs(const std::string& storeObjName,
                                          const std::string& stroke,
                                          const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCAxialSegmentPair> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCAxialSegmentPair>(storeObjName, styling);
}

void CDCSVGPlotter::drawSegmentPairs(const std::string& storeObjName,
                                     const std::string& stroke,
                                     const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCSegmentPair> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCSegmentPair>(storeObjName, styling);
}

void CDCSVGPlotter::drawSegmentTriples(const std::string& storeObjName,
                                       const std::string& stroke,
                                       const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCSegmentTriple> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCSegmentTriple>(storeObjName, styling);
}

void CDCSVGPlotter::drawSegmentTripleTrajectories(const std::string& storeObjName,
                                                  const std::string& stroke,
                                                  const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCSegmentTriple> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  const bool drawTrajectories = true;
  drawStoreVector<const CDCSegmentTriple, drawTrajectories>(storeObjName, styling);
}

void CDCSVGPlotter::drawTracks(const std::string& storeObjName,
                               const std::string& stroke,
                               const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCTrack> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreVector<const CDCTrack>(storeObjName, styling);
}

void CDCSVGPlotter::drawTrackTrajectories(const std::string& storeObjName,
                                          const std::string& stroke,
                                          const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const CDCTrack> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  const bool drawTrajectories = true;
  drawStoreVector<const CDCTrack, drawTrajectories>(storeObjName, styling);
}

void CDCSVGPlotter::drawRecoTracks(const std::string& storeArrayName,
                                   const std::string& stroke,
                                   const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const RecoTrack> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  drawStoreArray<const RecoTrack>(storeArrayName, styling);
}

void CDCSVGPlotter::drawRecoTrackTrajectories(const std::string& storeArrayName,
                                              const std::string& stroke,
                                              const std::string& strokeWidth)
{
  DefaultColorCycleStyling<const RecoTrack> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStrokeWidth(strokeWidth);
  const bool drawTrajectories = true;
  drawStoreArray<const RecoTrack, drawTrajectories>(storeArrayName, styling);
}

void CDCSVGPlotter::drawSimHitsConnectByToF(const std::string& simHitStoreArrayName,
                                            const std::string& stroke,
                                            const std::string& strokeWidth)
{
  B2INFO("Drawing simulated hits connected by tof");
  StoreArray<CDCHit> hitStoreArray(simHitStoreArrayName);
  if (not hitStoreArray) {
    B2WARNING("StoreArray " << simHitStoreArrayName << " not present");
    printDataStoreContent();
    return;
  }
  std::vector<CDCSimHit*> simHits;
  for (const CDCHit& hit : hitStoreArray) {
    simHits.push_back(hit.getRelated<CDCSimHit>("CDCSimHits"));
  }

  // group them by their mcparticle id
  std::map<int, std::set<CDCSimHit*, FlightTimeOrder>> simHitsByMcParticleId;
  for (CDCSimHit* simHit : simHits) {
    MCParticle* mcParticle = simHit->getRelated<MCParticle>("MCParticles");
    if (mcParticle != nullptr) {
      int mcTrackId = mcParticle->getArrayIndex();
      simHitsByMcParticleId[mcTrackId].insert(simHit);
    }
  }

  AttributeMap attributeMap = {{"stroke", stroke}, {"stroke-width", strokeWidth}};

  for (const auto& mcParticleIdAndSimHits : simHitsByMcParticleId) {
    const std::set<CDCSimHit*, FlightTimeOrder>& simHitsForMcParticle = mcParticleIdAndSimHits.second;

    for (auto itSimHit = simHitsForMcParticle.begin(), itEndSimHit = --simHitsForMcParticle.end();
         itSimHit != itEndSimHit;) {
      CDCSimHit* fromSimHit = *itSimHit;
      ++itSimHit;
      CDCSimHit* toSimHit = *itSimHit;

      CDCHit* fromHit = fromSimHit->getRelated<CDCHit>(simHitStoreArrayName);
      CDCHit* toHit = toSimHit->getRelated<CDCHit>(simHitStoreArrayName);

      if (fromHit != nullptr and toHit != nullptr) {
        CDCWireHit fromWireHit(fromHit);
        CDCWireHit toWireHit(toHit);

        CDCRLWireHit fromRLWireHit(&fromWireHit);
        CDCRLWireHit toRLWireHit(&toWireHit);

        Vector3D fromDisplacement(fromSimHit->getPosTrack() - fromSimHit->getPosWire());
        Vector3D toDisplacement(toSimHit->getPosTrack() - toSimHit->getPosWire());

        CDCRecoHit2D fromRecoHit2D(fromRLWireHit, fromDisplacement.xy());
        CDCRecoHit2D toRecoHit2D(toRLWireHit, toDisplacement.xy());

        draw(fromRecoHit2D, attributeMap);
        draw(toRecoHit2D, attributeMap);

        const Vector2D fromPos = fromRecoHit2D.getRecoPos2D();
        const float fromX =  fromPos.x();
        const float fromY =  fromPos.y();

        const Vector2D toPos = toRecoHit2D.getRecoPos2D();
        const float toX =  toPos.x();
        const float toY =  toPos.y();

        m_eventdataPlotter.drawLine(fromX, fromY, toX, toY, attributeMap);
      }
    }
  }
}

void CDCSVGPlotter::drawMCAxialSegmentPairs(const std::string& segmentsStoreObjName,
                                            const std::string& stroke,
                                            const std::string& strokeWidth)
{
  B2INFO("Draw axial to axial segment pairs");
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedSegments(segmentsStoreObjName);
  if (not storedSegments) {
    B2WARNING(segmentsStoreObjName << "does not exist in current DataStore");
    printDataStoreContent();
    return;
  }

  std::vector<CDCRecoSegment2D>& segments = *storedSegments;
  B2INFO("#Segments: " << segments.size());

  std::vector<const CDCAxialRecoSegment2D*> axialSegments;
  for (const CDCAxialRecoSegment2D& segment : segments) {
    if (segment.isAxial()) axialSegments.push_back(&segment);
  }

  MCAxialSegmentPairFilter mcAxialSegmentPairFilter;
  std::vector<CDCAxialSegmentPair> mcAxialSegmentPairs;
  for (const CDCAxialRecoSegment2D* fromSegment : axialSegments) {
    for (const CDCAxialRecoSegment2D* toSegment : axialSegments) {
      if (fromSegment == toSegment) continue;
      mcAxialSegmentPairs.emplace_back(fromSegment, toSegment);
      Weight mcWeight = mcAxialSegmentPairFilter(mcAxialSegmentPairs.back());
      // Remove segment pairs that are not true
      if (std::isnan(mcWeight)) {
        mcAxialSegmentPairs.pop_back();
      }
    }
  }
  B2INFO("# Axial segment pairs: " << mcAxialSegmentPairs.size());
  FixedStyling<const CDCAxialSegmentPair> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStroke(strokeWidth);
  drawIterable(mcAxialSegmentPairs, styling);
}

void CDCSVGPlotter::drawMCSegmentPairs(const std::string& segmentsStoreObjName,
                                       const std::string& stroke,
                                       const std::string& strokeWidth)
{
  B2INFO("Draw axial to stero segment pairs");
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedSegments(segmentsStoreObjName);
  if (not storedSegments) {
    B2WARNING(segmentsStoreObjName << "does not exist in current DataStore");
    printDataStoreContent();
    return;
  }

  std::vector<CDCRecoSegment2D>& segments = *storedSegments;
  B2INFO("#Segments: " << segments.size());

  std::vector<const CDCAxialRecoSegment2D*> axialSegments;
  std::vector<const CDCStereoRecoSegment2D*> stereoSegments;
  for (const CDCAxialRecoSegment2D& segment : segments) {
    if (segment.isAxial()) {
      axialSegments.push_back(&segment);
    } else {
      stereoSegments.push_back(&segment);
    }
  }

  MCSegmentPairFilter mcSegmentPairFilter;
  std::vector<CDCSegmentPair> mcSegmentPairs;
  for (const CDCAxialRecoSegment2D* axialSegment : axialSegments) {
    for (const CDCStereoRecoSegment2D* stereoSegment : stereoSegments) {
      // Axial first
      {
        mcSegmentPairs.emplace_back(axialSegment, stereoSegment);
        Weight mcWeight = mcSegmentPairFilter(mcSegmentPairs.back());
        // Remove segment pairs that are not true
        if (std::isnan(mcWeight)) {
          mcSegmentPairs.pop_back();
        }
      }
      // Stereo first
      {
        mcSegmentPairs.emplace_back(stereoSegment, axialSegment);
        Weight mcWeight = mcSegmentPairFilter(mcSegmentPairs.back());
        // Remove segment pairs that are not true
        if (std::isnan(mcWeight)) {
          mcSegmentPairs.pop_back();
        }
      }
    }
  }
  B2INFO("# Segment pairs: " << mcSegmentPairs.size());
  FixedStyling<const CDCSegmentPair> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStroke(strokeWidth);
  drawIterable(mcSegmentPairs, styling);
}

void CDCSVGPlotter::drawMCSegmentTriples(const std::string& segmentsStoreObjName,
                                         const std::string& stroke,
                                         const std::string& strokeWidth)
{
  B2INFO("Draw segment triples");
  StoreWrappedObjPtr<std::vector<CDCRecoSegment2D>> storedRecoSegments(segmentsStoreObjName);
  if (not storedRecoSegments) {
    B2WARNING(segmentsStoreObjName << "does not exist in current DataStore");
    printDataStoreContent();
    return;
  }

  std::vector<CDCRecoSegment2D>& recoSegments = *storedRecoSegments;
  B2INFO("#Segment " << recoSegments.size());

  std::vector<const CDCAxialRecoSegment2D*> axialSegments;
  std::vector<const CDCStereoRecoSegment2D*> stereoSegments;
  for (const CDCRecoSegment2D& segment : recoSegments) {
    if (segment.isAxial()) {
      axialSegments.push_back(&segment);
    } else {
      stereoSegments.push_back(&segment);
    }
  }

  MCSegmentTripleFilter mcSegmentTripleFilter;
  std::vector<CDCSegmentTriple> mcSegmentTriples;
  for (const CDCAxialRecoSegment2D* startSegment : axialSegments) {
    for (const CDCStereoRecoSegment2D* middleSegment : stereoSegments) {
      for (const CDCAxialRecoSegment2D* endSegment : axialSegments) {
        if (startSegment == endSegment) continue;
        mcSegmentTriples.emplace_back(startSegment, middleSegment, endSegment);
        Weight mcWeight = mcSegmentTripleFilter(mcSegmentTriples.back());
        // Remove segment pairs that are not true
        if (std::isnan(mcWeight)) {
          mcSegmentTriples.pop_back();
        }
      }
    }
  }
  B2INFO("# Segment triples: " << mcSegmentTriples.size());
  FixedStyling<const CDCSegmentTriple> styling;
  if (stroke != "") styling.setStroke(stroke);
  if (strokeWidth != "") styling.setStroke(strokeWidth);
  drawIterable(mcSegmentTriples, styling);
}

std::string CDCSVGPlotter::saveFile(const std::string& fileName)
{
  TrackFindingCDC::BoundingBox boundingBox = m_eventdataPlotter.getBoundingBox();

  float height = boundingBox.getHeight();
  float width = boundingBox.getWidth();

  int totalPoints = 1120 * 1120;
  float svgHeight = roundf(sqrt(totalPoints * height / width));
  float svgWidth = roundf(sqrt(totalPoints * width / height));

  m_eventdataPlotter.setCanvasHeight(svgHeight);
  m_eventdataPlotter.setCanvasWidth(svgWidth);

  return (m_eventdataPlotter.save(fileName));
}

template <class AItem, bool a_drawTrajectories>
void CDCSVGPlotter::drawStoreArray(const std::string& storeArrayName,
                                   Styling<AItem>& styling)
{
  if (a_drawTrajectories) {
    B2INFO("Drawing trajectories from StoreArray: " << storeArrayName);
  } else {
    B2INFO("Drawing StoreArray: " << storeArrayName);
  }

  using StoreItem = typename std::remove_cv<AItem>::type;
  StoreArray<StoreItem> storeArray(storeArrayName);
  if (not storeArray) {
    B2WARNING(storeArrayName << " not present in the DataStore");
    printDataStoreContent();
    return;
  }

  B2INFO("with " << storeArray.getEntries() << " entries");
  B2INFO("Attributes are");
  B2INFO(styling.info());
  drawIterable<a_drawTrajectories>(storeArray, styling);
}

template <class AItem, bool a_drawTrajectories>
void CDCSVGPlotter::drawStoreVector(const std::string& storeObjName,
                                    Styling<AItem>& styling)
{
  if (a_drawTrajectories) {
    B2INFO("Drawing trajectories for vector from DataStore: " << storeObjName);
  } else {
    B2INFO("Drawing vector from DataStore: " << storeObjName);
  }

  using StoreItem = typename std::remove_cv<AItem>::type;
  StoreWrappedObjPtr<std::vector<StoreItem>> storeVector(storeObjName);
  if (not storeVector) {
    B2WARNING(storeObjName << " not present in the DataStore");
    B2INFO("Current content of the DataStore:");
    printDataStoreContent();
    return;
  }

  const std::vector<StoreItem>& vector = *storeVector;
  B2INFO("with " << vector.size() << " entries");
  B2INFO("Attributes are");
  B2INFO(styling.info());
  drawIterable<a_drawTrajectories>(vector, styling);
}

template <bool a_drawTrajectory, class AIterable, class AStyling>
void CDCSVGPlotter::drawIterable(const AIterable& items, AStyling& styling)
{
  unsigned int index = -1;
  for (const auto& item : items) {

    ++index;
    AttributeMap attributeMap = styling.map(index, item);
    draw<a_drawTrajectory>(item, attributeMap);
  }
}

template <bool a_drawTrajectory, class AObject>
void CDCSVGPlotter::draw(const AObject& object, const AttributeMap& attributeMap)
{
  DrawTrajectoryHelper<a_drawTrajectory, const AObject>::draw(m_eventdataPlotter,
                                                              object,
                                                              attributeMap);
}
