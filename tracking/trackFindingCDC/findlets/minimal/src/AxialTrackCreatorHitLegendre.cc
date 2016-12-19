/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/AxialTrackCreatorHitLegendre.h>

#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.h>
#include <tracking/trackFindingCDC/hough/perigee/AxialLegendreLeafProcessor.icc.h>
#include <tracking/trackFindingCDC/hough/perigee/StandardBinSpec.h>

#include <tracking/trackFindingCDC/processing/TrackProcessor.h>
#include <tracking/trackFindingCDC/processing/TrackMerger.h>
#include <tracking/trackFindingCDC/processing/TrackQualityTools.h>
#include <tracking/trackFindingCDC/processing/HitProcessor.h>

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/utilities/MakeUnique.h>

#include <framework/core/ModuleParamList.h>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string AxialTrackCreatorHitLegendre::getDescription()
{
  return "Generates axial tracks from segments using a hough space over phi0 impact and curvature for the spares case.";
}

void AxialTrackCreatorHitLegendre::exposeParameters(ModuleParamList* moduleParamList,
                                                    const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "granularityLevel"),
                                m_param_granularityLevel,
                                "Level of divisions in the hough space.",
                                m_param_granularityLevel);

  moduleParamList->addParameter(prefixed(prefix, "sectorLevelSkip"),
                                m_param_sectorLevelSkip,
                                "Number of levels to be skipped on the first level to form sectors",
                                m_param_sectorLevelSkip);

  moduleParamList->addParameter(prefixed(prefix, "fineCurvBounds"),
                                m_param_fineCurvBounds,
                                "Curvature bounds of the fine hough space.",
                                m_param_fineCurvBounds);

  moduleParamList->addParameter(prefixed(prefix, "roughCurvBounds"),
                                m_param_roughCurvBounds,
                                "Curvature bounds of the rough hough space.",
                                m_param_roughCurvBounds);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Width"),
                                m_param_discretePhi0Width,
                                "Width of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Width);

  moduleParamList->addParameter(prefixed(prefix, "discretePhi0Overlap"),
                                m_param_discretePhi0Overlap,
                                "Overlap of the phi0 bins at the lowest level of the hough space.",
                                m_param_discretePhi0Overlap);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvWidth"),
                                m_param_discreteCurvWidth,
                                "Width of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvWidth);

  moduleParamList->addParameter(prefixed(prefix, "discreteCurvOverlap"),
                                m_param_discreteCurvOverlap,
                                "Overlap of the curvature bins at the lowest level of the hough space.",
                                m_param_discreteCurvOverlap);

  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Absolute minimal number of hits to make an axial track.",
                                m_param_minNHits);
}

void AxialTrackCreatorHitLegendre::initialize()
{
  Super::initialize();

  B2ASSERT("Need exactly two fine curv bounds", m_param_fineCurvBounds.size() == 2);
  B2ASSERT("Need exactly two rough curv bounds", m_param_roughCurvBounds.size() == 2);

  const size_t nPhi0Bins = std::pow(c_phi0Divisions, m_param_granularityLevel);
  const Phi0BinsSpec phi0BinsSpec(nPhi0Bins,
                                  m_param_discretePhi0Overlap,
                                  m_param_discretePhi0Width);

  {
    std::array<double, 2> fineCurvBounds{{m_param_fineCurvBounds.front(), m_param_fineCurvBounds.back()}};
    const size_t nFineCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
    const CurvBinsSpec fineCurvBinsSpec(fineCurvBounds[0],
                                        fineCurvBounds[1],
                                        nFineCurvBins,
                                        m_param_discreteCurvOverlap,
                                        m_param_discreteCurvWidth);

    int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
    m_fineHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
    m_fineHoughTree->setSectorLevelSkip(m_param_sectorLevelSkip);
    m_fineHoughTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    m_fineHoughTree->assignArray<DiscreteCurv>(fineCurvBinsSpec.constructArray(), fineCurvBinsSpec.getNOverlap());
    m_fineHoughTree->initialize();
  }

  {
    std::array<double, 2> roughCurvBounds{{m_param_roughCurvBounds.front(), m_param_roughCurvBounds.back()}};
    const size_t nRoughCurvBins = std::pow(c_curvDivisions, m_param_granularityLevel);
    const CurvBinsSpec roughCurvBinsSpec(roughCurvBounds[0],
                                         roughCurvBounds[1],
                                         nRoughCurvBins,
                                         m_param_discreteCurvOverlap,
                                         m_param_discreteCurvWidth);

    int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
    m_roughHoughTree = makeUnique<SimpleRLTaggedWireHitPhi0CurvHough>(maxTreeLevel, m_curlCurv);
    // No level skip !
    m_roughHoughTree->assignArray<DiscretePhi0>(phi0BinsSpec.constructArray(), phi0BinsSpec.getNOverlap());
    m_roughHoughTree->assignArray<DiscreteCurv>(roughCurvBinsSpec.constructArray(), roughCurvBinsSpec.getNOverlap());
    m_roughHoughTree->initialize();
  }
}

