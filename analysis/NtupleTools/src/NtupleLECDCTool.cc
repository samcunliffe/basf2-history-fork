/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2015 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Chunhua LI                                               *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <analysis/NtupleTools/NtupleLECDCTool.h>
#include <analysis/VariableManager/Variables.h>
#include <analysis/VariableManager/PhysicsTriggerVariables.h>
#include <TBranch.h>
#include <cmath>
using namespace Belle2;
using namespace std;

void NtupleLECDCTool::setupTree()
{
//  vector<string> strNames = m_decaydescriptor.getSelectionNames();
  m_nTracks = 0;
  m_nLongTracks = 0;
  m_nECLMatchTracks = 0;
  m_nKLMMatchTracks = 0;
  m_maxAng = 0.;
  m_minusTheta = 0.;
  m_VisiblePz = 0.;
  m_VisibleEnergy = 0.;
  m_P12CMS = 0.;
  m_P1CMSBhabha = 0.;
  m_P2CMSBhabha = 0.;

  m_tree->Branch("nTracks", &m_nTracks, "nTracks/I");
  m_tree->Branch("nLongTracks", &m_nLongTracks, "nLongTracks/I");
  m_tree->Branch("nECLMatchTracks", &m_nECLMatchTracks, "nECLMatchTracks/I");
  m_tree->Branch("nKLMMatchTracks", &m_nKLMMatchTracks, "nKLMMatchTracks/I");
  m_tree->Branch("maxAngtt", &m_maxAng, "maxAngtt/F");
  m_tree->Branch("minusTheta", &m_minusTheta, "minusTheta/F");
  m_tree->Branch("VisiblePz", &m_VisiblePz, "VisiblePz/F");
  m_tree->Branch("VisibleEnergy", &m_VisibleEnergy, "VisibleEnergy/F");
  m_tree->Branch("P12CMS", &m_P12CMS, "P12CMS/F");
  m_tree->Branch("P1CMS", &m_P1CMSBhabha, "P1CMS/F");
  m_tree->Branch("P2CMS", &m_P2CMSBhabha, "P2CMS/F");

  m_P1Bhabha = new double[5];
  m_P2Bhabha = new double[5];
  m_tree->Branch("InfTrk1", &m_P1Bhabha[0], "InfTrk1[5]/D");
  m_tree->Branch("InfTrk2", &m_P2Bhabha[0], "InfTrk2[5]/D");


}

void NtupleLECDCTool::eval(const Particle* particle)
{

  m_nTracks = Variable::nTracksLE(particle);
  m_nLongTracks = Variable::nLongTracksLE(particle);
  m_nECLMatchTracks = Variable::nECLMatchTracksLE(particle);
  m_nKLMMatchTracks = Variable::nKLMMatchTracksLE(particle);
  m_maxAng = Variable::maxAngleTTLE(particle);
  m_minusTheta = Variable::MinusThetaBhabhaLE(particle);
  m_VisiblePz = Variable::VisiblePzLE(particle);
  m_VisibleEnergy = Variable::VisibleEnergyLE(particle);
  m_P12CMS = Variable::P12CMSBhabhaLE(particle);
  m_P1CMSBhabha = Variable::P1CMSBhabhaLE(particle);
  m_P2CMSBhabha = Variable::P2CMSBhabhaLE(particle);

  m_P1Bhabha[0] = Variable::P1BhabhaLE(particle);
  m_P2Bhabha[0] = Variable::P2BhabhaLE(particle);
  m_P1Bhabha[1] = Variable::Theta1BhabhaLE(particle);
  m_P2Bhabha[1] = Variable::Theta2BhabhaLE(particle);
  m_P1Bhabha[2] = Variable::Phi1BhabhaLE(particle);
  m_P2Bhabha[2] = Variable::Phi2BhabhaLE(particle);
  m_P1Bhabha[3] = Variable::Charge1BhabhaLE(particle);
  m_P2Bhabha[3] = Variable::Charge2BhabhaLE(particle);
  m_P1Bhabha[4] = Variable::E1BhabhaLE(particle);
  m_P2Bhabha[4] = Variable::E2BhabhaLE(particle);

}
