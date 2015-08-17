/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/sim/CDCSimpleSimulation.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoHit3D.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCWireHit.h>

#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>

#include <tracking/trackFindingCDC/topology/CDCWire.h>
#include <tracking/trackFindingCDC/topology/CDCWireLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireSuperLayer.h>
#include <tracking/trackFindingCDC/topology/CDCWireTopology.h>

#include <framework/logging/Logger.h>

#include <TRandom.h>

#include <algorithm>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

CDCTrack CDCSimpleSimulation::simulate(const CDCTrajectory3D& trajectory3D)
{
  return std::move(simulate(std::vector<CDCTrajectory3D>(1, trajectory3D)).front());
}


vector<CDCTrack> CDCSimpleSimulation::simulate(const vector<CDCTrajectory3D>& trajectories3D)
{
  B2ASSERT("", m_wireHitTopology);

  vector<SimpleSimHit> simpleSimHits;
  const size_t nMCTracks = trajectories3D.size();

  for (size_t iMCTrack = 0;  iMCTrack < nMCTracks; ++iMCTrack) {
    const CDCTrajectory3D& trajectory3D = trajectories3D[iMCTrack];

    const UncertainHelix& localHelix = trajectory3D.getLocalHelix();
    const Vector3D& localOrigin = trajectory3D.getLocalOrigin();

    Helix globalHelix = localHelix;
    const FloatType arcLength2DOffset = globalHelix.passiveMoveBy(-localOrigin);
    vector<SimpleSimHit> simpleSimHitsForTrajectory = createHits(globalHelix, arcLength2DOffset);

    for (SimpleSimHit& simpleSimHit : simpleSimHitsForTrajectory) {
      simpleSimHit.m_iMCTrack = iMCTrack;
      simpleSimHits.push_back(simpleSimHit);
    }
  }

  vector<CDCTrack> mcTracks = constructMCTracks(nMCTracks, std::move(simpleSimHits));

  /// Assign mc trajectories to the tracks
  for (size_t iMCTrack = 0;  iMCTrack < nMCTracks; ++iMCTrack) {
    CDCTrack& mcTrack = mcTracks[iMCTrack];
    CDCTrajectory3D mcTrajectory = trajectories3D[iMCTrack];
    if (not mcTrack.empty()) {
      mcTrajectory.setLocalOrigin(mcTrack.front().getRecoPos3D());
      mcTrack.setStartTrajectory3D(mcTrajectory);
      mcTrajectory.setLocalOrigin(mcTrack.back().getRecoPos3D());
      mcTrack.setEndTrajectory3D(mcTrajectory);
    } else {
      mcTrack.setStartTrajectory3D(mcTrajectory);
      mcTrack.setEndTrajectory3D(mcTrajectory);
    }
  }
  return mcTracks;
}

