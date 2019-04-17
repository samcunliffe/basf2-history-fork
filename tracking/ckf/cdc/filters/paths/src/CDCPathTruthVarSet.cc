/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Simon Kurz, Nils Braun                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/ckf/cdc/filters/paths/CDCPathTruthVarSet.h>

#include <tracking/ckf/cdc/entities/CDCCKFState.h>
#include <tracking/ckf/cdc/entities/CDCCKFPath.h>

#include <ecl/dataobjects/ECLShower.h>
#include <mdst/dataobjects/MCParticle.h>

#include <tracking/dataobjects/RecoTrack.h>

using namespace std;
using namespace Belle2;

bool CDCPathTruthVarSet::extract(const BaseCDCPathFilter::Object* path)
{
  // check if hit belongs to same seed
  const auto& seed = path->front();
  auto* seedRecoTrack = seed.getSeed();
  auto* seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");

  MCParticle* seedMCParticle;
  if (seedMCTrack) {
    seedMCParticle = seedMCTrack->getRelated<MCParticle>();
  }
  // maybe used track from Ecl seeding?
  else {
    seedRecoTrack = seedRecoTrack->getRelated<RecoTrack>("EclSeedRecoTracks");
    seedMCTrack = seedRecoTrack->getRelated<RecoTrack>("MCRecoTracks");
    const auto* seedEclShower = seedRecoTrack->getRelated<ECLShower>("ECLShowers");
    seedMCParticle = seedEclShower->getRelated<MCParticle>();
  }

  while (seedMCParticle->getMother()) {
    seedMCParticle = seedMCParticle->getMother();
  }

  int matched = 0;
  /*
    for (auto const& state : *path) {
      if (state.isSeed()) {
        continue;
      }

      const auto wireHit = state.getWireHit();
      const auto cdcHit = wireHit->getHit();
      // const auto* hitMCParticle = cdcHit->getRelated<MCParticle>();
      const auto* hitMCTrack = cdcHit->getRelated<RecoTrack>("MCRecoTracks");

      if (seedMCTrack == hitMCTrack) {
        matched += 1;
      }
    }
  */

  var<named("matched")>() = matched;
  var<named("PDG")>() = seedMCParticle->getPDG();

  auto seedMom = seedMCParticle->getMomentum();
  var<named("seed_p_truth")>() = seedMom.Mag();
  var<named("seed_theta_truth")>() = seedMom.Theta() * 180 / M_PI;
  var<named("seed_pt_truth")>() = seedMom.Perp();
  var<named("seed_pz_truth")>() = seedMom.Z();
  var<named("seed_px_truth")>() = seedMom.X();
  var<named("seed_py_truth")>() = seedMom.Y();

  return true;
}
