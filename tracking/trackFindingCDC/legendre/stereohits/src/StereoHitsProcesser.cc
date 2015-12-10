#include <tracking/trackFindingCDC/legendre/stereohits/StereoHitsProcesser.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventtopology/CDCWireHitTopology.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>
#include <tracking/trackFindingCDC/fitting/CDCSZFitter.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

namespace {
  /** Returns a bool if the rlWire can be used as a stereo hit. */
  bool isValidStereoHit(const CDCWireHit& wireHit)
  {
    return not(wireHit.getStereoType() == StereoType::c_Axial or wireHit.getAutomatonCell().hasTakenFlag());
    // TODO Check for number of layers in between
  }

  /** Returns a bool if the segment can be used as a stereo hit. */
  bool isValidStereoSegment(const CDCRecoSegment2D& segment)
  {
    // Skip axial segments
    return not(segment.getStereoType() == StereoType::c_Axial or segment.isFullyTaken());
  }
}

void StereoHitsProcesser::reconstructHit(const CDCRLTaggedWireHit& rlWireHit, std::vector<CDCRecoHit3D>& hitsVector,
                                         const CDCTrajectory2D& trackTrajectory, const bool isCurler, const double radius) const
{
  Vector3D recoPos3D = rlWireHit.reconstruct3D(trackTrajectory);
  const CDCWire& wire = rlWireHit.getWire();
  // Skip hits out of CDC
  if (not wire.isInCellZBounds(recoPos3D)) {
    return;
  }

  // If the track is a curler, shift all perpS values to positive ones. Else do not use this hit if m_param_checkForB2BTracks is enabled.
  double perpS = trackTrajectory.calcArcLength2D(recoPos3D.xy());
  if (perpS < 0) {
    if (isCurler) {
      perpS += 2 * TMath::Pi() * radius;
    } else if (m_param_checkForB2BTracks) {
      return;
    }
  }
  hitsVector.emplace_back(rlWireHit, recoPos3D, perpS);
}

void StereoHitsProcesser::reconstructSegment(const CDCRecoSegment2D& segment,
                                             std::vector<const CDCRecoSegment3D*>& recoSegmentVector,
                                             const CDCTrajectory2D& trackTrajectory) const
{
  const double radius = trackTrajectory.getGlobalCircle().absRadius();
  const bool isCurler = trackTrajectory.isCurler();

  CDCRecoSegment3D newRecoSegment = CDCRecoSegment3D::reconstruct(segment, trackTrajectory);

  // Skip segments out of the CDC
  unsigned int numberOfHitsNotInCDC = 0;
  for (const CDCRecoHit3D& recoHit : newRecoSegment) {
    if (not recoHit.isInCellZBounds(1.1)) {
      numberOfHitsNotInCDC++;
    }
  }

  // Do not add this segment if all of the hits are out of the CDC
  if (numberOfHitsNotInCDC > newRecoSegment.size() - 1) {
    return;
  }

  // Skip segments with hits on the wrong side (if not curlers)
  bool oneHitIsOnWrongSide = false;
  for (CDCRecoHit3D& recoHit : newRecoSegment) {
    const double currentArcLength2D = recoHit.getArcLength2D();
    if (currentArcLength2D < 0) {
      oneHitIsOnWrongSide = true;
      recoHit.setArcLength2D(currentArcLength2D + 2 * TMath::Pi() * radius);
    }
  }

  if (not isCurler and m_param_checkForB2BTracks and oneHitIsOnWrongSide) {
    return;
  }

  CDCRecoSegment3D* newRecoSegmentPtr = new CDCRecoSegment3D(newRecoSegment);
  recoSegmentVector.push_back(newRecoSegmentPtr);
}


void StereoHitsProcesser::fillHitsVector(std::vector<CDCRecoHit3D>& hitsVector, const CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.getOuterExit().hasNAN();

  const CDCWireHitTopology& wireHitTopology = CDCWireHitTopology::getInstance();
  const auto& wireHits = wireHitTopology.getWireHits();
  hitsVector.reserve(2 * wireHits.size());

  for (const CDCWireHit& wireHit : wireHits) {
    // Try reconstucting the hit with left passage
    for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
      if (isValidStereoHit(wireHit)) {
        CDCRLTaggedWireHit rlWireHit(&wireHit, rlInfo);
        reconstructHit(rlWireHit, hitsVector, trajectory2D, isCurler, radius);
      }
    }
  }
}

void StereoHitsProcesser::fillSegmentsVector(std::vector<const CDCRecoSegment3D*> recoSegmentsVector,
                                             const std::vector<CDCRecoSegment2D>& segments, CDCTrack& track) const
{
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();

  recoSegmentsVector.reserve(segments.size());

  for (const CDCRecoSegment2D& segment : segments) {
    if (isValidStereoSegment(segment)) {
      // Reverse the segments if needed
      if (segment.getTrajectory2D().getChargeSign() == trajectory2D.getChargeSign()) {
        reconstructSegment(segment, recoSegmentsVector, trajectory2D);
      } else {
        reconstructSegment(std::move(segment.reversed()), recoSegmentsVector, trajectory2D);
      }
    }
  }
}

void StereoHitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track)
{
  // Reconstruct the hits to the track
  std::vector<CDCRecoHit3D> recoHits;
  fillHitsVector(recoHits, track);

  // Do the tree finding
  m_hitQuadTree.seed(recoHits);
  const auto& foundStereoHitsWithNode = m_hitQuadTree.findSingleBest(m_param_minimumNumberOfHits);
  m_hitQuadTree.fell();

  if (foundStereoHitsWithNode.size() != 1)
    return;


  // There is the possibility that we have added one cdc hits twice (as left and right one). We search for those cases here:
  auto foundStereoHits = foundStereoHitsWithNode[0].second;
  auto node = foundStereoHitsWithNode[0].first;


  for (const CDCRecoHit3D& recoHit : foundStereoHits) {
    recoHit.getWireHit().getAutomatonCell().unsetMaskedFlag();
  }

  // Copy all usable hits (not the duplicates) into this list.
  std::vector<CDCRecoHit3D> hitsToUse;

  for (auto outerIterator = foundStereoHits.begin(); outerIterator != foundStereoHits.end();
       ++outerIterator) {
    bool isDoubled = false;

    const CDCHit* currentHitOuter = (*outerIterator)->getWireHit().getHit();

    if (outerIterator->getWireHit().getAutomatonCell().hasMaskedFlag()) {
      continue;
    }

    for (auto innerIterator = outerIterator; innerIterator != foundStereoHits.end(); ++innerIterator) {
      if (innerIterator->getWireHit().getAutomatonCell().hasMaskedFlag()) {
        continue;
      }

      const CDCHit* currentHitInner = (*innerIterator)->getWireHit().getHit();

      if (innerIterator != outerIterator and currentHitOuter == currentHitInner) {
        const double lambda11 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda12 = 1 / (*innerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());
        const double lambda21 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getLowerZ0());
        const double lambda22 = 1 / (*outerIterator)->calculateZSlopeWithZ0(node.getUpperZ0());

        const double zSlopeMean = (node.getLowerTanLambda() + node.getUpperTanLambda()) / 2.0;

        if (fabs((lambda11 + lambda12) / 2 - zSlopeMean) < fabs((lambda21 + lambda22) / 2 - zSlopeMean)) {
          hitsToUse.push_back(*innerIterator);
          innerIterator->getWireHit().getAutomatonCell().setMaskedFlag();
          isDoubled = true;
          break;
        } else {
          hitsToUse.push_back(*outerIterator);
          outerIterator->getWireHit().getAutomatonCell().setMaskedFlag();
          isDoubled = true;
          break;
        }
      }
    }

    if (not isDoubled) {
      hitsToUse.push_back(*outerIterator);
    }
  }

  // Add the hits to the track
  for (const CDCRecoHit3D& recoHit : hitsToUse) {
    B2ASSERT("Found hit should not have taken flag!", not recoHit->getWireHit().getAutomatonCell().hasTakenFlag());
    track.push_back(recoHit);
    recoHit->getWireHit().getAutomatonCell().setTakenFlag();
  }
}


void StereoHitsProcesser::addStereoHitsWithQuadTree(CDCTrack& track, const std::vector<CDCRecoSegment2D>& segments)
{
  // Reconstruct the segments
  std::vector<const CDCRecoSegment3D*> recoSegments;
  fillSegmentsVector(recoSegments, segments, track);

  // Do the tree finding
  m_segmentQuadTree.seed(recoSegments);
  const auto& foundStereoSegmentsWithNode = m_segmentQuadTree.findSingleBest(m_param_minimumNumberOfHits);
  m_segmentQuadTree.fell();

  if (foundStereoSegmentsWithNode.size() != 1) {
    return;
  }

  // Add the hits of the segments to the track
  const auto& foundStereoSegmentWithNode = foundStereoSegmentsWithNode[0];
  const std::vector<const CDCRecoSegment3D*>& foundSegments = foundStereoSegmentWithNode.second;

  for (const CDCRecoSegment3D* segment : foundSegments) {
    for (const CDCRecoHit3D& recoHit : *segment) {
      B2ASSERT("Found hit should not have taken flag!", not recoHit.getWireHit().getAutomatonCell().hasTakenFlag());
      track.push_back(recoHit);
      recoHit.getWireHit().getAutomatonCell().setTakenFlag();
    }
  }

  // Delete the list of found segments and free memory
  for (const CDCRecoSegment3D* recoSegment : recoSegments) {
    delete recoSegment;
  }
}

void StereoHitsProcesser::postprocessTrack(CDCTrack& track) const
{
  const CDCSZFitter& szFitter = CDCSZFitter::getFitter();

  track.shiftToPositiveArcLengths2D();
  track.sortByArcLength2D();

  const CDCTrajectorySZ& szTrajectory = szFitter.fitWithStereoHits(track);
  CDCTrajectory3D newStartTrajectory(track.getStartTrajectory3D().getTrajectory2D(), szTrajectory);
  track.setStartTrajectory3D(newStartTrajectory);
}