vector<CDCTrack>
CDCSimpleSimulation::constructMCTracks(size_t nMCTracks, vector<SimpleSimHit> simpleSimHits) const
{

  // Sort the hits along side their wire hits
  std::stable_sort(simpleSimHits.begin(), simpleSimHits.end(),
  [](const SimpleSimHit & lhs, const SimpleSimHit & rhs) -> bool {
    return lhs.m_wireHit < rhs.m_wireHit;
  });

  // Discard multiple hits on the same wire up to the maximal exceeding the maximal desired number
  if (m_maxNHitOnWire > 0) {
    const CDCWire* lastWire = nullptr;
    size_t nSameWire = 0;
    const size_t maxNHitOnWire = m_maxNHitOnWire;

    auto exceedsMaxNHitOnWire =
    [&lastWire, &nSameWire, maxNHitOnWire](const SimpleSimHit & simpleSimHit) -> bool {

      if (&(simpleSimHit.m_wireHit.getWire()) == lastWire)
      {
        ++nSameWire;
      } else {
        nSameWire = 1;
        lastWire = &(simpleSimHit.m_wireHit.getWire());
      }
      return nSameWire > maxNHitOnWire ? true : false;
    };

    auto itLast = std::remove_if(simpleSimHits.begin(), simpleSimHits.end(), exceedsMaxNHitOnWire);
    simpleSimHits.erase(itLast, simpleSimHits.end());
  }

  // Write the created hits to the their storage place.
  CDCWireHitTopology& wireHitTopology = *m_wireHitTopology;
  {
    wireHitTopology.clear();
    for (SimpleSimHit& simpleSimHit : simpleSimHits) {
      wireHitTopology.m_wireHits.push_back(simpleSimHit.m_wireHit);
    }

    if (not wireHitTopology.m_wireHits.checkSorted()) {
      B2ERROR("Wire hits are not sorted after creation");
    }

    for (const CDCWireHit& wireHit : wireHitTopology.m_wireHits) {
      wireHitTopology.m_rlWireHits.push_back(CDCRLWireHit(&wireHit, LEFT));
      wireHitTopology.m_rlWireHits.push_back(CDCRLWireHit(&wireHit, RIGHT));
    }

    if (not wireHitTopology.m_rlWireHits.checkSorted()) {
      B2ERROR("Oriented wire hits are not sorted after creation");
    }
    // TODO: Decide if the EventMeta should be incremented after write.
  }

  // Now construct the tracks.
  vector<CDCTrack> mcTracks;
  mcTracks.resize(nMCTracks);
  const SortableVector<CDCWireHit>& wireHits = wireHitTopology.getWireHits();
  const size_t nWireHits = wireHits.size();

  for (size_t iWireHit = 0; iWireHit < nWireHits; ++iWireHit) {
    const CDCWireHit& wireHit = wireHits[iWireHit];
    const SimpleSimHit& simpleSimHit = simpleSimHits[iWireHit];

    CDCTrack& mcTrack = mcTracks[simpleSimHit.m_iMCTrack];

    const CDCRLWireHit* rlWireHit = wireHitTopology.getRLWireHit(wireHit, simpleSimHit.m_rlInfo);

    CDCRecoHit3D recoHit3D(rlWireHit, simpleSimHit.m_pos3D, simpleSimHit.m_arcLength2D);
    mcTrack.push_back(recoHit3D);
  }

  /// Sort the hits by the order of their occurance
  for (CDCTrack& mcTrack : mcTracks) {
    mcTrack.sortByPerpS();
  }

  return mcTracks;
}



std::vector<CDCSimpleSimulation::SimpleSimHit>
CDCSimpleSimulation::createHits(const Helix& globalHelix,
                                const FloatType& arcLength2DOffset) const
{

  vector<SimpleSimHit> simpleSimHits;

  CDCWireTopology& wireTopology = CDCWireTopology::getInstance();
  const double outerWallCylinderR = wireTopology.getOuterCylindricalR();

  const FloatType minR = globalHelix.minimalCylindricalR();
  const FloatType maxR = globalHelix.maximalCylindricalR();

  const double globalArcLength2DToOuterWall = globalHelix.arcLength2DToCylindricalR(outerWallCylinderR);
  const double localArcLength2DToOuterWall = arcLength2DOffset + globalArcLength2DToOuterWall;

  if (localArcLength2DToOuterWall < 0) {
    // Trajectory starts outside the CDC and initially flys away from it
    // Do not try to createHit hits for it
    B2WARNING("Simple simulation got trajectory outside CDC that moves away from the detector.");
    return simpleSimHits;
  }

  // Two dimensional arc length where the trajectory
  // * leaves the outer wall of the CDC or
  // * made a full circle (cut off for curlers)
  const bool isCurler = std::isnan(localArcLength2DToOuterWall);
  const double perimeterXY = globalHelix.perimeterXY();
  const double maxArcLength2D = isCurler ? fabs(perimeterXY) : localArcLength2DToOuterWall;

  if (isCurler) {
    B2INFO("Simulating curler");
  }

  for (const CDCWireLayer& wireLayer : wireTopology.getWireLayers()) {
    FloatType outerR = wireLayer.getOuterCylindricalR();
    FloatType innerR = wireLayer.getInnerCylindricalR();

    if ((maxR < innerR) or (outerR < minR)) {
      // Trajectory does not reaching the layer
      continue;
    }

    FloatType centerR = (min(outerR, maxR) + max(innerR, minR)) / 2;

    FloatType globalArcLength2D = globalHelix.arcLength2DToCylindricalR(centerR);
    FloatType localArcLength2D = arcLength2DOffset + globalArcLength2D;


    vector<SimpleSimHit> simpleSimHitsInLayer;
    if (localArcLength2D > 0 and localArcLength2D < maxArcLength2D) {

      Vector3D pos3DAtLayer = globalHelix.atArcLength2D(globalArcLength2D);
      const CDCWire& closestWire =  wireLayer.getClosestWire(pos3DAtLayer);

      simpleSimHitsInLayer = createHitsForLayer(closestWire, globalHelix, arcLength2DOffset);

      for (SimpleSimHit& simpleSimHit : simpleSimHitsInLayer) {
        if (simpleSimHit.m_arcLength2D < maxArcLength2D) {
          simpleSimHits.push_back(simpleSimHit);
        }
      }
    } else {
      B2INFO("Arc length to long");
    }

    bool oneSegment = outerR > maxR or innerR < minR;
    if (not oneSegment) {

      // Check the second branch for more hits
      double secondGlobalArcLength2D = -globalArcLength2D;
      double secondArcLength2DOffset = arcLength2DOffset;
      double secondLocalArcLength2D = secondArcLength2DOffset + secondGlobalArcLength2D;

      if (isCurler and secondLocalArcLength2D < 0) {
        secondLocalArcLength2D += perimeterXY;
        secondArcLength2DOffset += perimeterXY;
        secondGlobalArcLength2D += perimeterXY;
      }

      if (secondLocalArcLength2D > 0 and secondLocalArcLength2D < maxArcLength2D) {
        Vector3D pos3DAtLayer = globalHelix.atArcLength2D(secondGlobalArcLength2D);
        const CDCWire& closestWire =  wireLayer.getClosestWire(pos3DAtLayer);

        // Check again if the wire has been hit before
        bool wireAllReadyHit = false;
        for (const SimpleSimHit& simpleSimHit : simpleSimHits) {
          if (simpleSimHit.m_wireHit.hasWire(closestWire)) {
            wireAllReadyHit = true;
          }
        }
        if (not wireAllReadyHit) {
          std::vector<SimpleSimHit> secondSimpleSimHitsInLayer =
            createHitsForLayer(closestWire, globalHelix, arcLength2DOffset);

          for (SimpleSimHit& simpleSimHit : secondSimpleSimHitsInLayer) {
            if (simpleSimHit.m_arcLength2D < maxArcLength2D) {
              simpleSimHits.push_back(simpleSimHit);
            }
          }
        }
      }
    }
  }

  return simpleSimHits;
}

