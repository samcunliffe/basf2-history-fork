/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMChannelDataImporter.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMChannels.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

EKLMChannelDataImporter::EKLMChannelDataImporter()
{
  m_ExperimentLow = 0;
  m_RunLow = 0;
  m_ExperimentHigh = -1;
  m_RunHigh = -1;
}

EKLMChannelDataImporter::~EKLMChannelDataImporter()
{
}

void EKLMChannelDataImporter::setIOV(int experimentLow, int runLow,
                                     int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void EKLMChannelDataImporter::loadChannelData(EKLMChannelData* channelData)
{
  m_Channels.construct();
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  int iEndcap, iLayer, iSector, iPlane, iStrip, strip;
  for (iEndcap = 1; iEndcap <= geoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iStrip = 1; iStrip <= geoDat->getNStrips(); iStrip++) {
            strip = geoDat->stripNumber(iEndcap, iLayer, iSector, iPlane,
                                        iStrip);
            m_Channels->setChannelData(strip, channelData);
          }
        }
      }
    }
  }
}

void EKLMChannelDataImporter::setChannelData(
  int endcap, int layer, int sector, int plane, int strip,
  EKLMChannelData* channelData)
{
  int stripGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                            strip);
  m_Channels->setChannelData(stripGlobal, channelData);
}

