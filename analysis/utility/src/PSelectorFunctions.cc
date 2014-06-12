/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/utility/PSelectorFunctions.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <analysis/utility/VariableManager.h>
#include <analysis/utility/MCMatching.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>

// dataobjects
#include <analysis/dataobjects/RestOfEvent.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TVectorF.h>

#include <iostream>
#include <cmath>

using namespace std;

namespace Belle2 {
  namespace analysis {

    // momentum (lab) -------------------------------------------

    double particleP(const Particle* part)
    {
      return part->getP();
    }

    double particleE(const Particle* part)
    {
      return part->getEnergy();
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

    double particleE_CMS(const Particle* part)
    {
      PCmsLabTransform T;
      TLorentzVector vec = T.rotateLabToCms() * part->get4Vector();
      return vec.E();
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

    double particleInvariantMass(const Particle* part)
    {
      double result = 0.0;

      const std::vector<Particle*> daughters = part->getDaughters();
      if (daughters.size() > 0) {
        TLorentzVector sum;
        for (unsigned i = 0; i < daughters.size(); i++)
          sum += daughters[i]->get4Vector();

        result = sum.M();
      } else {
        result = part->getMass();
      }

      return result;
    }

    double particleInvariantMassError(const Particle* part)
    {
      float result = 0.0;

      float invMass = part->getMass();

      TMatrixFSym covarianceMatrix(Particle::c_DimMomentum);
      for (unsigned i = 0; i < part->getNDaughters(); i++) {
        covarianceMatrix += part->getDaughter(i)->getMomentumErrorMatrix();
      }

      TVectorF    jacobian(Particle::c_DimMomentum);
      jacobian[0] = -1.0 * part->getPx() / invMass;
      jacobian[1] = -1.0 * part->getPy() / invMass;
      jacobian[2] = -1.0 * part->getPz() / invMass;
      jacobian[3] =  1.0 * part->getEnergy() / invMass;

      result = jacobian * (covarianceMatrix * jacobian);

      if (result < 0.0)
        result = 0.0;

      return TMath::Sqrt(result);
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
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::electron, Const::pion);
    }

    double particlePionvsElectronId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::electron);
    }

    double particleElectrondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleElectronIdECL(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ECL;
      return pid->getProbability(Const::electron, Const::pion, set);
    }

