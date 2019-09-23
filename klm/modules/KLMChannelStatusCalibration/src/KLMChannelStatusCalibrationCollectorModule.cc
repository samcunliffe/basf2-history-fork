/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* External headers. */
#include <TFile.h>
#include <TH1.h>
#include <TTree.h>

/* Belle2 headers. */
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMChannelMapValue.h>
#include <klm/modules/KLMChannelStatusCalibration/KLMChannelStatusCalibrationCollectorModule.h>

using namespace Belle2;

REG_MODULE(KLMChannelStatusCalibrationCollector)

KLMChannelStatusCalibrationCollectorModule::KLMChannelStatusCalibrationCollectorModule() :
  CalibrationCollectorModule(),
  m_HitMap("KLMChannelMapHits", DataStore::c_Persistent)
{
  setDescription("Module for KLM channel status calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_ElementNumbers = &(KLMElementNumbers::Instance());
  m_ChannelArrayIndex = &(KLMChannelArrayIndex::Instance());
}

KLMChannelStatusCalibrationCollectorModule::~KLMChannelStatusCalibrationCollectorModule()
{
}

void KLMChannelStatusCalibrationCollectorModule::prepare()
{
  m_BKLMDigits.isRequired();
  m_EKLMDigits.isRequired();
  m_HitMap.registerInDataStore();
  m_HitMap.create();
  m_HitMap->setDataAllChannels(0);
  TTree* calibrationData = new TTree("calibration_data", "");
  registerObject<TTree>("calibration_data", calibrationData);
}

void KLMChannelStatusCalibrationCollectorModule::collect()
{
  for (BKLMDigit& digit : m_BKLMDigits) {
    uint16_t channel = m_ElementNumbers->channelNumberBKLM(
                         digit.getSection(), digit.getSector(), digit.getLayer(),
                         digit.isPhiReadout(), digit.getStrip());
    m_HitMap->setChannelData(channel, m_HitMap->getChannelData(channel) + 1);
  }
  for (EKLMDigit& digit : m_EKLMDigits) {
    uint16_t channel = m_ElementNumbers->channelNumberEKLM(
                         digit.getSection(), digit.getSector(), digit.getLayer(),
                         digit.getPlane(), digit.getStrip());
    m_HitMap->setChannelData(channel, m_HitMap->getChannelData(channel) + 1);
  }
}

void KLMChannelStatusCalibrationCollectorModule::closeRun()
{
  uint16_t channel;
  unsigned int hits;
  TTree* calibrationData = getObjectPtr<TTree>("calibration_data");
  calibrationData->Branch("channel", &channel, "channel/s");
  calibrationData->Branch("hits", &hits, "hits/i");
  KLMChannelIndex klmChannels;
  for (KLMChannelIndex& klmChannel : klmChannels) {
    channel = klmChannel.getKLMChannelNumber();
    hits = m_HitMap->getChannelData(channel);
    calibrationData->Fill();
  }
  /* Clear data for case of multiple runs. */
  m_HitMap->setDataAllChannels(0);
}

void KLMChannelStatusCalibrationCollectorModule::collectFromDQM(
  const char* dqmFile)
{
  std::string histogramName;
  TFile* f = new TFile(dqmFile);
  KLMChannelIndex klmSectors(KLMChannelIndex::c_IndexLevelSector);
  for (KLMChannelIndex& klmSector : klmSectors) {
    int nHistograms;
    if (klmSector.getSubdetector() == KLMElementNumbers::c_BKLM)
      nHistograms = 2;
    else
      nHistograms = 3;
    for (int j = 0; j < nHistograms; ++j) {
      histogramName = "KLM/strip_hits_subdetector_" +
                      std::to_string(klmSector.getSubdetector()) +
                      "_section_" + std::to_string(klmSector.getSection()) +
                      "_sector_" + std::to_string(klmSector.getSector()) +
                      "_" + std::to_string(j);
      TH1* histogram = (TH1*)f->Get(histogramName.c_str());
      if (histogram == nullptr) {
        B2ERROR("KLM DQM histogram " << histogramName << " is not found.");
        continue;
      }
      int n = histogram->GetXaxis()->GetNbins();
      for (int i = 1; i <= n; ++i) {
        uint16_t channelIndex = std::round(histogram->GetBinCenter(i));
        uint16_t channelNumber = m_ChannelArrayIndex->getNumber(channelIndex);
        m_HitMap->setChannelData(channelNumber, histogram->GetBinContent(i));
      }
    }
  }
}
