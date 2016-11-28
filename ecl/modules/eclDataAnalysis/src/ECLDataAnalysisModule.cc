/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <list>
#include <iostream>
#include <ecl/modules/eclDataAnalysis/ECLDataAnalysisModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
//#include <ecl/dataobjects/ECLTrig.h>

using namespace std;
using namespace Belle2;
//using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ECLDataAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDataAnalysisModule::ECLDataAnalysisModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_doTracking(0),
    m_tree(0),
    m_iExperiment(0),
    m_iRun(0),
    m_iEvent(0),
//Digit
    m_eclDigitMultip(0),
    m_eclDigitIdx(0),
    m_eclDigitToMC(0),
    m_eclDigitCellId(0),
    m_eclDigitAmp(0),
    m_eclDigitTimeFit(0),
    m_eclDigitFitQuality(0),
    m_eclDigitToCalDigit(0),
//CalDigit
    m_eclCalDigitMultip(0),
    m_eclCalDigitIdx(0),
    m_eclCalDigitToMC1(0),
    m_eclCalDigitToMC1PDG(0),
    m_eclCalDigitToMCWeight1(0),
    m_eclCalDigitToMC2(0),
    m_eclCalDigitToMC2PDG(0),
    m_eclCalDigitToMCWeight2(0),
    m_eclCalDigitToMC3(0),
    m_eclCalDigitToMC3PDG(0),
    m_eclCalDigitToMCWeight3(0),
    m_eclCalDigitToMC4(0),
    m_eclCalDigitToMC4PDG(0),
    m_eclCalDigitToMCWeight4(0),
    m_eclCalDigitToMC5(0),
    m_eclCalDigitToMC5PDG(0),
    m_eclCalDigitToMCWeight5(0),
    m_eclCalDigitToBkgWeight(0),
    m_eclCalDigitSimHitSum(0),
    m_eclCalDigitToShower(0),
    m_eclCalDigitCellId(0),
    m_eclCalDigitAmp(0),
    m_eclCalDigitTimeFit(0),
    m_eclCalDigitFitQuality(0),
    m_eclCalDigitToCR(0),
//Connected Region
    m_eclCRIdx(0),
    m_eclCRIsTrack(0),
    m_eclCRLikelihoodMIPNGamma(0),
    m_eclCRLikelihoodChargedHadron(0),
    m_eclCRLikelihoodElectronNGamma(0),
    m_eclCRLikelihoodNGamma(0),
    m_eclCRLikelihoodNeutralHadron(0),
    m_eclCRLikelihoodMergedPi0(0),
//SimHit
    m_eclSimHitMultip(0),
    m_eclSimHitIdx(0),
    m_eclSimHitToMC(0),
    m_eclSimHitCellId(0),
    m_eclSimHitPdg(0),
    m_eclSimHitEnergyDep(0),
    m_eclSimHitFlightTime(0),
    m_eclSimHitX(0),
    m_eclSimHitY(0),
    m_eclSimHitZ(0),
    m_eclSimHitPx(0),
    m_eclSimHitPy(0),
    m_eclSimHitPz(0),
//Hit
    m_eclHitMultip(0),
    m_eclHitIdx(0),
    m_eclHitToMC(0),
    m_eclHitToDigit(0),
    m_eclHitToDigitAmp(0),
//m_eclHitToPureDigit(0),
//m_eclHitToPureDigitAmp(0),
    m_eclHitCellId(0),
    m_eclHitEnergyDep(0),
    m_eclHitTimeAve(0),
//Cluster
    m_eclClusterMultip(0),
    m_eclClusterTrueMultip(0),
    m_eclClusterGammaMultip(0),
    m_eclClusterIdx(0),
    m_eclClusterToMC1(0),
    m_eclClusterToMCWeight1(0),
    m_eclClusterToMC1PDG(0),
    m_eclClusterToMC2(0),
    m_eclClusterToMCWeight2(0),
    m_eclClusterToMC2PDG(0),
    m_eclClusterToMC3(0),
    m_eclClusterToMCWeight3(0),
    m_eclClusterToMC3PDG(0),
    m_eclClusterToMC4(0),
    m_eclClusterToMCWeight4(0),
    m_eclClusterToMC4PDG(0),
    m_eclClusterToMC5(0),
    m_eclClusterToMCWeight5(0),
    m_eclClusterToMC5PDG(0),
    m_eclClusterToBkgWeight(0),
    m_eclClusterSimHitSum(0),
    m_eclClusterToShower(0),
    m_eclClusterEnergy(0),
    m_eclClusterEnergyError(0),
    m_eclClusterTheta(0),
    m_eclClusterThetaError(0),
    m_eclClusterPhi(0),
    m_eclClusterPhiError(0),
    m_eclClusterR(0),
    m_eclClusterEnergyDepSum(0),
    m_eclClusterTiming(0),
    m_eclClusterTimingError(0),
    m_eclClusterE9oE21(0),
    m_eclClusterHighestE(0),
    m_eclClusterNofCrystals(0),
    m_eclClusterCrystalHealth(0),
    m_eclClusterPx(0),
    m_eclClusterPy(0),
    m_eclClusterPz(0),
    m_eclClusterIsTrack(0),
    m_eclClusterClosestTrackDist(0),
    m_eclClusterDeltaL(0),
    m_eclClusterAbsZernike40(0),
    m_eclClusterAbsZernike51(0),
    m_eclClusterZernikeMVA(0),
    m_eclClusterE1oE9(0),
    m_eclClusterSecondMoment(0),
    m_eclClusterLAT(0),
    m_eclClusterDeltaTime99(0),
    m_eclClusterDetectorRegion(0),
    m_eclClusterHypothesisId(0),
//PureDigit
    m_eclPureDigitMultip(0),
    m_eclPureDigitIdx(0),
    m_eclPureDigitToMC(0),
    m_eclPureDigitCellId(0),
    m_eclPureDigitAmp(0),
    m_eclPureDigitTimeFit(0),
    m_eclPureDigitFitQuality(0),
    m_eclPureDigitToCluster(0),
//PureCluster
    m_eclPureClusterMultip(0),
    m_eclPureClusterIdx(0),
    m_eclPureClusterToMC(0),
    m_eclPureClusterEnergy(0),
    m_eclPureClusterEnergyError(0),
    m_eclPureClusterTheta(0),
    m_eclPureClusterThetaError(0),
    m_eclPureClusterPhi(0),
    m_eclPureClusterPhiError(0),
    m_eclPureClusterR(0),
    m_eclPureClusterEnergyDepSum(0),
    m_eclPureClusterTiming(0),
    m_eclPureClusterTimingError(0),
    m_eclPureClusterE9oE21(0),
    m_eclPureClusterHighestE(0),
    m_eclPureClusterLat(0),
    m_eclPureClusterNofCrystals(0),
    m_eclPureClusterCrystalHealth(0),
    m_eclPureClusterMergedPi0(0),
    m_eclPureClusterPx(0),
    m_eclPureClusterPy(0),
    m_eclPureClusterPz(0),
    m_eclPureClusterIsTrack(0),
    m_eclPureClusterDeltaL(0),
//Shower
    m_eclShowerMultip(0),
    m_eclShowerIdx(0),
    m_eclShowerToMC1(0),
    m_eclShowerToMCWeight1(0),
    m_eclShowerToMC1PDG(0),
    m_eclShowerToMC2(0),
    m_eclShowerToMCWeight2(0),
    m_eclShowerToMC2PDG(0),
    m_eclShowerToMC3(0),
    m_eclShowerToMCWeight3(0),
    m_eclShowerToMC3PDG(0),
    m_eclShowerToMC4(0),
    m_eclShowerToMCWeight4(0),
    m_eclShowerToMC4PDG(0),
    m_eclShowerToMC5(0),
    m_eclShowerToMCWeight5(0),
    m_eclShowerToMC5PDG(0),
    m_eclShowerToBkgWeight(0),
    m_eclShowerSimHitSum(0),
    m_eclShowerUncEnergy(0),
    m_eclShowerEnergy(0),
    m_eclShowerTheta(0),
    m_eclShowerPhi(0),
    m_eclShowerR(0),
    m_eclShowerNHits(0),
    m_eclShowerE9oE21(0),
    m_eclShowerTime(0),
    m_eclShowerConnectedRegionId(0),
    m_eclShowerHypothesisId(0),
    m_eclShowerCentralCellId(0),
    m_eclShowerEnergyError(0),
    m_eclShowerThetaError(0),
    m_eclShowerPhiError(0),
    m_eclShowerTimeResolution(0),
    m_eclShowerHighestEnergy(0),
    m_eclShowerLateralEnergy(0),
    m_eclShowerMinTrkDistance(0),
    m_eclShowerTrkDepth(0),
    m_eclShowerShowerDepth(0),
    m_eclShowerAbsZernike40(0),
    m_eclShowerAbsZernike51(0),
    m_eclShowerZernikeMVA(0),
    m_eclShowerSecondMoment(0),
    m_eclShowerE1oE9(0),
    m_eclShowerIsTrack(0),
    m_eclShowerIsCluster(0),
    m_eclShowerMCVtxInEcl(0),
    m_eclShowerHighestE1mE2(0),
//MC
    m_mcMultip(0),
    m_mcIdx(0),
    m_mcPdg(0),
    m_mcMothPdg(0),
    m_mcGMothPdg(0),
    m_mcGGMothPdg(0),
    m_mcEnergy(0),
    m_mcPx(0),
    m_mcPy(0),
    m_mcPz(0),
    m_mcDecayVtxX(0),
    m_mcDecayVtxY(0),
    m_mcDecayVtxZ(0),
    m_mcProdVtxX(0),
    m_mcProdVtxY(0),
    m_mcProdVtxZ(0),
    m_mcSecondaryPhysProc(0),
//Tracks
    m_trkMultip(0),
    m_trkPdg(0),
    m_trkIdx(0),
    m_trkCharge(0),
    m_trkPx(0),
    m_trkPy(0),
    m_trkPz(0),
    m_trkP(0),
    m_trkTheta(0),
    m_trkPhi(0),
    m_trkX(0),
    m_trkY(0),
    m_trkZ(0),
//PID
    m_eclpidtrkIdx(0),
    m_eclpidEnergy(0),
    m_eclpidEop(0),
    m_eclpidE9E21(0),
    m_eclpidNCrystals(0),
    m_eclpidNClusters(0),
    m_eclLogLikeEl(0),
    m_eclLogLikeMu(0),
    m_eclLogLikePi(0)

