/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/utility/PSelectorFunctions.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <analysis/utility/mcParticleMatching.h>

#include <analysis/utility/VariableManager.h>
#include <analysis/utility/mcParticleMatching.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/RestOfEvent.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <ecl/dataobjects/ECLGamma.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <iostream>
#include <math.h>

using namespace std;

namespace Belle2 {
  namespace analysis {

    // momentum (lab) -------------------------------------------

    double particleP(const Particle* part)
    {
      return part->getP();
    }

    double particlePx(const Particle* part)
    {
      return part->getPx();
    }

    double particlePy(const Particle* part)
    {
      return part->getPy();
    }

    double particlePz(const Particle* part)
    {
      return part->getPz();
    }

    double particlePt(const Particle* part)
    {
      float px = part->getPx();
      float py = part->getPy();
      return sqrt(px * px + py * py);
    }

    double particleCosTheta(const Particle* part)
    {
      return part->getPz() / part->getP();
    }

    double particlePhi(const Particle* part)
    {
      return atan2(part->getPy(), part->getPx()) / Unit::deg;
    }

    // momentum (CMS) -----------------------------------------------

    double particleP_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.P();
    }

    double particlePx_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Px();
    }

    double particlePy_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Py();
    }

    double particlePz_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pz();
    }

    double particlePt_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Pt();
    }

    double particleCosTheta_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.CosTheta();
    }

    double particlePhi_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.Phi();
    }

    // vertex or POCA in respect to IP ------------------------------

    double particleDX(const Particle* part)
    {
      return part->getX() - 0; // TODO replace with IP position
    }

    double particleDY(const Particle* part)
    {
      return part->getY() - 0; // TODO replace with IP position
    }

    double particleDZ(const Particle* part)
    {
      return part->getZ() - 0; // TODO replace with IP position
    }

    double particleDRho(const Particle* part)
    {
      float x = part->getX() - 0; // TODO replace with IP position
      float y = part->getY() - 0; // TODO replace with IP position
      return sqrt(x * x + y * y);
    }

    // mass ------------------------------------------------------------

    double particleMass(const Particle* part)
    {
      return part->getMass();
    }

    double particleDMass(const Particle* part)
    {
      return part->getMass() - part->getPDGMass();
    }

    // released energy --------------------------------------------------

    double particleQ(const Particle* part)
    {
      float m = part->getMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child) m -= child->getMass();
      }
      return m;
    }

    double particleDQ(const Particle* part)
    {
      float m = part->getMass() - part->getPDGMass();
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        const Particle* child = part->getDaughter(i);
        if (child) m -= (child->getMass() - child->getPDGMass());
      }
      return m;
    }

    // Mbc and deltaE

    double particleMbc(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      double E = T.getCMSEnergy() / 2;
      double m2 = E * E - vec.Vect().Mag2();
      double mbc = m2 > 0 ? sqrt(m2) : 0;
      return mbc;
    }

    double particleDeltaE(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E() - T.getCMSEnergy() / 2;
    }

    // PID ---------------------------------------------

    double particleElectronId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      return pid->getProbability(Const::electron, Const::pion);

    }

    double particleElectrondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::electron, Const::pion, set);

    }

    double particleElectronARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::electron, Const::pion, set);

    }

    double particleMuonId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      return pid->getProbability(Const::muon, Const::pion);
    }

    double particleMuondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particleMuonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particleMuonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particlePionId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::kaon);

    }

    double particlePiondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::kaon, set);

    }

    double particlePionTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::pion, Const::kaon, set);

    }

    double particlePionARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::pion, Const::kaon, set);

    }

    double particleKaonId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      return pid->getProbability(Const::kaon, Const::pion);

    }

    double particleKaondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::kaon, Const::pion, set);

    }

    double particleKaonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::kaon, Const::pion, set);

    }

    double particleKaonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::kaon, Const::pion, set);

    }

    double particleProtonId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      return pid->getProbability(Const::proton, Const::pion);
    }

    double particleProtondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::proton, Const::pion, set);

    }

    double particleProtonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::proton, Const::pion, set);

    }

    double particleProtonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = DataStore::getRelated<PIDLikelihood>(part);
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::proton, Const::pion, set);

    }

    // other ------------------------------------------------------------

    double particlePvalue(const Particle* part)
    {
      return part->getPValue();
    }

    double particleNDaughters(const Particle* part)
    {
      return part->getNDaughters();
    }

    double particleFlavorType(const Particle* part)
    {
      return part->getFlavorType();
    }


    double sumChildProb(const Particle* particle)
    {
      double signalProbabilitySum = 0.0;
      for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
        signalProbabilitySum += particle->getDaughter(j)->getExtraInfo("SignalProbability");
      }
      return signalProbabilitySum;
    }

    double prodChildProb(const Particle* particle)
    {
      double signalProbabilityProduct = 1.0;
      for (unsigned j = 0; j < particle->getNDaughters(); ++j) {
        signalProbabilityProduct *= particle->getDaughter(j)->getExtraInfo("SignalProbability");
      }
      return signalProbabilityProduct;
    }

    double isSignal(const Particle* part)
    {
      double result = 0.0;

      const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(part);

      if (mcparticle == nullptr)
        return result;

      int mcPDGCode = mcparticle->getPDG();
      int status    = getMCTruthStatus(part, mcparticle);

      if (!(status == 1 || status == 0))
        return result;

      if (part->getFlavorType() && mcPDGCode == part->getPDGCode())
        result = 1.0;
      else if (!part->getFlavorType() && abs(mcPDGCode) == abs(part->getPDGCode()))
        result = 1.0;

      return result;
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      double result = 0.0;

      const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(part);

      if (mcparticle == nullptr)
        return result;

      return mcparticle->getPDG();
    }

    double particleMCMatchStatus(const Particle* part)
    {
      double result = 0.0;

      const MCParticle* mcparticle = DataStore::getRelated<MCParticle>(part);

      if (mcparticle == nullptr)
        return result;

      return getMCTruthStatus(part, mcparticle);
    }

    double nROETracks(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(particle);

      if (roe)
        result = roe->getNTracks();

      return result;
    }

    double nROEShowers(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(particle);

      if (roe)
        result = roe->getNECLShowers();

      return result;
    }

    double nROEGammas(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(particle);

      if (roe)
        result = roe->getNECLGammas();

      return result;
    }

    double nROEPi0s(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(particle);

      if (roe)
        result = roe->getNECLPi0s();

      return result;
    }

    double recoilMomentum(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).P();
    }

    double recoilEnergy(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).E();
    }

    double recoilMass(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).M();
    }

    double recoilMassSquared(const Particle* particle)
    {
      PCmsLabTransform T;

      // Initial state (e+e- momentum in LAB)
      TLorentzVector pIN  = T.getBoostVector();

      return (pIN - particle->get4Vector()).M2();
    }

    double extraEnergy(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = DataStore::getRelated<RestOfEvent>(particle);
      if (!roe)
        return result;

      const std::vector<ECLGamma*> remainECLGammas = roe->getECLGammas();
      result = 0.0;
      for (unsigned i = 0; i < remainECLGammas.size(); i++)
        result += remainECLGammas[i]->getEnergy();

      return result;
    }

    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cth", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("p_CMS", particleP_CMS, "CMS momentum magnitude");
    REGISTER_VARIABLE("px_CMS", particlePx_CMS, "CMS momentum component x");
    REGISTER_VARIABLE("py_CMS", particlePy_CMS, "CMS momentum component y");
    REGISTER_VARIABLE("pz_CMS", particlePz_CMS, "CMS momentum component z");
    REGISTER_VARIABLE("pt_CMS", particlePt_CMS, "CMS transverse momentum");
    REGISTER_VARIABLE("cosTheta_CMS", particleCosTheta_CMS, "CMS momentum cosine of polar angle");
    REGISTER_VARIABLE("cth_CMS", particleCosTheta_CMS, "CMS momentum cosine of polar angle");
    REGISTER_VARIABLE("phi_CMS", particlePhi_CMS, "CMS momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("dx", particleDX, "x in respect to IP");
    REGISTER_VARIABLE("dy", particleDY, "y in respect to IP");
    REGISTER_VARIABLE("dz", particleDZ, "z in respect to IP");
    REGISTER_VARIABLE("dr", particleDRho, "transverse distance in respect to IP");

    REGISTER_VARIABLE("M", particleMass, "mass");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ, "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");

    REGISTER_VARIABLE("eid", particleElectronId, "electron identification probability");
    REGISTER_VARIABLE("muid", particleMuonId, "muon identification probability");
    REGISTER_VARIABLE("piid", particlePionId, "pion identification probability");
    REGISTER_VARIABLE("Kid", particleKaonId, "kaon identification probability");
    REGISTER_VARIABLE("prid", particleProtonId, "proton identification probability");

    REGISTER_VARIABLE("eid_dEdx", particleElectrondEdxId, "electron identification probability from dEdx measurement");
    REGISTER_VARIABLE("muid_dEdx", particleMuondEdxId, "muon identification probability from dEdx measurement");
    REGISTER_VARIABLE("piid_dEdx", particlePiondEdxId, "pion identification probability from dEdx measurement");
    REGISTER_VARIABLE("Kid_dEdx", particleKaondEdxId, "kaon identification probability from dEdx measurement");
    REGISTER_VARIABLE("prid_dEdx", particleProtondEdxId, "proton identification probability from dEdx measurement");

    REGISTER_VARIABLE("eid_TOP", particleElectronTOPId, "electron identification probability from TOP");
    REGISTER_VARIABLE("muid_TOP", particleMuonTOPId, "muon identification probability from TOP");
    REGISTER_VARIABLE("piid_TOP", particlePionTOPId, "pion identification probability from TOP");
    REGISTER_VARIABLE("Kid_TOP", particleKaonTOPId, "kaon identification probability from TOP");
    REGISTER_VARIABLE("prid_TOP", particleProtonTOPId, "proton identification probability from TOP");

    REGISTER_VARIABLE("eid_ARICH", particleElectronARICHId, "electron identification probability from ARICH");
    REGISTER_VARIABLE("muid_ARICH", particleMuonARICHId, "muon identification probability from ARICH");
    REGISTER_VARIABLE("piid_ARICH", particlePionARICHId, "pion identification probability from ARICH");
    REGISTER_VARIABLE("Kid_ARICH", particleKaonARICHId, "kaon identification probability from ARICH");
    REGISTER_VARIABLE("prid_ARICH", particleProtonARICHId, "proton identification probability from ARICH");

    REGISTER_VARIABLE("chiProb", particlePvalue, "chi^2 probability of the fit");
    REGISTER_VARIABLE("nDaughters", particleNDaughters, "number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType, "flavor type of decay (0=unflavored, 1=flavored)");
    REGISTER_VARIABLE("sumChildProb", sumChildProb, "sum of signal probabilities of daughters");
    REGISTER_VARIABLE("prodChildProb", prodChildProb, "product of signal probabilities of daughters");

    REGISTER_VARIABLE("isSignal", isSignal,               "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("mcPDG",    particleMCMatchPDGCode, "The PDG code of matched MCParticle");
    REGISTER_VARIABLE("mcStatus", particleMCMatchStatus,  "The bit pattern indicating the quality of MC match");

    REGISTER_VARIABLE("nROETracks",  nROETracks,  "number of remaining tracks as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEShowers", nROEShowers, "number of remaining ECL showers as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEGammas",  nROEGammas,  "number of remaining ECL gammas as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEPi0s",    nROEPi0s,    "number of remaining ECL pi0s as given by the related RestOfEvent object");

    REGISTER_VARIABLE("pRecoil",  recoilMomentum,    "magnitude of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("eRecoil",  recoilEnergy,      "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil",  recoilMass,        "invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared, "invariant mass squared of the system recoiling against given Particle");

    REGISTER_VARIABLE("eextra", extraEnergy, "extra energy in the calorimeter that is not associated to the given Particle");
  }
}