std::vector<CDCSimpleSimulation::SimpleSimHit>
CDCSimpleSimulation::createHitsForLayer(const CDCWire& nearWire,
                                        const Helix& globalHelix,
                                        const FloatType& arcLength2DOffset) const
{
  std::vector<SimpleSimHit> result;

  SimpleSimHit simpleSimHit = createHitForCell(nearWire, globalHelix, arcLength2DOffset);
  if (not std::isnan(simpleSimHit.m_wireHit.getRefDriftLength())) {
    result.push_back(simpleSimHit);
  }

  /// Iter counter clockwise for more hits
  const CDCWire* ccwWire = nearWire.getNeighborCCW();
  while (true) {
    SimpleSimHit simpleSimHitForWire = createHitForCell(*ccwWire, globalHelix, arcLength2DOffset);
    if (std::isnan(simpleSimHitForWire.m_arcLength2D) or
        std::isnan(simpleSimHitForWire.m_trueDriftLength)) {
      break;
    }
    result.push_back(simpleSimHitForWire);
    ccwWire = ccwWire->getNeighborCCW();
  }

  /// Iter clockwise for more hits
  const CDCWire* cwWire = nearWire.getNeighborCW();
  while (true) {
    SimpleSimHit simpleSimHitForWire = createHitForCell(*cwWire, globalHelix, arcLength2DOffset);
    if (std::isnan(simpleSimHitForWire.m_arcLength2D) or
        std::isnan(simpleSimHitForWire.m_trueDriftLength)) {
      break;
    }
    result.push_back(simpleSimHitForWire);
    cwWire = cwWire->getNeighborCW();
  }

  return result;
}


