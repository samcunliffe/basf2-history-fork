/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: czhearty                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <trg/grl/modules/trggrl/TRGGRLProjectsModule.h>
#include <trg/grl/dataobjects/TRGGRLInfo.h>
#include <trg/ecl/dataobjects/TRGECLCluster.h>
#include <trg/ecl/dataobjects/TRGECLTrg.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/ecl/TrgEclMapping.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/MCParticle.h>
#include <framework/datastore/StoreArray.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <analysis/utility/PCmsLabTransform.h>
#include <framework/logging/Logger.h>

#include <TLorentzVector.h>
#include <TMath.h>
#include <TRandom3.h>

#include <iostream>
#include <bitset>
#include <fstream>


using namespace std;
using namespace Belle2;
using namespace ECL;


//..ECL look up tables
std::vector<int> TCThetaID;
std::vector<float> TCPhiLab;
std::vector<float> TCcotThetaLab;
std::vector<float> TCPhiCOM;
std::vector<float> TCThetaCOM;
std::vector<float> TC1GeV;


//..Other
double radtodeg;
//int iEvent(0);
//int nInAcc=0;

//..Trigger counters
const int ntrig = 18;
//int trigbit[ntrig];
//int prescale[ntrig];
//int RawCount[ntrig];
//bool passBeforePrescale[ntrig];
//int pass2[ntrig][ntrig]={};
//int pass2Acc[ntrig][ntrig]={};
//std::vector<string> trigName;

