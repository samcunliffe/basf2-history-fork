#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <utility>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherQuadTree.h>

using namespace Belle2;
using namespace TrackFindingCDC;

template <class HoughTree>
void StereoHitTrackMatcherQuadTree<HoughTree>::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  QuadTreeBasedMatcher<HoughTree>::exposeParameters(moduleParameters, prefix);
  m_filterFactory.exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefixed(prefix, "checkForB2BTracks"), m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics).",
                                 m_param_checkForB2BTracks);
  moduleParameters->addParameter(prefixed(prefix, "checkForInWireBoundsFactor"), m_param_checkForInWireBoundsFactor,
                                 "Used to scale the CDC, before checking of a hit is in the CDC z bounds.",
                                 m_param_checkForInWireBoundsFactor);
}

template <class HoughTree>
Weight StereoHitTrackMatcherQuadTree<HoughTree>::getWeight(const CDCRecoHit3D& recoHit, const Z0TanLambdaBox&,
                                                           const CDCTrack& track) const
{
  return m_stereoHitFilter->operator()({&recoHit, &track});
}

template <class HoughTree>
std::vector<WithWeight<const CDCRLWireHit*>> StereoHitTrackMatcherQuadTree<HoughTree>::match(const CDCTrack& track,
                                          const std::vector<CDCRLWireHit>& rlWireHits)
{
  if (m_stereoHitFilter->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // Reconstruct the hits to the track
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  using CDCRecoHitWithRLPointer = std::pair<CDCRecoHit3D, const CDCRLWireHit*>;
  std::vector<CDCRecoHitWithRLPointer> recoHits;
  recoHits.reserve(rlWireHits.size() + track.size());

  /*
   * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
   * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
   */
  for (const CDCRLWireHit& rlWireHit : rlWireHits) {
    if (not rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      // Skip hits out of CDC
      if (not wire.isInCellZBounds(recoPos3D, m_param_checkForInWireBoundsFactor)) {
        continue;
      }

      // If the track is a curler, shift all perpS values to positive ones. Else do not use this hit if m_param_checkForB2BTracks is enabled.
      double perpS = trajectory2D.calcArcLength2D(recoPos3D.xy());
      if (perpS < 0) {
        if (isCurler) {
          perpS += 2 * TMath::Pi() * radius;
        } else if (m_param_checkForB2BTracks) {
          continue;
        }
      }
      recoHits.emplace_back(CDCRecoHit3D(rlWireHit, recoPos3D, perpS), &rlWireHit);
    }
  }

  for (const CDCRecoHit3D& recoHit : track) {
    if (not recoHit.isAxial()) {
      recoHit.getWireHit().getAutomatonCell().setAssignedFlag();
      const CDCRLWireHit& rlWireHit = recoHit.getRLWireHit();
      recoHits.emplace_back(recoHit, &rlWireHit);
    }
  }

  // Do the tree finding
  Super::m_quadTreeInstance.seed(recoHits);

  if (Super::m_param_writeDebugInformation) {
    Super::writeDebugInformation();
  }

  const auto& foundStereoHitsWithNode = Super::m_quadTreeInstance.findSingleBest(Super::m_param_minimumNumberOfHits);
  Super::m_quadTreeInstance.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return {};

  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;

  for (const auto& recoHit : foundStereoHits) {
    recoHit.first.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  // Copy all usable hits (not the duplicates) into this list.
  std::vector<WithWeight<const CDCRLWireHit*>> matches;

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    bool isDoubled = false;

    const CDCRecoHit3D& currentRecoHitOuter = outerIterator->first;
    const CDCRLWireHit* currentRLWireHitOuter = outerIterator->second;
    const CDCWireHit& currentWireHitOuter = currentRLWireHitOuter->getWireHit();
    const CDCHit* currentHitOuter = currentWireHitOuter.getHit();

    if (currentWireHitOuter.getAutomatonCell().hasMaskedFlag() or currentWireHitOuter.getAutomatonCell().hasAssignedFlag()) {
      continue;
    }

    for (auto innerIterator = outerIterator; innerIterator != foundStereoHits.end(); ++innerIterator) {
      const CDCRecoHit3D& currentRecoHitInner = innerIterator->first;
      const CDCRLWireHit* currentRLWireHitInner = innerIterator->second;
      const CDCWireHit& currentWireHitInner = currentRLWireHitInner->getWireHit();
      const CDCHit* currentHitInner = currentWireHitInner.getHit();

      if (currentWireHitInner.getAutomatonCell().hasMaskedFlag() or currentWireHitInner.getAutomatonCell().hasAssignedFlag()) {
        continue;
      }

      if (innerIterator != outerIterator and currentHitOuter == currentHitInner) {
        const double innerZ = currentRecoHitInner.getRecoZ();
        const double outerZ = currentRecoHitOuter.getRecoZ();

        const double innerR = currentRecoHitInner.getRecoPos2D().norm();
        const double outerR = currentRecoHitOuter.getRecoPos2D().norm();

        const double lambda11 = (innerZ - node.getLowerZ0()) / innerR;
        const double lambda12 = (innerZ - node.getUpperZ0()) / innerR;

        const double lambda21 = (outerZ - node.getLowerZ0()) / outerR;
        const double lambda22 = (outerZ - node.getUpperZ0()) / outerR;

        const double zSlopeMean = (node.getLowerTanLambda() + node.getUpperTanLambda()) / 2.0;

        if (fabs((lambda11 + lambda12) / 2 - zSlopeMean) < fabs((lambda21 + lambda22) / 2 - zSlopeMean)) {
          const Weight weight = getWeight(currentRecoHitInner, node, track);
          if (not std::isnan(weight)) {
            matches.emplace_back(currentRLWireHitInner, weight);
          }
        } else {
          const Weight weight = getWeight(currentRecoHitOuter, node, track);
          if (not std::isnan(weight)) {
            matches.emplace_back(currentRLWireHitOuter, weight);
          }
        }

        // currentWireHitInner = currentWireHitOuter, so it makes no difference here
        currentWireHitInner.getAutomatonCell().setMaskedFlag();
        isDoubled = true;
        break;
      }
    }

    if (not isDoubled) {
      const Weight weight = getWeight(currentRecoHitOuter, node, track);
      if (not std::isnan(weight)) {
        matches.emplace_back(currentRLWireHitOuter, weight);
      }
    }
  }

  for (const auto& recoHit : foundStereoHits) {
    recoHit.first.getWireHit().getAutomatonCell().unsetMaskedFlag();
    recoHit.first.getWireHit().getAutomatonCell().unsetAssignedFlag();
  }

  return matches;
}

template class Belle2::TrackFindingCDC::StereoHitTrackMatcherQuadTree<HitZ0TanLambdaLegendre>;
template class Belle2::TrackFindingCDC::StereoHitTrackMatcherQuadTree<HitZ0TanLambdaLegendreUsingZ>;
