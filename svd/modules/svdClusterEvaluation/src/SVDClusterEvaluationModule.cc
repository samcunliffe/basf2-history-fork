/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael De Nuccio, Giulia Casarosa                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdClusterEvaluation/SVDClusterEvaluationModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <mdst/dataobjects/MCParticle.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDRecoDigit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <TFile.h>
#include <TText.h>
#include <TH1F.h>
#include <TH2F.h>

#include <string>
#include "TMath.h"
#include <algorithm>
#include <functional>


using namespace Belle2;


REG_MODULE(SVDClusterEvaluation)


SVDClusterEvaluationModule::SVDClusterEvaluationModule() : Module()
{
  setDescription("This modules generates a TTree containing the hit profile for sensor 6, test beam.");

  addParam("outputFileName", m_outputFileName, "output rootfile name", std::string("ClusterEvaluation.root"));
}


SVDClusterEvaluationModule::~SVDClusterEvaluationModule()
{
}


void SVDClusterEvaluationModule::initialize()
{

  /* initialize of useful store array */
  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;

  SVDShaperDigits.isRequired();
  SVDRecoDigits.isRequired();
  SVDClusters.isRequired();
  SVDTrueHits.isRequired();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");

  m_histoList_ClusterPositionResolution = new TList;
  m_histoList_StripTimeResolution = new TList;
  m_histoList_ClusterTimeResolution = new TList;
  m_histoList_PurityInsideTMCluster = new TList;
  m_histo2DList_PurityInsideTMCluster = new TList;
  m_histoList_PurityInsideNOTMCluster = new TList;
  m_histoList_Puddlyness = new TList;
  m_histoList_PuddlynessTM = new TList;
  m_graphList = new TList;

  for (int i = 0; i < m_Nsets; i ++) {

    if (i % 2 == 0) { //even index, U side
      NameOfHisto = "histo_ClusterUPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster U Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterUPositionResolution[i / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -0.1, 0.1,
                                                  "U_Reconstructed - U_TrueHit",
                                                  m_histoList_ClusterPositionResolution);
    } else { //odd index, V side
      NameOfHisto = "histo_ClusterVPositionResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i);
      TitleOfHisto = "Cluster V Position Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ")";
      m_histo_ClusterVPositionResolution[(i - 1) / 2] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -0.1, 0.1,
                                                        "V_Reconstructed - V_TrueHit", m_histoList_ClusterPositionResolution);
    }

    NameOfHisto = "histo_StripTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Strip Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_StripTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_Reconstructed - t_TrueHit",
                                                       m_histoList_StripTimeResolution);

    NameOfHisto = "histo_ClusterTimeResolution_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Cluster Time Resolution (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo_ClusterTimeResolution[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 400, -100, 100, "t_Reconstructed - t_TrueHit",
                                                         m_histoList_ClusterTimeResolution);

    NameOfHisto = "histo_PurityInsideTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-matched Recos inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PurityInsideTMCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 110, 0, 1.10, "number of TM recos / cluster size",
                                                         m_histoList_PurityInsideTMCluster);

    NameOfHisto = "histo2D_PurityInsideTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of Truth-matched Recos vs Number of Recos inside a Truth-matched Cluster (" + IntExtFromIndex(
                     i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(i) + ")";
    m_histo2D_PurityInsideTMCluster[i] = createHistogram2D(NameOfHisto, TitleOfHisto, 42, 0, 42, "cluster size", 42, 0, 42,
                                                           "number of TM recos", m_histo2DList_PurityInsideTMCluster);

    NameOfHisto = "histo_PurityInsideNOTMCluster_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Fraction of Truth-matched Recos inside a NOT Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PurityInsideNOTMCluster[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 110, 0, 1.10, "number of TM recos / cluster size",
                                                           m_histoList_PurityInsideNOTMCluster);

    NameOfHisto = "m_histoList_Puddlyness_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(i) + ", side" + UVFromIndex(
                     i) + ")";
    m_histo_Puddlyness[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 10, 0 , 10, "number of TH per cluster",
                                              m_histoList_Puddlyness);

    NameOfHisto = "m_histoList_PuddlynessTM_" + IntExtFromIndex(i) + "_" + FWFromIndex(i) + "_Side" + UVFromIndex(i);
    TitleOfHisto = "Number of True Hits inside a Truth-matched Cluster (" + IntExtFromIndex(i) + ", " + FWFromIndex(
                     i) + ", side" + UVFromIndex(i) + ")";
    m_histo_PuddlynessTM[i] = createHistogram1D(NameOfHisto, TitleOfHisto, 10, 0 , 10, "number of TH per cluster",
                                                m_histoList_PuddlynessTM);
  }

}


