/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko, Anselm Vossen, Giacomo De Pietro          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* Own header. */
#include <klm/modules/KLMPacker/KLMPackerModule.h>

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

using namespace Belle2;

REG_MODULE(KLMPacker)

KLMPackerModule::KLMPackerModule() : Module()
{
  setDescription("KLM raw data packer (creates RawKLM from BKLMDigits and EKLMDigits).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_EklmElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMPackerModule::~KLMPackerModule()
{
}

void KLMPackerModule::initialize()
{
  m_BklmDigits.isRequired();
  m_EklmDigits.isRequired();
  m_EventMetaData.isRequired();
  m_RawKLMs.registerInDataStore();
}

void KLMPackerModule::beginRun()
{
  if (!m_BklmElectronicsMap.isValid())
    B2FATAL("BKLM electronics map is not available.");
  if (!m_EklmElectronicsMap.isValid())
    B2FATAL("EKLM electronics map is not available.");
}

void KLMPackerModule::event()
{
  /* Indices: copper, data concentrator.
   * Coppers from 0 to 3 are BKLM;
   * coppers from 4 to 7 are EKLM.
   */
  std::vector<uint32_t> dataWords[8][4];

  /* Pack BKLM digits. */
  for (const BKLMDigit& bklmDigit : m_BklmDigits) {
    uint32_t buf[2] = {0};
    uint16_t bword1 = 0;
    uint16_t bword2 = 0;
    uint16_t bword3 = 0;
    uint16_t bword4 = 0;
    int channel = BKLMElementNumbers::channelNumber(bklmDigit.getSection(), bklmDigit.getSector(), bklmDigit.getLayer(),
                                                    bklmDigit.isPhiReadout(), bklmDigit.getStrip());
    const BKLMElectronicsChannel* electronicsChannel =
      m_BklmElectronicsMap->getElectronicsChannel(channel);
    if (electronicsChannel == nullptr)
      B2FATAL("Incomplete BKLM electronics map.");
    int flag = 2; // RPC
    if (!bklmDigit.inRPC())
      flag = 4; // Scintillator
    int lane = electronicsChannel->getLane();
    int plane = electronicsChannel->getAxis();
    int strip = electronicsChannel->getChannel();
    formatData(flag, lane, plane,
               strip, bklmDigit.getCharge(),
               bklmDigit.getCTime(), bklmDigit.getTime(),
               bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    int copper = electronicsChannel->getCopper() - BKLM_ID - 1;
    int dataConcentrator = electronicsChannel->getSlot() - 1;
    dataWords[copper][dataConcentrator].push_back(buf[0]);
    dataWords[copper][dataConcentrator].push_back(buf[1]);
  }

  /* Pack EKLM digits. */
  for (const EKLMDigit& eklmDigit : m_EklmDigits) {
    uint32_t buf[2] = {0};
    uint16_t bword1 = 0;
    uint16_t bword2 = 0;
    uint16_t bword3 = 0;
    uint16_t bword4 = 0;
    int sectorGlobal = m_EklmElementNumbers->sectorNumber(eklmDigit.getSection(), eklmDigit.getLayer(), eklmDigit.getSector());
    const EKLMDataConcentratorLane* dataConcentratorLane = m_EklmElectronicsMap->getLaneBySector(sectorGlobal);
    if (dataConcentratorLane == nullptr)
      B2FATAL("Incomplete EKLM electronics map.");
    int flag = 4; // Scintillator
    int lane = dataConcentratorLane->getLane();
    int stripFirmware = m_EklmElementNumbers->getStripFirmwareBySoftware(eklmDigit.getStrip());
    formatData(flag, lane, eklmDigit.getPlane() - 1,
               stripFirmware, eklmDigit.getCharge(),
               eklmDigit.getCTime(), eklmDigit.getTDC(),
               bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    int copper = dataConcentratorLane->getCopper() - 1;
    int dataConcentrator = dataConcentratorLane->getDataConcentrator();
    dataWords[copper + 4][dataConcentrator].push_back(buf[0]);
    dataWords[copper + 4][dataConcentrator].push_back(buf[1]);
  }

  /* Create RawKLM objects. */
  RawCOPPERPackerInfo packerInfo;
  packerInfo.exp_num = m_EventMetaData->getExperiment();
  packerInfo.run_subrun_num = (m_EventMetaData->getRun() << 8) +
                              (m_EventMetaData->getSubrun() & 0xFF);
  packerInfo.eve_num = m_EventMetaData->getEvent();
  packerInfo.tt_ctime = 0;
  packerInfo.tt_utime = 0;
  packerInfo.b2l_ctime = 0;
  int* detectorBuf[4];
  int nWords[4];
  for (int i = 0; i < 8; i++) {
    if (i < 4) // BKLM
      packerInfo.node_id = BKLM_ID + 1 + i;
    else // EKLM
      packerInfo.node_id = EKLM_ID + 1 + i - 4;
    RawKLM* rawKlm = m_RawKLMs.appendNew();
    for (int j = 0; j < 4; j++) {
      nWords[j] = dataWords[i][j].size();
      detectorBuf[j] = new int[nWords[j] + 1];
      for (int k = 0; k < nWords[j]; k++)
        detectorBuf[j][k] = dataWords[i][j][k];
      detectorBuf[j][nWords[j]] = 0;
    }
    rawKlm->PackDetectorBuf(detectorBuf[0], nWords[0] + 1,
                            detectorBuf[1], nWords[1] + 1,
                            detectorBuf[2], nWords[2] + 1,
                            detectorBuf[3], nWords[3] + 1,
                            packerInfo);
    for (int j = 0; j < 4; j++)
      delete[] detectorBuf[j];
  }
}

/*
 * Data format:
 * Word 1: Bit 0-6   -> strip number.
 *         Bit 7     -> plane number.
 *         Bit 8-12  -> lane in the data concentrator.
 *         Bit 13-15 -> data type: RPC=0x010; scintillator=0x100.
 * Word 2: 15 bits of ctime.
 * Word 3: 10 bits of TDC.
 * Word 4: 12 bits of charge.
 */
void KLMPackerModule::formatData(int flag, int lane, int plane, int strip, int charge, uint16_t ctime, uint16_t tdc,
                                 uint16_t& bword1, uint16_t& bword2, uint16_t& bword3, uint16_t& bword4)
{
  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (strip & 0x7F);
  bword1 |= ((plane & 1) << 7);
  bword1 |= ((lane & 0x1F) << 8);
  bword1 |= (flag << 13);
  bword2 |= (ctime & 0xFFFF);
  bword3 |= (tdc & 0x3FF);
  bword4 |= (charge & 0xFFF);
}

void KLMPackerModule::endRun()
{
}

void KLMPackerModule::terminate()
{
}