void EKLMChannelDataImporter::loadActiveChannels(const char* activeChannelsData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel, active;
  /* cppcheck-suppress variableScope */
  int endcap, layer, sector, plane, strip, stripFirmware, stripGlobal;
  /* cppcheck-suppress variableScope */
  const int* sectorGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  DBObjPtr<EKLMElectronicsMap> electronicsMap;
  EKLMDataConcentratorLane dataConcentratorLane;
  TFile* file;
  TTree* tree;
  file = new TFile(activeChannelsData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("active", &active);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    dataConcentratorLane.setCopper(copper);
    dataConcentratorLane.setDataConcentrator(dataConcentrator);
    dataConcentratorLane.setLane(lane);
    sectorGlobal = electronicsMap->getSectorByLane(&dataConcentratorLane);
    if (sectorGlobal == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    elementNumbers->sectorNumberToElementNumbers(*sectorGlobal, &endcap,
                                                 &layer, &sector);
    plane = daughterCard / 5 + 1;
    stripFirmware = (daughterCard % 5) * 15 + channel + 1;
    strip = elementNumbers->getStripSoftwareByFirmware(stripFirmware);
    stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadHighVoltage(const char* highVoltageData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  float voltage;
  /* cppcheck-suppress variableScope */
  int endcap, layer, sector, plane, strip, stripFirmware, stripGlobal;
  /* cppcheck-suppress variableScope */
  const int* sectorGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  DBObjPtr<EKLMElectronicsMap> electronicsMap;
  EKLMDataConcentratorLane dataConcentratorLane;
  TFile* file;
  TTree* tree;
  file = new TFile(highVoltageData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("voltage", &voltage);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    dataConcentratorLane.setCopper(copper);
    dataConcentratorLane.setDataConcentrator(dataConcentrator);
    dataConcentratorLane.setLane(lane);
    sectorGlobal = electronicsMap->getSectorByLane(&dataConcentratorLane);
    if (sectorGlobal == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    elementNumbers->sectorNumberToElementNumbers(*sectorGlobal, &endcap,
                                                 &layer, &sector);
    plane = daughterCard / 5 + 1;
    stripFirmware = (daughterCard % 5) * 15 + channel + 1;
    strip = elementNumbers->getStripSoftwareByFirmware(stripFirmware);
    stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setVoltage(voltage);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadLookbackWindow(const char* lookbackWindowData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  int lookbackTime, lookbackWindowWidth;
  /* cppcheck-suppress variableScope */
  int endcap, layer, sector, plane, strip, stripFirmware, stripGlobal;
  /* cppcheck-suppress variableScope */
  const int* sectorGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  DBObjPtr<EKLMElectronicsMap> electronicsMap;
  EKLMDataConcentratorLane dataConcentratorLane;
  TFile* file;
  TTree* tree;
  file = new TFile(lookbackWindowData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("lookback_time", &lookbackTime);
  tree->SetBranchAddress("lookback_window_width", &lookbackWindowWidth);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    dataConcentratorLane.setCopper(copper);
    dataConcentratorLane.setDataConcentrator(dataConcentrator);
    dataConcentratorLane.setLane(lane);
    sectorGlobal = electronicsMap->getSectorByLane(&dataConcentratorLane);
    if (sectorGlobal == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    elementNumbers->sectorNumberToElementNumbers(*sectorGlobal, &endcap,
                                                 &layer, &sector);
    plane = daughterCard / 5 + 1;
    stripFirmware = (daughterCard % 5) * 15 + channel + 1;
    strip = elementNumbers->getStripSoftwareByFirmware(stripFirmware);
    stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setLookbackTime(lookbackTime);
    channelData->setLookbackWindowWidth(lookbackWindowWidth);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::loadThresholds(const char* thresholdsData)
{
  int i, n;
  int copper, dataConcentrator, lane, daughterCard, channel;
  int active, pedestalMin, threshold, adjustmentVoltage;
  /* cppcheck-suppress variableScope */
  int endcap, layer, sector, plane, strip, stripFirmware, stripGlobal;
  /* cppcheck-suppress variableScope */
  const int* sectorGlobal;
  const EKLM::ElementNumbersSingleton* elementNumbers =
    &(EKLM::ElementNumbersSingleton::Instance());
  DBObjPtr<EKLMElectronicsMap> electronicsMap;
  EKLMDataConcentratorLane dataConcentratorLane;
  TFile* file;
  TTree* tree;
  file = new TFile(thresholdsData, "");
  tree = (TTree*)file->Get("tree");
  n = tree->GetEntries();
  tree->SetBranchAddress("copper", &copper);
  tree->SetBranchAddress("data_concentrator", &dataConcentrator);
  tree->SetBranchAddress("lane", &lane);
  tree->SetBranchAddress("daughter_card", &daughterCard);
  tree->SetBranchAddress("channel", &channel);
  tree->SetBranchAddress("active", &active);
  tree->SetBranchAddress("pedestal_min", &pedestalMin);
  tree->SetBranchAddress("threshold", &threshold);
  tree->SetBranchAddress("adjustment_voltage", &adjustmentVoltage);
  for (i = 0; i < n; i++) {
    tree->GetEntry(i);
    dataConcentratorLane.setCopper(copper);
    dataConcentratorLane.setDataConcentrator(dataConcentrator);
    dataConcentratorLane.setLane(lane);
    sectorGlobal = electronicsMap->getSectorByLane(&dataConcentratorLane);
    if (sectorGlobal == nullptr) {
      B2FATAL("Wrong DAQ channel in calibration data: copper = " << copper <<
              ", data_concentrator = " << dataConcentrator << ", lane = " <<
              lane);
    }
    elementNumbers->sectorNumberToElementNumbers(*sectorGlobal, &endcap,
                                                 &layer, &sector);
    plane = daughterCard / 5 + 1;
    stripFirmware = (daughterCard % 5) * 15 + channel + 1;
    strip = elementNumbers->getStripSoftwareByFirmware(stripFirmware);
    stripGlobal = elementNumbers->stripNumber(endcap, layer, sector, plane,
                                              strip);
    EKLMChannelData* channelData = const_cast<EKLMChannelData*>(
                                     m_Channels->getChannelData(stripGlobal));
    if (channelData == nullptr)
      B2FATAL("Channel data are not loaded. Use loadChannelData().");
    channelData->setPedestal(pedestalMin);
    channelData->setThreshold(threshold);
    channelData->setAdjustmentVoltage(adjustmentVoltage);
  }
  delete tree;
  delete file;
}

void EKLMChannelDataImporter::importChannelData()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_Channels.import(iov);
}

