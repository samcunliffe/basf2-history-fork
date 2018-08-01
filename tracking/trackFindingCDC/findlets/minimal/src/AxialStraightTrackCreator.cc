/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dmitrii Neverov                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/trackFindingCDC/findlets/minimal/AxialStraightTrackCreator.h>

#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectorySZ.h>
#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.templateDetails.h>

using namespace Belle2;
using namespace TrackFindingCDC;

AxialStraightTrackCreator::AxialStraightTrackCreator() = default;

std::string AxialStraightTrackCreator::getDescription()
{
  return "A findlet looking for straight tracks between IP and reconstructed ECL clusters.";
}

void AxialStraightTrackCreator::exposeParameters(ModuleParamList* moduleParamList,
                                                 const std::string& prefix)
{
  moduleParamList->addParameter(prefixed(prefix, "minEnergy"),
                                m_param_minEnergy,
                                "Parameter to define minimal threshold of ECL cluster energy.",
                                m_param_minEnergy);
  moduleParamList->addParameter(prefixed(prefix, "minNHits"),
                                m_param_minNHits,
                                "Parameter to define minimal threshold of track number of hits.",
                                m_param_minNHits);
}

void AxialStraightTrackCreator::initialize()
{
  Super::initialize();
}

void AxialStraightTrackCreator::apply(const std::vector<const ECLCluster*>& eclClusters,
                                      const std::vector<const CDCWireHit*>& axialWireHits,
                                      std::vector<CDCTrack>& tracks)
{
  B2WARNING(eclClusters.size() <<  " clusters found!");
  for (const ECLCluster* cluster : eclClusters) {
    float phi = cluster->getPhi();
    UncertainPerigeeCircle circle(0, Vector2D::Phi(phi), 0); //no covariance matrix (yet?)
    CDCTrajectory2D trajectory2D(circle);
    CDCTrack track;
    trajectory2D.setLocalOrigin(Vector2D(0, 0));
    std::vector<const CDCWireHit*> foundHits = search(axialWireHits, trajectory2D);
    for (const CDCWireHit* wireHit : foundHits) {
      //NOTE can be done by AxialTrackUtil::addCandidateFromHits
      CDCRecoHit3D recoHit3D = CDCRecoHit3D::reconstructNearest(wireHit, trajectory2D);
      //TODO set taken flag?
      track.push_back(std::move(recoHit3D));
    }
    B2WARNING(track.size() <<  " track size!");
    track.sortByArcLength2D();
    track.setStartTrajectory3D(CDCTrajectory3D(trajectory2D, CDCTrajectorySZ::basicAssumption()));
    tracks.emplace_back(std::move(track));
  }
}

std::vector<const CDCWireHit*> AxialStraightTrackCreator::search(const std::vector<const CDCWireHit*>& axialWireHits,
    const CDCTrajectory2D& trajectory)
{
  std::vector<const CDCWireHit*> foundHits;
  for (const CDCWireHit* hit : axialWireHits) {
    //TODO check for taken hits?
    float distance = trajectory.getDist2D(hit->reconstruct2D(trajectory));
    if (distance < m_param_maxDistance) {
      foundHits.push_back(hit);
    }
  }
  return foundHits;
}