    double particleMuonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::muon, Const::pion);
    }

    double particleMuondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::muon, Const::pion, set);

    }

    double particleMuonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particleMuonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::muon, Const::pion, set);
    }

    double particlePionId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::pion, Const::kaon);
    }

    double particlePiondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particlePionARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::pion, Const::kaon, set);
    }

    double particleKaonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::kaon, Const::pion);
    }

    double particleKaondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleKaonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::kaon, Const::pion, set);
    }

    double particleProtonId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      return pid->getProbability(Const::proton, Const::pion);
    }

    double particleProtondEdxId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::CDC + Const::SVD;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::TOP;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleProtonARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid) return 0.5;

      Const::PIDDetectorSet set = Const::ARICH;
      return pid->getProbability(Const::proton, Const::pion, set);
    }

    double particleMissingARICHId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::ARICH;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
    }

    double particleMissingTOPId(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::TOP;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
    }

    double particleMissingECL(const Particle* part)
    {
      const PIDLikelihood* pid = part->getRelatedTo<PIDLikelihood>();
      if (!pid)
        return 1.0;

      Const::PIDDetectorSet set = Const::ECL;
      if (not pid->isAvailable(set))
        return 1.0;

      return 0;
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

    double particleCharge(const Particle* part)
    {
      return part->getCharge();

    }

    // MC related ------------------------------------------------------------

    double isSignal(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      //abort early if PDG codes are different
      if (abs(mcparticle->getPDG()) != abs(part->getPDGCode()))
        return 0.0;

      int status    = MCMatching::getMCTruthStatus(part, mcparticle);
      //remove the following bits, these are usually ok
      status &= (~MCMatching::c_MissFSR);
      status &= (~MCMatching::c_MissNeutrino);
      //status &= (~MCMatching::c_DecayInFlight);

      return (status == MCMatching::c_Correct) ? 1.0 : 0.0;
    }

    double particleMCMatchPDGCode(const Particle* part)
    {
      const MCParticle* mcparticle = part->getRelatedTo<MCParticle>();
      if (mcparticle == nullptr)
        return 0.0;

      return mcparticle->getPDG();
    }

    double particleAbsMCMatchPDGCode(const Particle* part)
    {
      return std::abs(particleMCMatchPDGCode(part));
    }

    double particleMCMatchStatus(const Particle* part)
    {
      return MCMatching::getMCTruthStatus(part);
    }

    // RestOfEvent related --------------------------------------------------

    double nROETracks(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (roe)
        result = roe->getNTracks();

      return result;
    }

    double nROEClusters(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();

      if (roe)
        result = roe->getNECLClusters();

      return result;
    }

    // Recoil Kinematics related ---------------------------------------------

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

    // Extra energy --------------------------------------------------------

    double extraEnergy(const Particle* particle)
    {
      double result = -1.0;

      const RestOfEvent* roe = particle->getRelatedTo<RestOfEvent>();
      if (!roe)
        return result;

      const std::vector<ECLCluster*> remainECLClusters = roe->getECLClusters();
      result = 0.0;
      for (unsigned i = 0; i < remainECLClusters.size(); i++)
        result += remainECLClusters[i]->getEnergy();

      return result;
    }

    // ECLCluster related variables -----------------------------------------

    double eclClusterDetectionRegion(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        /// TODO: check if ECLCluster will provide this on its own
        float theta = shower->getMomentum().Theta();
        if (theta < 0.555) {
          result = 1.0;
        } else if (theta < 2.26) {
          result = 2.0;
        } else {
          result = 3.0;
        }
      }

      return result;
    }

    double goodGammaUncalibrated(const Particle* particle)
    {
      double energy = particle->getEnergy();
      double e9e25  = eclClusterE9E25(particle);
      int    region = eclClusterDetectionRegion(particle);

      bool goodGammaRegion1 = region > 0.5 && region < 1.5 && energy > 0.125 && e9e25 > 0.7;
      bool goodGammaRegion2 = region > 1.5 && region < 2.5 && energy > 0.100;
      bool goodGammaRegion3 = region > 2.5 && region < 3.5 && energy > 0.150;

      if (goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3)
        return 1.0;
      else
        return 0.0;
    }


    double goodGamma(const Particle* particle)
    {
      //double timing = eclClusterTiming(particle);
      double energy = particle->getEnergy();
      double e9e25  = eclClusterE9E25(particle);
      int    region = eclClusterDetectionRegion(particle);

      bool goodGammaRegion1 = region > 0.5 && region < 1.5 && energy > 0.085 && e9e25 > 0.7;
      bool goodGammaRegion2 = region > 1.5 && region < 2.5 && energy > 0.060;
      bool goodGammaRegion3 = region > 2.5 && region < 3.5 && energy > 0.110;
      //bool goodTiming       = timing > 800 && timing < 2400;

      //if ((goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3) && goodTiming)
      if (goodGammaRegion1 || goodGammaRegion2 || goodGammaRegion3)
        return 1.0;
      else
        return 0.0;
    }

    double eclClusterUncorrectedE(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getEnedepSum();
      }
      return result;
    }

    double eclClusterHighestE(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getHighestE();
      }
      return result;
    }

    double eclClusterTiming(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getTiming();
      }
      return result;
    }

    double eclClusterTheta(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getTheta();
      }
      return result;
    }

    double eclClusterPhi(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getPhi();
      }
      return result;
    }

    double eclClusterR(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getR();
      }
      return result;
    }



    double eclClusterE9E25(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getE9oE25();
      }
      return result;
    }

    double eclClusterNHits(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];

        result = shower->getNofCrystals();
      }
      return result;
    }

    double eclClusterTrackMatched(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getParticleType() == Particle::c_ECLCluster) {
        StoreArray<ECLCluster> ECLClusters;
        const ECLCluster* shower = ECLClusters[particle->getMdstArrayIndex()];
        const Track* track = shower->getRelated<Track>();

        if (track)
          result = 1.0;
      }
      return result;

    }

    // Decay Kinematics -------------------------------------------------------
    double particleDecayAngle(const Particle* particle)
    {
      double result = 0.0;

      if (particle->getNDaughters() != 2)
        return result;

      TLorentzVector motherMomentum = particle->get4Vector();
      TVector3       motherBoost    = -(motherMomentum.BoostVector());

      TLorentzVector daugMomentum = particle->getDaughter(0)->get4Vector();
      daugMomentum.Boost(motherBoost);

      result = cos(daugMomentum.Angle(motherMomentum.Vect()));

      return result;
    }

    double particleDaughterAngle(const Particle* particle)
    {
      if (particle->getNDaughters() != 2)
        return 0.0;

      const TVector3 a = particle->getDaughter(0)->getMomentum();
      const TVector3 b = particle->getDaughter(1)->getMomentum();
      return cos(a.Angle(b));
    }

    VARIABLE_GROUP("Kinematics");
    REGISTER_VARIABLE("p", particleP, "momentum magnitude");
    REGISTER_VARIABLE("E", particleE, "energy");
    REGISTER_VARIABLE("px", particlePx, "momentum component x");
    REGISTER_VARIABLE("py", particlePy, "momentum component y");
    REGISTER_VARIABLE("pz", particlePz, "momentum component z");
    REGISTER_VARIABLE("pt", particlePt, "transverse momentum");
    REGISTER_VARIABLE("cosTheta", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("cth", particleCosTheta, "momentum cosine of polar angle");
    REGISTER_VARIABLE("phi", particlePhi, "momentum azimuthal angle in degrees");

    REGISTER_VARIABLE("p_CMS", particleP_CMS, "CMS momentum magnitude");
    REGISTER_VARIABLE("E_CMS", particleE_CMS, "CMS energy");
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

    REGISTER_VARIABLE("M", particleMass, "invariant mass (determined from particle's 4-momentum vector)");
    REGISTER_VARIABLE("dM", particleDMass, "mass minus nominal mass");
    REGISTER_VARIABLE("Q", particleQ, "released energy in decay");
    REGISTER_VARIABLE("dQ", particleDQ, "released energy in decay minus nominal one");
    REGISTER_VARIABLE("Mbc", particleMbc, "beam constrained mass");
    REGISTER_VARIABLE("deltaE", particleDeltaE, "energy difference");

    REGISTER_VARIABLE("InvM", particleInvariantMass, "invariant mass (determined from particle's daughter 4-momentum vectors)");
    REGISTER_VARIABLE("ErrM", particleInvariantMassError, "uncertainty of invariant mass (determined from particle's daughter 4-momentum vectors)");

    REGISTER_VARIABLE("decayAngle", particleDecayAngle, "cosine of the angle between the mother momentum vector and the direction of the first daughter in the mother's rest frame");
    REGISTER_VARIABLE("daughterAngle", particleDaughterAngle, "cosine of the angle between the first two daughters, in lab frame");

    VARIABLE_GROUP("PID");
    REGISTER_VARIABLE("eid", particleElectronId, "electron identification probability");
    REGISTER_VARIABLE("muid", particleMuonId, "muon identification probability");
    REGISTER_VARIABLE("piid", particlePionId, "pion identification probability");
    REGISTER_VARIABLE("Kid", particleKaonId, "kaon identification probability");
    REGISTER_VARIABLE("prid", particleProtonId, "proton identification probability");

    REGISTER_VARIABLE("K_vs_piid", particleKaonId, "kaon vs pion identification probability");
    REGISTER_VARIABLE("pi_vs_eid", particlePionvsElectronId, "pion vs electron identification probability");

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
    REGISTER_VARIABLE("missing_TOP", particleMissingTOPId, "1.0 if identification probability from TOP is missing");

    REGISTER_VARIABLE("eid_ARICH", particleElectronARICHId, "electron identification probability from ARICH");
    REGISTER_VARIABLE("muid_ARICH", particleMuonARICHId, "muon identification probability from ARICH");
    REGISTER_VARIABLE("piid_ARICH", particlePionARICHId, "pion identification probability from ARICH");
    REGISTER_VARIABLE("Kid_ARICH", particleKaonARICHId, "kaon identification probability from ARICH");
    REGISTER_VARIABLE("prid_ARICH", particleProtonARICHId, "proton identification probability from ARICH");
    REGISTER_VARIABLE("missing_ARICH", particleMissingARICHId, "1.0 if identification probability from ARICH is missing");

    REGISTER_VARIABLE("eid_ECL", particleElectronIdECL, "electron identification probability from ECL");
    REGISTER_VARIABLE("missing_ECL", particleMissingECL, "1.0 if identification probability from ECL is missing");

    VARIABLE_GROUP("PID variables for NeuroBayes (-999 added)");
    REGISTER_VARIABLE("NB_eid_TOP", NeuroBayesifyTOP<particleElectronTOPId>, "electron identification probability from TOP (returns -999 when not available)");
    REGISTER_VARIABLE("NB_muid_TOP", NeuroBayesifyTOP<particleMuonTOPId>, "muon identification probability from TOP (returns -999 when not available)");
    REGISTER_VARIABLE("NB_piid_TOP", NeuroBayesifyTOP<particlePionTOPId>, "pion identification probability from TOP (returns -999 when not available)");
    REGISTER_VARIABLE("NB_Kid_TOP", NeuroBayesifyTOP<particleKaonTOPId>, "kaon identification probability from TOP (returns -999 when not available)");
    REGISTER_VARIABLE("NB_prid_TOP", NeuroBayesifyTOP<particleProtonTOPId>, "proton identification probability from TOP (returns -999 when not available)");

    REGISTER_VARIABLE("NB_eid_ARICH", NeuroBayesifyARICH<particleElectronARICHId>, "electron identification probability from ARICH (returns -999 when not available)");
    REGISTER_VARIABLE("NB_muid_ARICH", NeuroBayesifyARICH<particleMuonARICHId>, "muon identification probability from ARICH (returns -999 when not available)");
    REGISTER_VARIABLE("NB_piid_ARICH", NeuroBayesifyARICH<particlePionARICHId>, "pion identification probability from ARICH (returns -999 when not available)");
    REGISTER_VARIABLE("NB_Kid_ARICH", NeuroBayesifyARICH<particleKaonARICHId>, "kaon identification probability from ARICH (returns -999 when not available)");
    REGISTER_VARIABLE("NB_prid_ARICH", NeuroBayesifyARICH<particleProtonARICHId>, "proton identification probability from ARICH (returns -999 when not available)");

    VARIABLE_GROUP("MC Matching");
    REGISTER_VARIABLE("isSignal", isSignal,               "1.0 if Particle is correctly reconstructed (SIGNAL), 0.0 otherwise");
    REGISTER_VARIABLE("mcPDG",    particleMCMatchPDGCode, "The PDG code of matched MCParticle");
    REGISTER_VARIABLE("abs_mcPDG", particleAbsMCMatchPDGCode, "The absolute PDG code of matched MCParticle");
    REGISTER_VARIABLE("mcStatus", particleMCMatchStatus,  "The bit pattern indicating the quality of MC match (see MCMatching::MCMatchStatus)");

    VARIABLE_GROUP("Rest Of Event");
    REGISTER_VARIABLE("nROETracks",  nROETracks,  "number of remaining tracks as given by the related RestOfEvent object");
    REGISTER_VARIABLE("nROEClusters", nROEClusters, "number of remaining ECL clusters as given by the related RestOfEvent object");

    VARIABLE_GROUP("Miscellaneous");
    REGISTER_VARIABLE("chiProb", particlePvalue, "chi^2 probability of the fit");
    REGISTER_VARIABLE("nDaughters", particleNDaughters, "number of daughter particles");
    REGISTER_VARIABLE("flavor", particleFlavorType, "flavor type of decay (0=unflavored, 1=flavored)");
    REGISTER_VARIABLE("charge", particleCharge, "charge of particle");

    REGISTER_VARIABLE("pRecoil",  recoilMomentum,    "magnitude of 3-momentum recoiling against given Particle");
    REGISTER_VARIABLE("eRecoil",  recoilEnergy,      "energy recoiling against given Particle");
    REGISTER_VARIABLE("mRecoil",  recoilMass,        "invariant mass of the system recoiling against given Particle");
    REGISTER_VARIABLE("m2Recoil", recoilMassSquared, "invariant mass squared of the system recoiling against given Particle");

    REGISTER_VARIABLE("eextra", extraEnergy, "extra energy in the calorimeter that is not associated to the given Particle");

    VARIABLE_GROUP("ECL Cluster related");
    REGISTER_VARIABLE("goodGamma",         goodGamma, "1.0 if photon candidate passes good photon selection criteria");
    REGISTER_VARIABLE("goodGammaUnCal",    goodGammaUncalibrated, "1.0 if photon candidate passes good photon selection criteria (to be used if photon's energy is not calibrated)");
    REGISTER_VARIABLE("clusterReg",        eclClusterDetectionRegion, "detection region in the ECL [1 - forward, 2 - barrel, 3 - backward]");
    REGISTER_VARIABLE("clusterE9E25",      eclClusterE9E25,           "ratio of energies in inner 3x3 and 5x5 cells");
    REGISTER_VARIABLE("clusterNHits",      eclClusterNHits,           "number of hits associated to this cluster");
    REGISTER_VARIABLE("clusterTrackMatch", eclClusterTrackMatched,    "number of charged track matched to this cluster");
  }
}