{
  //Set module properties

  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclDataAnalysis"));
  addParam("doTracking", m_doTracking, "set true if you want to save the informations from TrackFitResults'rootFileName'",
           bool(false));
  //addParam("doMC", m_doMC, "set true if you want to save MC information", bool(false));
  addParam("doSimulation", m_doSimulation, "set true if you want to save the Hit and SimHit informations'", bool(false));
  addParam("doPureCsIStudy", m_doPureCsIStudy, "set true if you want to save the informations for upgrade option'", bool(false));
  addParam("pure_clusters", m_pure_clusters, "name of input Pure CsI Clusters", string("ECLClustersPureCsI"));
  addParam("pure_digits", m_pure_digits, "name of input Pure CsI Digits", string("ECLDigitsPureCsI"));
  addParam("pure_cal_digits", m_pure_cal_digits, "name of input Pure CsI Cal Digits", string("ECLCalDigitsPureCsI"));
}

ECLDataAnalysisModule::~ECLDataAnalysisModule()
{
}


void ECLDataAnalysisModule::initialize()
{

  B2INFO("[ECLDataAnalysis Module]: Starting initialization of ECLDataAnalysis Module.");

  //StoreArray<ECLTrig>::required();
  StoreArray<ECLDigit>::required();
  StoreArray<ECLCalDigit>::required();
  if (m_doSimulation == true) {
    StoreArray<ECLSimHit>::required();
    StoreArray<ECLHit>::required();
  }
  StoreArray<ECLCluster>::required();
  StoreArray<ECLShower>::required();
  StoreArray<MCParticle>::required();
  StoreArray<ECLConnectedRegion>::required();

  if (m_doPureCsIStudy == true) {
    StoreArray<ECLDigit>::required(m_pure_digits);
    StoreArray<ECLCluster>::required(m_pure_clusters);
    StoreArray<ECLCalDigit>::required(m_pure_cal_digits);
  }

  if (m_doTracking == true) {
    StoreArray<Track>::required();
    StoreArray<TrackFitResult>::required();
    StoreArray<ECLPidLikelihood>::required();
  }

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Analysis tree");

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  //m_tree->Branch("eclTriggerMultip",     &m_eclTriggerMultip,         "eclTriggerMultip/I");
  //m_tree->Branch("eclTriggerIdx",     "std::vector<int>",       &m_eclTriggerIdx);
  //m_tree->Branch("eclTriggerCellId",     "std::vector<int>",       &m_eclTriggerCellId);
  //m_tree->Branch("eclTriggerTime",       "std::vector<double>",    &m_eclTriggerTime);

  m_tree->Branch("eclDigitMultip",     &m_eclDigitMultip,         "ecldigit_Multip/I");
  m_tree->Branch("eclDigitIdx",        "std::vector<int>",         &m_eclDigitIdx);
  m_tree->Branch("eclDigitToMC",      "std::vector<int>",          &m_eclDigitToMC);
  m_tree->Branch("eclDigitCellId",     "std::vector<int>",         &m_eclDigitCellId);
  m_tree->Branch("eclDigitAmp",        "std::vector<int>",         &m_eclDigitAmp);
  m_tree->Branch("eclDigitTimeFit",    "std::vector<int>",         &m_eclDigitTimeFit);
  m_tree->Branch("eclDigitFitQuality",    "std::vector<int>",         &m_eclDigitFitQuality);
  m_tree->Branch("eclDigitToCalDigit",      "std::vector<int>",          &m_eclDigitToCalDigit);

  m_tree->Branch("eclCalDigitMultip",     &m_eclCalDigitMultip,         "eclCaldigit_Multip/I");
  m_tree->Branch("eclCalDigitIdx",        "std::vector<int>",         &m_eclCalDigitIdx);
  m_tree->Branch("eclCalDigitToMC1",      "std::vector<int>",       &m_eclCalDigitToMC1);
  m_tree->Branch("eclCalDigitToMC1PDG",      "std::vector<int>",       &m_eclCalDigitToMC1PDG);
  m_tree->Branch("eclCalDigitToMCWeight1",      "std::vector<double>",       &m_eclCalDigitToMCWeight1);
  m_tree->Branch("eclCalDigitToMC2",      "std::vector<int>",       &m_eclCalDigitToMC2);
  m_tree->Branch("eclCalDigitToMC2PDG",      "std::vector<int>",       &m_eclCalDigitToMC2PDG);
  m_tree->Branch("eclCalDigitToMCWeight2",      "std::vector<double>",       &m_eclCalDigitToMCWeight2);
  m_tree->Branch("eclCalDigitToMC3",      "std::vector<int>",       &m_eclCalDigitToMC3);
  m_tree->Branch("eclCalDigitToMC3PDG",      "std::vector<int>",       &m_eclCalDigitToMC3PDG);
  m_tree->Branch("eclCalDigitToMCWeight3",      "std::vector<double>",       &m_eclCalDigitToMCWeight3);
  m_tree->Branch("eclCalDigitToMC4",      "std::vector<int>",       &m_eclCalDigitToMC4);
  m_tree->Branch("eclCalDigitToMC4PDG",      "std::vector<int>",       &m_eclCalDigitToMC4PDG);
  m_tree->Branch("eclCalDigitToMCWeight4",      "std::vector<double>",       &m_eclCalDigitToMCWeight4);
  m_tree->Branch("eclCalDigitToMC5",      "std::vector<int>",       &m_eclCalDigitToMC5);
  m_tree->Branch("eclCalDigitToMC5PDG",      "std::vector<int>",       &m_eclCalDigitToMC5PDG);
  m_tree->Branch("eclCalDigitToMCWeight5",      "std::vector<double>",       &m_eclCalDigitToMCWeight5);
  m_tree->Branch("eclCalDigitToBkgWeight",      "std::vector<double>",       &m_eclCalDigitToBkgWeight);
  m_tree->Branch("eclCalDigitSimHitSum",      "std::vector<double>",       &m_eclCalDigitSimHitSum);
  m_tree->Branch("eclCalDigitToShower",      "std::vector<int>",          &m_eclCalDigitToShower);
  m_tree->Branch("eclCalDigitCellId",     "std::vector<int>",         &m_eclCalDigitCellId);
  m_tree->Branch("eclCalDigitAmp",        "std::vector<double>",         &m_eclCalDigitAmp);
  m_tree->Branch("eclCalDigitTimeFit",    "std::vector<double>",         &m_eclCalDigitTimeFit);
  m_tree->Branch("eclCalDigitFitQuality",    "std::vector<int>",         &m_eclCalDigitFitQuality);
  m_tree->Branch("eclCalDigitToCR",      "std::vector<int>",          &m_eclCalDigitToCR);

  m_tree->Branch("eclCRIdx", "std::vector<int>", &m_eclCRIdx);
  m_tree->Branch("eclCRIsTrack", "std::vector<bool>", &m_eclCRIsTrack);
  m_tree->Branch("eclCRLikelihoodMIPNGamma", "std::vector<double>", &m_eclCRLikelihoodMIPNGamma);
  m_tree->Branch("eclCRLikelihoodChargedHadron", "std::vector<double>", &m_eclCRLikelihoodChargedHadron);
  m_tree->Branch("eclCRLikelihoodElectronNGamma", "std::vector<double>", &m_eclCRLikelihoodElectronNGamma);
  m_tree->Branch("eclCRLikelihoodNGamma", "std::vector<double>", &m_eclCRLikelihoodNGamma);
  m_tree->Branch("eclCRLikelihoodNeutralHadron", "std::vector<double>", &m_eclCRLikelihoodNeutralHadron);
  m_tree->Branch("eclCRLikelihoodMergedPi0", "std::vector<double>", &m_eclCRLikelihoodMergedPi0);

  if (m_doSimulation == 1) {
    m_tree->Branch("eclSimHitMultip",     &m_eclSimHitMultip,      "eclSimHitMultip/I");
    m_tree->Branch("eclSimHitIdx",     "std::vector<int>",       &m_eclSimHitIdx);
    m_tree->Branch("eclSimHitToMC",      "std::vector<int>",       &m_eclSimHitToMC);
    m_tree->Branch("eclSimHitCellId",     "std::vector<int>",       &m_eclSimHitCellId);
    m_tree->Branch("eclSimHitPdg",        "std::vector<int>",       &m_eclSimHitPdg);
    m_tree->Branch("eclSimHitEnergyDep",      "std::vector<double>",    &m_eclSimHitEnergyDep);
    m_tree->Branch("eclSimHitFlightTime",      "std::vector<double>",    &m_eclSimHitFlightTime);
    m_tree->Branch("eclSimHitX",          "std::vector<double>",    &m_eclSimHitX);
    m_tree->Branch("eclSimHitY",          "std::vector<double>",    &m_eclSimHitY);
    m_tree->Branch("eclSimHitZ",          "std::vector<double>",    &m_eclSimHitZ);
    m_tree->Branch("eclSimHitPx",         "std::vector<double>",    &m_eclSimHitPx);
    m_tree->Branch("eclSimHitPy",         "std::vector<double>",    &m_eclSimHitPy);
    m_tree->Branch("eclSimHitPz",         "std::vector<double>",    &m_eclSimHitPz);

    m_tree->Branch("eclHitMultip",     &m_eclHitMultip,      "eclHitMultip/I");
    m_tree->Branch("eclHitIdx",     "std::vector<int>",       &m_eclHitIdx);
    m_tree->Branch("eclHitToMC",      "std::vector<int>",       &m_eclHitToMC);
    m_tree->Branch("eclHitToDigit",      "std::vector<int>",       &m_eclHitToDigit);
    m_tree->Branch("eclHitToDigitAmp",      "std::vector<int>",       &m_eclHitToDigitAmp);
    m_tree->Branch("eclHitCellId",     "std::vector<int>",    &m_eclHitCellId);
    m_tree->Branch("eclHitEnergyDep",      "std::vector<double>", &m_eclHitEnergyDep);
    m_tree->Branch("eclHitTimeAve",       "std::vector<double>", &m_eclHitTimeAve);
  }

  m_tree->Branch("eclClusterMultip",     &m_eclClusterMultip,     "eclClusterMultip/I");
  m_tree->Branch("eclClusterTrueMultip",     &m_eclClusterTrueMultip,     "eclClusterTrueMultip/I");
  m_tree->Branch("eclClusterGammaMultip",     &m_eclClusterGammaMultip,     "eclClusterGammaMultip/I");
  m_tree->Branch("eclClusterIdx",     "std::vector<int>",       &m_eclClusterIdx);
  m_tree->Branch("eclClusterToMC1",      "std::vector<int>",       &m_eclClusterToMC1);
  m_tree->Branch("eclClusterToMCWeight1",      "std::vector<double>",       &m_eclClusterToMCWeight1);
  m_tree->Branch("eclClusterToMC1PDG",      "std::vector<int>",       &m_eclClusterToMC1PDG);
  m_tree->Branch("eclClusterToMC2",      "std::vector<int>",       &m_eclClusterToMC2);
  m_tree->Branch("eclClusterToMCWeight2",      "std::vector<double>",       &m_eclClusterToMCWeight2);
  m_tree->Branch("eclClusterToMC2PDG",      "std::vector<int>",       &m_eclClusterToMC2PDG);
  m_tree->Branch("eclClusterToMC3",      "std::vector<int>",       &m_eclClusterToMC3);
  m_tree->Branch("eclClusterToMCWeight3",      "std::vector<double>",       &m_eclClusterToMCWeight3);
  m_tree->Branch("eclClusterToMC3PDG",      "std::vector<int>",       &m_eclClusterToMC3PDG);
  m_tree->Branch("eclClusterToMC4",      "std::vector<int>",       &m_eclClusterToMC4);
  m_tree->Branch("eclClusterToMCWeight4",      "std::vector<double>",       &m_eclClusterToMCWeight4);
  m_tree->Branch("eclClusterToMC4PDG",      "std::vector<int>",       &m_eclClusterToMC4PDG);
  m_tree->Branch("eclClusterToMC5",      "std::vector<int>",       &m_eclClusterToMC5);
  m_tree->Branch("eclClusterToMCWeight5",      "std::vector<double>",       &m_eclClusterToMCWeight5);
  m_tree->Branch("eclClusterToMC5PDG",      "std::vector<int>",       &m_eclClusterToMC5PDG);
  m_tree->Branch("eclClusterToBkgWeight",      "std::vector<double>",       &m_eclClusterToBkgWeight);
  m_tree->Branch("eclClusterSimHitSum",      "std::vector<double>",       &m_eclClusterSimHitSum);
  m_tree->Branch("eclClusterToShower",      "std::vector<int>",       &m_eclClusterToShower);
  m_tree->Branch("eclClusterEnergy",     "std::vector<double>",    &m_eclClusterEnergy);
  m_tree->Branch("eclClusterEnergyError",  "std::vector<double>",    &m_eclClusterEnergyError);
  m_tree->Branch("eclClusterTheta",      "std::vector<double>",    &m_eclClusterTheta);
  m_tree->Branch("eclClusterThetaError",   "std::vector<double>",    &m_eclClusterThetaError);
  m_tree->Branch("eclClusterPhi",        "std::vector<double>",    &m_eclClusterPhi);
  m_tree->Branch("eclClusterPhiError",     "std::vector<double>",    &m_eclClusterPhiError);
  m_tree->Branch("eclClusterR",          "std::vector<double>",    &m_eclClusterR);
  m_tree->Branch("eclClusterEnergyDepSum",   "std::vector<double>",    &m_eclClusterEnergyDepSum);
  m_tree->Branch("eclClusterTiming",     "std::vector<double>",    &m_eclClusterTiming);
  m_tree->Branch("eclClusterTimingError",  "std::vector<double>",    &m_eclClusterTimingError);
  m_tree->Branch("eclClusterE9oE21",     "std::vector<double>",    &m_eclClusterE9oE21);
  m_tree->Branch("eclClusterHighestE",   "std::vector<double>",    &m_eclClusterHighestE);
  m_tree->Branch("eclClusterNofCrystals",   "std::vector<int>",       &m_eclClusterNofCrystals);
  m_tree->Branch("eclClusterCrystalHealth", "std::vector<int>",       &m_eclClusterCrystalHealth);
  m_tree->Branch("eclClusterPx",         "std::vector<double>",    &m_eclClusterPx);
  m_tree->Branch("eclClusterPy",         "std::vector<double>",    &m_eclClusterPy);
  m_tree->Branch("eclClusterPz",         "std::vector<double>",    &m_eclClusterPz);
  m_tree->Branch("eclClusterIsTrack",    "std::vector<bool>",       &m_eclClusterIsTrack);
  m_tree->Branch("eclClusterClosestTrackDist",    "std::vector<double>",       &m_eclClusterClosestTrackDist);
  m_tree->Branch("eclClusterDeltaL",     "std::vector<double>",    &m_eclClusterDeltaL);
  m_tree->Branch("eclClusterAbsZernike40",     "std::vector<double>",    &m_eclClusterAbsZernike40);
  m_tree->Branch("eclClusterAbsZernike51",     "std::vector<double>",    &m_eclClusterAbsZernike51);
  m_tree->Branch("eclClusterZernikeMVA",     "std::vector<double>",    &m_eclClusterZernikeMVA);
  m_tree->Branch("eclClusterE1oE9",     "std::vector<double>",    &m_eclClusterE1oE9);
  m_tree->Branch("eclClusterSecondMoment",     "std::vector<double>",    &m_eclClusterSecondMoment);
  m_tree->Branch("eclClusterLAT",     "std::vector<double>",    &m_eclClusterLAT);
  m_tree->Branch("eclClusterDeltaTime99",     "std::vector<double>",    &m_eclClusterDeltaTime99);
  m_tree->Branch("eclClusterDetectorRegion",     "std::vector<int>",    &m_eclClusterDetectorRegion);
  m_tree->Branch("eclClusterHypothesisId",     "std::vector<int>",    &m_eclClusterHypothesisId);

  if (m_doPureCsIStudy == true) {
    m_tree->Branch("eclHitToPureDigit",      "std::vector<int>",       &m_eclHitToPureDigit);
    m_tree->Branch("eclHitToPureDigitAmp",      "std::vector<int>",       &m_eclHitToPureDigitAmp);
    m_tree->Branch("eclPureDigitMultip",     &m_eclPureDigitMultip,         "ecdigit_Multip/I");
    m_tree->Branch("eclPureDigitIdx",        "std::vector<int>",         &m_eclPureDigitIdx);
    m_tree->Branch("eclPureDigitToMC",      "std::vector<int>",          &m_eclPureDigitToMC);
    m_tree->Branch("eclPureDigitCellId",     "std::vector<int>",         &m_eclPureDigitCellId);
    m_tree->Branch("eclPureDigitAmp",        "std::vector<int>",         &m_eclPureDigitAmp);
    m_tree->Branch("eclPureDigitTimeFit",    "std::vector<int>",         &m_eclPureDigitTimeFit);
    m_tree->Branch("eclPureDigitFitQuality",    "std::vector<int>",         &m_eclPureDigitFitQuality);
    m_tree->Branch("eclPureDigitToCluster",        "std::vector<int>",         &m_eclPureDigitToCluster);
    m_tree->Branch("eclPureClusterMultip",     &m_eclPureClusterMultip,     "eclPureClusterMultip/I");
    m_tree->Branch("eclPureClusterIdx",     "std::vector<int>",       &m_eclPureClusterIdx);
    m_tree->Branch("eclPureClusterToMC",      "std::vector<int>",          &m_eclPureClusterToMC);
    m_tree->Branch("eclPureClusterEnergy",     "std::vector<double>",    &m_eclPureClusterEnergy);
    m_tree->Branch("eclPureClusterEnergyError",  "std::vector<double>",    &m_eclPureClusterEnergyError);
    m_tree->Branch("eclPureClusterTheta",      "std::vector<double>",    &m_eclPureClusterTheta);
    m_tree->Branch("eclPureClusterThetaError",   "std::vector<double>",    &m_eclPureClusterThetaError);
    m_tree->Branch("eclPureClusterPhi",        "std::vector<double>",    &m_eclPureClusterPhi);
    m_tree->Branch("eclPureClusterPhiError",     "std::vector<double>",    &m_eclPureClusterPhiError);
    m_tree->Branch("eclPureClusterR",          "std::vector<double>",    &m_eclPureClusterR);
    m_tree->Branch("eclPureClusterEnergyDepSum",   "std::vector<double>",    &m_eclPureClusterEnergyDepSum);
    m_tree->Branch("eclPureClusterTiming",     "std::vector<double>",    &m_eclPureClusterTiming);
    m_tree->Branch("eclPureClusterTimingError",  "std::vector<double>",    &m_eclPureClusterTimingError);
    m_tree->Branch("eclPureClusterE9oE21",     "std::vector<double>",    &m_eclPureClusterE9oE21);
    m_tree->Branch("eclPureClusterHighestE",   "std::vector<double>",    &m_eclPureClusterHighestE);
    m_tree->Branch("eclPureClusterLat",        "std::vector<double>",    &m_eclPureClusterLat);
    m_tree->Branch("eclPureClusterNofCrystals",   "std::vector<int>",       &m_eclPureClusterNofCrystals);
    m_tree->Branch("eclPureClusterCrystalHealth", "std::vector<int>",       &m_eclPureClusterCrystalHealth);
    m_tree->Branch("eclPureClusterPx",         "std::vector<double>",    &m_eclPureClusterPx);
    m_tree->Branch("eclPureClusterPy",         "std::vector<double>",    &m_eclPureClusterPy);
    m_tree->Branch("eclPureClusterPz",         "std::vector<double>",    &m_eclPureClusterPz);
    m_tree->Branch("eclPureClusterIsTrack",    "std::vector<bool>",       &m_eclPureClusterIsTrack);
    m_tree->Branch("eclPureClusterDeltaL",     "std::vector<double>",    &m_eclPureClusterDeltaL);
  }

  ///SHOWERS

  m_tree->Branch("eclShowerMultip",     &m_eclShowerMultip,     "eclShowerMultip/I");
  m_tree->Branch("eclShowerIdx",     "std::vector<int>",       &m_eclShowerIdx);
  m_tree->Branch("eclShowerToMC1",      "std::vector<int>",       &m_eclShowerToMC1);
  m_tree->Branch("eclShowerToMCWeight1",      "std::vector<double>",       &m_eclShowerToMCWeight1);
  m_tree->Branch("eclShowerToMC1PDG",      "std::vector<int>",       &m_eclShowerToMC1PDG);
  m_tree->Branch("eclShowerToMC2",      "std::vector<int>",       &m_eclShowerToMC2);
  m_tree->Branch("eclShowerToMCWeight2",      "std::vector<double>",       &m_eclShowerToMCWeight2);
  m_tree->Branch("eclShowerToMC2PDG",      "std::vector<int>",       &m_eclShowerToMC2PDG);
  m_tree->Branch("eclShowerToMC3",      "std::vector<int>",       &m_eclShowerToMC3);
  m_tree->Branch("eclShowerToMCWeight3",      "std::vector<double>",       &m_eclShowerToMCWeight3);
  m_tree->Branch("eclShowerToMC3PDG",      "std::vector<int>",       &m_eclShowerToMC3PDG);
  m_tree->Branch("eclShowerToMC4",      "std::vector<int>",       &m_eclShowerToMC4);
  m_tree->Branch("eclShowerToMCWeight4",      "std::vector<double>",       &m_eclShowerToMCWeight4);
  m_tree->Branch("eclShowerToMC4PDG",      "std::vector<int>",       &m_eclShowerToMC4PDG);
  m_tree->Branch("eclShowerToMC5",      "std::vector<int>",       &m_eclShowerToMC5);
  m_tree->Branch("eclShowerToMCWeight5",      "std::vector<double>",       &m_eclShowerToMCWeight5);
  m_tree->Branch("eclShowerToMC5PDG",      "std::vector<int>",       &m_eclShowerToMC5PDG);
  m_tree->Branch("eclShowerToBkgWeight",      "std::vector<double>",       &m_eclShowerToBkgWeight);
  m_tree->Branch("eclShowerSimHitSum",      "std::vector<double>",       &m_eclShowerSimHitSum);
  m_tree->Branch("eclShowerEnergy",     "std::vector<double>",    &m_eclShowerEnergy);
  m_tree->Branch("eclShowerUncEnergy",  "std::vector<double>",    &m_eclShowerUncEnergy);
  m_tree->Branch("eclShowerTheta",      "std::vector<double>",    &m_eclShowerTheta);
  m_tree->Branch("eclShowerPhi",        "std::vector<double>",    &m_eclShowerPhi);
  m_tree->Branch("eclShowerR",          "std::vector<double>",    &m_eclShowerR);
  m_tree->Branch("eclShowerNHits",   "std::vector<double>",       &m_eclShowerNHits);
  m_tree->Branch("eclShowerE9oE21",     "std::vector<double>",    &m_eclShowerE9oE21);
  m_tree->Branch("eclShowerTime",     "std::vector<double>",    &m_eclShowerTime);
  m_tree->Branch("eclShowerConnectedRegionId",     "std::vector<int>",      &m_eclShowerConnectedRegionId);
  m_tree->Branch("eclShowerHypothesisId",     "std::vector<int>",    &m_eclShowerHypothesisId);
  m_tree->Branch("eclShowerCentralCellId",     "std::vector<int>",      &m_eclShowerCentralCellId);
  m_tree->Branch("eclShowerEnergyError",     "std::vector<double>",       &m_eclShowerEnergyError);
  m_tree->Branch("eclShowerThetaError",     "std::vector<double>",      &m_eclShowerThetaError);
  m_tree->Branch("eclShowerPhiError",     "std::vector<double>",      &m_eclShowerPhiError);
  m_tree->Branch("eclShowerTimeResolution",     "std::vector<double>",      &m_eclShowerTimeResolution);
  m_tree->Branch("eclShowerHighestEnergy",     "std::vector<double>",      &m_eclShowerHighestEnergy);
  m_tree->Branch("eclShowerLateralEnergy",     "std::vector<double>",      &m_eclShowerLateralEnergy);
  m_tree->Branch("eclShowerMinTrkDistance",     "std::vector<double>",    &m_eclShowerMinTrkDistance);
  m_tree->Branch("eclShowerTrkDepth",     "std::vector<double>",    &m_eclShowerTrkDepth);
  m_tree->Branch("eclShowerShowerDepth",     "std::vector<double>",      &m_eclShowerShowerDepth);
  m_tree->Branch("eclShowerAbsZernike40",     "std::vector<double>",       &m_eclShowerAbsZernike40);
  m_tree->Branch("eclShowerAbsZernike51",     "std::vector<double>",       &m_eclShowerAbsZernike51);
  m_tree->Branch("eclShowerZernikeMVA",     "std::vector<double>",       &m_eclShowerZernikeMVA);
  m_tree->Branch("eclShowerSecondMoment",     "std::vector<double>",        &m_eclShowerSecondMoment);
  m_tree->Branch("eclShowerE1oE9",     "std::vector<double>",      &m_eclShowerE1oE9);
  m_tree->Branch("eclShowerIsTrack",     "std::vector<int>",    &m_eclShowerIsTrack);
  m_tree->Branch("eclShowerIsCluster",     "std::vector<bool>",        &m_eclShowerIsCluster);
  m_tree->Branch("eclShowerMCVtxInEcl",     "std::vector<int>",    &m_eclShowerMCVtxInEcl);
  m_tree->Branch("eclShowerHighestE1mE2",     "std::vector<double>",    &m_eclShowerHighestE1mE2);

  m_tree->Branch("mcMultip",     &m_mcMultip,           "mcMultip/I");
  m_tree->Branch("mcIdx",        "std::vector<int>",    &m_mcIdx);
  m_tree->Branch("mcPdg",        "std::vector<int>",    &m_mcPdg);
  m_tree->Branch("mcMothPdg",    "std::vector<int>",    &m_mcMothPdg);
  m_tree->Branch("mcGMothPdg",   "std::vector<int>",    &m_mcGMothPdg);
  m_tree->Branch("mcGGMothPdg",  "std::vector<int>",    &m_mcGGMothPdg);
  m_tree->Branch("mcEnergy",     "std::vector<double>", &m_mcEnergy);
  m_tree->Branch("mcPx",         "std::vector<double>", &m_mcPx);
  m_tree->Branch("mcPy",         "std::vector<double>", &m_mcPy);
  m_tree->Branch("mcPz",         "std::vector<double>", &m_mcPz);
  m_tree->Branch("mcDecVtxx",    "std::vector<double>", &m_mcDecayVtxX);
  m_tree->Branch("mcDecVtxy",    "std::vector<double>", &m_mcDecayVtxY);
  m_tree->Branch("mcDecVtxz",    "std::vector<double>", &m_mcDecayVtxZ);
  m_tree->Branch("mcProdVtxx",   "std::vector<double>", &m_mcProdVtxX);
  m_tree->Branch("mcProdVtxy",   "std::vector<double>", &m_mcProdVtxY);
  m_tree->Branch("mcProdVtxz",   "std::vector<double>", &m_mcProdVtxZ);
  m_tree->Branch("mcSecProc",    "std::vector<int>",    &m_mcSecondaryPhysProc);

  if (m_doTracking == true) {
    m_tree->Branch("trkMultip",     &m_trkMultip,          "trkMulti/I");
    m_tree->Branch("trkIdx",     "std::vector<int>",       &m_trkIdx);
    m_tree->Branch("trkPdg",        "std::vector<int>",    &m_trkPdg);
    m_tree->Branch("trkCharge",        "std::vector<int>",    &m_trkCharge);
    m_tree->Branch("trkPx",         "std::vector<double>", &m_trkPx);
    m_tree->Branch("trkPy",         "std::vector<double>", &m_trkPy);
    m_tree->Branch("trkPz",         "std::vector<double>", &m_trkPz);
    m_tree->Branch("trkP",         "std::vector<double>", &m_trkP);
    m_tree->Branch("trkTheta",         "std::vector<double>", &m_trkTheta);
    m_tree->Branch("trkPhi",         "std::vector<double>", &m_trkPhi);
    m_tree->Branch("trkPosx",       "std::vector<double>", &m_trkX);
    m_tree->Branch("trkPosy",       "std::vector<double>", &m_trkY);
    m_tree->Branch("trkPosz",      "std::vector<double>",  &m_trkZ);

    m_tree->Branch("eclpidtrkIdx",     "std::vector<int>",  &m_eclpidtrkIdx);
    m_tree->Branch("eclpidEnergy",     "std::vector<double>",  &m_eclpidEnergy);
    m_tree->Branch("eclpidEop",        "std::vector<double>",  &m_eclpidEop);
    m_tree->Branch("eclpidE9E21",      "std::vector<double>",  &m_eclpidE9E21);
    m_tree->Branch("eclpidNCrystals",  "std::vector<int>",  &m_eclpidNCrystals);
    m_tree->Branch("eclpidNClusters",  "std::vector<int>",  &m_eclpidNClusters);
    m_tree->Branch("eclLogLikeEl",      "std::vector<double>",  &m_eclLogLikeEl);
    m_tree->Branch("eclLogLikeMu",      "std::vector<double>",  &m_eclLogLikeMu);
    m_tree->Branch("eclLogLikePi",      "std::vector<double>",  &m_eclLogLikePi);
  }

  B2INFO("[ECLDataAnalysis Module]: Initialization of ECLDataAnalysis Module completed.");

}

