/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Andreas Moll, Marko Staric                    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/UserInfo.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>

#include <G4TrackingManager.hh>
#include <G4Track.hh>
#include <G4ParticleDefinition.hh>
#include <G4ParticleTypes.hh>

using namespace Belle2;
using namespace Belle2::Simulation;

TrackingAction::TrackingAction(MCParticleGraph& mcParticleGraph): G4UserTrackingAction(), m_mcParticleGraph(mcParticleGraph), m_IgnoreOpticalPhotons(false), m_IgnoreSecondaries(false), m_EnergyCut(0.0)
{

}


TrackingAction::~TrackingAction()
{

}


void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  //We only want to do the following for new tracks, not for suspended and reactivated ones"
  if (track->GetCurrentStepNumber() > 0) return;

  const G4DynamicParticle* dynamicParticle = track->GetDynamicParticle();

  try {
    //Check if the dynamic particle has a primary particle attached.
    //If yes, the UserInfo of the primary particle as UserInfo to the track.
    if (dynamicParticle->GetPrimaryParticle() != NULL) {
      const G4PrimaryParticle* primaryParticle = dynamicParticle->GetPrimaryParticle();
      if (primaryParticle->GetUserInformation() != NULL) {
        const_cast<G4Track*>(track)->SetUserInformation(new TrackInfo(ParticleInfo::getInfo(*primaryParticle)));
      } else {
        B2WARNING(track->GetDefinition()->GetPDGEncoding() << " has no MCParticle user information !")
      }
    }

    //Get particle of current track
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);
    //Set the Values of the particle which are already known
    G4ThreeVector dpMom  = dynamicParticle->GetMomentum() * Unit::MeV;
    G4ThreeVector trVtxPos = track->GetVertexPosition() * Unit::mm;
    currParticle.setTrackID(track->GetTrackID());
    currParticle.setPDG(dynamicParticle->GetPDGcode());
    currParticle.setMass(dynamicParticle->GetMass() * Unit::MeV);
    currParticle.setCharge(dynamicParticle->GetCharge());
    currParticle.setEnergy(dynamicParticle->GetTotalEnergy() * Unit::MeV);
    currParticle.setMomentum(dpMom.x(), dpMom.y(), dpMom.z());
    currParticle.setProductionTime(track->GetGlobalTime() * Unit::ns);
    currParticle.setProductionVertex(trVtxPos.x(), trVtxPos.y(), trVtxPos.z());
    //Get the physics process type for a secondary particle
    if (dynamicParticle->GetPrimaryParticle() != NULL) {
      currParticle.setSecondaryPhysicsProcess(0);
    } else if (track->GetCreatorProcess() != NULL) {
      currParticle.setSecondaryPhysicsProcess(track->GetCreatorProcess()->GetProcessSubType());
    } else {
      currParticle.setSecondaryPhysicsProcess(-1);
    }

  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what())
  }
}


void TrackingAction::PostUserTrackingAction(const G4Track* track)
{
  G4StepPoint* postStep = track->GetStep()->GetPostStepPoint();

  // Get particle of current track
  try {
    MCParticleGraph::GraphParticle& currParticle = TrackInfo::getInfo(*track);

    // Add particle and decay Information to all secondaries
    BOOST_FOREACH(G4Track * daughterTrack, *fpTrackingManager->GimmeSecondaries()) {

      // Add the particle to the particle graph and as UserInfo to the track
      // if it is a secondary particle created by Geant4.
      if (daughterTrack->GetDynamicParticle()->GetPrimaryParticle() == NULL && daughterTrack->GetUserInformation() == NULL) {
        MCParticleGraph::GraphParticle& daughterParticle = m_mcParticleGraph.addParticle();
        const_cast<G4Track*>(daughterTrack)->SetUserInformation(new TrackInfo(daughterParticle));

        currParticle.decaysInto(daughterParticle); //Add the decay

        // Optical photons and low energy secondaries:  steering of output to MCParticles
        if (daughterTrack->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
          if (m_IgnoreOpticalPhotons) daughterParticle.setIgnore();
          // to apply quantum efficiency only once, if optical photon is a daugher of optical photon
          if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
            TrackInfo* currInfo = dynamic_cast<TrackInfo*>(track->GetUserInformation());
            TrackInfo* daughterInfo = dynamic_cast<TrackInfo*>(daughterTrack->GetUserInformation());
            daughterInfo->setStatus(currInfo->getStatus());
            daughterInfo->setFraction(currInfo->getFraction());
          }
        } else {
          if (m_IgnoreSecondaries && daughterTrack->GetKineticEnergy() < m_EnergyCut)
            daughterParticle.setIgnore();
        }

      }
    }
    //If the track is just suspended we can return here: the rest should be filled once the track is done
    if (track->GetTrackStatus() == fSuspend) return;

    //Check if particle left detector.
    //fWorldBoundary seems to be broken, check if poststep is on boundary and next volume is 0
    if (postStep->GetStepStatus() == fGeomBoundary && track->GetNextVolume() == NULL) {
      currParticle.addStatus(MCParticle::c_LeftDetector);
    }

    //Check if particle was stopped in the detector
    if (track->GetKineticEnergy() <= 0.0) {
      currParticle.addStatus(MCParticle::c_StoppedInDetector);
    }

    //Set the values for the particle
    G4ThreeVector decVtx = postStep->GetPosition() * Unit::mm;
    currParticle.setDecayVertex(decVtx.x(), decVtx.y(), decVtx.z());
    currParticle.setDecayTime(postStep->GetGlobalTime() * Unit::ns);
    currParticle.setValidVertex(true);

  } catch (CouldNotFindUserInfo& exc) {
    B2FATAL(exc.what())
  }
}
