/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Matic Lubej                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/ECLClusterInfo/ECLClusterInfoModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/MCParticle.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <utility>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLClusterInfo)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLClusterInfoModule::ECLClusterInfoModule() : Module()
{
  // Set module properties
  setDescription("Creates an output root file which contains info from ECL clusters. Used for b2bii conversion validation.");

  //Parameter definition
  addParam("outputFileName", m_fileName, "The name of the output .root file", string("ECLClusterInfo.root"));
  addParam("treeName", m_treeName, "The name of the tree in the output .root file", string("ecl_mdst"));

  m_file = nullptr;
  m_tree = nullptr;
}

void ECLClusterInfoModule::terminate()
{
  // Write and close file
  B2INFO("Writing NTuple " << m_treeName);
  m_tree->Write();

  B2INFO("Closing file " << m_fileName);
  delete m_file;
}

void ECLClusterInfoModule::initialize()
{
  // Initialize ECLClusters StoreArray
  StoreArray<ECLCluster> eclClusters;

  // Define output file name, tree name and branches
  m_file = new TFile(m_fileName.c_str(), "RECREATE");
  if (!m_file->IsOpen()) {
    B2WARNING("Could not create file " << m_fileName);
    return;
  }

  m_file->cd();

  // Check if TTree with that name already exists
  if (m_file->Get(m_treeName.c_str())) {
    B2WARNING("Tree with this name already exists: " << m_fileName);
    return;
  }

  m_tree = new TTree(m_treeName.c_str(), "");

  m_E = 0;
  m_Theta = 0;
  m_Phi = 0;
  m_R = 0;
  m_Edep = 0;
  m_E9oE25 = 0;
  m_HE = 0;
  m_NC = 0;
  m_Err00 = 0;
  m_Err10 = 0;
  m_Err11 = 0;
  m_Err20 = 0;
  m_Err21 = 0;
  m_Err22 = 0;
  m_Truth_Px = 0;
  m_Truth_Py = 0;
  m_Truth_Pz = 0;
  m_Truth_E = 0;
  m_PDG = 0;

  m_tree->Branch("Cluster_E",        &m_E,        "Cluster_E/F");
  m_tree->Branch("Cluster_Theta",    &m_Theta,    "Cluster_Theta/F");
  m_tree->Branch("Cluster_Phi",      &m_Phi,      "Cluster_Phi/F");
  m_tree->Branch("Cluster_R",        &m_R,        "Cluster_R/F");

  m_tree->Branch("Cluster_EnedepSum",        &m_Edep,        "Cluster_EnedepSum/F");
  m_tree->Branch("Cluster_E9oE25",        &m_E9oE25,        "Cluster_E9oE25/F");
  m_tree->Branch("Cluster_HighestE",        &m_HE,        "Cluster_HighestE/F");
  m_tree->Branch("Cluster_NofCrystals",        &m_NC,        "Cluster_NofCrystals/I");

  m_tree->Branch("Cluster_Err00",    &m_Err00,    "Cluster_Err00/F");
  m_tree->Branch("Cluster_Err10",    &m_Err10,    "Cluster_Err10/F");
  m_tree->Branch("Cluster_Err11",    &m_Err11,    "Cluster_Err11/F");
  m_tree->Branch("Cluster_Err20",    &m_Err20,    "Cluster_Err20/F");
  m_tree->Branch("Cluster_Err21",    &m_Err21,    "Cluster_Err21/F");
  m_tree->Branch("Cluster_Err22",    &m_Err22,    "Cluster_Err22/F");

  m_tree->Branch("Cluster_Truth_Px", &m_Truth_Px, "Cluster_Truth_Px/F");
  m_tree->Branch("Cluster_Truth_Py", &m_Truth_Py, "Cluster_Truth_Py/F");
  m_tree->Branch("Cluster_Truth_Pz", &m_Truth_Pz, "Cluster_Truth_Pz/F");
  m_tree->Branch("Cluster_Truth_E",  &m_Truth_E,  "Cluster_Truth_E/F");
  m_tree->Branch("Cluster_PDG",      &m_PDG,      "Cluster_PDG/I");

}

void ECLClusterInfoModule::event()
{
  // Load ECLClusters StoreArray
  StoreArray<ECLCluster> eclClusters;

  // Loop over existing ECLClusters in StoreArray
  for (int iECL = 0; iECL < eclClusters.getEntries(); iECL++) {
    const ECLCluster* ecl = eclClusters[iECL];

    // Get ecl values
    m_E = ecl->getEnergy();
    m_Theta = ecl->getTheta();
    m_Phi = ecl->getPhi();
    m_R = ecl->getR();

    // Get auxiliary values
    m_Edep = ecl->getEnedepSum();
    m_E9oE25 = ecl->getE9oE25();
    m_HE = ecl->getHighestE();
    m_NC = ecl->getNofCrystals();

    TMatrixFSym Err = ecl->getError3x3();

    m_Err00 = Err[0][0];
    m_Err10 = Err[1][0];
    m_Err11 = Err[1][1];
    m_Err20 = Err[2][0];
    m_Err21 = Err[2][1];
    m_Err22 = Err[2][2];

    // Get Related MCParticle
    MCParticle* mcParticle = ecl->getRelated<MCParticle>();

    TVector3 mom3;
    m_PDG = 0;

    if (mcParticle) {
      mom3 = mcParticle->getMomentum();
      m_Truth_Px = mom3[0];
      m_Truth_Py = mom3[1];
      m_Truth_Pz = mom3[2];
      m_Truth_E = mcParticle->getEnergy();
      m_PDG = mcParticle->getPDG();
    }

    // Fill tree
    m_tree->Fill();
  }
}

