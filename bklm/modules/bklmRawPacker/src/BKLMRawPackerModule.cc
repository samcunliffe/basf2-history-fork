//+
// File : BKLMRawPackerModule.cc
// Description : Module to store dummy data in RawKLM object
//
// Author : Anselm Vossen, based on Satoru Yamada's BKLMRawPackerModule
//-

#include <bklm/modules/bklmRawPacker/BKLMRawPackerModule.h>
#include <framework/database/DBArray.h>
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;
#define NUM_BKLM_COPPER 4

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(BKLMRawPacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

BKLMRawPackerModule::BKLMRawPackerModule() : Module()
{
  ///Set module properties
  setDescription("This module packs the BKLMDigits to RawKLMs data-objects.");

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("MaxEventNum", max_nevt, "Maximum event number to make", -1);

  ///  maximum # of events to produce( -1 : inifinite)
  addParam("NodeID", m_nodeid, "Node ID", 0);

  // initialize event #
  n_basf2evt = 0;


}



BKLMRawPackerModule::~BKLMRawPackerModule()
{

}


void BKLMRawPackerModule::initialize()
{
  // Initialize EvtMetaData
  m_eventMetaDataPtr.registerInDataStore();

  rawklmarray.registerInDataStore();

  loadMapFromDB();
}


void BKLMRawPackerModule::beginRun()
{
  //load ADCoffset from database
  m_scintADCOffset = m_ADCParams->getADCOffset();
}


void BKLMRawPackerModule::event()
{
  vector<uint32_t> data_words[4][4]; //4 copper, 16 finesse
  for (unsigned int ii = 0; ii < 4; ++ii) {
    for (unsigned int jj = 0; jj < 4; ++jj) {
      data_words[ii][jj].clear();
    }
  }

  B2DEBUG(20, "BKLMRawPackerModule:: entries of BKLMDigits " << digits.getEntries());
  //fill data_words
  for (int d = 0; d < digits.getEntries(); d++) {
    int* buf = new int[2]; //for one hit, hit length is 2;
    buf[0] = 0;
    buf[1] = 0;

    BKLMDigit* bklmDigit = digits[d];
    int iChannelNr = bklmDigit->getStrip();
    int iAx = bklmDigit->isPhiReadout();
    int iLayer = bklmDigit->getLayer();
    int iSector = bklmDigit->getSector();
    int iSection = bklmDigit->getSection();
    float iTdc = bklmDigit->getTime();
    float icharge = bklmDigit->getCharge();
    short iCTime = bklmDigit->getCTime();
    bool isRPC = bklmDigit->inRPC();
    bool isAboveThresh = bklmDigit->isAboveThreshold();
    int moduleId = BKLMElementNumbers::channelNumber(iSection, iSector, iLayer,
                                                     iAx, iChannelNr);
    B2DEBUG(20, "BKLMRawPackerModule:: digi before packer: sector: " << iSector << " issection: " << iSection << " layer: " << iLayer <<
            " plane: " << iAx << " icharge " << icharge << " tdc " << iTdc << " ctime " << iCTime << " isAboveThresh " << isAboveThresh <<
            " isRPC " << isRPC << " " << moduleId << bklmDigit->getModuleID());

    int electId = 0;
    if (m_ModuleIdToelectId.find(moduleId) == m_ModuleIdToelectId.end()) {
      B2DEBUG(20, "BKLMRawPacker::can not find in mapping for moduleId " << moduleId << " section? " << iSection << " , sector " <<
              iSector);
      continue;
    } else {
      electId = m_ModuleIdToelectId[moduleId];
    }

    int copperId;
    int finesse;
    int lane;
    int axis;
    int channelId;
    intToElectCoo(electId, copperId, finesse, lane, axis, channelId);

    B2DEBUG(20, "BKLMRawPacker::copperId " << copperId << " " << iSection << " " << iSector << " " << lane << " " << axis << " " <<
            channelId << " " << iTdc << " " << icharge << " " << iCTime);

    unsigned short bword1 = 0;
    unsigned short bword2 = 0;
    unsigned short bword3 = 0;
    unsigned short bword4 = 0;
    int flag;
    if (isRPC) flag = 2; //010
    else flag = 4; //100
    formatData(flag, channelId, axis, lane, iTdc, icharge, iCTime, bword1, bword2, bword3, bword4);
    buf[0] |= bword2;
    buf[0] |= ((bword1 << 16));
    buf[1] |= bword4;
    buf[1] |= ((bword3 << 16));
    if (copperId < 1 || copperId > 4) {
      B2WARNING("BKLMRawPacker::event() out-of-range (1..4):" << LogVar("COPPER ID", copperId));
      continue;
    }
    data_words[copperId - 1][finesse].push_back(buf[0]);
    data_words[copperId - 1][finesse].push_back(buf[1]);

    delete [] buf;
  }

  //    Make RawKLM array

  RawCOPPERPackerInfo rawcprpacker_info;
  for (int i = 0 ; i < NUM_BKLM_COPPER; i++) {
    //
    // Fill event info (These values will be stored in RawHeader )
    //
    rawcprpacker_info.exp_num = 1;
    rawcprpacker_info.run_subrun_num = 2; // run number : 14bits, subrun # : 8bits
    rawcprpacker_info.eve_num = n_basf2evt;
    rawcprpacker_info.node_id = BKLM_ID + i + 1;
    rawcprpacker_info.tt_ctime = 0x7123456;
    rawcprpacker_info.tt_utime = 0xF1234567;
    rawcprpacker_info.b2l_ctime = 0x7654321;

    //one call per copper
    RawKLM* raw_klm = rawklmarray.appendNew();

    int nwords_1st = data_words[i][0].size();
    int nwords_2nd = data_words[i][1].size();
    int nwords_3rd = data_words[i][2].size();
    int nwords_4th = data_words[i][3].size();

    int* buf1 = new int[ nwords_1st + 1];
    int* buf2 = new int[ nwords_2nd + 1];
    int* buf3 = new int[ nwords_3rd + 1];
    int* buf4 = new int[ nwords_4th + 1];

    for (int j = 0; j < nwords_1st; j++) {
      buf1[j] = data_words[i][0][j];
    }
    buf1[nwords_1st] = 0; // DIVOT

    for (int j = 0; j < nwords_2nd; j++) {
      buf2[j] = data_words[i][1][j];
    }
    buf2[nwords_2nd] = 0; // DIVOT

    for (int j = 0; j < nwords_3rd; j++) {
      buf3[j] = data_words[i][2][j];
    }
    buf3[nwords_3rd] = 0; // DIVOT

    for (int j = 0; j < nwords_4th; j++) {
      buf4[j] = data_words[i][3][j];
    }
    buf4[nwords_4th] = 0; // DIVOT


    raw_klm->PackDetectorBuf(buf1, nwords_1st + 1,
                             buf2, nwords_2nd + 1,
                             buf3, nwords_3rd + 1,
                             buf4, nwords_4th + 1,
                             rawcprpacker_info);

    delete [] buf1;
    delete [] buf2;
    delete [] buf3;
    delete [] buf4;

  }


  //
  // Monitor
  //
  if (max_nevt >= 0) {
    if (n_basf2evt >= max_nevt && max_nevt > 0) {
      printf("[DEBUG] RunStop was detected. ( Setting:  Max event # %d ) Processed Event %d \n",
             max_nevt , n_basf2evt);
      fflush(stdout);
      m_eventMetaDataPtr->setEndOfData();
    }
  }

  n_basf2evt++;
  return;
}

void BKLMRawPackerModule::endRun()
{

}


void BKLMRawPackerModule::formatData(int flag, int channel, int axis, int lane, int tdc, int charge, int ctime,
                                     unsigned short& bword1,
                                     unsigned short& bword2, unsigned short& bword3, unsigned short& bword4)
{

  bword1 = 0;
  bword2 = 0;
  bword3 = 0;
  bword4 = 0;
  bword1 |= (channel & 0x7F);
  bword1 |= ((axis & 1) << 7);
  bword1 |= ((lane & 0x1F) << 8);
  bword1 |= ((flag & 0x7) << 13);
  bword2 |= (ctime & 0xFFFF);
  bword3 |= (tdc & 0x7FF);
  bword4 |= (charge & 0xFFF);

}

void BKLMRawPackerModule::loadMapFromDB()
{
  DBArray<BKLMElectronicMapping> elements;
  elements.getEntries();
  for (const auto& element : elements) {
    B2DEBUG(20, "Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
            ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
            ", section = " << element.getSection() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
            " plane(z/phi) = " << element.getPlane());

    int copperId = element.getCopperId();
    int slotId = element.getSlotId();
    int laneId = element.getLaneId();
    int axisId = element.getAxisId();
    int channelId = element.getChannelId();
    int sector = element.getSector();
    int section = element.getSection();
    int layer = element.getLayer();
    int plane =  element.getPlane();
    int stripId = element.getStripId();
    int elecId = electCooToInt(copperId - BKLM_ID, slotId - 1, laneId, axisId, channelId);
    int moduleId = 0;
    moduleId = BKLMElementNumbers::channelNumber(section, sector, layer,
                                                 plane, stripId);
    m_ModuleIdToelectId[moduleId] = elecId;
    B2DEBUG(20, " electId: " << elecId << " modId: " << moduleId);
  }

}

void BKLMRawPackerModule::intToElectCoo(int id, int& copper, int& finesse, int& lane, int& axis, int& channelId)
{
  copper = 0;
  finesse = 0;
  lane = 0;
  copper = (id & 0xF);
  finesse = (id >> 4) & 3;
  lane = 0;
  lane = (id >> 6) & 0x1F;
  axis = 0;
  axis = (id >> 11) & 0x1;
  channelId = 0;
  channelId = (id >> 12) & 0x3F;

}

int BKLMRawPackerModule::electCooToInt(int copper, int finesse, int lane, int axis, int channel)
{
  //  there are at most 16 copper -->4 bit
  // 4 finesse --> 2 bit
  // < 20 lanes -->5 bit
  // axis --> 1 bit
  // channel --> 6 bit
  int ret = 0;
  copper = copper & 0xF;
  ret |= copper;
  finesse = finesse & 3;
  ret |= (finesse << 4);
  lane = lane & 0x1F;
  ret |= (lane << 6);
  axis = axis & 0x1;
  ret |= (axis << 11);
  channel = channel & 0x3F;
  ret |= (channel << 12);

  return ret;

}