void SVDClusterEvaluationModule::beginRun()
{
}


void SVDClusterEvaluationModule::event()
{

  StoreArray<SVDShaperDigit> SVDShaperDigits;
  StoreArray<SVDRecoDigit> SVDRecoDigits;
  StoreArray<SVDCluster> SVDClusters;
  StoreArray<SVDTrueHit> SVDTrueHits;

  //////////
  //STRIPS//
  //////////

  //loop on ShaperDigits
  for (const SVDShaperDigit& shape : SVDShaperDigits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(shape.getSensorID().getLayerNumber() , shape.getSensorID().getSensorNumber() ,
                                                       shape.isUStrip());

    RelationVector<SVDRecoDigit> relatVectorShaperToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&shape);

    //efficiency shaper to reco
    m_NumberOfShaperDigit[indexForHistosAndGraphs] ++;
    if (relatVectorShaperToReco.size() > 0)
      m_NumberOfRecoDigit[indexForHistosAndGraphs] ++;
  }
  //close loop on ShaperDigits

  //loop on RecoDigits
  for (const SVDRecoDigit& reco : SVDRecoDigits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(reco.getSensorID().getLayerNumber() , reco.getSensorID().getSensorNumber() ,
                                                       reco.isUStrip());

    RelationVector<SVDTrueHit> relatVectorRecoToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&reco);

    //strip time resolution
    if (relatVectorRecoToTH.size() > 0)
      m_histo_StripTimeResolution[indexForHistosAndGraphs]->Fill(reco.getTime() - (relatVectorRecoToTH[0])->getGlobalTime());

  }
  //close loop on RecoDigits

  ////////////
  //CLUSTERS//
  ////////////

  //loop on TrueHits
  for (const SVDTrueHit& trhi : SVDTrueHits) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(trhi.getSensorID().getLayerNumber() , trhi.getSensorID().getSensorNumber() , 1);

    RelationVector<SVDCluster> relatVectorTHToClus = DataStore::getRelationsWithObj<SVDCluster>(&trhi);

    //efficiencies TH to cluster
    m_NumberOfTH[indexForHistosAndGraphs] ++; //U
    m_NumberOfTH[indexForHistosAndGraphs + 1] ++; //V
    for (int j = 0; j < (int) relatVectorTHToClus.size(); j ++) {
      indexForHistosAndGraphs = indexFromLayerSensorSide(relatVectorTHToClus[j]->getSensorID().getLayerNumber() ,
                                                         relatVectorTHToClus[j]->getSensorID().getSensorNumber() , relatVectorTHToClus[j]->isUCluster());
      m_NumberOfClustersRelatedToTH[indexForHistosAndGraphs] ++;
    }
  }
  //close loop on TrueHits

  //loop on Clusters
  for (const SVDCluster& clus : SVDClusters) {
    indexForHistosAndGraphs = indexFromLayerSensorSide(clus.getSensorID().getLayerNumber() , clus.getSensorID().getSensorNumber() ,
                                                       clus.isUCluster());

    RelationVector<SVDTrueHit> relatVectorClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(&clus);

    //purity "outside" clusters
    m_NumberOfClusters[indexForHistosAndGraphs] ++;
    if (relatVectorClusToTH.size() > 0)
      m_NumberOfTMClusters[indexForHistosAndGraphs] ++;

    //fill the puddlyness histo with the number of TH a cluster is composed of
    m_histo_Puddlyness[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());

    //loop on the TH related to the cluster
    for (int q = 0; q < (int)relatVectorClusToTH.size(); q ++) {
      //cluster time resolution
      m_histo_ClusterTimeResolution[indexForHistosAndGraphs]->Fill(clus.getClsTime() - (relatVectorClusToTH[q])->getGlobalTime());

      //cluster position resolution
      if (clus.isUCluster()) {
        m_histo_ClusterUPositionResolution[indexForHistosAndGraphs / 2]->Fill(clus.getPosition((relatVectorClusToTH[q])->getV()) -
            (relatVectorClusToTH[q])->getU());
      } else {
        m_histo_ClusterVPositionResolution[(indexForHistosAndGraphs - 1) / 2]->Fill(clus.getPosition() -
            (relatVectorClusToTH[q])->getV());
      }
    }

    RelationVector<SVDRecoDigit> relatVectorClusToReco = DataStore::getRelationsWithObj<SVDRecoDigit>(&clus);
    //enter only if the cluster is TM
    if (relatVectorClusToTH.size() > 0) {

      //fill the puddlyness histo with the number of TH a TM cluster is composed of
      m_histo_PuddlynessTM[indexForHistosAndGraphs]->Fill(relatVectorClusToTH.size());

      //count number of recodigit, composing the Truth-matched cluster, that are linked with a TH (internal purity)
      m_NumberOfTMRecoInTMCluster = 0;
      for (int k = 0; k < (int)relatVectorClusToReco.size(); k++) { //loop on the recodigits composing the TM cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(relatVectorClusToReco[k]);

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInTMCluster ++;
      }

      m_histo_PurityInsideTMCluster[indexForHistosAndGraphs]->Fill((float)m_NumberOfTMRecoInTMCluster / (float)(clus.getSize()));
      m_histo2D_PurityInsideTMCluster[indexForHistosAndGraphs]->Fill(clus.getSize(), m_NumberOfTMRecoInTMCluster);

    }
    //count number of recodigit, composing a NOT Truth-matched cluster, that are linked with a TH
    else {

      m_NumberOfTMRecoInNOTMCluster = 0;
      for (int k = 0; k < (int)relatVectorClusToReco.size(); k++) { //loop on the recodigits composing the NOTM cluster
        RelationVector<SVDTrueHit> relatVectorRecoFromClusToTH = DataStore::getRelationsWithObj<SVDTrueHit>(relatVectorClusToReco[k]);

        if (relatVectorRecoFromClusToTH.size() > 0)
          m_NumberOfTMRecoInNOTMCluster ++;
      }

      m_histo_PurityInsideNOTMCluster[indexForHistosAndGraphs]->Fill((float)m_NumberOfTMRecoInNOTMCluster / (float)(clus.getSize()));

    }
  }
  //close loop on clusters

}


