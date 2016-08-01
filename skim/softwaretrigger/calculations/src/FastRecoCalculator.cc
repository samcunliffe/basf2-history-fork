/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <skim/softwaretrigger/calculations/FastRecoCalculator.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <TVector3.h>

namespace Belle2 {
  namespace SoftwareTrigger {
    void FastRecoCalculator::requireStoreArrays()
    {
      m_cdcRecoTracks.isRequired("CDCRecoTracks");
      m_eclClusters.isRequired();
    };

    template <class T>
    TLorentzVector getFourVector(const T& item);

    template <>
    TLorentzVector getFourVector(const ECLCluster& cluster)
    {
      return TLorentzVector(cluster.getPx(), cluster.getPy(), cluster.getPz(), cluster.getEnergy());
    }

    template <>
    TLorentzVector getFourVector(const RecoTrack& cluster)
    {
      const TVector3& positionSeed = cluster.getPositionSeed();
      return TLorentzVector(positionSeed.X(), positionSeed.Y(), positionSeed.Z(),
                            sqrt(positionSeed.Mag2() + Const::pionMass * Const::pionMass));
    }

    template <class T>
    std::vector<double> getSortedEnergiesFrom(const StoreArray<T>& storeArray)
    {
      std::vector<TLorentzVector> lorentzVectors;
      lorentzVectors.reserve(storeArray.getEntries());

      for (const T& item : storeArray) {
        const TLorentzVector& fourVector = getFourVector(item);
        lorentzVectors.push_back(std::move(PCmsLabTransform::labToCms(fourVector)));
      }

      std::sort(lorentzVectors.begin(), lorentzVectors.end(),
      [](const TLorentzVector & lhs, const TLorentzVector & rhs) {
        return lhs.Rho() < rhs.Rho();
      });

      std::vector<double> energies;
      energies.reserve(lorentzVectors.size());

      for (const TLorentzVector& lorentzVector : lorentzVectors) {
        energies.push_back(std::move(lorentzVector.Energy()));
      }

      return energies;
    }

    void FastRecoCalculator::doCalculation(SoftwareTriggerObject& calculationResult) const
    {
      std::vector<TVector3> momenta;
      momenta.reserve(m_cdcRecoTracks.getEntries());

      for (const RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
        momenta.push_back(std::move(cdcRecoTrack.getMomentumSeed()));
      }

      // TODO: Do only return the first (we do not need more).
      const std::vector<double>& sortedRhoECLEnergyList = getSortedEnergiesFrom(m_eclClusters);
      const std::vector<double>& sortedRhoCDCEnergyList = getSortedEnergiesFrom(m_cdcRecoTracks);

      double visibleEnergy = 0;
      if (not momenta.empty()) {
        visibleEnergy = std::accumulate(momenta.begin(), momenta.end(), visibleEnergy,
        [](const double & value, const TVector3 & momentum) {
          return value + momentum.Mag();
        });
      }
      if (m_eclClusters.getEntries() > 0) {
        visibleEnergy = std::accumulate(std::begin(m_eclClusters), std::end(m_eclClusters), visibleEnergy,
        [](const double & value, const ECLCluster & eclCluster) {
          return value + eclCluster.getMomentum().Mag();
        });
      }

      calculationResult["visible_energy"] = visibleEnergy;

      if (sortedRhoECLEnergyList.empty()) {
        calculationResult["highest_1_ecl"] = 0;
      } else {
        calculationResult["highest_1_ecl"] = sortedRhoECLEnergyList.front();
      }

      if (momenta.empty()) {
        calculationResult["max_pt"] = 0;
        calculationResult["max_pz"] = 0;
      } else {
        calculationResult["max_pt"] = std::max_element(momenta.begin(), momenta.end(),
        [](const TVector3 & lhs, const TVector3 & rhs) {
          return rhs.Pt() > lhs.Pt();
        })->Pt();

        calculationResult["max_pz"] = std::max_element(momenta.begin(), momenta.end(),
        [](const TVector3 & lhs, const TVector3 & rhs) {
          return rhs.Z() > lhs.Z();
        })->Z();
      }

      if (sortedRhoCDCEnergyList.empty()) {
        calculationResult["first_highest_cdc_energies"] = 0;
      } else {
        calculationResult["first_highest_cdc_energies"] = sortedRhoCDCEnergyList.front();
      }
    }

  }
}