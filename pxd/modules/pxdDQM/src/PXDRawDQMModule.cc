/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Klemens Lautenbach, Bjoern Spruck                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdDQM/PXDRawDQMModule.h>

#include "TDirectory.h"
#include <string>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

using boost::format;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDRawDQM)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDRawDQMModule::PXDRawDQMModule() : HistoModule() , m_storeRawPxdrarray() , m_storeRawHits(), m_storeRawAdcs(),
  m_storeRawPedestals()
{
  //Set module properties
  setDescription("Monitor raw PXD");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("histgramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("pxdraw"));

  addParam("RawPXDsName", m_storeRawPxdrarrayName, "The name of the StoreArray of RawPXDs to be processed", string(""));
  addParam("PXDRawHitsName", m_storeRawHitsName, "The name of the StoreArray of PXDRawHits to be processed", string(""));
  addParam("PXDRawAdcsName", m_storeRawAdcsName, "The name of the StoreArray of PXDRawAdcs to be processed", string(""));
  addParam("PXDRawPedestalsName", m_storeRawPedestalsName, "The name of the StoreArray of PXDRawPedestals to be processed",
           string(""));
}

void PXDRawDQMModule::defineHisto()
{
  // Create a separate histogram directory and cd into it.
  TDirectory* oldDir = gDirectory;
  oldDir->mkdir(m_histogramDirectoryName.c_str())->cd();

  hrawPxdPackets = new TH1F("hrawPxdPackets", "Pxd Raw Packet Nr;Nr per Event", 16, 0, 16);
  hrawPxdPacketSize = new TH1F("hrawPxdPacketSize", "Pxd Raw Packetsize;Words per packet", 1024, 0, 1024);
  hrawPxdHitMapAll = new TH2F("hrawPxdHitMapAll",
                              "Pxd Raw Hit Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370, 0, 3700, 350, 0, 3500);
  // ADC map not supported by DHC anymore ... deactive filling, later remove
  hrawPxdAdcMapAll =
    NULL;// new TH2F("hrawPxdAdcMapAll",                               "Pxd Raw Adc Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370/*0*/, 0, 3700, 350/*0*/, 0, 3500);
  // Pedestal map for Calibration should not be in DQM ... deactive filling, later remove
  hrawPxdPedestalMapAll =
    NULL;// new TH2F("hrawPxdPedestalMapAll",                                    "Pxd Raw Pedestal Map Overview;column+(ladder-1)*300+100;row+850*((layer-1)*2+(sensor-1))", 370/*0*/, 0, 3700, 350/*0*/, 0, 3500);

  hrawPxdHitsCount = new TH1F("hrawPxdCount", "Pxd Raw Count ;Nr per Event", 8192, 0, 8192);
  int limit_dhhid = 4; // workaround for testbeam online DQM to prevent memory overflow for too many histograms
  for (auto i = 0; i < 64; i++) {
    auto num1 = (((i >> 5) & 0x1) + 1);
    auto num2 = ((i >> 1) & 0xF);
    auto num3 = ((i & 0x1) + 1);
    //cppcheck-suppress zerodiv
    string s = str(format("Sensor %d:%d:%d (DHH ID %02Xh)") % num1 % num2 % num3 % i);
    //cppcheck-suppress zerodiv
    string s2 = str(format("_%d.%d.%d") % num1 % num2 % num3);

    hrawPxdHitMap[i] = (i > limit_dhhid) ? NULL : new TH2F(("hrawPxdHitMap" + s2).c_str(),
                                                           ("Pxd Raw Hit Map, " + s + ";column;row").c_str(), 256,
                                                           0, 256, 786, 0, 786);
    hrawPxdChargeMap[i] = (i > limit_dhhid) ? NULL : new TH2F(("hrawPxdChargeMap" + s2).c_str(),
                                                              ("Pxd Raw Charge Map, " + s + ";column;row").c_str(), 256, 0, 256, 786, 0, 786);
    hrawPxdHitsCharge[i] = (i > limit_dhhid) ? NULL : new TH1F(("hrawPxdHitsCharge" + s2).c_str(),
                                                               ("Pxd Raw Hit Charge, " + s + ";Charge").c_str(), 256, 0, 256);
    hrawPxdHitsCommonMode[i] = (i > limit_dhhid) ? NULL : new TH1F(("hrawPxdHitsCommonMode" + s2).c_str(),
                               ("Pxd Raw Hit Common Mode, " + s + ";Value").c_str(),
                               256, 0, 256);
    hrawPxdHitsTimeWindow[i] = (i > limit_dhhid) ? NULL : new TH1F(("hrawPxdHitsTimeWindow" + s2).c_str(),
                               ("Pxd Raw Hit Time Window (framenr*1024-startrow), " + s + ";Time [a.u.]").c_str(), 8192, -1024, 8192 - 1024);
  }

  // cd back to root directory
  oldDir->cd();
}

void PXDRawDQMModule::initialize()
{
  REG_HISTOGRAM
  m_storeRawPxdrarray.required(m_storeRawPxdrarrayName);
  m_storeRawHits.required(m_storeRawHitsName);
  m_storeRawPedestals.required(m_storeRawPedestalsName);
  m_storeRawAdcs.required(m_storeRawAdcsName);
}

void PXDRawDQMModule::beginRun()
{
  // Just to make sure, reset all the histograms.
  if (hrawPxdPackets) hrawPxdPackets->Reset();
  if (hrawPxdPacketSize) hrawPxdPacketSize->Reset();
  if (hrawPxdHitsCount) hrawPxdHitsCount->Reset();
  if (hrawPxdHitMapAll) hrawPxdHitMapAll->Reset();
  if (hrawPxdAdcMapAll) hrawPxdAdcMapAll->Reset();
  if (hrawPxdPedestalMapAll) hrawPxdPedestalMapAll->Reset();
  for (int i = 0; i < 64; i++) {
    if (hrawPxdHitMap[i]) hrawPxdHitMap[i]->Reset();
    if (hrawPxdChargeMap[i]) hrawPxdChargeMap[i]->Reset();
    if (hrawPxdHitsCharge[i]) hrawPxdHitsCharge[i]->Reset();
    if (hrawPxdHitsCommonMode[i]) hrawPxdHitsCommonMode[i]->Reset();
    if (hrawPxdHitsTimeWindow[i]) hrawPxdHitsTimeWindow[i]->Reset();
  }
}

void PXDRawDQMModule::event()
{
  hrawPxdPackets->Fill(m_storeRawPxdrarray.getEntries());

  for (auto& it : m_storeRawPxdrarray) {
    if (hrawPxdPacketSize) hrawPxdPacketSize->Fill(it.size());
  }

  if (hrawPxdHitsCount) hrawPxdHitsCount->Fill(m_storeRawHits.getEntries());

  for (auto& it : m_storeRawHits) {
    int dhh_id;
    // calculate DHH id from Vxd Id
    unsigned int layer, ladder, sensor;//, segment;
    VxdID currentVxdId;
    currentVxdId = it.getSensorID();
    layer = currentVxdId.getLayerNumber();/// 1 ... 2
    ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
    sensor = currentVxdId.getSensorNumber();/// 1 ... 2
    // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
    dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
    if (dhh_id <= 0 || dhh_id >= 64) {
      B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
      continue;
    }
    if (hrawPxdHitMap[dhh_id]) hrawPxdHitMap[dhh_id]->Fill(it.getColumn(), it.getRow());
    if (hrawPxdHitMapAll) hrawPxdHitMapAll->Fill(it.getColumn() + ladder * 300 - 200,
                                                   100 + it.getRow() + 850 * (layer + layer + sensor - 3));
    if (hrawPxdChargeMap[dhh_id]) hrawPxdChargeMap[dhh_id]->Fill(it.getColumn(), it.getRow(), it.getCharge());
    if (hrawPxdHitsCharge[dhh_id]) hrawPxdHitsCharge[dhh_id]->Fill(it.getCharge());
    if (hrawPxdHitsCommonMode[dhh_id]) hrawPxdHitsCommonMode[dhh_id]->Fill(it.getCommonMode());
    if (hrawPxdHitsTimeWindow[dhh_id]) hrawPxdHitsTimeWindow[dhh_id]->Fill(it.getFrameNr() * 1024 - it.getStartRow());
  }

  if (hrawPxdAdcMapAll) {
    for (auto& it : m_storeRawAdcs) {
      int dhh_id;
      // calculate DHH id from Vxd Id
      unsigned int layer, ladder, sensor;//, segment;
      VxdID currentVxdId;
      currentVxdId = it.getSensorID();
      layer = currentVxdId.getLayerNumber();/// 1 ... 2
      ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
      sensor = currentVxdId.getSensorNumber();/// 1 ... 2
      // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
      dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
      if (dhh_id <= 0 || dhh_id >= 64) {
        B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
        continue;
      }

      unsigned int chip_offset;
      chip_offset = it.getChip() * 64;
      const unsigned char* data = it.getData();
      for (int row = 0; row < 786; row++) {
        for (int col = 0; col < 64; col++) {
          hrawPxdAdcMapAll->Fill(col + chip_offset + ladder * 300 - 200, 100 + row + 850 * (layer + layer + sensor - 3), *(data++));
        }
      }
    }
  }
  if (hrawPxdPedestalMapAll) {
    for (auto& it : m_storeRawPedestals) {
      int dhh_id;
      // calculate DHH id from Vxd Id
      unsigned int layer, ladder, sensor;//, segment;
      VxdID currentVxdId;
      currentVxdId = it.getSensorID();
      layer = currentVxdId.getLayerNumber();/// 1 ... 2
      ladder = currentVxdId.getLadderNumber();/// 1 ... 8 and 1 ... 12
      sensor = currentVxdId.getSensorNumber();/// 1 ... 2
      // segment = currentVxdId.getSegmentNumber();// Frame nr? ... ignore
      dhh_id = ((layer - 1) << 5) | ((ladder) << 1) | (sensor - 1);
      if (dhh_id <= 0 || dhh_id >= 64) {
        B2ERROR("SensorId (DHH ID) out of range: " << dhh_id);
        continue;
      }

      const unsigned char* data = it.getData();
      unsigned int chip_offset;
      chip_offset = it.getChip() * 64;
      for (int row = 0; row < 786; row++) {
        for (int col = 0; col < 64; col++) {
          hrawPxdPedestalMapAll->Fill(col + chip_offset + ladder * 300 - 200, 100 + row + 850 * (layer + layer + sensor - 3), *(data++));
        }
      }
    }
  }
}