void SVDClusterEvaluationModule::endRun()
{

  //extract mean and sigma values from histos to plot them in graphs
  for (int k = 0; k < m_Nsets; k ++) {
    m_mean_StripTimeResolution[k] = m_histo_StripTimeResolution[k]->GetMean();
    m_RMS_StripTimeResolution[k] = m_histo_StripTimeResolution[k]->GetRMS();

    m_mean_ClusterTimeResolution[k] = m_histo_ClusterTimeResolution[k]->GetMean();
    m_RMS_ClusterTimeResolution[k] = m_histo_ClusterTimeResolution[k]->GetRMS();

    m_mean_PurityInsideTMCluster[k] = m_histo_PurityInsideTMCluster[k]->GetMean();
    m_RMS_PurityInsideTMCluster[k] = m_histo_PurityInsideTMCluster[k]->GetRMS();

    m_mean_Puddlyness[k] = m_histo_Puddlyness[k]->GetMean();
    m_RMS_Puddlyness[k] = m_histo_Puddlyness[k]->GetRMS();

    m_mean_PuddlynessTM[k] = m_histo_PuddlynessTM[k]->GetMean();
    m_RMS_PuddlynessTM[k] = m_histo_PuddlynessTM[k]->GetRMS();
  }
  for (int k = 0; k < m_NsetsRed; k ++) {
    m_mean_ClusterUPositionResolution[k] = m_histo_ClusterUPositionResolution[k]->GetMean();
    m_RMS_ClusterUPositionResolution[k] = m_histo_ClusterUPositionResolution[k]->GetRMS();

    m_mean_ClusterVPositionResolution[k] = m_histo_ClusterVPositionResolution[k]->GetMean();
    m_RMS_ClusterVPositionResolution[k] = m_histo_ClusterVPositionResolution[k]->GetRMS();
  }

  //GRAPHS
  createEfficiencyGraph("recoEff", "Efficiency of Recoing ( RecoDigits / ShaperDigits )", m_NumberOfRecoDigit, m_NumberOfShaperDigit,
                        "set", "efficiency", m_graphList);

  createEfficiencyGraph("clusterEff", "Efficiency of Clustering ( TrueHits / Truth-Matched Clusters )", m_NumberOfClustersRelatedToTH,
                        m_NumberOfTH, "set", "efficiency", m_graphList);

  createEfficiencyGraph("clusterPurity", "Purity of Clusters ( Truth-Matched Clusters / All Clusters )", m_NumberOfTMClusters,
                        m_NumberOfClusters, "set", "purity", m_graphList);

  //means-from-histos graphs
  createArbitraryGraphErrorChooser("stripTime_Means", "Strip Time Resolution", m_OrderingVec, m_NullVec, m_mean_StripTimeResolution,
                                   m_RMS_StripTimeResolution, "set", "mean t_Reconstructed - t_TrueHit", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("clusterTime_Means", "Cluster Time Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterTimeResolution, m_RMS_ClusterTimeResolution, "set", "mean t_Reconstructed - t_TrueHit", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("clusterUposition_Means", "Cluster U Position Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterUPositionResolution, m_RMS_ClusterUPositionResolution, "set", "mean U_Reconstructed - U_TrueHit", m_graphList,
                                   m_NsetsRed);

  createArbitraryGraphErrorChooser("clusterVposition_Means", "Cluster V Position Resolution", m_OrderingVec, m_NullVec,
                                   m_mean_ClusterVPositionResolution, m_RMS_ClusterVPositionResolution, "set", "mean V_Reconstructed - V_TrueHit", m_graphList,
                                   m_NsetsRed);

  createArbitraryGraphErrorChooser("clusterInternalPurity_Means", "Fraction of Truth-matched Recos inside a Truth-matched Cluster",
                                   m_OrderingVec, m_NullVec, m_mean_PurityInsideTMCluster, m_RMS_PurityInsideTMCluster, "set",
                                   "mean number of TM recos / cluster size", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("puddlyness_Means", "Number of True Hits inside a Cluster", m_OrderingVec, m_NullVec,
                                   m_mean_Puddlyness, m_RMS_Puddlyness, "set", "mean number of TH per cluster", m_graphList, m_Nsets);

  createArbitraryGraphErrorChooser("puddlynessTM_Means", "Number of True Hits inside a TM Cluster", m_OrderingVec, m_NullVec,
                                   m_mean_PuddlynessTM, m_RMS_PuddlynessTM, "set", "mean number of TH per TM cluster", m_graphList, m_Nsets);

  ///////////////////////////
  //WRITE HISTOS AND GRAPHS//
  ///////////////////////////

  if (m_outputFile != NULL) {
    m_outputFile->cd();

    TDirectory* oldDir = gDirectory;
    TObject* obj;

    TDirectory* dir_strtime = oldDir->mkdir("strip_time");
    dir_strtime->cd();
    TIter nextH_strtime(m_histoList_StripTimeResolution);
    while ((obj = nextH_strtime()))
      obj->Write();

    TDirectory* dir_cltime = oldDir->mkdir("cluster_time");
    dir_cltime->cd();
    TIter nextH_cltime(m_histoList_ClusterTimeResolution);
    while ((obj = nextH_cltime()))
      obj->Write();

    TDirectory* dir_clpos = oldDir->mkdir("cluster_position");
    dir_clpos->cd();
    TIter nextH_clpos(m_histoList_ClusterPositionResolution);
    while ((obj = nextH_clpos()))
      obj->Write();

    TDirectory* dir_clinpurTM = oldDir->mkdir("intra_cluster_purity_TM");
    dir_clinpurTM->cd();
    TIter nextH_clinpurTM(m_histoList_PurityInsideTMCluster);
    while ((obj = nextH_clinpurTM()))
      obj->Write();

    TDirectory* dir_clinpurTM2D = oldDir->mkdir("intra_cluster_purity_TM2D");
    dir_clinpurTM2D->cd();
    TIter nextH_clinpurTM2D(m_histo2DList_PurityInsideTMCluster);
    while ((obj = nextH_clinpurTM2D()))
      obj->Write();

    TDirectory* dir_clinpurNOTM = oldDir->mkdir("intra_cluster_purity_NOTM");
    dir_clinpurNOTM->cd();
    TIter nextH_clinpurNOTM(m_histoList_PurityInsideNOTMCluster);
    while ((obj = nextH_clinpurNOTM()))
      obj->Write();

    TDirectory* dir_puddle = oldDir->mkdir("trueHits_in_cluster");
    dir_puddle->cd();
    TIter nextH_puddle(m_histoList_Puddlyness);
    while ((obj = nextH_puddle()))
      obj->Write();

    TDirectory* dir_puddleTM = oldDir->mkdir("trueHits_in_TMcluster");
    dir_puddleTM->cd();
    TIter nextH_puddleTM(m_histoList_PuddlynessTM);
    while ((obj = nextH_puddleTM()))
      obj->Write();

    TDirectory* dir_graph = oldDir->mkdir("graphs");
    dir_graph->cd();
    TIter nextH_graph(m_graphList);
    while ((obj = nextH_graph()))
      obj->Write();


    m_outputFile->Close();
  }
}


void SVDClusterEvaluationModule::terminate()
{
}


///////////////////
//EXTRA FUNCTIONS//
///////////////////

TH1F* SVDClusterEvaluationModule::createHistogram1D(const char* name, const char* title,
                                                    Int_t nbins, Double_t min, Double_t max,
                                                    const char* xtitle, TList* histoList)
{
  TH1F* h = new TH1F(name, title, nbins, min, max);

  h->GetXaxis()->SetTitle(xtitle);

  if (histoList)
    histoList->Add(h);

  return h;
}

TH2F* SVDClusterEvaluationModule::createHistogram2D(const char* name, const char* title,
                                                    Int_t nbinsX, Double_t minX, Double_t maxX,
                                                    const char* titleX,
                                                    Int_t nbinsY, Double_t minY, Double_t maxY,
                                                    const char* titleY, TList* histoList)
{

  TH2F* h = new TH2F(name, title, nbinsX, minX, maxX, nbinsY, minY, maxY);

  h->GetXaxis()->SetTitle(titleX);
  h->GetYaxis()->SetTitle(titleY);

  if (histoList)
    histoList->Add(h);

  return h;
}

int SVDClusterEvaluationModule::indexFromLayerSensorSide(int LayerNumber, int SensorNumber, int UVNumber)
{
  int Index;

  if (LayerNumber == 3) { //L3
    if (UVNumber) //U
      Index = 0;
    else //V
      Index = 1;
  } else { //L456
    if (SensorNumber == 1) { //FW
      if (UVNumber) //U
        Index = 2;
      else //V
        Index = 3;
    } else { //barrel
      if (UVNumber) //U
        Index = 4;
      else //V
        Index = 5;
    }
  }

  return Index;
}

TString SVDClusterEvaluationModule::IntExtFromIndex(int idx)
{
  TString name = "";

  if (idx < 2)
    name = "L3";
  else
    name = "L456";

  return name;
}

TString SVDClusterEvaluationModule::FWFromIndex(int idx)
{
  TString name = "";

  if (idx == 2 || idx == 3)
    name = "FWD";
  else
    name = "Barrel";

  return name;
}

TString SVDClusterEvaluationModule::UVFromIndex(int idx)
{
  TString name = "";

  if (idx % 2 == 0)
    name = "U";
  else
    name = "V";

  return name;
}

void SVDClusterEvaluationModule::createEfficiencyGraph(const char* name, const char* title, int vNum[m_Nsets], int vDen[m_Nsets],
                                                       TString xTitle, TString yTitle, TList* list)
{

  float ratio[m_Nsets];
  float ratioErr[m_Nsets];
  float x[m_Nsets];
  float xErr[m_Nsets];

  for (int set = 0; set < m_Nsets; set++) {

    x[set] = set + 1;
    xErr[set] = 0;

    if (vDen[set] > 0) {
      ratio[set] = (float)vNum[set] / (float)vDen[set];
      ratioErr[set] = sqrt(ratio[set] * (1 - ratio[set]) / (float)vDen[set]);
    }

  }

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_Nsets, x, ratio, xErr, ratioErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->GetYaxis()->SetRangeUser(0.00001, 1.10);
  g->Draw();
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_Nsets] = {"3U", "3V", "456FU", "456FV", "456BU", "456BV"};
  for (Int_t i = 0; i < m_Nsets; i++) {
    //xAxis->SetBinLabel(i + 1, labels[i].Data());
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

void SVDClusterEvaluationModule::createArbitraryGraphErrorChooser(const char* name, const char* title, float x[m_Nsets],
    float xErr[m_Nsets], float y[m_Nsets], float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list, int len)
{
  if (len == m_NsetsRed)
    createArbitraryGraphError_Red(name, title, x, xErr, y, yErr, xTitle, yTitle, list);
  else if (len == m_Nsets)
    createArbitraryGraphError_Std(name, title, x, xErr, y, yErr, xTitle, yTitle, list);
  else
    B2INFO("ERROR, WRONG LENGTH FOR MEANS TGRAPH CREATION!!!");
}

void SVDClusterEvaluationModule::createArbitraryGraphError_Std(const char* name, const char* title, float x[m_Nsets],
    float xErr[m_Nsets], float y[m_Nsets], float yErr[m_Nsets], TString xTitle, TString yTitle, TList* list)
{

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_Nsets, x, y, xErr, yErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->Draw();
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_Nsets] = {"3U", "3V", "456FU", "456FV", "456BU", "456BV"};
  for (Int_t i = 0; i < m_Nsets; i++) {
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

void SVDClusterEvaluationModule::createArbitraryGraphError_Red(const char* name, const char* title, float x[m_NsetsRed],
    float xErr[m_NsetsRed], float y[m_NsetsRed], float yErr[m_NsetsRed], TString xTitle, TString yTitle, TList* list)
{

  TCanvas* c = new TCanvas(name, title);
  TGraphErrors* g = new TGraphErrors(m_NsetsRed, x, y, xErr, yErr);
  g->SetName(name);
  g->SetTitle(title);
  g->GetXaxis()->SetTitle(xTitle.Data());
  g->GetYaxis()->SetTitle(yTitle.Data());
  g->Draw();
  TAxis* xAxis = g->GetXaxis();

  TText* t = new TText();
  t->SetTextAlign(32);
  t->SetTextSize(0.035);
  t->SetTextFont(72);
  TString labels[m_NsetsRed] = {"3", "456F", "456B"};
  for (Int_t i = 0; i < m_NsetsRed; i++) {
    xAxis->SetBinLabel(xAxis->FindBin(i + 1) , labels[i].Data());
  }

  if (list)
    list->Add(c);

}

/*float SVDClusterEvaluationModule::getMeanFromHistoWithoutABin(TH1F* histo, int BadBin)
{
  float almostMean = 0, almostEntries = 0, numer = 0;
  float binval, bincont;
  int NumberOfBin;

  NumberOfBin = histo->GetSize();
  NumberOfBin = NumberOfBin - 2;
  B2INFO ("NumberOfBin = " << NumberOfBin);
  if (NumberOfBin >= 1)
  {
    for (int k = 1; k <= NumberOfBin; k ++)
    {
      if (k != BadBin)
      {
        bincont = histo->GetBinContent(k);
        almostEntries += bincont;
        binval = histo->GetBinLowEdge();
        numer += bincont * binval;
      }
    }
  }

  almostMean = numer / almostEntries;

  return almostMean;
}*/























