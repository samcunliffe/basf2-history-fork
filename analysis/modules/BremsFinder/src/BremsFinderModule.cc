/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Alejandro Mora                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/BremsFinder/BremsFinderModule.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/PIDLikelihood.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <cmath>
#include <algorithm>
#include <TMatrixFSym.h>

using namespace std;

namespace Belle2 {
//-----------------------------------------------------------------
//                 Register module
//-----------------------------------------------------------------

  REG_MODULE(BremsFinder)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

  BremsFinderModule::BremsFinderModule() :
    Module(), m_pdgCode(0)
  {
    // set module description (e.g. insert text)
    setDescription(R"DOC(This module copies each particle in the `inputList` to the `outputList` and uses
    the results of the **eclTrackBremFinder** module to look for possible bremsstrahlung photons; if these 
    photons exists, it adds their four momentum to the particle in the `outputList`. 
    It also adds the original particle and these photons as daughters of the new, corrected particle. 
    Track and PID information of the original particle are copied onto the new one to facilitate their access 
    in the analysis scripts.

    The **eclTrackBremFinder** module uses the lepton track PXD and SVD hits and extrapolates them to the ECL; 
    then looks for ECL clusters with energies between 0.2 and 1 times the track energy and without associated 
    tracks, and checks if the distance between each of these clusters 
    and the extrapolated hit is smaller than 0.5 mm. If it is, a *Bremsstrahlung* weighted relation 
    between said cluster and the track is established. The weight is determined as

    .. math:: 
                   
       \text{max}\left(\frac{\left|\phi_{\text{cluster}}-\phi_{\text{hit}}\right|}{\Delta\phi_{\text{cluster}}+\Delta\phi_{\text{hit}}}, \, \frac{\left|\theta_{\text{cluster}}-\theta_{\text{hit}}\right|}{\Delta\theta_{\text{cluster}}+\Delta\theta_{\text{hit}}}\right)

    where :math:`\phi_i` and :math:`\theta_i` are the azimutal and polar angles of the ECL cluster and the 
    extrapolated hit, and :math:`\Delta x` represents the uncertainty of the value :math:`x`. The details of 
    the calculation of these quantities are `here`_. By default, only relations with a weight
    smaller than 3.0 are stored. The user can further determine the maximum value of this weight required in order 
    to perform the bremsstrahlung correction.

    This module looks for photons in the `gammaList` whose clusters have a *Bremsstrahlung* relation with the track 
    of one of the particles in the `inputList`, and adds their 4-momentum to the particle's one. It also stores the value
    of each relation weight as `extraInfo` of the corrected particle, under the name `"bremsWeightWithPhotonN"`, where
    N is the index of the photon as daughter of the corrected particle; thus `"bremsWeightWithPhoton0" gives the weight
    of the Bremsstrahlung relation between the new, corrected particle, and the first photon daughter.

    Warning:
      Even in the event of no bremsstrahlung photons found, a new particle is still created, and the original one is still 
      added as its daughter.
                  
    See also:
      `eclTrackBremFinder module`_
                   
    .. _eclTrackBremFinder module: https://stash.desy.de/projects/B2/repos/software/browse/ecl/modules/eclTrackBremFinder
    .. _here: https://stash.desy.de/projects/B2/repos/software/browse/ecl/modules/eclTrackBremFinder/src/BremFindingMatchCompute.cc)DOC");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("outputList", m_outputListName, "The output particle list name.");
    addParam("inputList", m_inputListName,
             R"DOC(The initial particle list name containing the particles to correct. *It should already exist* and *the particles must have an associated track.*)DOC");
    addParam("gammaList", m_gammaListName,
             R"DOC(The photon list containing the preselected bremsstrahlung candidates. *It should already exist* and *the particles in the list must be photons*)DOC");
    addParam("maximumAcceptance", m_maximumAcceptance,
             "The maximum value of the relation weight between a bremsstrahlung cluster and a particle track",
             m_maximumAcceptance);
    addParam("multiplePhotons", m_addMultiplePhotons, "If true, use all possible photons to correct the particle's 4-momentum",
             m_addMultiplePhotons);
    addParam("writeOut", m_writeOut,
             R"DOC(If true, the output `ParticleList` will be saved by `RootOutput`. If false, it will be ignored when writing the file.)DOC",
             m_writeOut);
  }

  void BremsFinderModule::initialize()
  {
    //Get input lepton particle list
    m_inputList.isRequired(m_inputListName);

    DecayDescriptor decDes;
    decDes.init(m_inputListName);

    m_pdgCode  = decDes.getMother()->getPDGCode();
    //Check for the particles in the lepton list to have a track associated
    if (Const::chargedStableSet.find(abs(m_pdgCode)) == Const::invalidParticle)
      B2ERROR("[BremsFinderModule] Invalid particle list. the particles in " << m_inputListName << " should have an associated track.");

    //Get input photon particle list
    m_gammaList.isRequired(m_gammaListName);

    decDes.init(m_gammaListName);
    int temp_pdg = decDes.getMother()->getPDGCode();

    //Check that this is a gamma list
    if (temp_pdg != Const::photon.getPDGCode())
      B2ERROR("[BremsFinderModule] Invalid particle list. the particles in " << m_gammaListName << " should be photons!");

    decDes.init(m_outputListName);
    temp_pdg = decDes.getMother()->getPDGCode();
    // check the validity of output ParticleList name
    if (m_inputListName == m_outputListName)
      B2ERROR("[BremsFinderModule] Input and output particle list names are the same: " << m_inputListName);
    else if (temp_pdg != m_pdgCode) {
      B2ERROR("[BremsFinderModule] The input and output particle list correspond to different particles: " << m_inputListName << " --> "
              << m_outputListName);
    }

    // output particle
    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);

    // make output lists
    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_outputList.registerInDataStore(m_outputListName, flags);
    m_outputAntiList.registerInDataStore(m_outputAntiListName, flags);

    StoreArray<Particle> particles;
    StoreArray<PIDLikelihood> pidlikelihoods;
    particles.registerRelationTo(pidlikelihoods);
  }

  void BremsFinderModule::event()
  {
    StoreArray<Particle> particles;
    StoreArray<MCParticle> mcParticles;

    RelationArray particlesToMCParticles(particles, mcParticles);

    // new output particle list
    m_outputList.create();
    m_outputList->initialize(m_pdgCode, m_outputListName);

    m_outputAntiList.create();
    m_outputAntiList->initialize(-1 * m_pdgCode, m_outputAntiListName);
    m_outputAntiList->bindAntiParticleList(*(m_outputList));

    // Number of photons (calculate it here only once)
    const unsigned int nGamma = m_gammaList->getListSize();

    // loop over charged particles, correct them and add them to the output list
    const unsigned int nLep = m_inputList->getListSize();
    for (unsigned i = 0; i < nLep; i++) {
      const Particle* lepton = m_inputList->getParticle(i);

      //Get the track of this lepton
      auto track = lepton->getTrack();

      //... and get the bremsstrahlung clusters related to this track
      const std::string relationName = "Bremsstrahlung";
      RelationVector<ECLCluster> bremClusters = track->getRelationsFrom<ECLCluster>("", relationName);

      std::vector<std::pair <double, Particle*> > selectedGammas;

      unsigned j = 0;
      for (auto bremCluster = bremClusters.begin(); bremCluster != bremClusters.end(); bremCluster++, j++) {
        double weight = bremClusters.weight(j);

        if (weight > m_maximumAcceptance) continue;

        for (unsigned k = 0; k < nGamma; k++) {

          Particle* gamma = m_gammaList->getParticle(k);
          auto cluster = gamma->getECLCluster();

          if (bremCluster->getClusterId() == cluster->getClusterId()) {
            selectedGammas.push_back(std::make_pair(weight, gamma));
          }

        } // Closes for loop on gammas

      } // Closes for loop on brem clusters

      //The 4-momentum of the new lepton in the output particle list
      TLorentzVector new4Vec = lepton->get4Vector();

      //Sort weight-particle pairs by weight. Smaller weights go first
      std::sort(selectedGammas.begin(), selectedGammas.end());

      //Add to this 4-momentum those of the selected photon(s)
      if (selectedGammas.size() > 0) { //for the case of more than one brems photon
        if (m_addMultiplePhotons) {
          for (auto const& bremsPair : selectedGammas) {
            Particle* g = bremsPair.second;
            new4Vec += g->get4Vector();
          }
        } else { //If not multiple gammas, add the photon with the smallest weight
          Particle* g = selectedGammas[0].second;
          new4Vec += g->get4Vector();
        }
      }

      //Create the new particle with the 4-momentum calculated before
      Particle correctedLepton(new4Vec, lepton->getPDGCode(), Particle::EFlavorType::c_Flavored, Particle::c_Track,
                               track->getArrayIndex());

      //And add the original lepton as its daughter
      correctedLepton.appendDaughter(lepton, false);

      const TMatrixFSym& lepErrorMatrix = lepton->getMomentumVertexErrorMatrix();
      TMatrixFSym corLepMatrix(lepErrorMatrix);

      bool bremsGammaFound = false;
      //Now, if there are any, add the brems photons as daughters as well. As before, we distinguish between the multiple and only one brems photon cases
      if (selectedGammas.size() > 0) {
        bremsGammaFound = true;
        if (m_addMultiplePhotons) {
          int photonIndex = 0;
          for (auto const& bremsPair : selectedGammas) {
            //Add the weights as extra info of the mother
            Particle* bremsGamma = bremsPair.second;
            std::string extraInfoName = "bremsWeightWithPhoton" + std::to_string(photonIndex);
            correctedLepton.addExtraInfo(extraInfoName, bremsPair.first);
            photonIndex++;

            const TMatrixFSym& gammaErrorMatrix = bremsGamma->getMomentumVertexErrorMatrix();
            for (int irow = 0; irow <= 3; irow++) {
              for (int icol = irow; icol <= 3; icol++) corLepMatrix(irow, icol) += gammaErrorMatrix(irow, icol);
            }
            correctedLepton.appendDaughter(bremsGamma, false);
            B2DEBUG(1, "[BremsFinderModule] Found a bremsstrahlung gamma and added its 4-vector to the charged particle");
          }
        } else {
          auto bestPair = selectedGammas[0];
          Particle* bestGamma = bestPair.second;
          std::string extraInfoName = "bremsWeightWithPhoton0";
          correctedLepton.addExtraInfo(extraInfoName, bestPair.first);

          const TMatrixFSym& gammaErrorMatrix = bestGamma->getMomentumVertexErrorMatrix();
          for (int irow = 0; irow <= 3; irow++) {
            for (int icol = irow; icol <= 3; icol++) corLepMatrix(irow, icol) += gammaErrorMatrix(irow, icol);
          }
          correctedLepton.appendDaughter(bestGamma, false);
          B2DEBUG(1, "[BremsFinderModule] Found a bremsstrahlung gamma and added its 4-vector to the charged particle");
        }
      }

      correctedLepton.setMomentumVertexErrorMatrix(corLepMatrix);

      // add the info from original lepton to the new lepton
      correctedLepton.setVertex(lepton->getVertex());
      correctedLepton.setPValue(lepton->getPValue());
      correctedLepton.addExtraInfo("bremsCorrected", float(bremsGammaFound));

      // add the mc relation
      Particle* newLepton = particles.appendNew(correctedLepton);
      const MCParticle* mcLepton = lepton->getRelated<MCParticle>();
      const PIDLikelihood* pid = lepton->getPIDLikelihood();

      if (pid) newLepton->addRelationTo(pid);

      if (mcLepton != nullptr) newLepton->addRelationTo(mcLepton);

      m_outputList->addParticle(newLepton);

    } //Closes for loop on leptons

  } //Close event()


} // end Belle2 namespace