void ECLDataAnalysisModule::beginRun()
{
}


void ECLDataAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLDataAnalysisModule");

  // re-initialize vars
  //m_eclTriggerMultip=0; m_eclTriggerCellId->clear();  m_eclTriggerTime->clear();   m_eclTriggerIdx->clear();

  ///Digits
  m_eclDigitMultip = 0;
  m_eclDigitIdx->clear();
  m_eclDigitToMC->clear();
  m_eclDigitCellId->clear();
  m_eclDigitAmp->clear();
  m_eclDigitTimeFit->clear();
  m_eclDigitFitQuality->clear();
  m_eclDigitToCalDigit->clear(); //m_eclDigitToHit->clear();

  ///CalDigits
  m_eclCalDigitMultip = 0;
  m_eclCalDigitCellId->clear();
  m_eclCalDigitAmp->clear();
  m_eclCalDigitTimeFit->clear();
  m_eclCalDigitFitQuality->clear();
  m_eclCalDigitIdx->clear();
  m_eclCalDigitToMC1->clear();
  m_eclCalDigitToMCWeight1->clear();
  m_eclCalDigitToMC2->clear();
  m_eclCalDigitToMCWeight2->clear();
  m_eclCalDigitToMC3->clear();
  m_eclCalDigitToMCWeight3->clear();
  m_eclCalDigitToMC4->clear();
  m_eclCalDigitToMCWeight4->clear();
  m_eclCalDigitToMC5->clear();
  m_eclCalDigitToMCWeight5->clear();
  m_eclCalDigitToMC1PDG->clear();
  m_eclCalDigitToMC2PDG->clear();
  m_eclCalDigitToMC3PDG->clear();
  m_eclCalDigitToMC4PDG->clear();
  m_eclCalDigitToMC5PDG->clear();
  m_eclCalDigitToBkgWeight->clear();
  m_eclCalDigitSimHitSum->clear();
  m_eclCalDigitToShower->clear();
  m_eclCalDigitToCR->clear();

  m_eclCRIdx->clear();
  m_eclCRIsTrack->clear();
  m_eclCRLikelihoodMIPNGamma->clear();
  m_eclCRLikelihoodChargedHadron->clear();
  m_eclCRLikelihoodElectronNGamma->clear();
  m_eclCRLikelihoodNGamma->clear();
  m_eclCRLikelihoodNeutralHadron->clear();
  m_eclCRLikelihoodMergedPi0->clear();

  if (m_doSimulation == 1) {
    //SimHit
    m_eclSimHitMultip = 0;
    m_eclSimHitCellId->clear();
    m_eclSimHitPdg->clear();
    m_eclSimHitEnergyDep->clear();
    m_eclSimHitFlightTime->clear();
    m_eclSimHitIdx->clear();
    m_eclSimHitToMC->clear();
    m_eclSimHitX->clear();
    m_eclSimHitY->clear();
    m_eclSimHitZ->clear();
    m_eclSimHitPx->clear();
    m_eclSimHitPy->clear();
    m_eclSimHitPz->clear();

    ///Hit
    m_eclHitMultip = 0;
    m_eclHitCellId->clear();
    m_eclHitEnergyDep->clear();
    m_eclHitTimeAve->clear();
    m_eclHitIdx->clear();
    m_eclHitToMC->clear();
    m_eclHitToDigit->clear();
    m_eclHitToDigitAmp->clear();
  }

  ///Clusters
  m_eclClusterMultip = 0;
  m_eclClusterTrueMultip = 0;
  m_eclClusterGammaMultip = 0;
  m_eclClusterEnergy->clear();
  m_eclClusterEnergyError->clear();
  m_eclClusterTheta->clear();
  m_eclClusterThetaError->clear();
  m_eclClusterPhi->clear();
  m_eclClusterPhiError->clear();
  m_eclClusterR->clear();
  m_eclClusterIdx->clear();
  m_eclClusterToMC1->clear();
  m_eclClusterToMCWeight1->clear();
  m_eclClusterToMC1PDG->clear();
  m_eclClusterToMC2->clear();
  m_eclClusterToMCWeight2->clear();
  m_eclClusterToMC2PDG->clear();
  m_eclClusterToMC3->clear();
  m_eclClusterToMCWeight3->clear();
  m_eclClusterToMC3PDG->clear();
  m_eclClusterToMC4->clear();
  m_eclClusterToMCWeight4->clear();
  m_eclClusterToMC4PDG->clear();
  m_eclClusterToMC5->clear();
  m_eclClusterToMCWeight5->clear();
  m_eclClusterToMC5PDG->clear();
  m_eclClusterToBkgWeight->clear();
  m_eclClusterSimHitSum->clear();
  m_eclClusterEnergyDepSum->clear();
  m_eclClusterTiming->clear();
  m_eclClusterTimingError->clear();
  m_eclClusterE9oE21->clear();
  m_eclClusterHighestE->clear();
  m_eclClusterNofCrystals->clear();
  m_eclClusterCrystalHealth->clear();
  m_eclClusterPx->clear();
  m_eclClusterPy->clear();
  m_eclClusterPz->clear();
  m_eclClusterIsTrack->clear();
  m_eclClusterClosestTrackDist->clear();
  m_eclClusterDeltaL->clear();
  m_eclClusterToShower->clear();
  m_eclClusterAbsZernike40->clear();
  m_eclClusterAbsZernike51->clear();
  m_eclClusterZernikeMVA->clear();
  m_eclClusterE1oE9->clear();
  m_eclClusterSecondMoment->clear();
  m_eclClusterLAT->clear();
  m_eclClusterDeltaTime99->clear();
  m_eclClusterDetectorRegion->clear();
  m_eclClusterHypothesisId->clear();

  ///Showers
  m_eclShowerMultip = 0;
  m_eclShowerIdx->clear();
  m_eclShowerToMC1->clear();
  m_eclShowerToMCWeight1->clear();
  m_eclShowerToMC1PDG->clear();
  m_eclShowerToMC2->clear();
  m_eclShowerToMCWeight2->clear();
  m_eclShowerToMC2PDG->clear();
  m_eclShowerToMC3->clear();
  m_eclShowerToMCWeight3->clear();
  m_eclShowerToMC3PDG->clear();
  m_eclShowerToMC4->clear();
  m_eclShowerToMCWeight4->clear();
  m_eclShowerToMC4PDG->clear();
  m_eclShowerToMC5->clear();
  m_eclShowerToMCWeight5->clear();
  m_eclShowerToMC5PDG->clear();
  m_eclShowerToBkgWeight->clear();
  m_eclShowerSimHitSum->clear();
  m_eclShowerUncEnergy->clear();
  m_eclShowerEnergy->clear();
  m_eclShowerTheta->clear();
  m_eclShowerPhi->clear();
  m_eclShowerR->clear();
  m_eclShowerNHits->clear();
  m_eclShowerE9oE21->clear();
  m_eclShowerTime->clear();
  m_eclShowerConnectedRegionId->clear();
  m_eclShowerHypothesisId->clear();
  m_eclShowerCentralCellId->clear();
  m_eclShowerEnergyError->clear();
  m_eclShowerThetaError->clear();
  m_eclShowerPhiError->clear();
  m_eclShowerTimeResolution->clear();
  m_eclShowerHighestEnergy->clear();
  m_eclShowerLateralEnergy->clear();
  m_eclShowerMinTrkDistance->clear();
  m_eclShowerTrkDepth->clear();
  m_eclShowerShowerDepth->clear();
  m_eclShowerAbsZernike40->clear();
  m_eclShowerAbsZernike51->clear();
  m_eclShowerZernikeMVA->clear();
  m_eclShowerSecondMoment->clear();
  m_eclShowerE1oE9->clear();
  m_eclShowerIsTrack->clear();
  m_eclShowerIsCluster->clear();
  m_eclShowerMCVtxInEcl->clear();
  m_eclShowerHighestE1mE2->clear();

  ///Pure Digits
  if (m_doPureCsIStudy == true) {
    m_eclPureDigitMultip = 0;
    m_eclPureDigitIdx->clear();
    m_eclPureDigitToMC->clear();
    m_eclPureDigitCellId->clear();
    m_eclPureDigitAmp->clear();
    m_eclPureDigitTimeFit->clear();
    m_eclPureDigitFitQuality->clear();
    m_eclPureDigitToCluster->clear();
    m_eclHitToPureDigit->clear();
    m_eclHitToPureDigitAmp->clear();

    ///Pure Clusters
    m_eclPureClusterMultip = 0;
    m_eclPureClusterToMC->clear();
    m_eclPureClusterEnergy->clear();
    m_eclPureClusterEnergyError->clear();
    m_eclPureClusterTheta->clear();
    m_eclPureClusterThetaError->clear();
    m_eclPureClusterPhi->clear();
    m_eclPureClusterPhiError->clear();
    m_eclPureClusterR->clear();
    m_eclPureClusterIdx->clear();
    m_eclPureClusterTiming->clear();
    m_eclPureClusterTimingError->clear();
    m_eclPureClusterE9oE21->clear();
    m_eclPureClusterHighestE->clear();
    m_eclPureClusterLat->clear();
    m_eclPureClusterNofCrystals->clear();
    m_eclPureClusterCrystalHealth->clear();
    m_eclPureClusterMergedPi0->clear();
    m_eclPureClusterPx->clear();
    m_eclPureClusterPy->clear();
    m_eclPureClusterPz->clear();
    m_eclPureClusterIsTrack->clear();
    m_eclPureClusterDeltaL->clear();
  }

  ///MC
  m_mcMultip = 0;
  m_mcIdx->clear();
  m_mcPdg->clear();
  m_mcMothPdg->clear();
  m_mcGMothPdg->clear();
  m_mcGGMothPdg->clear();
  m_mcEnergy->clear();
  m_mcPx->clear();
  m_mcPy->clear();
  m_mcPz->clear();
  m_mcDecayVtxX->clear();
  m_mcDecayVtxY->clear();
  m_mcDecayVtxZ->clear();
  m_mcProdVtxX->clear();
  m_mcProdVtxY->clear();
  m_mcProdVtxZ->clear();
  m_mcSecondaryPhysProc->clear();

  ///Tracks
  if (m_doTracking == true) {
    m_trkMultip = 0;
    m_trkIdx->clear();
    m_trkPdg->clear();
    m_trkCharge->clear();
    m_trkPx->clear();
    m_trkPy->clear();
    m_trkPz->clear();
    m_trkP->clear();
    m_trkPhi->clear();
    m_trkTheta->clear();
    m_trkPhi->clear();
    m_trkX->clear();
    m_trkY->clear();
    m_trkZ->clear();

    ///PID
    m_eclpidtrkIdx->clear();
    m_eclpidEnergy->clear();
    m_eclpidEop->clear();
    m_eclpidE9E21->clear();
    m_eclpidNCrystals->clear();
    m_eclpidNClusters->clear();
    m_eclLogLikeEl->clear();
    m_eclLogLikeMu->clear();
    m_eclLogLikePi->clear();
  }

  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_iExperiment = eventmetadata->getExperiment();
    m_iRun = eventmetadata->getRun();
    m_iEvent = eventmetadata->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }

  //StoreArray<ECLTrig> trgs;
  StoreArray<ECLDigit> digits;
  StoreArray<ECLCalDigit> caldigits;
  StoreArray<ECLShower> showers;
  StoreArray<ECLCluster> clusters;
  StoreArray<MCParticle> mcParticles;
  StoreArray<ECLConnectedRegion> cr;
  //caldigits.requireRelationTo(showers);
  RelationArray ECLClusterToMC(clusters, mcParticles);
  RelationArray ECLShowerToMC(showers, mcParticles);
  RelationArray ECLCalDigitToMC(caldigits, mcParticles);
  RelationArray ECLCalDigitToCR(caldigits, cr);
  RelationArray ECLCalDigitToShower(caldigits, showers);

  /* TRIGGER, NOT YET IMPLEMENTED
  m_eclTriggerMultip=trgs.getEntries();
  for (int itrgs = 0; itrgs < trgs.getEntries() ; itrgs++){
    ECLTrig* aECLTrigs = trgs[itrgs];
    m_eclTriggerIdx->push_back(itrgs);
    m_eclTriggerCellId->push_back(aECLTrigs->getCellId());
    m_eclTriggerTime->push_back(aECLTrigs->getTimeTrig());
  }
  */

  //DIGITS
  m_eclDigitMultip = digits.getEntries();
  for (int idigits = 0; idigits < digits.getEntries() ; idigits++) {
    ECLDigit* aECLDigits = digits[idigits];
    m_eclDigitIdx->push_back(idigits);
    m_eclDigitCellId->push_back(aECLDigits->getCellId());
    m_eclDigitAmp->push_back(aECLDigits->getAmp());
    m_eclDigitTimeFit->push_back(aECLDigits->getTimeFit());
    m_eclDigitFitQuality->push_back(aECLDigits->getQuality());

    if (aECLDigits->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_digit = aECLDigits->getRelated<MCParticle>();
      m_eclDigitToMC->push_back(mc_digit->getArrayIndex());
    } else
      m_eclDigitToMC->push_back(-1);

    if (aECLDigits->getRelated<ECLCalDigit>() != (nullptr)) {
      const ECLCalDigit* cal_digit = aECLDigits->getRelated<ECLCalDigit>();
      m_eclDigitToCalDigit->push_back(cal_digit->getArrayIndex());
    } else
      m_eclDigitToCalDigit->push_back(-1);
  }

  //CAL DIGITS
  m_eclCalDigitMultip = caldigits.getEntries();
  for (uint icaldigits = 0; icaldigits < (uint)caldigits.getEntries() ; icaldigits++) {
    ECLCalDigit* aECLCalDigits = caldigits[icaldigits];

    m_eclCalDigitIdx->push_back(icaldigits);
    m_eclCalDigitCellId->push_back(aECLCalDigits->getCellId());
    m_eclCalDigitAmp->push_back(aECLCalDigits->getEnergy());
    m_eclCalDigitTimeFit->push_back(aECLCalDigits->getTime());
    m_eclCalDigitFitQuality->push_back(aECLCalDigits->isFailedFit());

    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    double wi[10];
    for (int i = 0; i < 10; i++)
      wi[i] = -1;

    int ii = 0;
    sumHit = 0;
    //int mc_idx = 0;

    auto digitMCRelations = aECLCalDigits->getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < digitMCRelations.size(); ++i) {
      if (ii < 10) {
        const auto mcParticle = digitMCRelations.object(i);
        idx[ii] = mcParticle->getIndex() - 1;
        wi[ii] = digitMCRelations.weight(i);
        sumHit = sumHit + digitMCRelations.weight(i);
        ii++;
      }
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 9; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (wi[i] < wi[i + 1]) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;

            double wtemp = wi[i];
            wi[i] = wi[i + 1];
            wi[i + 1] = wtemp;
          }
        }
      }
      y++;
    }

    m_eclCalDigitToBkgWeight->push_back(aECLCalDigits->getEnergy() - sumHit);
    m_eclCalDigitSimHitSum->push_back(sumHit);
    if (idx[0] > -1) {
      m_eclCalDigitToMCWeight1->push_back(wi[0]);
      m_eclCalDigitToMC1->push_back(idx[0]);
      MCParticle* amcParticle = mcParticles[idx[0]];
      m_eclCalDigitToMC1PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMCWeight1->push_back(-1);
      m_eclCalDigitToMC1->push_back(-1);
      m_eclCalDigitToMC1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclCalDigitToMCWeight2->push_back(wi[1]);
      m_eclCalDigitToMC2->push_back(idx[1]);
      MCParticle* amcParticle = mcParticles[idx[1]];
      m_eclCalDigitToMC2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMCWeight2->push_back(-1);
      m_eclCalDigitToMC2->push_back(-1);
      m_eclCalDigitToMC2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclCalDigitToMCWeight3->push_back(wi[2]);
      m_eclCalDigitToMC3->push_back(idx[2]);
      MCParticle* amcParticle = mcParticles[idx[2]];
      m_eclCalDigitToMC3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMCWeight3->push_back(-1);
      m_eclCalDigitToMC3->push_back(-1);
      m_eclCalDigitToMC3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclCalDigitToMCWeight4->push_back(wi[3]);
      m_eclCalDigitToMC4->push_back(idx[3]);
      MCParticle* amcParticle = mcParticles[idx[3]];
      m_eclCalDigitToMC4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMCWeight4->push_back(-1);
      m_eclCalDigitToMC4->push_back(-1);
      m_eclCalDigitToMC4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclCalDigitToMCWeight5->push_back(wi[4]);
      m_eclCalDigitToMC5->push_back(idx[4]);
      MCParticle* amcParticle = mcParticles[idx[4]];
      m_eclCalDigitToMC5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMCWeight5->push_back(-1);
      m_eclCalDigitToMC5->push_back(-1);
      m_eclCalDigitToMC5PDG->push_back(-1);
    }

    if (aECLCalDigits->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_caldigit = aECLCalDigits->getRelated<ECLShower>();
      m_eclCalDigitToShower->push_back(shower_caldigit->getArrayIndex());
    } else
      m_eclCalDigitToShower->push_back(-1);

    if (aECLCalDigits->getRelated<ECLConnectedRegion>() != (nullptr)) {
      const ECLConnectedRegion* cr_caldigit = aECLCalDigits->getRelated<ECLConnectedRegion>();
      m_eclCalDigitToCR->push_back(cr_caldigit->getCRId());
    } else
      m_eclCalDigitToCR->push_back(-1);

  }


  //CR
  int CRmultip = cr.getEntries();
  for (int i = 0; i < CRmultip; i++) {
    ECLConnectedRegion* aECLCR = cr[i];
    m_eclCRIdx->push_back(aECLCR->getCRId());
    m_eclCRIsTrack->push_back(aECLCR->isTrack());
    m_eclCRLikelihoodMIPNGamma->push_back(aECLCR->getLikelihoodMIPNGamma());
    m_eclCRLikelihoodChargedHadron->push_back(aECLCR->getLikelihoodChargedHadron());
    m_eclCRLikelihoodElectronNGamma->push_back(aECLCR->getLikelihoodElectronNGamma());
    m_eclCRLikelihoodNGamma->push_back(aECLCR->getLikelihoodNGamma());
    m_eclCRLikelihoodNeutralHadron->push_back(aECLCR->getLikelihoodNeutralHadron());
    m_eclCRLikelihoodMergedPi0->push_back(aECLCR->getLikelihoodMergedPi0());
  }

  if (m_doSimulation == 1) {

    StoreArray<ECLSimHit> simhits;
    StoreArray<ECLHit> hits;
    RelationArray ECLHitsToDigit(hits, digits);
    RelationArray ECLHitsToMC(hits, mcParticles);

    //SIM HITS
    m_eclSimHitMultip = simhits.getEntries();
    for (int isimhits = 0; isimhits < simhits.getEntries() ; isimhits++) {
      ECLSimHit* aECLSimHits = simhits[isimhits];

      m_eclSimHitIdx->push_back(isimhits);
      m_eclSimHitCellId->push_back(aECLSimHits->getCellId());
      m_eclSimHitPdg->push_back(aECLSimHits->getPDGCode());
      m_eclSimHitEnergyDep->push_back(aECLSimHits->getEnergyDep());
      m_eclSimHitFlightTime->push_back(aECLSimHits->getFlightTime());
      m_eclSimHitX->push_back(aECLSimHits->getPosition().x());
      m_eclSimHitY->push_back(aECLSimHits->getPosition().y());
      m_eclSimHitZ->push_back(aECLSimHits->getPosition().z());
      m_eclSimHitPx->push_back(aECLSimHits->getMomentum().x());
      m_eclSimHitPy->push_back(aECLSimHits->getMomentum().y());
      m_eclSimHitPz->push_back(aECLSimHits->getMomentum().z());

      if (aECLSimHits->getRelated<MCParticle>() != (nullptr)) {
        const MCParticle* mc_simhit = aECLSimHits->getRelated<MCParticle>();
        m_eclSimHitToMC->push_back(mc_simhit->getArrayIndex());
      } else
        m_eclSimHitToMC->push_back(-1);
    }

    //HITS
    m_eclHitMultip = hits.getEntries();
    for (int ihits = 0; ihits < hits.getEntries() ; ihits++) {
      ECLHit* aECLHits = hits[ihits];
      m_eclHitIdx->push_back(ihits);
      m_eclHitCellId->push_back(aECLHits->getCellId());
      m_eclHitEnergyDep->push_back(aECLHits->getEnergyDep());
      m_eclHitTimeAve->push_back(aECLHits->getTimeAve());

      if (aECLHits->getRelated<ECLDigit>() != (nullptr)) {
        //const MCParticle* mc_digit = aECLHits->getRelated<MCParticle>();
        const ECLDigit* hit_digit = aECLHits->getRelated<ECLDigit>();
        m_eclHitToDigit->push_back(hit_digit->getArrayIndex());
        m_eclHitToDigitAmp->push_back(hit_digit->getAmp());
      } else {
        m_eclHitToDigit->push_back(-1);
        m_eclHitToDigitAmp->push_back(-1);
      }

      if (m_doPureCsIStudy == true) {
        StoreArray<ECLDigit> pure_digits(m_pure_digits);
        if (aECLHits->getRelated<ECLDigit>(m_pure_digits) != (nullptr)) {
          const ECLDigit* hit_pdigit = aECLHits->getRelated<ECLDigit>(m_pure_digits);
          m_eclHitToPureDigit->push_back(hit_pdigit->getArrayIndex());
          m_eclHitToPureDigitAmp->push_back(hit_pdigit->getAmp());
        } else {
          m_eclHitToPureDigit->push_back(-1);
          m_eclHitToPureDigitAmp->push_back(-1);
        }
      }

      if (aECLHits->getRelated<MCParticle>() != (nullptr)) {
        const MCParticle* mc_hit = aECLHits->getRelated<MCParticle>();
        m_eclHitToMC->push_back(mc_hit->getArrayIndex());
      } else
        m_eclHitToMC->push_back(-1);
    }
  }

  //CLUSTERS
  int true_clu = 0;
  int gamma_clu = 0;
  m_eclClusterMultip = clusters.getEntries();
  for (unsigned int iclusters = 0; iclusters < (unsigned int)clusters.getEntries() ; iclusters++) {
    ECLCluster* aECLClusters = clusters[iclusters];
    m_eclClusterIdx->push_back(iclusters);
    m_eclClusterEnergy->push_back(aECLClusters->getEnergy());
    m_eclClusterEnergyError->push_back(aECLClusters->getUncertaintyEnergy());
    m_eclClusterTheta->push_back(aECLClusters->getTheta());
    m_eclClusterThetaError->push_back(aECLClusters->getUncertaintyTheta());
    m_eclClusterPhi->push_back(aECLClusters->getPhi());
    m_eclClusterPhiError->push_back(aECLClusters->getUncertaintyPhi());
    m_eclClusterR->push_back(aECLClusters->getR());
    m_eclClusterEnergyDepSum->push_back(aECLClusters->getEnergyRaw());
    m_eclClusterTiming->push_back(aECLClusters->getTime());
    m_eclClusterTimingError->push_back(aECLClusters->getDeltaTime99());
    m_eclClusterE9oE21->push_back(aECLClusters->getE9oE21());
    m_eclClusterHighestE->push_back(aECLClusters->getEnergyHighestCrystal());
    m_eclClusterNofCrystals->push_back(aECLClusters->getNumberOfCrystals());
    m_eclClusterCrystalHealth->push_back(aECLClusters->getStatus());
    m_eclClusterPx->push_back(aECLClusters->getPx());
    m_eclClusterPy->push_back(aECLClusters->getPy());
    m_eclClusterPz->push_back(aECLClusters->getPz());
    m_eclClusterIsTrack->push_back(aECLClusters->isTrack());
    m_eclClusterClosestTrackDist->push_back(aECLClusters->getMinTrkDistance());
    m_eclClusterDeltaL->push_back(aECLClusters->getDeltaL());

    m_eclClusterAbsZernike40->push_back(aECLClusters->getAbsZernike40());
    m_eclClusterAbsZernike51->push_back(aECLClusters->getAbsZernike51());
    m_eclClusterZernikeMVA->push_back(aECLClusters->getZernikeMVA());
    m_eclClusterE1oE9->push_back(aECLClusters->getE1oE9());
    m_eclClusterSecondMoment->push_back(aECLClusters->getSecondMoment());
    m_eclClusterLAT->push_back(aECLClusters->getLAT());
    m_eclClusterDeltaTime99->push_back(aECLClusters->getDeltaTime99());
    m_eclClusterDetectorRegion->push_back(aECLClusters->getDetectorRegion());
    m_eclClusterHypothesisId->push_back(aECLClusters->getHypothesisId());

    if (aECLClusters->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_cluster = aECLClusters->getRelated<ECLShower>();
      m_eclClusterToShower->push_back(shower_cluster->getArrayIndex());
    } else
      m_eclClusterToShower->push_back(-1);


    //Dump MC Info - Multiple Matching
    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    double wi[10];
    for (int i = 0; i < 10; i++)
      wi[i] = -1;

    int ii = 0;
    sumHit = 0;

    auto clusterMCRelations = aECLClusters->getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < clusterMCRelations.size(); ++i) {
      if (ii < 10) {
        const auto mcParticle = clusterMCRelations.object(i);
        idx[ii] = mcParticle->getIndex() - 1;
        wi[ii] = clusterMCRelations.weight(i);
        sumHit = sumHit + clusterMCRelations.weight(i);
        ii++;
      }
    }

    if ((aECLClusters->getHypothesisId() == 5)) {
      gamma_clu++;
      if ((idx[0] > -1))
        true_clu++;
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 9; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (wi[i] < wi[i + 1]) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;

            double wtemp = wi[i];
            wi[i] = wi[i + 1];
            wi[i + 1] = wtemp;
          }
        }
      }
      y++;
    }

    m_eclClusterToBkgWeight->push_back(aECLClusters->getEnergy() - sumHit);
    m_eclClusterSimHitSum->push_back(sumHit);
    if (idx[0] > -1) {
      m_eclClusterToMCWeight1->push_back(wi[0]);
      m_eclClusterToMC1->push_back(idx[0]);
      MCParticle* amcParticle = mcParticles[idx[0]];
      m_eclClusterToMC1PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMCWeight1->push_back(-1);
      m_eclClusterToMC1->push_back(-1);
      m_eclClusterToMC1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclClusterToMCWeight2->push_back(wi[1]);
      m_eclClusterToMC2->push_back(idx[1]);
      MCParticle* amcParticle = mcParticles[idx[1]];
      m_eclClusterToMC2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMCWeight2->push_back(-1);
      m_eclClusterToMC2->push_back(-1);
      m_eclClusterToMC2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclClusterToMCWeight3->push_back(wi[2]);
      m_eclClusterToMC3->push_back(idx[2]);
      MCParticle* amcParticle = mcParticles[idx[2]];
      m_eclClusterToMC3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMCWeight3->push_back(-1);
      m_eclClusterToMC3->push_back(-1);
      m_eclClusterToMC3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclClusterToMCWeight4->push_back(wi[3]);
      m_eclClusterToMC4->push_back(idx[3]);
      MCParticle* amcParticle = mcParticles[idx[3]];
      m_eclClusterToMC4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMCWeight4->push_back(-1);
      m_eclClusterToMC4->push_back(-1);
      m_eclClusterToMC4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclClusterToMCWeight5->push_back(wi[4]);
      m_eclClusterToMC5->push_back(idx[4]);
      MCParticle* amcParticle = mcParticles[idx[4]];
      m_eclClusterToMC5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMCWeight5->push_back(-1);
      m_eclClusterToMC5->push_back(-1);
      m_eclClusterToMC5PDG->push_back(-1);
    }

  }
  m_eclClusterTrueMultip = true_clu;
  m_eclClusterGammaMultip = gamma_clu;

  if (m_doPureCsIStudy == true) {

    StoreArray<ECLDigit> pure_digits(m_pure_digits);
    StoreArray<ECLCalDigit> pure_cal_digits(m_pure_cal_digits);
    StoreArray<ECLCluster> pure_clusters(m_pure_clusters);
    pure_digits.requireRelationTo(mcParticles);
    pure_cal_digits.requireRelationTo(pure_clusters);

    m_eclPureDigitMultip = pure_digits.getEntries();
    for (int idigits = 0; idigits < pure_digits.getEntries() ; idigits++) {
      ECLDigit* aECLPureDigits = pure_digits[idigits];

      m_eclPureDigitIdx->push_back(idigits);
      m_eclPureDigitCellId->push_back(aECLPureDigits->getCellId());
      m_eclPureDigitAmp->push_back(aECLPureDigits->getAmp());
      m_eclPureDigitTimeFit->push_back(aECLPureDigits->getTimeFit());
      m_eclPureDigitFitQuality->push_back(aECLPureDigits->getQuality());

      ECLCalDigit* aECLPureCalDigits = pure_cal_digits[idigits];

      if (aECLPureCalDigits->getRelated<ECLCluster>(m_pure_clusters) != (nullptr)) {
        const ECLCluster* cluster_digit = aECLPureCalDigits->getRelated<ECLCluster>(m_pure_clusters);
        m_eclPureDigitToCluster->push_back(cluster_digit->getArrayIndex());
      } else
        m_eclPureDigitToCluster->push_back(-1);

      if (aECLPureDigits->getRelated<MCParticle>() != (nullptr)) {
        const MCParticle* mc_digit = aECLPureDigits->getRelated<MCParticle>();
        m_eclPureDigitToMC->push_back(mc_digit->getArrayIndex());
      } else
        m_eclPureDigitToMC->push_back(-1);
    }

    m_eclPureClusterMultip = pure_clusters.getEntries();
    for (unsigned int iclusters = 0; iclusters < (unsigned int)pure_clusters.getEntries() ; iclusters++) {
      ECLCluster* aECLClusters = pure_clusters[iclusters];
      m_eclPureClusterIdx->push_back(iclusters);
      m_eclPureClusterEnergy->push_back(aECLClusters->getEnergy());
      m_eclPureClusterEnergyError->push_back(aECLClusters->getUncertaintyEnergy());
      m_eclPureClusterTheta->push_back(aECLClusters->getTheta());
      m_eclPureClusterThetaError->push_back(aECLClusters->getUncertaintyTheta());
      m_eclPureClusterPhi->push_back(aECLClusters->getPhi());
      m_eclPureClusterPhiError->push_back(aECLClusters->getUncertaintyPhi());
      m_eclPureClusterR->push_back(aECLClusters->getR());
      m_eclPureClusterEnergyDepSum->push_back(aECLClusters->getEnergyRaw());
      m_eclPureClusterTiming->push_back(aECLClusters->getTime());
      m_eclPureClusterTimingError->push_back(aECLClusters->getDeltaTime99());
      m_eclPureClusterE9oE21->push_back(aECLClusters->getE9oE21());
      m_eclPureClusterHighestE->push_back(aECLClusters->getEnergyHighestCrystal());
      m_eclPureClusterLat->push_back(aECLClusters->getLAT());
      m_eclPureClusterNofCrystals->push_back(aECLClusters->getNumberOfCrystals());
      m_eclPureClusterCrystalHealth->push_back(aECLClusters->getStatus());
      m_eclPureClusterPx->push_back(aECLClusters->getPx());
      m_eclPureClusterPy->push_back(aECLClusters->getPy());
      m_eclPureClusterPz->push_back(aECLClusters->getPz());
      m_eclPureClusterIsTrack->push_back(aECLClusters->isTrack());
      m_eclPureClusterDeltaL->push_back(aECLClusters->getDeltaL());

      if (aECLClusters->getRelated<MCParticle>() != (nullptr)) {
        const MCParticle* mc_cluster = aECLClusters->getRelated<MCParticle>();
        m_eclPureClusterToMC->push_back(mc_cluster->getArrayIndex());
      } else
        m_eclPureClusterToMC->push_back(-1);
    }
  }

  m_eclShowerMultip = showers.getEntries();
  for (uint ishowers = 0; ishowers < (uint)showers.getEntries() ; ishowers++) {
    ECLShower* aECLShowers = showers[ishowers];
    m_eclShowerIdx->push_back(ishowers);
    m_eclShowerEnergy->push_back(aECLShowers->getEnergy());
    m_eclShowerTheta->push_back(aECLShowers->getTheta());
    m_eclShowerPhi->push_back(aECLShowers->getPhi());
    m_eclShowerR->push_back(aECLShowers->getR());
    m_eclShowerNHits->push_back(aECLShowers->getNumberOfCrystals());
    m_eclShowerE9oE21->push_back(aECLShowers->getE9oE21());
    m_eclShowerUncEnergy->push_back(aECLShowers->getEnergyRaw());
    m_eclShowerTime->push_back(aECLShowers->getTime());
    m_eclShowerConnectedRegionId->push_back(aECLShowers->getConnectedRegionId());
    m_eclShowerHypothesisId->push_back(aECLShowers->getHypothesisId());
    m_eclShowerCentralCellId->push_back(aECLShowers->getCentralCellId());
    m_eclShowerEnergyError->push_back(aECLShowers->getUncertaintyEnergy());
    m_eclShowerThetaError->push_back(aECLShowers->getUncertaintyTheta());
    m_eclShowerPhiError->push_back(aECLShowers->getUncertaintyPhi());
    m_eclShowerTimeResolution->push_back(aECLShowers->getDeltaTime99());
    m_eclShowerHighestEnergy->push_back(aECLShowers->getEnergyHighestCrystal());
    m_eclShowerLateralEnergy->push_back(aECLShowers->getLateralEnergy());
    m_eclShowerMinTrkDistance->push_back(aECLShowers->getMinTrkDistance());
    m_eclShowerTrkDepth->push_back(aECLShowers->getTrkDepth());
    m_eclShowerShowerDepth->push_back(aECLShowers->getShowerDepth());
    m_eclShowerAbsZernike40->push_back(aECLShowers->getAbsZernike40());
    m_eclShowerAbsZernike51->push_back(aECLShowers->getAbsZernike51());
    m_eclShowerZernikeMVA->push_back(aECLShowers->getZernikeMVA());
    m_eclShowerSecondMoment->push_back(aECLShowers->getSecondMoment());
    m_eclShowerE1oE9->push_back(aECLShowers->getE1oE9());

    double fe = 0.;
    double se = 0.;

    auto showerDigitRelations = aECLShowers->getRelationsTo<ECLCalDigit>();
    for (unsigned int i = 0; i < showerDigitRelations.size(); ++i) {
      const auto aECLCalDigits = showerDigitRelations.object(i);
      if (aECLCalDigits->getEnergy() > fe) {
        se = fe;
        fe = aECLCalDigits->getEnergy();
      }
    }
    if (fe > 0 && se > 0)
      m_eclShowerHighestE1mE2->push_back(fe - se);
    else
      m_eclShowerHighestE1mE2->push_back(-1);

    double sumHit = 0;

    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    double wi[10];
    for (int i = 0; i < 10; i++)
      wi[i] = -1;

    int ii = 0;
    sumHit = 0;

    auto showerMCRelations = aECLShowers->getRelationsTo<MCParticle>();
    for (unsigned int i = 0; i < showerMCRelations.size(); ++i) {
      if (ii < 10) {
        const auto mcParticle = showerMCRelations.object(i);
        idx[ii] = mcParticle->getIndex() - 1;
        wi[ii] = showerMCRelations.weight(i);
        sumHit = sumHit + showerMCRelations.weight(i);
        ii++;
      }
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 9; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (wi[i] < wi[i + 1]) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;

            double wtemp = wi[i];
            wi[i] = wi[i + 1];
            wi[i + 1] = wtemp;
          }
        }
      }
      y++;
    }

    int no_rel = 0;
    int no_Primary = 1;

    for (unsigned int i = 0; i < showerMCRelations.size(); ++i) {
      no_rel++;
      const auto mcParticle = showerMCRelations.object(i);
      if (mcParticle->getSecondaryPhysicsProcess() == 0) {
        double vtxx = mcParticle->getDecayVertex().X();
        double vtxy = mcParticle->getDecayVertex().Y();
        double vtxz = mcParticle->getDecayVertex().Z();
        if ((TMath::Sqrt(vtxx * vtxx + vtxy * vtxy) > 118) || (vtxz > 196.16) || (vtxz < -102.16))
          no_Primary = 0;
      } else if (mcParticle->getSecondaryPhysicsProcess() != 0) {
        double vtxx = mcParticle->getProductionVertex().X();
        double vtxy = mcParticle->getProductionVertex().Y();
        double vtxz = mcParticle->getProductionVertex().Z();
        if ((TMath::Sqrt(vtxx * vtxx + vtxy * vtxy) > 118) || (vtxz > 196.16) || (vtxz < -102.16))
          no_Primary = 0;
      }
    }

    if (no_Primary == 0)
      m_eclShowerMCVtxInEcl->push_back(1);
    else
      m_eclShowerMCVtxInEcl->push_back(0);

    if (idx[0] > -1) {
      m_eclShowerToMCWeight1->push_back(wi[0]);
      m_eclShowerToMC1->push_back(idx[0]);
      MCParticle* amcParticle = mcParticles[idx[0]];
      m_eclShowerToMC1PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMCWeight1->push_back(-1);
      m_eclShowerToMC1->push_back(-1);
      m_eclShowerToMC1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclShowerToMCWeight2->push_back(wi[1]);
      m_eclShowerToMC2->push_back(idx[1]);
      MCParticle* amcParticle = mcParticles[idx[1]];
      m_eclShowerToMC2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMCWeight2->push_back(-1);
      m_eclShowerToMC2->push_back(-1);
      m_eclShowerToMC2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclShowerToMCWeight3->push_back(wi[2]);
      m_eclShowerToMC3->push_back(idx[2]);
      MCParticle* amcParticle = mcParticles[idx[2]];
      m_eclShowerToMC3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMCWeight3->push_back(-1);
      m_eclShowerToMC3->push_back(-1);
      m_eclShowerToMC3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclShowerToMCWeight4->push_back(wi[3]);
      m_eclShowerToMC4->push_back(idx[3]);
      MCParticle* amcParticle = mcParticles[idx[3]];
      m_eclShowerToMC4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMCWeight4->push_back(-1);
      m_eclShowerToMC4->push_back(-1);
      m_eclShowerToMC4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclShowerToMCWeight5->push_back(wi[4]);
      m_eclShowerToMC5->push_back(idx[4]);
      MCParticle* amcParticle = mcParticles[idx[4]];
      m_eclShowerToMC5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMCWeight5->push_back(-1);
      m_eclShowerToMC5->push_back(-1);
      m_eclShowerToMC5PDG->push_back(-1);
    }
  }

  m_mcMultip = mcParticles.getEntries();
  for (int imcpart = 0; imcpart < mcParticles.getEntries(); imcpart++) {
    MCParticle* amcParticle = mcParticles[imcpart];
    m_mcIdx->push_back(amcParticle->getArrayIndex());
    m_mcPdg->push_back(amcParticle->getPDG());
    if (amcParticle->getMother() != NULL) m_mcMothPdg->push_back(amcParticle->getMother()->getPDG());
    else m_mcMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL
        && amcParticle->getMother()->getMother() != NULL) m_mcGMothPdg->push_back(amcParticle->getMother()->getMother()->getPDG());
    else m_mcGMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL && amcParticle->getMother()->getMother() != NULL
        && amcParticle->getMother()->getMother()->getMother() != NULL)
      m_mcGGMothPdg->push_back(amcParticle->getMother()->getMother()->getMother()->getPDG());
    else m_mcGGMothPdg->push_back(-999);
    m_mcEnergy->push_back(amcParticle->getEnergy());
    m_mcSecondaryPhysProc->push_back(amcParticle->getSecondaryPhysicsProcess());

    m_mcPx->push_back(amcParticle->getMomentum().X());
    m_mcPy->push_back(amcParticle->getMomentum().Y());
    m_mcPz->push_back(amcParticle->getMomentum().Z());

    m_mcDecayVtxX->push_back(amcParticle->getDecayVertex().X());
    m_mcDecayVtxY->push_back(amcParticle->getDecayVertex().Y());
    m_mcDecayVtxZ->push_back(amcParticle->getDecayVertex().Z());

    m_mcProdVtxX->push_back(amcParticle->getProductionVertex().X());
    m_mcProdVtxY->push_back(amcParticle->getProductionVertex().Y());
    m_mcProdVtxZ->push_back(amcParticle->getProductionVertex().Z());

  }

  if (m_doTracking == true) {
    StoreArray<TrackFitResult> trks;
    StoreArray<Track> tracks;
    m_trkMultip = 0;
    for (const Track& itrk : tracks) {
      const TrackFitResult* atrk = itrk.getTrackFitResult(Const::pion);
      if (atrk == nullptr) continue;

      m_trkIdx->push_back(m_trkMultip);
      m_trkPdg->push_back(atrk->getParticleType().getPDGCode());
      m_trkCharge->push_back(atrk->getChargeSign());
      m_trkPx->push_back(atrk->getMomentum().X());
      m_trkPy->push_back(atrk->getMomentum().Y());
      m_trkPz->push_back(atrk->getMomentum().Z());
      m_trkP->push_back(atrk->getMomentum().Mag());
      m_trkTheta->push_back(atrk->getMomentum().Theta());
      m_trkPhi->push_back(atrk->getMomentum().Phi());
      m_trkX->push_back(atrk->getPosition().X());
      m_trkY->push_back(atrk->getPosition().Y());
      m_trkZ->push_back(atrk->getPosition().Z());

      const ECLPidLikelihood* eclpid = itrk.getRelatedTo<ECLPidLikelihood>() ;

      if (eclpid != nullptr) {
        m_eclpidtrkIdx -> push_back(m_trkMultip);
        m_eclpidEnergy -> push_back(eclpid-> energy());
        m_eclpidEop    -> push_back(eclpid-> eop());
        m_eclpidE9E21  -> push_back(eclpid-> e9e25());
        m_eclpidNCrystals -> push_back(eclpid-> nCrystals());
        m_eclpidNClusters -> push_back(eclpid-> nClusters());
        m_eclLogLikeEl -> push_back(eclpid-> getLogLikelihood(Const::electron));
        m_eclLogLikeMu -> push_back(eclpid-> getLogLikelihood(Const::muon));
        m_eclLogLikePi -> push_back(eclpid-> getLogLikelihood(Const::pion));
      } else {
        m_eclpidtrkIdx -> push_back(m_trkMultip);
        m_eclpidEnergy -> push_back(0);
        m_eclpidEop    -> push_back(0);
        m_eclpidE9E21  -> push_back(0);
        m_eclpidNCrystals -> push_back(0);
        m_eclpidNClusters -> push_back(0);
        m_eclLogLikeEl -> push_back(0);
        m_eclLogLikeMu -> push_back(0);
        m_eclLogLikePi -> push_back(0);
      }
      m_trkMultip++;
    }
  }

  m_tree->Fill();

}


void ECLDataAnalysisModule::endRun()
{
}


void ECLDataAnalysisModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    //m_rootFilePtr->cd(); //important! without this the famework root I/O could mix with the root I/O of this module
    m_tree->Write();
    m_rootFilePtr->Close();
  }

}


