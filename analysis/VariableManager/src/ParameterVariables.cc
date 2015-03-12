/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Keck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VariableManager/ParameterVariables.h>
#include <analysis/VariableManager/Manager.h>
#include <analysis/dataobjects/EventExtraInfo.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ContinuumSuppression.h>

#include <framework/logging/Logger.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

#include <TLorentzVector.h>
#include <TVectorF.h>
#include <TVector3.h>

namespace Belle2 {
  namespace Variable {

    double NumberOfMCParticlesInEvent(const Particle*, const std::vector<double>& pdg)
    {
      StoreArray<MCParticle> mcParticles;
      int counter = 0;
      for (int i = 0; i < mcParticles.getEntries(); ++i) {
        if (mcParticles[i]->getStatus(MCParticle::c_PrimaryParticle) and abs(mcParticles[i]->getPDG()) == pdg[0])
          counter++;
      }
      return counter;
    }

    double daughterInvariantMass(const Particle* particle, const std::vector<double>& daughter_indexes)
    {
      if (!particle)
        return -999;

      TLorentzVector sum;
      const std::vector<Particle*> daughters = particle->getDaughters();
      int nDaughters = int(daughters.size());

      for (auto & double_daughter : daughter_indexes) {
        int daughter = static_cast<int>(double_daughter + 0.5);
        if (daughter >= nDaughters)
          return -999;

        sum += daughters[daughter]->get4Vector();
      }

      return sum.M();
    }

    double massDifference(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      int daughter = static_cast<int>(daughters[0] + 0.5);
      if (daughter >= int(particle->getNDaughters()))
        return -999;

      double motherMass = particle->getMass();
      double daughterMass = particle->getDaughter(daughter)->getMass();

      return motherMass - daughterMass;
    }

    double massDifferenceError(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      int daughter = static_cast<int>(daughters[0] + 0.5);
      if (daughter >= int(particle->getNDaughters()))
        return -999;

      float result = 0.0;

      TLorentzVector thisDaughterMomentum = particle->getDaughter(daughter)->get4Vector();

      TMatrixFSym thisDaughterCovM(Particle::c_DimMomentum);
      thisDaughterCovM = particle->getDaughter(daughter)->getMomentumErrorMatrix();
      TMatrixFSym othrDaughterCovM(Particle::c_DimMomentum);

      for (int j = 0; j < int(particle->getNDaughters()); ++j) {
        if (j == daughter)
          continue;

        othrDaughterCovM += particle->getDaughter(j)->getMomentumErrorMatrix();
      }

      TMatrixFSym covarianceMatrix(2 * Particle::c_DimMomentum);
      covarianceMatrix.SetSub(0, thisDaughterCovM);
      covarianceMatrix.SetSub(4, othrDaughterCovM);

      double motherMass = particle->getMass();
      double daughterMass = particle->getDaughter(daughter)->getMass();

      TVectorF    jacobian(2 * Particle::c_DimMomentum);
      jacobian[0] =  thisDaughterMomentum.Px() / daughterMass - particle->getPx() / motherMass;
      jacobian[1] =  thisDaughterMomentum.Py() / daughterMass - particle->getPy() / motherMass;
      jacobian[2] =  thisDaughterMomentum.Pz() / daughterMass - particle->getPz() / motherMass;
      jacobian[3] =  particle->getEnergy() / motherMass - thisDaughterMomentum.E() / daughterMass;
      jacobian[4] = -1.0 * particle->getPx() / motherMass;
      jacobian[5] = -1.0 * particle->getPy() / motherMass;
      jacobian[6] = -1.0 * particle->getPz() / motherMass;
      jacobian[7] =  1.0 * particle->getEnergy() / motherMass;

      result = jacobian * (covarianceMatrix * jacobian);

      if (result < 0.0)
        result = 0.0;

      return TMath::Sqrt(result);
    }

    double massDifferenceSignificance(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      int daughter = static_cast<int>(daughters[0] + 0.5);
      if (daughter >= int(particle->getNDaughters()))
        return -999;

      double massDiff = massDifference(particle, daughters);
      double massDiffErr = massDifferenceError(particle, daughters);

      double massDiffNominal = particle->getPDGMass() - particle->getDaughter(daughter)->getPDGMass();

      return (massDiff - massDiffNominal) / massDiffErr;
    }

    // Decay Kinematics -------------------------------------------------------
    double particleDecayAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      double result = 0.0;

      TLorentzVector motherMomentum = particle->get4Vector();
      TVector3       motherBoost    = -(motherMomentum.BoostVector());

