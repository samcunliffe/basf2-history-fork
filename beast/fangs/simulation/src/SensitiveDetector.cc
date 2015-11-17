/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2015 Belle II Collaboration                          *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/fangs/simulation/SensitiveDetector.h>
#include <framework/datastore/DataStore.h>
#include <framework/gearbox/Unit.h>
#include <G4Track.hh>
#include <G4Step.hh>

#include <array>

namespace Belle2 {
  /** Namespace to encapsulate code needed for the FANGS detector */
  namespace fangs {
    namespace {
      std::array<float, 3> vecToFloat(const G4ThreeVector& vec)
      {
        return std::array<float, 3> {{(float)vec.x(), (float)vec.y(), (float)vec.z()}};
      }
    }

    SensitiveDetector::SensitiveDetector():
      Simulation::SensitiveDetectorBase("FANGSSensitiveDetector", Const::invalidDetector)
    {
      //Register all collections we want to modify and require those we want to use
      m_mcParticles.registerInDataStore();
      m_simHits.registerInDataStore();
      m_relMCSimHit.registerInDataStore();

      //Register the Relation so that the TrackIDs get replaced by the actual
      //MCParticle indices after simulating the events. This is needed as
      //secondary particles might not be stored so everything relating to those
      //particles will be attributed to the last saved mother particle
      registerMCParticleRelation(m_relMCSimHit, RelationArray::c_negativeWeight);
    }

    bool SensitiveDetector::step(G4Step* step, G4TouchableHistory*)
    {
      //Get scintilator ladder and sensor number
      const G4TouchableHistory* touchable = dynamic_cast<const G4TouchableHistory*>(step->GetPreStepPoint()->GetTouchable());
      int ladderID = touchable->GetVolume(2)->GetCopyNo();
      int sensorID = touchable->GetVolume(1)->GetCopyNo();

      //Get Track information
      const G4Track& track = *step->GetTrack();
      const int trackID = track.GetTrackID();
      const int pdgCode = step->GetTrack()->GetDefinition()->GetPDGEncoding();
      // deposited Energy in Geant4 Units
      double depEnergy  = step->GetTotalEnergyDeposit();
      // convert into Belle2 Units
      depEnergy *= Unit::GeV / CLHEP::GeV;

      // get pre and post step point
      const G4StepPoint& preStep = *step->GetPreStepPoint();
      const G4StepPoint& postStep = *step->GetPostStepPoint();

      // check if this is the same sensor traversal, otherwise add one to the stack
      if (m_tracks.empty() || (!m_tracks.top().check(trackID, ladderID, sensorID))) {
        m_tracks.push(SensorTraversal());
      }
      // get the top of the stack
      SensorTraversal& traversal = m_tracks.top();

      //If new track, remember values
      if (traversal.getTrackID() == 0) {
        bool isPrimary = Simulation::TrackInfo::getInfo(track).hasStatus(MCParticle::c_PrimaryParticle);
        //Add start position
        const G4ThreeVector preStepPos = preStep.GetPosition() / CLHEP::mm * Unit::mm;
        const G4ThreeVector preStepMom = preStep.GetMomentum() / CLHEP::MeV * Unit::MeV;
        //const G4AffineTransform& localToGlobalTransform = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform().Inverse();
        const G4AffineTransform& localToGlobalTransform = preStep.GetTouchableHandle()->GetHistory()->GetTopTransform();
        const G4ThreeVector localpreStepPos = localToGlobalTransform.TransformPoint(preStep.GetPosition()) / CLHEP::mm * Unit::mm;
        const double time = preStep.GetGlobalTime() / CLHEP::ns * Unit::ns;
        traversal.setInitial(trackID, ladderID, sensorID, pdgCode, isPrimary, preStepPos, localpreStepPos, preStepMom, time);
        //Remember if the track came from the outside
        if (preStep.GetStepStatus() == fGeomBoundary) traversal.hasEntered();
      }
      //Add current step
      const G4ThreeVector postStepPos = postStep.GetPosition() / CLHEP::mm * Unit::mm;
      const double length = step->GetStepLength() / CLHEP::cm * Unit::cm;
      traversal.add(postStepPos, depEnergy, length);

      //check if we are leaving the volume
      bool isLeaving = (postStep.GetStepStatus() == fGeomBoundary);
      if (isLeaving) traversal.hasLeft();

      //if we are leaving or the track is stopped, finish it
      if (isLeaving || track.GetTrackStatus() >= fStopAndKill) {
        bool contained = traversal.isContained();
        bool saved = finishTrack();
        //we mark all particles as important which created a hit and entered or left the volume
        if (saved && !contained) {
          Simulation::TrackInfo::getInfo(track).setIgnore(false);
        }
        return saved;
      }
      // Track not finished, return false right now
      return false;
    }

    bool SensitiveDetector::finishTrack()
    {
      SensorTraversal& traversal = m_tracks.top();
      // ignore everything below 1eV
      bool save = traversal.getDepEnergy() > Unit::eV;
      if (save) {
        auto momEntry = vecToFloat(traversal.getEntryMomentum());
        auto posEntry = vecToFloat(traversal.getEntryPosition());
        auto localposEntry = vecToFloat(traversal.getLocalEntryPosition());
        auto posExit = vecToFloat(traversal.getExitPosition());
        int hitIndex = m_simHits.getEntries();
        m_simHits.appendNew(
          traversal.getTrackID(),
          traversal.getLadderID(), traversal.getSensorID(),
          traversal.getPDGCode(), traversal.getEntryTime(),
          traversal.getDepEnergy(),
          traversal.getLength(), posEntry.data(),
          localposEntry.data(),
          posExit.data(), momEntry.data()
        );
        m_relMCSimHit.add(traversal.getTrackID(), hitIndex, traversal.getDepEnergy());
      }
      //No we just need to take care of the stack of traversals
      if (m_tracks.size() == 1) {
        //reuse traversal to keep memory if this is the first one
        traversal.reset();
      } else {
        //this should only happen if the parent track got suspended. As this
        //rarely happens in PXD we do not care for re-usability here
        m_tracks.pop();
      }
      return save;
    }
  } //claw namespace
} //Belle2 namespace