CDCSimpleSimulation::SimpleSimHit
CDCSimpleSimulation::createHitForCell(const CDCWire& wire,
                                      const Helix& globalHelix,
                                      const FloatType& arcLength2DOffset) const
{
  double arclength2D = globalHelix.arcLength2DToXY(wire.getRefPos2D());
  if ((arclength2D + arcLength2DOffset) < 0) {
    arclength2D += globalHelix.perimeterXY();
  }

  Vector3D pos3D = globalHelix.atArcLength2D(arclength2D);

  // Iterate the extrapolation once more to the stereo shifted position.
  const Vector2D correctedWirePos = wire.getWirePos2DAtZ(pos3D.z());
  double correctedArcLength2D = globalHelix.arcLength2DToXY(correctedWirePos);

  if ((correctedArcLength2D + arcLength2DOffset) < 0) {
    correctedArcLength2D += globalHelix.perimeterXY();
  }

  Vector3D correctedPos3D = globalHelix.atArcLength2D(correctedArcLength2D);

  const double trueDriftLength = wire.getDriftLength(correctedPos3D);
  const double smearedDriftLength = trueDriftLength + gRandom->Gaus(0, m_driftLengthSigma) * 0;

  double delayTime = getEventTime();
  if (m_addTOFDelay) {
    double arcLength3D = std::hypot(1, globalHelix.tanLambda()) * (correctedArcLength2D + arcLength2DOffset);
    delayTime += arcLength3D / Const::speedOfLight;
  }

  if (m_addInWireSignalDelay) {
    double backwardZ = wire.getBackwardZ();
    // Position where wire has been hit
    Vector3D wirePos = wire.getClosest(correctedPos3D);
    double distanceToBack = (wirePos.z() - backwardZ) * hypot(1, wire.getSkew() * wire.getRefCylindricalR());

    delayTime += distanceToBack / m_propSpeed;
  }

  double measuredDriftLength = smearedDriftLength + delayTime * m_driftSpeed;

  RightLeftInfo rlInfo = globalHelix.circleXY().isRightOrLeft(correctedWirePos);

  // if (not std::isnan(trueDriftLength)){
  //   B2INFO("Delay time " << delayTime);
  //   B2INFO("True dirft length " << trueDriftLength);
  //   B2INFO("Measured drift length " << measuredDriftLength);
  //   B2INFO("Absolute deviation " << measuredDriftLength - trueDriftLength);
  //   B2INFO("Relative deviation " << (measuredDriftLength / trueDriftLength - 1) * 100 << "%");
  // }

  return SimpleSimHit{
    CDCWireHit(wire.getWireID(), measuredDriftLength, m_driftLengthVariance),
    0,
    rlInfo,
    correctedPos3D,
    correctedArcLength2D,
    trueDriftLength
  };
}