void AxialTrackCreatorHitLegendre::apply(const std::vector<CDCWireHit>& wireHits,
                                         std::vector<CDCTrack>& tracks)
{
  // Acquire the axial hits
  std::vector<const CDCWireHit*> axialWireHits;
  axialWireHits.reserve(wireHits.size());
  for (const CDCWireHit& wireHit : wireHits) {
    if (not wireHit.isAxial()) continue;
    axialWireHits.emplace_back(&wireHit);
  }

  // Setup the level processor and obtain its parameter list to be set.
  using Node = typename SimpleRLTaggedWireHitPhi0CurvHough::Node;
  int maxTreeLevel = m_param_granularityLevel - m_param_sectorLevelSkip;
  AxialLegendreLeafProcessor<Node> leafProcessor(maxTreeLevel);
  leafProcessor.setAxialWireHits(axialWireHits);
  ModuleParamList moduleParamList;
  const std::string prefix = "";
  leafProcessor.exposeParameters(&moduleParamList, prefix);

  // Find tracks with increasingly relaxed conditions in the fine hough grid
  m_fineHoughTree->seed(leafProcessor.getUnusedWireHits());
  for (const ParameterVariantMap& passParameters : getFineRelaxationSchedule()) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    leafProcessor.beginWalk();
    m_fineHoughTree->findUsing(leafProcessor);
  }
  m_fineHoughTree->fell();

  // One step of migrating hits between the already found tracks
  leafProcessor.migrateHits();

  /*
  // Find tracks with increasingly relaxed conditions in the rougher hough grid
  m_roughHoughTree->seed(leafProcessor.getUnusedWireHits());
  for (const ParameterVariantMap& passParameters : getRoughRelaxationSchedule()) {
    AssignParameterVisitor::update(&moduleParamList, passParameters);
    leafProcessor.beginWalk();
    m_roughHoughTree->findUsing(leafProcessor);
  }
  m_roughHoughTree->fell();

  // One step of migrating hits between the already found tracks
  leafProcessor.migrateHits();
  */

  leafProcessor.finalizeTracks();

  std::list<CDCTrack> foundTracks = leafProcessor.getTracks();
  tracks.insert(tracks.end(), foundTracks.begin(), foundTracks.end());
  return;

  // Pick up the found candidates and make tracks from them
  std::vector<std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit>>> candidates =
    leafProcessor.getCandidates();

  for (const std::pair<CDCTrajectory2D, std::vector<CDCRLWireHit>>& candidate : candidates) {
    const CDCTrajectory2D& trajectory2D = candidate.first;
    const std::vector<CDCRLWireHit>& foundRLWireHits = candidate.second;
    CDCTrack track;
    for (const CDCRLWireHit& rlWireHit : foundRLWireHits) {
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(&rlWireHit.getWireHit(), trajectory2D);
      // CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstruct(rlWireHit, trajectory2D);
      track.push_back(recoHit3D);
    }
    track.sortByArcLength2D();

    /// Setting trajectories
    if (track.empty()) continue;
    const CDCRecoHit3D& startRecoHit3D = track.front();
    CDCTrajectory3D startTrajectory3D(trajectory2D);
    startTrajectory3D.setLocalOrigin(startRecoHit3D.getRecoPos3D());
    track.setStartTrajectory3D(startTrajectory3D);

    const CDCRecoHit3D& endRecoHit3D = track.back();
    CDCTrajectory3D endTrajectory3D(trajectory2D);
    endTrajectory3D.setLocalOrigin(endRecoHit3D.getRecoPos3D());
    track.setEndTrajectory3D(endTrajectory3D);

    tracks.push_back(std::move(track));
  }
}

void AxialTrackCreatorHitLegendre::terminate()
{
  m_fineHoughTree->raze();
  m_fineHoughTree.reset();
  m_roughHoughTree->raze();
  m_roughHoughTree.reset();
  Super::terminate();
}

std::vector<ParameterVariantMap>
AxialTrackCreatorHitLegendre::getFineRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;
  // Relaxation schedule of the original legendre implemenation
  // Augmented by the road search parameters
  // Note: distinction between integer and float literals is necessary
  // For the record: the setting seem a bit non-sensical

  // NonCurler pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 12 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("origin")},
    {"nRoadSearches", 1},
    {"roadLevel", 4 - m_param_sectorLevelSkip},

  });


  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 12 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", 0.07},
      {"curvResolution", std::string("origin")},
      {"nRoadSearches", 1},
      {"roadLevel", 4 - m_param_sectorLevelSkip},
    });
  }

  return result;
  B2ERROR("nPass " << result.size());

  // Skipping other passes for the moment
  return result;

  // NonCurlerWithIncreasedThreshold pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},
  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10 - m_param_sectorLevelSkip},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 2},
    {"roadLevel", 4 - m_param_sectorLevelSkip},

  });

  int iPass = 0;
  for (double minWeight = 50.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10 - m_param_sectorLevelSkip},
      {"minWeight", minWeight},
      {"maxCurv", iPass == 0 ? 0.07 : 0.14},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 2},
      {"roadLevel", 4 - m_param_sectorLevelSkip},
    });
    ++iPass;
  }
  return result;
}

std::vector<ParameterVariantMap>
AxialTrackCreatorHitLegendre::getRoughRelaxationSchedule() const
{
  std::vector<ParameterVariantMap> result;

  // FullRange pass
  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 50.0},
    {"maxCurv", 1.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},

  });

  result.push_back(ParameterVariantMap{
    {"maxLevel", 10},
    {"minWeight", 70.0},
    {"maxCurv", 2.0 * m_curlCurv},
    {"curvResolution", std::string("nonOrigin")},
    {"nRoadSearches", 3},
    {"roadLevel", 0},

  });

  for (double minWeight = 30.0; minWeight > 10.0; minWeight *= 0.75) {
    result.push_back(ParameterVariantMap{
      {"maxLevel", 10},
      {"minWeight", minWeight},
      {"maxCurv", 0.15},
      {"curvResolution", std::string("nonOrigin")},
      {"nRoadSearches", 3},
      {"roadLevel", 0},
    });
  }

  return result;
}
