/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <string>

#include <calibration/modules/TestHisto/TestHistoModule.h>

#include <TTree.h>
#include <TH1F.h>
#include <TRandom.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TestHisto)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TestHistoModule::TestHistoModule() : CalibrationCollectorModule()
{
  // Set module properties
  setDescription("Test Module for saving big data in CAF");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Parameter definitions
  addParam("entriesPerEvent", m_entriesPerEvent,
           "Number of entries that we fill into the saved tree per event. As we increase this we start storing larger amonuts of data in a smaller number of events to test the limits.",
           int(10));
  addParam("spread", m_spread,
           "Spread of gaussian (mean=42) filling of test histogram (range=<0,100>) - probability of algo iterations depend on it", int(17.));
}

void TestHistoModule::prepare()
{
  std::string objectName = "MyTree";
  // Data object creation --------------------------------------------------
  TTree* tree = new TTree(objectName.c_str(), "");
  tree->Branch<int>("event", &m_evt);
  tree->Branch<int>("run", &m_run);
  tree->Branch<int>("exp", &m_exp);
  tree->Branch<double>("hitX", &m_hitX);
  tree->Branch<double>("hitY", &m_hitY);
  tree->Branch<double>("hitZ", &m_hitZ);
  tree->Branch<double>("trackX", &m_trackX);
  tree->Branch<double>("trackY", &m_trackY);
  tree->Branch<double>("trackZ", &m_trackZ);
  tree->Branch<double>("chisq", &m_chisq);
  tree->Branch<double>("pvalue", &m_pvalue);
  tree->Branch<double>("dof", &m_dof);

  registerObject<TTree>(objectName, tree);

  auto hist = new TH1F("histo", "Test histogram, which mean value should be found by test calibration algo", 100, 0., 100.);
  registerObject<TH1F>("MyHisto", hist);
}

void TestHistoModule::inDefineHisto()
{
  ;
}

void TestHistoModule::startRun()
{
  ;
}

void TestHistoModule::collect()
{
  m_evt = m_emd->getEvent();
  m_run = m_emd->getRun();
  m_exp = m_emd->getExperiment();

  std::string objectName = "MyTree";
  for (int i = 0; i < m_entriesPerEvent; ++i) {
    m_hitX = gRandom->Gaus();
    m_hitY = gRandom->Gaus();
    m_hitZ = gRandom->Gaus();
    m_trackX = gRandom->Gaus();
    m_trackY = gRandom->Gaus();
    m_trackZ = gRandom->Gaus();
    m_chisq = gRandom->Gaus();
    m_pvalue = gRandom->Gaus();
    m_dof = gRandom->Gaus();
    getObject<TTree>(objectName).Fill();
  }

  getObject<TH1F>("MyHisto").Fill(gRandom->Gaus(42., m_spread));
}

//void TestHistoModule::terminate()
//{
//  TDirectory* oldDir = gDirectory;
//  m_dir->cd();
//  m_obj->Write("MyCaliObj");
//  oldDir->cd();
//}