std::vector<Belle2::TrackFindingCDC::CDCTrack>
CDCSimpleSimulation::loadPreparedEvent() const
{
  const size_t nMCTracks = 2;
  vector<SimpleSimHit> simpleSimHits;
  simpleSimHits.reserve(128 + 64); // First plus second mc track

  // First MC track
  /////////////////////////////////////////////////////////////////////////////////////////////
  size_t iMCTrack = 0;

  // SL 6
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 4, 251), 0.104), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 4, 250), 0.272), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 4, 249), 0.488), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 4, 248), 0.764), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 247), 0.9), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 4, 247), 1.024), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 246), 0.64), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 245), 0.304), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 244), 0.012), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 243), 0.352), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 3, 242), 0.74), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 241), 0.46), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 240), 0.02), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 239), 0.46), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 238), 0.884), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 238), 1.104), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 237), 0.612), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 236), 0.12), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 235), 0.356), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 234), 0.884), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 0, 235), 1.032), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 0, 234), 0.52), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 0, 233), 0.06), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 0, 232), 0.62), iMCTrack, LEFT});

  // SL 5
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 5, 206), 1.116), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 5, 205), 0.464), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 5, 204), 0.168), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 4, 204), 1.08), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 4, 203), 0.392), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 4, 202), 0.304), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 3, 201), 0.968), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 3, 200), 0.252), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 3, 199), 0.476), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 2, 199), 0.736), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 2, 198), 0.008), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 2, 197), 0.788), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 1, 197), 1.188), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 1, 196), 0.404), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 1, 195), 0.356), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 0, 195), 0.74), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 0, 194), 0.04), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 0, 193), 0.832), iMCTrack, LEFT});

  // SL 4
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 5, 173), 0.692), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 5, 172), 0.22), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 5, 171), 1.132), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 4, 172), 0.816), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 4, 171), 0.136), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 4, 170), 1.048), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 3, 170), 0.884), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 3, 169), 0.032), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 3, 168), 0.96), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 2, 169), 0.972), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 2, 168), 0.044), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 2, 167), 0.872), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 1, 167), 1.004), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 1, 166), 0.1), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 1, 165), 0.828), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 0, 166), 1.004), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 0, 165), 0.084), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 0, 164), 0.82), iMCTrack, LEFT});

  // SL 3
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 5, 145), 0.508), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 5, 144), 0.5), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 4, 145), 1.348), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 4, 144), 0.292), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 4, 143), 0.68), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 3, 143), 1.136), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 3, 142), 0.12), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 3, 141), 0.872), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 2, 142), 0.96), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 2, 141), 0.036), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 1, 140), 0.756), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 1, 139), 0.204), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 0, 139), 0.588), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 0, 138), 0.332), iMCTrack, LEFT});

  // SL 2
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 5, 116), 1.1), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 5, 115), 0.008), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 5, 114), 1.048), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 4, 115), 0.712), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 4, 114), 0.316), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 3, 113), 0.428), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 3, 112), 0.572), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 2, 112), 0.188), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 2, 111), 0.776), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 1, 111), 0.92), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 1, 110), 0.024), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 1, 109), 0.928), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 0, 110), 0.776), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 0, 109), 0.116), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 0, 108), 0.992), iMCTrack, LEFT});

  // SL 1
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 5, 87), 0.664), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 5, 86), 0.3), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 4, 86), 0.504), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 4, 85), 0.424), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 3, 85), 1.256), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 3, 84), 0.388), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 3, 83), 0.5), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 2, 84), 1.128), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 2, 83), 0.28), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 2, 82), 0.532), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 1, 82), 1.084), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 1, 81), 0.3), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 1, 80), 0.472), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 81), 1.124), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 80), 0.428), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 79), 0.296), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 78), 0.972), iMCTrack, LEFT});

  // SL 0
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 7, 81), 0.192), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 7, 80), 0.452), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 6, 80), 0.596), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 6, 79), 0.024), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 6, 78), 0.66), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 5, 79), 0.388), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 5, 78), 0.184), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 4, 77), 0.296), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 4, 76), 0.244), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 3, 76), 0.268), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 3, 75), 0.212), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 2, 74), 0.316), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 2, 73), 0.112), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 2, 72), 0.588), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 1, 73), 0.464), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 1, 72), 0.028), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 0, 70), 0.284), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 0, 69), 0.088), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 0, 68), 0.416), iMCTrack, LEFT});

  // Second MC track
  /////////////////////////////////////////////////////////////////////////////////////////////
  iMCTrack = 1;

  // SL 0
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 1, 140), 0.308), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 2, 139), 0.08), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 3, 139), 0.16), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 4, 139), 0.404), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 4, 138), 0.38), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 5, 139), 0.132), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 6, 138), 0.108), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 7, 139), 0.48), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(0, 7, 138), 0.424), iMCTrack, RIGHT});

  // SL 1
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 136), 0.532), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 0, 135), 0.452), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 1, 135), 0.396), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 2, 135), 0.26), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 1, 134), 0.64), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 3, 134), 0.092), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 4, 134), 0.16), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(1, 5, 133), 0.524), iMCTrack, RIGHT});

  // SL 2
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 0, 163), 0.228), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 1, 162), 0.356), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 2, 163), 0.776), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 2, 162), 0.46), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 3, 162), 0.744), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 3, 161), 0.58), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 4, 162), 0.656), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 4, 161), 0.68), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 5, 161), 0.568), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(2, 5, 160), 0.812), iMCTrack, RIGHT});

  // SL 3
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 0, 190), 0.54), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 1, 188), 0.688), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 2, 188), 0.656), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 3, 188), 0.664), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 3, 187), 0.68), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 4, 188), 0.724), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 4, 187), 0.656), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(3, 5, 186), 0.676), iMCTrack, RIGHT});

  // SL 4
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 0, 211), 0.42), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 0, 210), 0.872), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 1, 210), 0.548), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 1, 209), 0.796), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 2, 210), 0.716), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 2, 209), 0.656), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 3, 209), 0.856), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 4, 209), 1.056), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 4, 208), 0.36), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(4, 5, 207), 0.232), iMCTrack, RIGHT});

  // SL 5
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 0, 231), 0.224), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 0, 230), 1.088), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 1, 230), 0.452), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 1, 229), 0.912), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 2, 230), 0.72), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 2, 229), 0.632), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 3, 229), 1.016), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 3, 228), 0.34), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 4, 228), 0.04), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 5, 227), 0.22), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(5, 5, 226), 1.196), iMCTrack, RIGHT});

  // SL 6
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 0, 254), 0.104), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 253), 0.504), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 1, 252), 0.78), iMCTrack, RIGHT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 253), 0.968), iMCTrack, LEFT});
  simpleSimHits.push_back(SimpleSimHit{CDCWireHit(WireID(6, 2, 252), 0.332), iMCTrack, RIGHT});

  vector<CDCTrack> mcTracks = constructMCTracks(nMCTracks, std::move(simpleSimHits));
  return mcTracks;
}
