/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cstdint>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMDigit.h>
#include <eklm/modules/EKLMUnpacker/EKLMUnpackerModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>
#include <rawdata/dataobjects/RawKLM.h>

using namespace std;
using namespace Belle2;

REG_MODULE(EKLMUnpacker)

EKLMUnpackerModule::EKLMUnpackerModule() : Module()
{
  setDescription("EKLM unpacker (creates EKLMDigit from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputDigitsName", m_outputDigitsName,
           "Name of EKLMDigit store array", string("EKLMDigits"));
}

EKLMUnpackerModule::~EKLMUnpackerModule()
{
}

void EKLMUnpackerModule::initialize()
{
  StoreArray<EKLMDigit>eklmDigits(m_outputDigitsName);
  eklmDigits.registerInDataStore();
}

void EKLMUnpackerModule::beginRun()
{
}

void EKLMUnpackerModule::event()
{
  StoreArray<RawKLM> rawKLM;
  StoreArray<EKLMDigit> eklmDigits(m_outputDigitsName);

  B2DEBUG(1, "Unpacker has have " << rawKLM.getEntries() << " entries");
  for (int i = 0; i < rawKLM.getEntries(); i++) {
    if (rawKLM[i]->GetNumEvents() != 1) {
      B2DEBUG(1, "rawKLM index " << i << " has more than one entry: " <<
              rawKLM[i]->GetNumEvents());
      continue;
    }
    B2DEBUG(1, "num events in buffer: " << rawKLM[i]->GetNumEvents() <<
            " number of nodes (copper boards) " << rawKLM[i]->GetNumNodes());
    /*
     * getNumEntries is defined in RawDataBlock.h and gives the
     * numberOfNodes*numberOfEvents. Number of nodes is num copper boards.
     */
    for (int j = 0; j < rawKLM[i]->GetNumEntries(); j++) {
      unsigned int copperId = rawKLM[i]->GetNodeID(j);
      if (copperId < EKLM_ID || copperId > EKLM_ID + 3)
        continue;
      rawKLM[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        //addendum: There is always an additional word (count) in the end
        int numDetNwords = rawKLM[i]->GetDetectorNwords(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        int* buf_slot = rawKLM[i]->GetDetectorBuffer(j, finesse_num);
        //either no data (finesse not connected) or with the count word
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2DEBUG(1, "word count incorrect: " << numDetNwords);
          continue;
        }
        B2DEBUG(1, "this finesse has " << numHits << " hits");
        if (numDetNwords > 0)
          B2DEBUG(1, "counter is: " << (buf_slot[numDetNwords - 1] & 0xFFFF));
        for (int iHit = 0; iHit < numHits; iHit++) {
          B2DEBUG(1, "unpacking first word: " <<
                  buf_slot[iHit * hitLength + 0] << ", second: " <<
                  buf_slot[iHit * hitLength + 1]);
          uint16_t bword2 = buf_slot[iHit * hitLength + 0] & 0xFFFF;
          uint16_t bword1 = (buf_slot[iHit * hitLength + 0] >> 16) & 0xFFFF;
          uint16_t bword4 = buf_slot[iHit * hitLength + 1] & 0xFFFF;
          /* Unused yet? */
          uint16_t bword3 = (buf_slot[iHit * hitLength + 1] >> 16) & 0xFFFF;
          B2DEBUG(1, "unpacking " << bword1 << ", " << bword2 << ", " <<
                  bword3 << ", " << bword4);
          uint16_t strip = bword1 & 0x7F;
          uint16_t plane = ((bword1 >> 7) & 1) + 1;
          uint16_t sector = ((bword1 >> 8) & 3) + 1;
          uint16_t layer = ((bword1 >> 10) & 0xF) + 1;
          uint16_t endcap = ((bword1 >> 14) & 1) + 1;
          uint16_t ctime = bword2 & 0xFFFF; //full bword
          /* Unused yet? */
          //uint16_t tdc = bword3 & 0x7FF;
          uint16_t charge = bword4 & 0x7FFF;
          B2DEBUG(1, "copper: " << copperId << " finesse: " << finesse_num);
          EKLMDigit* digit = eklmDigits.appendNew();
          digit->setTime(ctime);
          digit->setEndcap(endcap);
          digit->setLayer(layer);
          digit->setSector(sector);
          digit->setPlane(plane);
          digit->setStrip(strip);
          digit->isGood(true);
          digit->setCharge(charge);
          //digit->setEDep(charge);
          B2DEBUG(1, "from digit:endcap " << digit->getEndcap() <<
                  " layer: " << digit->getLayer() << " strip: " <<
                  digit->getSector() << ", " << " plane " <<
                  digit->getPlane() << " :strip " << digit->getStrip() <<
                  " charge=" << digit->getEDep() << " time=" <<
                  digit->getTime());
        }
      } //finesse boards
    } //copper boards
  }  // events... should be only 1...
}

void EKLMUnpackerModule::endRun()
{
}

void EKLMUnpackerModule::terminate()
{
}

