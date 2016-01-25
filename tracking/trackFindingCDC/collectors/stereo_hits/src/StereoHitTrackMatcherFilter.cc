#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment3D.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLTaggedWireHit.h>

#include <tracking/trackFindingCDC/mclookup/CDCMCManager.h>

#include <utility>
#include <tracking/trackFindingCDC/collectors/stereo_hits/StereoHitTrackMatcherFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void StereoHitTrackMatcherFilter::exposeParameters(ModuleParamList* moduleParameters, const std::string& prefix)
{
  m_filterFactory.exposeParameters(moduleParameters, prefix);

  moduleParameters->addParameter(prefix + "checkForB2BTracks", m_param_checkForB2BTracks,
                                 "Set to false to skip the check for back-2-back tracks (good for cosmics)",
                                 m_param_checkForB2BTracks);

}

std::vector<WithWeight<const CDCRLTaggedWireHit*>> StereoHitTrackMatcherFilter::match(const CDCTrack& track,
                                                const std::vector<CDCRLTaggedWireHit>& rlWireHits)
{
  if (m_stereoHitFilter->needsTruthInformation()) {
    CDCMCManager::getInstance().fill();
  }

  // List of matches
  std::vector<WithWeight<const CDCRLTaggedWireHit*>> matches;

  // Reconstruct the hits to the track and check their filter result
  const CDCTrajectory2D& trajectory2D = track.getStartTrajectory3D().getTrajectory2D();
  const double radius = trajectory2D.getGlobalCircle().absRadius();
  const bool isCurler = trajectory2D.isCurler();

  /*
   * Use the given trajectory to reconstruct the 2d hits in the vector in z direction
   * to match the trajectory perfectly. Then add the newly created reconstructed 3D hit to the given list.
   */
  for (const CDCRLTaggedWireHit& rlWireHit : rlWireHits) {
    if (rlWireHit.getStereoKind() != EStereoKind::c_Axial and not rlWireHit.getWireHit().getAutomatonCell().hasTakenFlag()) {
      Vector3D recoPos3D = rlWireHit.reconstruct3D(trajectory2D);
      const CDCWire& wire = rlWireHit.getWire();
      // Skip hits out of CDC
      if (not wire.isInCellZBounds(recoPos3D)) {
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
      CDCRecoHit3D reconstructedHit(rlWireHit, recoPos3D, perpS);

      const Weight weight = m_stereoHitFilter->operator()({&reconstructedHit, &track});
      if (not std::isnan(weight)) {
        matches.emplace_back(&rlWireHit, weight);
      }
    }
  }

  return matches;
}