      int daughter = static_cast<int>(daughters[0] + 0.5);
      if (daughter >= int(particle->getNDaughters()))
        return -999;

      TLorentzVector daugMomentum = particle->getDaughter(daughter)->get4Vector();
      daugMomentum.Boost(motherBoost);

      result = cos(daugMomentum.Angle(motherMomentum.Vect()));

      return result;
    }

    double particleDaughterAngle(const Particle* particle, const std::vector<double>& daughters)
    {
      if (!particle)
        return -999;

      int nDaughters = int(particle->getNDaughters());
      if (nDaughters != 2)
        return -999;

      int daughter1 = static_cast<int>(daughters[0] + 0.5);
      int daughter2 = static_cast<int>(daughters[1] + 0.5);
      if (daughter1 >= nDaughters || daughter2 >= nDaughters)
        return -999;

      const TVector3 a = particle->getDaughter(daughter1)->getMomentum();
      const TVector3 b = particle->getDaughter(daughter2)->getMomentum();
      return cos(a.Angle(b));
    }

    // Continuum Suppression --------------------------------------------------
    double CleoCones(const Particle* particle, const std::vector<double>& cone)
    {
      if (!particle)
        return -999;

      const ContinuumSuppression* qq = particle->getRelatedTo<ContinuumSuppression>();
      if (!qq)
        return -999;

      const auto& cleoCones = qq->getCleoCones();
      return cleoCones.at(cone[0] - 1);
    }

    // Continuum Suppression --------------------------------------------------
    double Constant(const Particle*, const std::vector<double>& constant)
    {
      return constant[0];
    }

    // Flavor Information in Transition --------------------------------------------------
    double hasCharmedDaughter(const Particle* particle, const std::vector<double>& transition)
    {
      int Status = 0;

      if (abs(transition[0]) != 1)
        B2ERROR("The parameter variable hasCharmedDaughter() only accepts 1 or -1 as an argument.");

      if (!particle)
        Status = -999;

      int nDaughters = int(particle->getNDaughters());
      if (nDaughters < 1)
        Status = -999;

      int motherPDGSign = (particle->getPDGCode()) / (abs(particle->getPDGCode()));
      const std::vector<Particle*> daughters = particle->getDaughters();

      for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
        int daughterPDG = daughters[iDaughter]->getPDGCode();
        int daughterPDGSign = daughterPDG / (abs(daughterPDG));

        if (transition[0] == 1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4) && motherPDGSign == daughterPDGSign) // charmed meson or baryon and b->anti-c transition
            Status = 1;
        } else if (transition[0] == -1) {
          if (((abs(daughterPDG) / 100) % 10 == 4 || (abs(daughterPDG) / 1000) % 10 == 4) && motherPDGSign == -daughterPDGSign) // charmed meson or baryon and b->c transition
            Status = 1;
        }
      }
      return Status;
    }

    VARIABLE_GROUP("ParameterFunctions");
    REGISTER_VARIABLE("NumberOfMCParticlesInEvent(pdg)", NumberOfMCParticlesInEvent , "Returns number of MC Particles (including anti particles) with the given pdg in the event.");
    REGISTER_VARIABLE("daughterInvariantMass(i, j, ...)", daughterInvariantMass , "Returns invariant mass of the given daughter particles.");
    REGISTER_VARIABLE("decayAngle(i)", particleDecayAngle, "cosine of the angle between the mother momentum vector and the direction of the i-th daughter in the mother's rest frame");
    REGISTER_VARIABLE("daughterAngle(i,j)", particleDaughterAngle, "cosine of the angle between i-th and j-th daughters, in lab frame");

    REGISTER_VARIABLE("massDifference(i)", massDifference, "Difference in invariant masses of this particle and its i-th daughter");
    REGISTER_VARIABLE("massDifferenceError(i)", massDifferenceError, "Estimated uncertainty on difference in invariant masses of this particle and its i-th daughter");
    REGISTER_VARIABLE("massDifferenceSignificance(i)", massDifferenceSignificance, "Signed significance of the deviation from the nominal mass difference of this particle and its i-th daughter [(massDiff - NOMINAL_MASS_DIFF)/ErrMassDiff]");

    REGISTER_VARIABLE("constant(i)", Constant, "Returns constant number");

    REGISTER_VARIABLE("CleoCone(i)", CleoCones, "Cleo cones (i-th cone)");

    REGISTER_VARIABLE("hasCharmedDaughter(i)", hasCharmedDaughter, "Returns information regarding the charm quark presence in the decay.");
  }
}
