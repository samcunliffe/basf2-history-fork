/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Jan Strube (jan.strube@desy.de)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// analysis
#include <analysis/VariableManager/Manager.h>

// framework
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <tracking/dataobjects/ExtHit.h>
#include <mdst/dataobjects/Track.h>
#include <analysis/dataobjects/Particle.h>
#include <top/dataobjects/TOPDigit.h>
#include <framework/gearbox/Const.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/dataobjects/TOPLikelihood.h>
#include <top/dataobjects/TOPRecBunch.h>

#include <algorithm> // for sort
using namespace std;

namespace Belle2 {
  // contains a couple of helper functions that are related to TOP variables
  namespace topDigitVariables {
    // returns the TOP likelihood that is associated with a given particle
    const TOPLikelihood* getTOPLikelihood(const Particle* particle)
    {
      const auto* track = particle->getTrack();
      return track ? track->getRelated<TOPLikelihood>() : nullptr;
    }

    // returns the ExtHit that is associated with a given particle
    const ExtHit* getExtHit(const Particle* particle)
    {
      const auto* topLikelihood = getTOPLikelihood(particle);
      return topLikelihood ? topLikelihood->getRelated<ExtHit>() : nullptr;
    }

    // returns the TOP slot ID of the particle
    double getSlotID(const Particle* particle)
    {
      const auto* extHit = getExtHit(particle);
      // zero is invalid slot ID, e.g. particle didn't hit the TOP
      return extHit ? extHit->getCopyID() : 0;
    }

    // returns the local coordinate of the particle's entry point to the TOP
    TVector3 getLocalPosition(const Particle* particle)
    {
      const auto* extHit = getExtHit(particle);
      if (not extHit) return TVector3(0, 0, 0);
      int slotID = extHit->getCopyID();
      const auto& position = extHit->getPosition(); // TVector3
      const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
      if (not geo or not geo->isModuleIDValid(slotID)) return TVector3(0, 0, 0);
      const auto& module = geo->getModule(slotID);
      return module.pointToLocal(position); // TVector3
    }

    // returns the local coordinates of the particles momentum in the TOP
    TVector3 getLocalMomentum(const Particle* particle)
    {
      const auto* extHit = getExtHit(particle);
      if (not extHit) return TVector3(0, 0, 0);
      int slotID = extHit->getCopyID();
      const auto& momentum = extHit->getMomentum(); // TVector3
      const auto* geo = TOP::TOPGeometryPar::Instance()->getGeometry();
      if ((not geo) or (not geo->isModuleIDValid(slotID))) return TVector3(0, 0, 0);
      const auto& module = geo->getModule(slotID);
      return module.momentumToLocal(momentum); // TVector3
    }

    // counts the number of photons in the TOP in a given time frame
    // if tmin < 0, count from the time of the first photon
    int countHits(const Particle* particle, double tmin, double tmax, bool clean = true)
    {
      int slotID = getSlotID(particle);
      StoreArray<TOPDigit> digits;
      vector<double> digitTimes;
      for (const auto& digit : digits) {
        if (abs(digit.getModuleID()) != abs(slotID)) continue;
        // skip bad digits only when we want to clean
        if (clean && digit.getHitQuality() != TOPDigit::c_Good) continue;
        digitTimes.push_back(digit.getTime());
      }
      if (digitTimes.empty()) return 0;
      sort(digitTimes.begin(), digitTimes.end());
      int count = 0;
      if (tmin < 0) tmin = digitTimes[0];
      for (auto t : digitTimes) {
        if (t >= tmin) ++count;
        if (t > tmax) break;
      }
      return count;
    }

    // counts the number of photons regardless of hit quality
    int countRawHits(const Particle* particle, double tmin, double tmax)
    {
      return countHits(particle, tmin, tmax, false);
    }