//int nUseful;
//std::vector<int> itUseful;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TRGGRLProjects)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TRGGRLProjectsModule::TRGGRLProjectsModule() : Module()
{
  // Set module properties

  // string desc = "TRGGRLProjectsModule(" + version() + ")";
  setDescription("TRGGRLProjectsModule");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("DebugLevel", _debugLevel, "TRGGRL debug level", _debugLevel);
  addParam("ConfigFile",
           m_configFilename,
           "The filename of CDC trigger config file",
           m_configFilename);
  addParam("SimulationMode",
           m_simulationMode,
           "TRGGRL simulation switch",
           1);
  addParam("FastSimulationMode",
           m_fastSimulationMode,
           "TRGGRL fast simulation mode",
           m_fastSimulationMode);
  addParam("FirmwareSimulationMode",
           m_firmwareSimulationMode,
           "TRGGRL firmware simulation mode",
           m_firmwareSimulationMode);
  addParam("2DfinderCollection", m_2DfinderCollectionName,
           "Name of the StoreArray holding the tracks made by the 2D finder to be used as input.",
           string("TRGCDC2DFinderTracks"));
  addParam("2DfitterCollection", m_2DfitterCollectionName,
           "Name of the StoreArray holding the tracks made by the 2D fitter to be used as input.",
           string("TRGCDC2DFitterTracks"));
  addParam("3DfitterCollection", m_3DfitterCollectionName,
           "Name of the StoreArray holding the tracks made by the 3D fitter to be used as input.",
           string("TRGCDC3DFitterTracks"));
  addParam("NNCollection", m_NNCollectionName,
           "Name of the StoreArray holding the tracks made by the neural network (NN).",
           string("TRGCDCNeuroTracks"));
  addParam("2DmatchCollection", m_2DmatchCollectionName,
           "Name of the StoreArray holding the macthed tracks and clusters made by the 2D fitter.",
           string("TRG2DMatchTracks"));
  addParam("3DmatchCollection", m_3DmatchCollectionName,
           "Name of the StoreArray holding the matched 3D NN tracks and clusters made",
           string("TRG3DMatchTracks"));
  addParam("TrgGrlInformation", m_TrgGrlInformationName,
           "Name of the StoreArray holding the information of tracks and clusters from cdc ecl klm.",
           string("TRGGRLObjects"));
  addParam("TRGECLClusters", m_TrgECLClusterName,
           "Name of the StoreArray holding the information of trigger ecl clusters ",
           string("TRGECLClusters"));
  addParam("TRGECLTrgs", m_TrgECLTrgsName,
           "Name of the StoreArray holding the information of ecl trigger",
           string("TRGECLTrgs"));
  addParam("TrgKLMTracks", m_KLMTrackName,
           "Name of the StoreArray holding the information of klm track ",
           string("TRGKLMTracks"));
  addParam("TrgKLMHits", m_KLMHitName,
           "Name of the StoreArray holding the information of klm hit",
           string("TRGKLMHits"));
  addParam("ECLClusterTimeWindow", m_eclClusterTimeWindow,
           "The time window of the signal eclclusters",
           100.0);
  addParam("ClusEngThreshold", m_energythreshold, "The energy threshold of clusters", {0.1, 0.3, 1.0, 2.0});
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void TRGGRLProjectsModule::initialize()
{
  radtodeg = 180. / TMath::Pi();
  //..Trigger ThetaID for each trigger cell. Could be replaced by getMaxThetaId() for newer MC
  TrgEclMapping* trgecl_obj = new TrgEclMapping();
  for (int tc = 1; tc <= 576; tc++) {
    TCThetaID.push_back(trgecl_obj->getTCThetaIdFromTCId(tc));
  }

  //-----------------------------------------------------------------------------------------
  //..ECL look up tables
  PCmsLabTransform boostrotate;
  for (int tc = 1; tc <= 576; tc++) {

    //..Four vector of a 1 GeV lab photon at this TC
    TVector3 CellPosition = trgecl_obj->getTCPosition(tc);
    TLorentzVector CellLab(1., 1., 1., 1.);
    CellLab.SetTheta(CellPosition.Theta());
    CellLab.SetPhi(CellPosition.Phi());
    CellLab.SetRho(1.);
    CellLab.SetE(1.);

    //..cotan Theta and phi in lab
    TCPhiLab.push_back(CellPosition.Phi()*radtodeg);
    double tantheta = tan(CellPosition.Theta());
    TCcotThetaLab.push_back(1. / tantheta);

    //..Corresponding 4 vector in the COM frame
    TLorentzVector CellCOM = boostrotate.rotateLabToCms() * CellLab;
    TCThetaCOM.push_back(CellCOM.Theta()*radtodeg);
    TCPhiCOM.push_back(CellCOM.Phi()*radtodeg);

    //..Scale to give 1 GeV in the COM frame
    TC1GeV.push_back(1. / CellCOM.E());
  }

  StoreObjPtr<TRGGRLInfo>::registerPersistent(m_TrgGrlInformationName);

}

void
TRGGRLProjectsModule::beginRun()
{
  B2DEBUG(200, "TRGGDLModule ... beginRun called ");
  //...GDL config. name...
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
void TRGGRLProjectsModule::event()
{
  //if(iEvent%1000==0) {cout << "TRGGRLProjects event " << iEvent << endl;}
  //iEvent++;

  //---------------------------------------------------------------------
  //..Read in the necessary arrays
  StoreArray<TRGECLTrg> trgArray;
  //StoreArray<MCParticle> MCParticleArray;
  StoreArray<CDCTriggerTrack> cdc3DTrkArray("TRGCDC3DFitterTracks");
  StoreArray<TRGECLCluster> eclTrgClusterArray("TRGECLClusters");
  StoreObjPtr<TRGGRLInfo> trgInfo(m_TrgGrlInformationName);
  trgInfo.create();
  //---------------------------------------------------------------------
  //..Use only clusters within 100 ns of event timing (from ECL).
  int ntrgArray = trgArray.getEntries();
  double EventTiming = -9999.;
  if (ntrgArray > 0) {EventTiming = trgArray[0]->getEventTiming();}
  std::vector<int> selTC;
  std::vector<int> selTheta;
  std::vector<float> selE;
  for (int ic = 0; ic < eclTrgClusterArray.getEntries(); ic++) {
    double tcT = abs(eclTrgClusterArray[ic]->getTimeAve() - EventTiming);
    if (tcT < 100.) {
      int TC = eclTrgClusterArray[ic]->getMaxTCId();
      selTC.push_back(TC);
      selTheta.push_back(TCThetaID[TC - 1]);
      selE.push_back(eclTrgClusterArray[ic]->getEnergyDep());
    }
  }

  //---------------------------------------------------------------------
  //..Trigger objects from CDC alone
  //  nTrk3D  nTrkZ10  nTrkZ25
  int nTrk3D = cdc3DTrkArray.getEntries();
  int nTrkZ10 = 0;
  int nTrkZ25 = 0;
  for (int itrk = 0; itrk < nTrk3D; itrk++) {
    double z0 = cdc3DTrkArray[itrk]->getZ0();
    if (abs(z0) < 10.) {nTrkZ10++;}
    if (abs(z0) < 25.) {nTrkZ25++;}
  }

  trgInfo->setN3Dfittertrk(nTrk3D);
  trgInfo->setN3DfittertrkZ10(nTrkZ10);
  trgInfo->setN3DfittertrkZ25(nTrkZ25);

  //---------------------------------------------------------------------
  //..Trigger objects using single ECL clusters
  // nClust n300MeV n2GeV n2GeV414 n2GeV231516 n2GeV117 n1GeV415 n1GeV2316 n1GeV117
  int nClust = selTC.size();
  int n300MeV = 0;
  int n2GeV = 0;
  int n2GeV414 = 0;
  int n2GeV231516 = 0;
  int n2GeV117 = 0;
  int n1GeV415 = 0;
  int n1GeV2316 = 0;
  int n1GeV117 = 0;
  for (int ic = 0; ic < nClust; ic++) {
    if (selE[ic] > 0.3) {n300MeV++;}
    float thresh = TC1GeV[selTC[ic] - 1];
    if (selE[ic] > (thresh + thresh)) {
      n2GeV++;
      if (selTheta[ic] >= 4 && selTheta[ic] <= 14) {n2GeV414++;}
      if (selTheta[ic] == 2 || selTheta[ic] == 3 || selTheta[ic] == 15 || selTheta[ic] == 16) {n2GeV231516++;}
      if (selTheta[ic] == 1 || selTheta[ic] == 17) {n2GeV117++;}
    }
    if (selE[ic] > thresh) {
      if (selTheta[ic] >= 4 && selTheta[ic] <= 15) {n1GeV415++;}
      if (selTheta[ic] == 2 || selTheta[ic] == 3 || selTheta[ic] == 16) {n1GeV2316++;}
      if (selTheta[ic] == 1 || selTheta[ic] == 17) {n1GeV117++;}
    }
  }
  trgInfo->setNcluster(nClust);
  trgInfo->setNhigh300cluster(n300MeV);
  trgInfo->setNhigh1GeVcluster415(n1GeV415);
  trgInfo->setNhigh1GeVcluster2316(n1GeV2316);
  trgInfo->setNhigh1GeVcluster117(n1GeV117);
  trgInfo->setNhigh2GeVcluster(n2GeV);
  trgInfo->setNhigh2GeVcluster414(n2GeV414);
  trgInfo->setNhigh2GeVcluster231516(n2GeV231516);
  trgInfo->setNhigh2GeVcluster117(n2GeV117);

  //---------------------------------------------------------------------
  //..Trigger objects using back-to-back ECL clusters, plus Bhabha vetoes
  //  nPhiPairHigh nPhiPairLow n3DPair nECLBhabha nTrkBhabha
  int nPhiPairHigh = 0;
  int nPhiPairLow = 0;
  int n3DPair = 0;
  int nECLBhabha = 0;
  int nTrkBhabha = 0;
  for (int i0 = 0; i0 < nClust - 1; i0++) {
    for (int i1 = i0 + 1; i1 < nClust; i1++) {

      //..back to back in phi
      float dphi = abs(TCPhiCOM[selTC[i1] - 1] - TCPhiCOM[selTC[i0] - 1]);
      if (dphi > 180.) {dphi = 360 - dphi;}
      if (dphi > 170. && selE[i0] > 0.25 && selE[i1] > 0.25) {nPhiPairHigh++;}
      if (dphi > 170. && (selE[i0] < 0.25 || selE[i1] < 0.25)) {nPhiPairLow++;}

      //..3D
      float thetaSum = TCThetaCOM[selTC[i0] - 1] + TCThetaCOM[selTC[i1] - 1];
      if (dphi > 160. && thetaSum > 160. && thetaSum < 200.) {n3DPair++;}

      //..ecl Bhabha
      if (dphi > 160. && thetaSum > 165. && thetaSum < 190. && selE[i0] > 3.*TC1GeV[selTC[i0] - 1] && selE[i1] > 3.*TC1GeV[selTC[i1] - 1]
          && (selE[i0] > 4.5 * TC1GeV[selTC[i0] - 1] ||  selE[i1] > 4.5 * TC1GeV[selTC[i1] - 1])) {
        nECLBhabha++;

        //..Bhabha also using the CDC
        bool c0matched = false;
        bool c1matched = false;
        for (int itrk = 0; itrk < nTrk3D; itrk++) {
          double phiTrk = cdc3DTrkArray[itrk]->getPhi0() * radtodeg;
          double ptTrk = cdc3DTrkArray[itrk]->getTransverseMomentum(1.5);
          float dphi0 = abs(phiTrk - TCPhiLab[selTC[i0] - 1]);
          if (dphi0 > 180.) {dphi0 = 360. - dphi0;}
          float dphi1 = abs(phiTrk - TCPhiLab[selTC[i1] - 1]);
          if (dphi1 > 180.) {dphi1 = 360. - dphi1;}
          if (ptTrk > 1. && dphi0 < 15.) {c0matched = true;}
          if (ptTrk > 1. && dphi1 < 15.) {c1matched = true;}
        }
        if (c0matched && c1matched) {nTrkBhabha++;}
      }
    }
  }

  trgInfo->setBhabhaVeto(nTrkBhabha);
  trgInfo->seteclBhabhaVeto(nECLBhabha);
  trgInfo->setPairHigh(nPhiPairHigh);
  trgInfo->setPairLow(nPhiPairLow);
  trgInfo->set3DPair(n3DPair);


  //---------------------------------------------------------------------
  //..Trigger objects using 1 track and at least 1 cluster
  // nSameHem1Trk nOppHem1Trk
  int nSameHem1Trk = 0;
  int nOppHem1Trk = 0;
  if (nTrk3D == 1) {
    double phiTrk = cdc3DTrkArray[0]->getPhi0() * radtodeg;
    double cotTrk = cdc3DTrkArray[0]->getCotTheta();
    for (int i0 = 0; i0 < nClust; i0++) {
      float dphi = abs(phiTrk - TCPhiLab[selTC[i0] - 1]);
      if (dphi > 180.) {dphi = 360. - dphi;}
      float dCot = cotTrk - TCcotThetaLab[selTC[i0] - 1];
      if (dphi > 80.) {nOppHem1Trk++;}
      if (dphi < 80. && (dCot < -0.8 || dCot > 0.6)) {nSameHem1Trk++;}
    }
  }

  trgInfo->setNSameHem1Trk(nSameHem1Trk);
  trgInfo->setNOppHem1Trk(nOppHem1Trk);

}

void
TRGGRLProjectsModule::endRun()
{
  B2DEBUG(200, "TRGGRLProjectsModule ... endRun called ");
}



void TRGGRLProjectsModule::terminate()
{

}


