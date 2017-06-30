/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "pxd/modules/pxdDQM/PXDDQMExpressRecoModule.h"

#include <framework/core/HistoModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDFrame.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>

//#include <set>
#include <boost/format.hpp>

#include "TVector3.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TVectorD.h"
#include "TF1.h"
#include "TRandom3.h"

using namespace std;
using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDQMExpressReco)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDQMExpressRecoModule::PXDDQMExpressRecoModule() : HistoModule()
{
  //Set module properties
  setDescription("PXD DQM module for ExpressReco");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CutPXDCharge", m_CutPXDCharge,
           "cut for accepting to hitmap histogram, using strips only, default = 0.0 ", m_CutPXDCharge);
  addParam("ReferenceHistosFileName", m_RefHistFileName,
           "Name of file contain reference histograms, default=PXD-ReferenceHistos", m_RefHistFileName);

}


PXDDQMExpressRecoModule::~PXDDQMExpressRecoModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void PXDDQMExpressRecoModule::defineHisto()
{
  // Create a separate histogram directories and cd into it.
  m_oldDir = gDirectory;

  // basic constants presets:
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  c_nVXDLayers = geo.getLayers().size();
  c_firstVXDLayer = 1;  // counting start from 1...
  c_lastVXDLayer = c_nVXDLayers;
  c_nPXDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::PXD).size();
  c_firstPXDLayer = c_firstVXDLayer;
  c_lastPXDLayer = c_nPXDLayers;
  c_nSVDLayers = geo.getLayers(VXD::SensorInfoBase::SensorType::SVD).size();
  c_firstSVDLayer = c_nPXDLayers + c_firstPXDLayer;
  c_lastSVDLayer = c_firstSVDLayer + c_nSVDLayers;

  c_nPXDSensors = 0;
  for (VxdID layer : geo.getLayers()) {
    for (VxdID ladder : geo.getLadders(layer)) {
      if (layer.getLayerNumber() <= c_lastPXDLayer) {  // PXD
        c_nPXDSensors += geo.getLadders(layer).size() * geo.getSensors(ladder).size();
      }
      break;
    }
  }

  TDirectory* DirPXDBasic = NULL;
  TDirectory* DirPXDFlags = NULL;
  DirPXDFlags = m_oldDir->mkdir("PXDExpReco_Flags");
  DirPXDBasic = m_oldDir->mkdir("PXDExpReco");

  // Create basic histograms:
  DirPXDBasic->cd();
  m_hitMapCounts = new TH1I("PixelHitmapCounts", "PXD Pixel Hitmaps Counts",
                            c_nPXDSensors, 0, c_nPXDSensors);
  m_hitMapCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapCounts->GetYaxis()->SetTitle("counts");
  m_hitMapClCounts = new TH1I("ClusterHitmapCounts", "PXD Cluster Hitmaps Counts",
                              c_nPXDSensors, 0, c_nPXDSensors);
  m_hitMapClCounts->GetXaxis()->SetTitle("Sensor ID");
  m_hitMapClCounts->GetYaxis()->SetTitle("counts");
  m_fired = new TH1F*[c_nPXDSensors];
  m_clusters = new TH1F*[c_nPXDSensors];
  m_startRow = new TH1F*[c_nPXDSensors];
  m_chargStartRow = new TH1F*[c_nPXDSensors];
  m_startRowCount = new TH1F*[c_nPXDSensors];
  m_clusterCharge = new TH1F*[c_nPXDSensors];
  m_pixelSignal = new TH1F*[c_nPXDSensors];
  m_clusterSizeU = new TH1F*[c_nPXDSensors];
  m_clusterSizeV = new TH1F*[c_nPXDSensors];
  m_clusterSizeUV = new TH1F*[c_nPXDSensors];
  for (int i = 0; i < c_nPXDSensors; i++) {
    DirPXDBasic->cd();
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
    //----------------------------------------------------------------
    // Number of fired pixels per frame
    //----------------------------------------------------------------
    string name = str(format("PXD_%1%_Fired") % sensorDescr);
    string title = str(format("PXD Sensor %1% Fired pixels") % sensorDescr);
    m_fired[i] = NULL;
    m_fired[i] = new TH1F(name.c_str(), title.c_str(), 50, 0, 50);
    m_fired[i]->GetXaxis()->SetTitle("# of fired u pixels");
    m_fired[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Number of clusters per frame
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_Clusters") % sensorDescr);
    title = str(format("PXD Sensor %1% Number of clusters") % sensorDescr);
    m_clusters[i] = NULL;
    m_clusters[i] = new TH1F(name.c_str(), title.c_str(), 20, 0, 20);
    m_clusters[i]->GetXaxis()->SetTitle("# of u clusters");
    m_clusters[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Start row distribution
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_StartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Start row distribution") % sensorDescr);
    m_nPixels = SensorInfo.getVCells();
    m_startRow[i] = new TH1F(name.c_str(), title.c_str(), m_nPixels / 4, 0.0, m_nPixels);
    m_startRow[i]->GetXaxis()->SetTitle("start row [pitch units]");
    m_startRow[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster seed charge by distance from the start row
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_AverageSeedByStartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Average seed charge by distance from the start row") % sensorDescr);
    m_chargStartRow[i] = new TH1F(name.c_str(), title.c_str(), m_nPixels / 4, 0.0, m_nPixels);
    m_chargStartRow[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_chargStartRow[i]->GetYaxis()->SetTitle("average seed [ADU]");
    name = str(format("PXD_%1%_SeedCountsByStartRow") % sensorDescr);
    title = str(format("PXD Sensor %1% Seed charge count by distance from the start row") % sensorDescr);
    m_startRowCount[i] = new TH1F(name.c_str(), title.c_str(), m_nPixels / 4, 0.0, m_nPixels);
    m_startRowCount[i]->GetXaxis()->SetTitle("distance from the start row [pitch units]");
    m_startRowCount[i]->GetYaxis()->SetTitle("count");
    //----------------------------------------------------------------
    // Cluster Charge
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_ClusterCharge") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Charge") % sensorDescr);
    m_clusterCharge[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_clusterCharge[i]->GetXaxis()->SetTitle("charge of clusters [ADU]");
    m_clusterCharge[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Pixel Signal
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_PixelSignal") % sensorDescr);
    title = str(format("PXD Sensor %1% Pixel Signal") % sensorDescr);
    m_pixelSignal[i] = new TH1F(name.c_str(), title.c_str(), 256, 0, 256);
    m_pixelSignal[i]->GetXaxis()->SetTitle("signal of pixels [ADU]");
    m_pixelSignal[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_ClusterSizeU") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U") % sensorDescr);
    m_clusterSizeU[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeU[i]->GetXaxis()->SetTitle("size of u clusters");
    m_clusterSizeU[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in V
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_ClusterSizeV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size V") % sensorDescr);
    m_clusterSizeV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeV[i]->GetXaxis()->SetTitle("size of v clusters");
    m_clusterSizeV[i]->GetYaxis()->SetTitle("counts");
    //----------------------------------------------------------------
    // Cluster Size in U+V
    //----------------------------------------------------------------
    name = str(format("PXD_%1%_ClusterSizeUV") % sensorDescr);
    title = str(format("PXD Sensor %1% Cluster Size U+V") % sensorDescr);
    m_clusterSizeUV[i] = new TH1F(name.c_str(), title.c_str(), 10, 0, 10);
    m_clusterSizeUV[i]->GetXaxis()->SetTitle("size of u+v clusters");
    m_clusterSizeUV[i]->GetYaxis()->SetTitle("counts");
  }

  // Create flag histograms:
  DirPXDFlags->cd();
  m_fFiredFlag = new TH1I("FiredFlag", "PXD Fired Flag",
                          c_nPXDSensors, 0, c_nPXDSensors);
  m_fFiredFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fFiredFlag->GetYaxis()->SetTitle("flag");
  m_fClustersFlag = new TH1I("ClustersFlag", "PXD Clusters Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fClustersFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClustersFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowFlag = new TH1I("StartRowFlag", "PXD Start Row Flag",
                             c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fChargStartRowFlag = new TH1I("ChargStartRowFlag", "PXD Charg Start Row Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fChargStartRowFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fChargStartRowFlag->GetYaxis()->SetTitle("flag");
  m_fStartRowCountFlag = new TH1I("StartRowCountFlag", "PXD Row Count Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fStartRowCountFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fStartRowCountFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapCountsFlag = new TH1I("PixelHitmapCountsFlag", "PXD Pixel Hitmaps Counts Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fHitMapCountsFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapCountsFlag->GetYaxis()->SetTitle("flag");
  m_fHitMapClCountsFlag = new TH1I("ClusterHitmapCountsFlag", "PXD Cluster Hitmaps Counts Flag",
                                   c_nPXDSensors, 0, c_nPXDSensors);
  m_fHitMapClCountsFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fHitMapClCountsFlag->GetYaxis()->SetTitle("flag");
  m_fClusterChargeFlag = new TH1I("ClusterChargeFlag", "PXD Cluster Charge Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterChargeFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterChargeFlag->GetYaxis()->SetTitle("flag");
  m_fPixelSignalFlag = new TH1I("PixelSignalFlag", "PXD Pixel Signal Flag",
                                c_nPXDSensors, 0, c_nPXDSensors);
  m_fPixelSignalFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fPixelSignalFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUFlag = new TH1I("ClasterSizeUFlag", "PXD Cluster Size U Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeVFlag = new TH1I("ClasterSizeVFlag", "PXD Cluster Size V Flag",
                                 c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeVFlag->GetYaxis()->SetTitle("flag");
  m_fClusterSizeUVFlag = new TH1I("ClasterSizeUVFlag", "PXD Cluster Size UV Flag",
                                  c_nPXDSensors, 0, c_nPXDSensors);
  m_fClusterSizeUVFlag->GetXaxis()->SetTitle("Sensor ID");
  m_fClusterSizeUVFlag->GetYaxis()->SetTitle("flag");

  for (int i = 0; i < c_nPXDSensors; i++) {
    int iLayer = 0;
    int iLadder = 0;
    int iSensor = 0;
    getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
    TString AxisTicks = Form("%i_%i_%i", iLayer, iLadder, iSensor);
    m_hitMapCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_hitMapClCounts->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fFiredFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClustersFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fChargStartRowFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fStartRowCountFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fHitMapClCountsFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterChargeFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fPixelSignalFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
    m_fClusterSizeUVFlag->GetXaxis()->SetBinLabel(i + 1, AxisTicks.Data());
  }
  m_oldDir->cd();
}


void PXDDQMExpressRecoModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM

  //Register collections
  StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits);
  m_storePXDClustersName = storePXDClusters.getName();
  m_relPXDClusterDigitName = relPXDClusterDigits.getName();

  //Store names to speed up creation later
  m_storePXDDigitsName = storePXDDigits.getName();

  StoreArray<PXDFrame> storeFrames(m_storeFramesName);
  m_storeFramesName = storeFrames.getName();

}

void PXDDQMExpressRecoModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (m_fHitMapCountsFlag != NULL) m_fHitMapCountsFlag->Reset();
  if (m_fHitMapClCountsFlag != NULL) m_fHitMapClCountsFlag->Reset();
  if (m_fFiredFlag != NULL) m_fFiredFlag->Reset();
  if (m_fClustersFlag != NULL) m_fClustersFlag->Reset();
  if (m_fStartRowFlag != NULL) m_fStartRowFlag->Reset();
  if (m_fChargStartRowFlag != NULL) m_fChargStartRowFlag->Reset();
  if (m_fStartRowCountFlag != NULL) m_fStartRowCountFlag->Reset();
  if (m_fClusterChargeFlag != NULL) m_fClusterChargeFlag->Reset();
  if (m_fPixelSignalFlag != NULL) m_fPixelSignalFlag->Reset();
  if (m_fClusterSizeUFlag != NULL) m_fClusterSizeUFlag->Reset();
  if (m_fClusterSizeVFlag != NULL) m_fClusterSizeVFlag->Reset();
  if (m_fClusterSizeUVFlag != NULL) m_fClusterSizeUVFlag->Reset();

  if (m_hitMapCounts != NULL) m_hitMapCounts->Reset();
  if (m_hitMapClCounts != NULL) m_hitMapClCounts->Reset();

  for (int i = 0; i < c_nPXDSensors; i++) {
    if (m_fired[i] != NULL) m_fired[i]->Reset();
    if (m_clusters[i] != NULL) m_clusters[i]->Reset();
    if (m_startRow[i] != NULL) m_startRow[i]->Reset();
    if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Reset();
    if (m_startRowCount[i] != NULL) m_startRowCount[i]->Reset();
    if (m_clusterCharge[i] != NULL) m_clusterCharge[i]->Reset();
    if (m_pixelSignal[i] != NULL) m_pixelSignal[i]->Reset();
    if (m_clusterSizeU[i] != NULL) m_clusterSizeU[i]->Reset();
    if (m_clusterSizeV[i] != NULL) m_clusterSizeV[i]->Reset();
    if (m_clusterSizeUV[i] != NULL) m_clusterSizeUV[i]->Reset();
  }

  m_NoOfEvents = 0;
}


void PXDDQMExpressRecoModule::event()
{
  const StoreArray<PXDDigit> storePXDDigits(m_storePXDDigitsName);
  const StoreArray<PXDCluster> storePXDClusters(m_storePXDClustersName);
  const RelationArray relPXDClusterDigits(storePXDClusters, storePXDDigits, m_relPXDClusterDigitName);
  const StoreArray<PXDFrame> storeFrames(m_storeFramesName);

  m_NoOfEvents++;

  // If there are no digits, leave
  if (!storePXDDigits || !storePXDDigits.getEntries()) return;

  // PXD basic histograms:
  // Fired strips
  vector< set<int> > Pixels(c_nPXDSensors); // sets to eliminate multiple samples per strip
  for (const PXDDigit& digit : storePXDDigits) {
    int iLayer = digit.getSensorID().getLayerNumber();
    if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
    int iLadder = digit.getSensorID().getLadderNumber();
    int iSensor = digit.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    Pixels.at(index).insert(digit.getUniqueChannelID());
    if (m_pixelSignal[index] != NULL) m_pixelSignal[index]->Fill(digit.getCharge());
    if ((m_hitMapCounts != NULL) && (digit.getCharge() > m_CutPXDCharge))
      m_hitMapCounts->Fill(index);
  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    if ((m_fired[i] != NULL) && (Pixels[i].size() > 0)) m_fired[i]->Fill(Pixels[i].size());
  }

  vector< set<int> > counts(c_nPXDSensors);
  // Hitmaps, Charge, Seed, Size, Time, ...
  for (const PXDCluster& cluster : storePXDClusters) {
    int iLayer = cluster.getSensorID().getLayerNumber();
    if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
    int iLadder = cluster.getSensorID().getLadderNumber();
    int iSensor = cluster.getSensorID().getSensorNumber();
    int index = getSensorIndex(iLayer, iLadder, iSensor);
    VxdID sensorID(iLayer, iLadder, iSensor);
    PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    counts.at(index).insert(cluster.GetUniqueID());
    if (m_hitMapClCounts != NULL) m_hitMapClCounts->Fill(index);
    if (m_clusterCharge[index] != NULL) m_clusterCharge[index]->Fill(cluster.getCharge());
    if (m_clusterSizeU[index] != NULL) m_clusterSizeU[index]->Fill(cluster.getUSize());
    if (m_clusterSizeV[index] != NULL) m_clusterSizeV[index]->Fill(cluster.getVSize());
    if (m_clusterSizeUV[index] != NULL) m_clusterSizeUV[index]->Fill(cluster.getSize());
  }
  for (int i = 0; i < c_nPXDSensors; i++) {
    if ((m_clusters[i] != NULL) && (counts[i].size() > 0))
      m_clusters[i]->Fill(counts[i].size());
  }
  if (storeFrames && storeFrames.getEntries()) {
    // Start rows
    for (const PXDFrame& frame : storeFrames) {
      int iLayer = frame.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = frame.getSensorID().getLadderNumber();
      int iSensor = frame.getSensorID().getSensorNumber();
      int index = getSensorIndex(iLayer, iLadder, iSensor);
      if (m_startRow[index] != NULL) m_startRow[index]->Fill(frame.getStartRow());
    }
    // Cluster seed charge by start row
    std::map<VxdID, unsigned short> startRows;
    for (auto frame : storeFrames)
      startRows.insert(std::make_pair(frame.getSensorID(), frame.getStartRow()));
    for (auto cluster : storePXDClusters) {
      int iLayer = cluster.getSensorID().getLayerNumber();
      if ((iLayer < c_firstPXDLayer) || (iLayer > c_lastPXDLayer)) continue;
      int iLadder = cluster.getSensorID().getLadderNumber();
      int iSensor = cluster.getSensorID().getSensorNumber();
      int index = getSensorIndex(iLayer, iLadder, iSensor);
      VxdID sensorID(iLayer, iLadder, iSensor);
      PXD::SensorInfo SensorInfo = dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));

      float fDistance = SensorInfo.getVCellID(cluster.getV()) - startRows[cluster.getSensorID()];
      if (fDistance < 0) fDistance += SensorInfo.getVCells();
      if (m_chargStartRow[index] != NULL) m_chargStartRow[index]->Fill(fDistance, cluster.getSeedCharge());
      if (m_startRowCount[index] != NULL) m_startRowCount[index]->Fill(fDistance);
    }
  }
}


void PXDDQMExpressRecoModule::endRun()
{
  // Make average value on histogram
  for (int i = 0; i < c_nPXDSensors; i++) {
//   if (m_chargStartRow[i] != NULL) m_chargStartRow[i]->Divide(m_StartRowCount[i]);
    //m_averageSeedByU[i]->Divide(m_seedCountsByU[i]);
    //m_averageSeedByV[i]->Divide(m_seedCountsByV[i]);
  }
  TVectorD* NoOfEvents;
  NoOfEvents = new TVectorD(1);
  double fNoOfEvents[1];
  fNoOfEvents[0] = m_NoOfEvents;
  NoOfEvents->SetElements(fNoOfEvents);
  TString nameBS = Form("NoOfEvents");
  m_oldDir->cd();
  NoOfEvents->Write(nameBS.Data());

  // Load reference file of histograms:
  TH1I* r_hitMapCounts;
  TH1I* r_hitMapClCounts;
  TH1F** r_fired = new TH1F*[c_nPXDSensors];
  TH1F** r_clusters = new TH1F*[c_nPXDSensors];
  TH1F** r_startRow = new TH1F*[c_nPXDSensors];
  TH1F** r_chargStartRow = new TH1F*[c_nPXDSensors];
  TH1F** r_startRowCount = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterCharge = new TH1F*[c_nPXDSensors];
  TH1F** r_pixelSignal = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeU = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeV = new TH1F*[c_nPXDSensors];
  TH1F** r_clusterSizeUV = new TH1F*[c_nPXDSensors];

  TFile* f_RefHistFile = new TFile(m_RefHistFileName.c_str(), "read");
  if (f_RefHistFile->IsOpen()) {
    B2INFO("Reference file name: " << m_RefHistFileName.c_str());
    TVectorD* NoOfEventsRef = NULL;
    f_RefHistFile->GetObject("NoOfEvents", NoOfEventsRef);
    m_NoOfEventsRef = (int)NoOfEventsRef->GetMatrixArray()[0];
//    m_NoOfEventsRef = 2;
    string name = str(format("PXDExpReco/PixelHitmapCounts;1"));
    f_RefHistFile->GetObject(name.c_str(), r_hitMapCounts);
    if (r_hitMapCounts == NULL) {
      B2INFO("There is missing histogram in reference file: " << name.c_str());
      return;
    }
    name = str(format("PXDExpReco/ClusterHitmapCounts;1"));
    f_RefHistFile->GetObject(name.c_str(), r_hitMapClCounts);
    if (r_hitMapClCounts == NULL) {
      B2INFO("There is missing histogram in reference file: " << name.c_str());
      return;
    }
    for (int i = 0; i < c_nPXDSensors; i++) {
      int iLayer = 0;
      int iLadder = 0;
      int iSensor = 0;
      getIDsFromIndex(i, &iLayer, &iLadder, &iSensor);
      string sensorDescr = str(format("%1%_%2%_%3%") % iLayer % iLadder % iSensor);
      name = str(format("PXDExpReco/PXD_%1%_Fired") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_fired[i]);
      if (r_fired[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_Clusters") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_clusters[i]);
      if (r_clusters[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_StartRow") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_startRow[i]);
      if (r_startRow[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_AverageSeedByStartRow") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_chargStartRow[i]);
      if (r_chargStartRow[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_SeedCountsByStartRow") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_startRowCount[i]);
      if (r_startRowCount[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_ClusterCharge") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_clusterCharge[i]);
      if (r_clusterCharge[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_PixelSignal") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_pixelSignal[i]);
      if (r_pixelSignal[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_ClusterSizeU") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_clusterSizeU[i]);
      if (r_clusterSizeU[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_ClusterSizeV") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_clusterSizeV[i]);
      if (r_clusterSizeV[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
      name = str(format("PXDExpReco/PXD_%1%_ClusterSizeUV") % sensorDescr);
      f_RefHistFile->GetObject(name.c_str(), r_clusterSizeUV[i]);
      if (r_clusterSizeUV[i] == NULL) {
        B2INFO("There is missing histogram in reference file: " << name.c_str());
        return;
      }
    }
    // f_RefHistFile->Close();
  } else {
    B2INFO("File of reference histograms: " << m_RefHistFileName.c_str() << " is not available, please check it!");
    return;
  }

  // Compare histograms with reference histograms and create flags:
  for (int i = 0; i < c_nPXDSensors; i++) {
    double pars[2];
    pars[0] = 0.01;
    pars[1] = 0.05;
    SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_hitMapCounts, r_hitMapCounts, m_fHitMapCountsFlag);
    SetFlag(9, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_hitMapClCounts, r_hitMapClCounts, m_fHitMapClCountsFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_fired[i], r_fired[i], m_fFiredFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusters[i], r_clusters[i], m_fClustersFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_startRow[i], r_startRow[i], m_fStartRowFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_chargStartRow[i], r_chargStartRow[i], m_fChargStartRowFlag);
    SetFlag(100, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_startRowCount[i], r_startRowCount[i], m_fStartRowCountFlag);
    SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterCharge[i], r_clusterCharge[i], m_fClusterChargeFlag);
    SetFlag(5, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_pixelSignal[i], r_pixelSignal[i], m_fPixelSignalFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeU[i], r_clusterSizeU[i], m_fClusterSizeUFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeV[i], r_clusterSizeV[i], m_fClusterSizeVFlag);
    SetFlag(2, i, pars, (double)m_NoOfEvents / m_NoOfEventsRef,
            m_clusterSizeUV[i], r_clusterSizeUV[i], m_fClusterSizeUVFlag);
  }
}

void PXDDQMExpressRecoModule::terminate()
{
}

int PXDDQMExpressRecoModule::getSensorIndex(int Layer, int Ladder, int Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if ((Layer == layer.getLayerNumber()) &&
            (Ladder == ladder.getLadderNumber()) &&
            (Sensor == sensor.getSensorNumber())) {
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
  return tempcounter;
}

void PXDDQMExpressRecoModule::getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor)
{
  VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  int tempcounter = 0;
  int tempend = 0;
  for (VxdID layer : geo.getLayers()) {
    if (layer.getLayerNumber() > c_lastPXDLayer) continue;  // need PXD
    for (VxdID ladder : geo.getLadders(layer)) {
      for (VxdID sensor : geo.getSensors(ladder)) {
        if (tempcounter == Index) {
          *Layer = layer.getLayerNumber();
          *Ladder = ladder.getLadderNumber();
          *Sensor = sensor.getSensorNumber();
          tempend = 1;
        }
        if (tempend == 1) break;
        tempcounter++;
      }
      if (tempend == 1) break;
    }
    if (tempend == 1) break;
  }
}

int PXDDQMExpressRecoModule::SetFlag(int Type, int bin, double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist)
{
  int iret = 0;
  float WarningLevel = 6.0;
  float ErrorLevel = 10.0;
  TH1F* temp = new TH1F("temp", "temp", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
  double NEvents = 0;
  double flagInt = 0;
  double flagrInt = 0;
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    double val = hist->GetBinContent(j + 1);
    NEvents += val;
    val = val / ratio;
    temp->SetBinContent(j + 1, val);
    flagInt += temp->GetBinContent(j + 1);
    flagrInt += refhist->GetBinContent(j + 1);
  }
  if (NEvents < 100) {  // not enough information for comparition
    delete temp;
    iret = -1;
    flaghist->SetBinContent(bin + 1, -1);
    return iret;
  }
  double flag  = temp->GetMean();
  double flagErr  = temp->GetMeanError();
  double flagRMS  = temp->GetRMS();
  double flagRMSErr  = temp->GetRMSError();
  double flagr  = refhist->GetMean();
  double flagrErr  = refhist->GetMeanError();
  double flagrRMS  = refhist->GetRMS();
  double flagrRMSErr  = refhist->GetRMSError();
  TString strDebugInfo = Form("Conditions for Flag--->\n   source %f %f+-%f %f+-%f\n  referen %f %f+-%f %f+-%f\n",
                              flagInt, flag, flagErr, flagRMS, flagRMSErr,
                              flagrInt, flagr, flagrErr, flagrRMS, flagrRMSErr
                             );
  B2DEBUG(130, strDebugInfo.Data());
  if (Type == 1) {  // counts+mean+RMS use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 2) { // counts use
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 3) { // mean use
    if (fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 4) { // RMS use
    if (fabs(flagRMS - flagrRMS) > ErrorLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagRMS - flagrRMS) > WarningLevel * (flagRMSErr + flagrRMSErr)) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 5) { // counts+mean use
    if ((fabs(flag - flagr) > ErrorLevel * (flagErr + flagrErr)) ||
        (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt)))
       ) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if ((fabs(flag - flagr) > WarningLevel * (flagErr + flagrErr)) ||
               (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt)))
              ) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 9) { // bin content use
    flagInt = temp->GetBinContent(bin + 1);
    flagrInt = refhist->GetBinContent(bin + 1);
    if (fabs(flagInt - flagrInt) > ErrorLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 2);
    } else if (fabs(flagInt - flagrInt) > WarningLevel * (sqrt(flagInt) + sqrt(flagrInt))) {
      flaghist->SetBinContent(bin + 1, 1);
    } else {
      flaghist->SetBinContent(bin + 1, 0);
    }
    iret = 1;
  } else if (Type == 10) {
    float flag  = refhist->Chi2Test(temp);
    flaghist->SetBinContent(bin + 1, 0);
    if (flag > pars[1])
      flaghist->SetBinContent(bin + 1, 2);
    if (flag > pars[0])
      flaghist->SetBinContent(bin + 1, 1);
    iret = 1;
  } else if (Type == 100) {
    flaghist->SetBinContent(bin + 1, 0);
    iret = 1;
  } else {
    flaghist->SetBinContent(bin + 1, -1);
    iret = -1;
  }
  delete temp;
  strDebugInfo = Form("SetFlag---> %f, type %i\n", flaghist->GetBinContent(bin + 1), Type);
  B2DEBUG(130, strDebugInfo.Data());
  return iret;
}

int PXDDQMExpressRecoModule::SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist)
{

  TH1F* histF = new TH1F("histF", "histF", hist->GetNbinsX(), hist->GetXaxis()->GetXmin(), hist->GetXaxis()->GetXmax());
  TH1F* refhistF = new TH1F("refhistF", "refhistF", refhist->GetNbinsX(), refhist->GetXaxis()->GetXmin(),
                            refhist->GetXaxis()->GetXmax());
  for (int j = 0; j < hist->GetNbinsX(); j++) {
    histF->SetBinContent(j + 1, hist->GetBinContent(j + 1));
    refhistF->SetBinContent(j + 1, refhist->GetBinContent(j + 1));
  }
  int ret = SetFlag(Type, bin, pars, ratio, histF, refhistF, flaghist);
  delete histF;
  delete refhistF;
  return ret;
}