    // returns the expected number of photons for a given hypothesis
    double getExpectedPhotonCount(const Particle* particle, int pdg = 0)
    {
      const auto* topLikelihood = getTOPLikelihood(particle);
      if (not topLikelihood) return 0;
      // if the user does select a hypothesis, use the particle's pdg code
      pdg = pdg != 0 ? pdg : particle->getPDGCode();
      const auto& chargedStable = Const::chargedStableSet.find(abs(pdg));
      if (chargedStable == Const::invalidParticle) return 0; // PDG code not one of e, mu, pi, K, p, d
      return topLikelihood->getEstPhot(chargedStable);
    }

  }
  namespace Variable {
    //! @returns the number of digits in the same module as the particle
    double topDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != Const::EDetector::TOP) continue;
        if (h.getStatus() != EXT_ENTER) continue;
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1;
      }
      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
        if (t.getHitQuality() != TOPDigit::c_Good) continue;
        count += 1;
      }
      return count;
    }

    //! @returns the number of digits in all other module as the particle
    double topBackgroundDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != Const::EDetector::TOP) continue;
        if (h.getStatus() != EXT_ENTER) continue;
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1;
      }
      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) == abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
        if (t.getHitQuality() != TOPDigit::c_Good) continue;
        count += 1;
      }
      return count;
    }

    //! @returns the number of digits in all other module as the particle
    double topBackgroundDigitCountRaw(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != Const::EDetector::TOP) continue;
        if (h.getStatus() != EXT_ENTER) continue;
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1;
      }
      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) == abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
        count += 1;
      }
      return count;
    }

    //! @returns the number of all digits regardless of hit quality in the same module as the particle
    double topRawDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != Const::EDetector::TOP) continue;
        if (h.getStatus() != EXT_ENTER) continue;
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1;
      }
      StoreArray<TOPDigit> topDigits;
      int count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue; // catch the case where one of the module IDs is negative
        count += 1;
      }
      return count;
    }

    //! @returns the largest time between to subsequent digits in the same module as the particle
    double topDigitGapSize(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != 4) continue; // 4 == iTOP, see Const::EDetectors
        if (h.getStatus() != 0) continue; // 0 == EXT_ENTER
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1.0;
      }
      StoreArray<TOPDigit> topDigits;
      double maxGap = 0; // the largest time difference between two consecutive hits
      vector<double> digitTimes; // all digits in the module that the track entered
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        if (t.getHitQuality() != TOPDigit::c_Good) continue;
        digitTimes.push_back(t.getTime());
      }
      if (digitTimes.empty()) {
        return -1.0;
      }
      sort(digitTimes.begin(), digitTimes.end());
      for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
        double gap = digitTimes[i + 1] - digitTimes[i];
        if (gap > maxGap) {
          maxGap = gap;
        }
      }
      return maxGap;
    }

    // The number of reflected digits is defined as the number of digits after the gap
    //! @returns the number of reflected digits in the same module as the particle
    double topReflectedDigitCount(const Particle* particle)
    {
      auto trk = particle->getTrack();
      if (not trk) {
        return -1.0;
      }
      auto extHits = trk->getRelationsWith<ExtHit>();
      int thisModuleID = 77; // default to sentinel value
      for (auto h : extHits) {
        if (h.getDetectorID() != 4) continue; // 4 == iTOP, see Const::EDetectors
        if (h.getStatus() != 0) continue; // 0 == EXT_ENTER
        // now find the module of this hit.
        thisModuleID = h.getCopyID(); // could be positive or negative
        break;
      }
      if (thisModuleID == 77) {
        return -1.0;
      }
      StoreArray<TOPDigit> topDigits;
      double maxGap = 0; // the largest time difference between two consecutive hits
      size_t maxGapIndex = 0; // the index of the first hit *after* the gap
      vector<double> digitTimes; // all digits in the module that the track entered
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        if (t.getHitQuality() != TOPDigit::c_Good) continue;
        digitTimes.push_back(t.getTime());
      }
      if (digitTimes.empty()) {
        return -1.0;
      }
      sort(digitTimes.begin(), digitTimes.end());
      for (size_t i = 0; i < digitTimes.size() - 1; ++i) {
        double gap = digitTimes[i + 1] - digitTimes[i];
        if (gap > maxGap) {
          maxGap = gap;
          maxGapIndex = i + 1;
        }
      }
      return digitTimes.size() - maxGapIndex;
    }

    //! @returns the X coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalX(const Particle* particle)
    {
      return topDigitVariables::getLocalPosition(particle).X();
    }

    //! @returns the Y coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalY(const Particle* particle)
    {
      return topDigitVariables::getLocalPosition(particle).Y();
    }

    //! @returns the Z coordinate of the particle entry point to the TOP in the local frame
    double getTOPLocalZ(const Particle* particle)
    {
      return topDigitVariables::getLocalPosition(particle).Z();
    }

    //! @returns the local phi component of the particle's momentum in the TOP
    double getTOPLocalPhi(const Particle* particle)
    {
      return topDigitVariables::getLocalMomentum(particle).Phi();
    }

    //! @returns the local theta component of the particle's momentum in the TOP
    double getTOPLocalTheta(const Particle* particle)
    {
      return topDigitVariables::getLocalMomentum(particle).Theta();
    }

    //! @returns the number of photons associated with the particle likelihood
    double getTOPPhotonCount(const Particle* particle)
    {
      const auto* topLikelihood = topDigitVariables::getTOPLikelihood(particle);
      return topLikelihood ? topLikelihood->getNphot() : 0;
    }

    //! @returns the expected number of photons for the assigned hypothesis
    double getExpectedTOPPhotonCount(const Particle* particle, const vector<double>& vars)
    {
      if (vars.size() != 1) {
        B2FATAL("Need exactly one parameter (pdg id).");
      }
      return topDigitVariables::getExpectedPhotonCount(particle, static_cast<int>(vars[0]));
    }

    //! @returns the slot ID of the TOP for the particle
    double getTOPSlotID(const Particle* particle)
    {
      return topDigitVariables::getSlotID(particle);
    }

    //! @returns the number of TOP photons in the given time interval
    double countTOPHitsInInterval(const Particle* particle, const vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (tmin, tmax)");
      }
      return topDigitVariables::countHits(particle, vars[0], vars[1]);
    }

    //! @returns the number of TOP photons in the first 20 ns
    double countTOPHitsInFirst20ns(const Particle* particle)
    {
      return topDigitVariables::countHits(particle, -1.0, 20.0);
    }

    //! @returns the number of raw TOP photons in the given time interval
    double countRawTOPHitsInInterval(const Particle* particle, const vector<double>& vars)
    {
      if (vars.size() != 2) {
        B2FATAL("Need exactly two parameters (tmin, tmax)");
      }
      return topDigitVariables::countRawHits(particle, vars[0], vars[1]);
    }
    //---------------- TOPRecBunch related --------------------

    //! @returns whether the rec bunch is reconstructed
    double isTOPRecBunchReconstructed(const Particle* particle)
    {
      StoreObjPtr<TOPRecBunch> recBunch;
      // Attention! 0.0 is false, everything else is true
      // returning -1, like for the others will most likely lead to bugs
      // if the caller is not careful about return values.
      if (not recBunch.isValid()) return 0.0;
      return recBunch->isReconstructed();
    }

    //! returns the bunch number. Use -9999 to indicate error
    double TOPRecBunchNumber(const Particle* particle)
    {
      StoreObjPtr<TOPRecBunch> recBunch;
      if (not recBunch.isValid()) return -9999.0;
      return recBunch->getBunchNo();
    }

    //! returns the current offset
    double TOPRecBunchCurrentOffset(const Particle* particle)
    {
      StoreObjPtr<TOPRecBunch> recBunch;
      if (not recBunch.isValid()) return -9999.0;
      return recBunch->getCurrentOffset();
    }

    //! returns the number of tracks in the TOP acceptance
    double TOPRecBunchTrackCount(const Particle* particle)
    {
      StoreObjPtr<TOPRecBunch> recBunch;
      if (not recBunch.isValid()) return -9999.0;
      return recBunch->getNumTracks();
    }

    //! returns the number of tracks used in the bunch reconstruction
    double TOPRecBunchUsedTrackCount(const Particle* particle)
    {
      StoreObjPtr<TOPRecBunch> recBunch;
      if (not recBunch.isValid()) return -9999.0;
      return recBunch->getUsedTracks();
    }
    //-------------- Event based -----------------------------------

    //! returns the number of photons in a given slot without cleaning
    double TOPRawPhotonsInSlot(const Particle* particle, const vector<double>& vars)
    {
      if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
      StoreArray<TOPDigit> topDigits;
      int thisModuleID = static_cast<int>(vars[0]);
      size_t count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        count += 1;
      }
      return count;
    }
    //! returns the number of good photons in a given slot
    double TOPGoodPhotonsInSlot(const Particle* particle, const vector<double>& vars)
    {
      if (vars.size() != 1) { B2FATAL("Need exactly one parameter (slot id).");}
      StoreArray<TOPDigit> topDigits;
      int thisModuleID = static_cast<int>(vars[0]);
      size_t count = 0;
      for (auto t : topDigits) {
        if (abs(t.getModuleID()) != abs(thisModuleID)) continue;
        if (t.getHitQuality() != TOPDigit::c_Good) continue;
        count += 1;
      }
      return count;
    }
    VARIABLE_GROUP("TOP Calibration");
    REGISTER_VARIABLE("topDigitCount", topDigitCount,
                      "[calibration] The number of TOPDigits in the module to which the track was extrapolated");
    REGISTER_VARIABLE("topBackgroundDigitCount", topBackgroundDigitCount,
                      "[calibration] The number of TOPDigits in all modules except the one to which the track was extrapolated");
    REGISTER_VARIABLE("topBackgroundDigitCountRaw", topBackgroundDigitCountRaw,
                      "[calibration] The number of TOPDigits in all modules except the one to which the track was extrapolated");
    REGISTER_VARIABLE("topDigitCountRaw", topDigitCount,
                      "[calibration] The number of TOPDigits in the module to which the track was extrapolated, regardless of hit quality");
    REGISTER_VARIABLE("topReflectedDigitCount", topReflectedDigitCount,
                      "[calibration] The number of reflected photons in the same module");
    REGISTER_VARIABLE("topDigitGapSize", topDigitGapSize,
                      "[calibration] The largest time difference between two consecutive hits in the same module");
    REGISTER_VARIABLE("topLocalX", getTOPLocalX,
                      "[calibration] The local x coordinate of the particle's entry point to the TOP module");
    REGISTER_VARIABLE("topLocalY", getTOPLocalY,
                      "[calibration] The local y coordinate of the particle's entry point to the TOP module");
    REGISTER_VARIABLE("topLocalZ", getTOPLocalZ,
                      "[calibration] The local z coordinate of the particle's entry point to the TOP module");
    REGISTER_VARIABLE("topLocalPhi", getTOPLocalPhi,
                      "[calibration] The local phi coordinate of the particle's momentum in the TOP module");
    REGISTER_VARIABLE("topLocalTheta", getTOPLocalTheta,
                      "[calibration] The local phi coordinate of the particle's momentum in the TOP module");
    REGISTER_VARIABLE("topSlotID", getTOPSlotID,
                      "[calibration] The ID of the TOP slot that was hit by the particle");
    REGISTER_VARIABLE("topExpectedPhotonCount(pdg)", getExpectedTOPPhotonCount,
                      "[calibration] The expected number of photons in the TOP for the particle under the given hypothesis");
    REGISTER_VARIABLE("topPhotonCount", getTOPPhotonCount,
                      "[calibration] The number of (bg-subtracted) TOP photons in for the given particle");
    REGISTER_VARIABLE("countTOPHitsInInterval(tmin, tmax)", countTOPHitsInInterval,
                      "[calibration] The number of photons in the given interval");
    REGISTER_VARIABLE("countTOPHitsInFirst20ns", countTOPHitsInFirst20ns,
                      "[calibration] The number of photons in the first 20 ns after the first photon");
    REGISTER_VARIABLE("countRawTOPHitsInInterval(tmin, tmax)", countRawTOPHitsInInterval,
                      "[calibration] The number of photons in the given interval");
    REGISTER_VARIABLE("topRecBunchUsedTrackCount", TOPRecBunchUsedTrackCount,
                      "[calibration] The number of tracks used in the bunch reconstruction");
    REGISTER_VARIABLE("topRecBunchTrackCount", TOPRecBunchTrackCount,
                      "[calibration] The number of tracks in the TOP acceptance");
    REGISTER_VARIABLE("topRecBunchCurrentOffset", TOPRecBunchCurrentOffset,
                      "[calibration] The current offset");
    REGISTER_VARIABLE("topRecBunchNumber", TOPRecBunchNumber,
                      "[calibration] The number of the bunch relative to the interaction");
    REGISTER_VARIABLE("isTopRecBunchReconstructed", isTOPRecBunchReconstructed,
                      "[calibration] Flag to indicate whether the bunch was reconstructed");
    REGISTER_VARIABLE("topRawPhotonsInSlot(id)", TOPRawPhotonsInSlot,
                      "[calibration] The number of all photons in the given slot");
    REGISTER_VARIABLE("topGoodPhotonsInSlot(id)", TOPGoodPhotonsInSlot,
                      "[calibration] The number of good photons in the given slot");
  }
// Create an empty module which allows basf2 to easily find the library and load it from the steering file
  class EnableTOPDigitVariablesModule: public Module {}; // Register this module to create a .map lookup file.
  REG_MODULE(EnableTOPDigitVariables);
}
