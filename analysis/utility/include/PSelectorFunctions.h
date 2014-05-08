/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PSELECTORFUNCTIONS_H
#define PSELECTORFUNCTIONS_H

#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/PIDLikelihood.h>

namespace Belle2 {

  namespace analysis {

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum magnitude
     */
    double particleP(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return energy
     */
    double particleE(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component x
     */
    double particlePx(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component y
     */
    double particlePy(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component z
     */
    double particlePz(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse momentum
     */
    double particlePt(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return  momentum cosine of polar angle
     */
    double particleCosTheta(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum azimuthal angle
     */
    double particlePhi(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum magnitude in CMS
     */
    double particleP_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return energy in CMS
     */
    double particleE_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component x in CMS
     */
    double particlePx_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component y in CMS
     */
    double particlePy_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum component z in CMS
     */
    double particlePz_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse momentum in CMS
     */
    double particlePt_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum cosine of polar angle in CMS
     */
    double particleCosTheta_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return momentum azimuthal angle in CMS
     */
    double particlePhi_CMS(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in x relative to interaction point
     */
    double particleDX(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in y relative to interaction point
     */
    double particleDY(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return position in z relative to interaction point
     */
    double particleDZ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return transverse distance relative to interaction point
     */
    double particleDRho(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return mass (determined from particle's 4-momentum vector)
     */
    double particleMass(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return mass minus nominal mass
     */
    double particleDMass(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMass(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return uncertainty of the invariant mass (determined from particle's daughter 4-momentum vectors)
     */
    double particleInvariantMassError(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return released energy in decay
     */
    double particleQ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return released energy in decay minus nominal one
     */
    double particleDQ(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return beam constrained mass
     */
    double particleMbc(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return energy difference in CMS
     */
    double particleDeltaE(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleElectronId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleElectrondEdxId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleElectronTOPId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleElectronARICHId(const Particle* part);
    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return muon Id
     */
    double particleMuonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleMuondEdxId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleMuonTOPId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleMuonARICHId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return pion Id
     */
    double particlePionId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particlePiondEdxId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particlePionTOPId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particlePionARICHId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return kaon Id
     */
    double particleKaonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleKaondEdxId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleKaonTOPId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleKaonARICHId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return proton Id
     */
    double particleProtonId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleProtondEdxId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleProtonTOPId(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return electron Id
     */
    double particleProtonARICHId(const Particle* part);


    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return prob(chi^2,ndf) of fit
     */
    double particlePvalue(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return number of daughter particles
     */
    double particleNDaughters(const Particle* part);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return flavor type
     */
    double particleFlavorType(const Particle* part);


    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return product of signal probabilities of daughters
     */
    double prodChildProb(const Particle* particle);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return signal probability of daughter particle
     */
    template<unsigned int N>
    double childProb(const Particle* particle)
    {
      return particle->getDaughter(N)->getExtraInfo("SignalProbability");
    }

    /**
     * NeuroBayesifys this function
     * @param part cconst pointer to Particle
     * @return function value, except if information from ARICH is not available which is set to -999
     */
    template<double(*T)(const Particle*)>
    double NeuroBayesifyARICH(const Particle* particle)
    {

      const PIDLikelihood* pid = particle->getRelatedTo<PIDLikelihood>();
      if (!pid)
        return -999;

      Const::PIDDetectorSet set = Const::ARICH;
      if (not pid->isAvailable(set))
        return -999;
      return T(particle);
    }

    /**
     * NeuroBayesifys this function
     * @param part cconst pointer to Particle
     * @return function value, except if information from TOP is not available which is set to -999
     */
    template<double(*T)(const Particle*)>
    double NeuroBayesifyTOP(const Particle* particle)
    {

      const PIDLikelihood* pid = particle->getRelated<PIDLikelihood>();
      if (!pid)
        return -999;

      Const::PIDDetectorSet set = Const::TOP;
      if (not pid->isAvailable(set))
        return -999;
      return T(particle);
    }

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return 1 if ARICH Id is missing
     */
    double particleMissingARICHId(const Particle*);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return 1 if TOPId is missing
     */
    double particleMissingTOPId(const Particle*);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return 1 if Particle is correctly reconstructed (SIGNAL), 0 otherwise
     */
    double isSignal(const Particle* particle);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return absolute PDG code of matched MCParticle
     */
    double particleAbsMCMatchPDGCode(const Particle* particle);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return PDG code of matched MCParticle
     */
    double particleMCMatchPDGCode(const Particle* particle);

    /**
     * function for PSelector
     * @param part const pointer to Particle
     * @return ORed combination of MCMatchStatus flags.
     */
    double particleMCMatchStatus(const Particle* particle);

    /**
     * function for PSelector
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     *
     * @param part const pointer to Particle
     * @return number of remaining tracks as given by the related RestOfEvent object
     */
    double nROETracks(const Particle* particle);

    /**
     * function for PSelector
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     *
     * @param part const pointer to Particle
     * @return number of remaining ECL clusters as given by the related RestOfEvent object
     */
    double nROEClusters(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return magnitude of 3-momentum recoiling against given Particle
     */
    double recoilMomentum(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return energy recoiling against given Particle
     */
    double recoilEnergy(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return invariant mass of the system recoiling against given Particle
     */
    double recoilMass(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return invarian mass squared of the system recoiling against given Particle
     */
    double recoilMassSquared(const Particle* particle);

    /**
     * function for PSelector
     * requires that RestOfEvent <-> Particle relation exists (returns -1 if it doesn't)
     *
     * @param part const pointer to Particle
     * @return extra energy in the calorimeter that is not associated to the given Particle
     */
    double extraEnergy(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return 1/2/3 if the ECL Cluster is detected in the forward/barrel/backward region
     */
    double eclClusterDetectionRegion(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return ratio of energies in inner 3x3 and 5x5 cells
     */
    double eclClusterE9E25(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return number of hits associated to this cluster
     */
    double eclClusterNHits(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return 1/0 if charged track is/is not Matched to this cluster
     */
    double eclClusterTrackMatched(const Particle* particle);

    /**
     * function for PSelector
     *
     * @param part const pointer to Particle
     * @return cosine of the angle between the mother momentum vector and the direction of the first daughter in the mother's rest frame
     */
    double particleDecayAngle(const Particle* particle);

  }
} // Belle2 namespace

#endif
