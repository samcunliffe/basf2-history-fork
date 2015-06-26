/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * CDC unpacker module                                                    *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcUnpacker/CDCUnpackerModule.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <cdc/dataobjects/CDCRawHitWaveForm.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>

using namespace std;
using namespace Belle2;
using namespace CDC;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CDCUnpackerModule::CDCUnpackerModule() : Module()
{
  //Set module properties
  setDescription("Generate CDCHit from Raw data.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawCDCName", m_rawCDCName, "Name of the RawCDC List name..", string(""));
  addParam("cdcRawHitWaveFormName", m_cdcRawHitWaveFormName, "Name of the CDCRawHit (Raw data mode).", string(""));
  addParam("cdcRawHitName", m_cdcRawHitName, "Name of the CDCRawHit (Suppressed mode).", string(""));
  addParam("cdcHitName", m_cdcHitName, "Name of the CDCHit List name..", string(""));
  addParam("fadcThreshold", m_fadcThreshold, "Threshold voltage (mV).", 10);
  addParam("tdcOffset", m_tdcOffset, "TDC offset (nsec).", 5562);
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", string(""));
  addParam("enableStoreRawCDC", m_enableStoreCDCRawHit, "Enable to store to the RawCDC object", true);
  addParam("enablePrintOut", m_enablePrintOut, "Enable to print out the data to the terminal", true);
  addParam("setRelationRaw2Hit", m_setRelationRaw2Hit, "Set/unset relation between CDCHit and RawCDC.", false);

}

CDCUnpackerModule::~CDCUnpackerModule()
{
}

void CDCUnpackerModule::initialize()
{

  B2INFO("CDCUnpacker: initialize() Called.");

  StoreArray<RawCDC>::required(m_rawCDCName);

  StoreArray<CDCRawHitWaveForm> storeCDCRawHitWFs(m_cdcRawHitWaveFormName);
  storeCDCRawHitWFs.registerInDataStore();

  StoreArray<CDCRawHit> storeCDCRawHits(m_cdcRawHitName);
  storeCDCRawHits.registerInDataStore();

  StoreArray<CDCHit> storeDigit(m_cdcHitName);
  storeDigit.registerInDataStore();

  // Relation.
  storeDigit.registerRelationTo(storeCDCRawHitWFs);
  storeDigit.registerRelationTo(storeCDCRawHits);

  // Set default names for the relations.
  m_relCDCRawHitToCDCHitName = DataStore::relationName(
                                 DataStore::arrayName<CDCRawHit>(m_cdcRawHitName),
                                 DataStore::arrayName<CDCHit>(m_cdcHitName));

  m_relCDCRawHitWFToCDCHitName = DataStore::relationName(
                                   DataStore::arrayName<CDCRawHitWaveForm>(m_cdcRawHitWaveFormName),
                                   DataStore::arrayName<CDCHit>(m_cdcHitName));

  loadMap();

  B2INFO("CDCUnpacker: FADC threshold: " << m_fadcThreshold);
  m_event = 0;
}

void CDCUnpackerModule::beginRun()
{

  B2INFO("CDCUnpacker: beginRun() called.");

}

void CDCUnpackerModule::event()
{
  B2INFO("CDCUnpacker: event() started.");

  // Create Data objects.

  StoreArray<CDCRawHitWaveForm> cdcRawHitWFs(m_cdcRawHitWaveFormName);
  StoreArray<CDCRawHit> cdcRawHits(m_cdcRawHitName);
  StoreArray<CDCHit> cdcHits(m_cdcHitName);

  RelationArray rawCDCsToCDCHits(cdcRawHits, cdcHits, m_relCDCRawHitToCDCHitName); // CDCRawHit <-> CDCHit
  RelationArray rawCDCWFsToCDCHits(cdcRawHitWFs, cdcHits, m_relCDCRawHitWFToCDCHitName); // CDCRawHitWaveForm <-> CDCHit

  if (m_enableStoreCDCRawHit == true) {
    if (!cdcRawHitWFs.isValid()) {
      cdcRawHits.create();
    } else {
      cdcRawHits.getPtr()->Clear();
    }
    if (!cdcRawHits.isValid()) {
      cdcRawHits.create();
    } else {
      cdcRawHits.getPtr()->Clear();
    }
  }

  if (!cdcHits.isValid()) {
    cdcHits.create();
  } else {
    cdcHits.getPtr()->Clear();
  }
  //
  // Proccess RawCDC data block.
  //
  StoreArray<RawCDC> rawCDCs;
  const int nEntries = rawCDCs.getEntries();
  for (int i = 0; i < nEntries; ++i) {
    const int subDetectorId = rawCDCs[i]->GetNodeID(0);
    const int iNode = (subDetectorId & 0xFFFFFF);
    const int nEntries_rawCDC = rawCDCs[i]->GetNumEntries();
    for (int j = 0; j < nEntries_rawCDC; ++j) {



      int nWords[4];
      nWords[0] = rawCDCs[i]->Get1stDetectorNwords(j);
      nWords[1] = rawCDCs[i]->Get2ndDetectorNwords(j);
      nWords[2] = rawCDCs[i]->Get3rdDetectorNwords(j);
      nWords[3] = rawCDCs[i]->Get4thDetectorNwords(j);

      int* data32tab[4];
      data32tab[0] = (int*)rawCDCs[i]->Get1stDetectorBuffer(j);
      data32tab[1] = (int*)rawCDCs[i]->Get2ndDetectorBuffer(j);
      data32tab[2] = (int*)rawCDCs[i]->Get3rdDetectorBuffer(j);
      data32tab[3] = (int*)rawCDCs[i]->Get4thDetectorBuffer(j);



      //
      // Search Data from Finess A to D (0->3).
      //

      for (int iFiness = 0; iFiness < 4; ++iFiness) {
        int* ibuf = data32tab[iFiness];
        const int nWord = nWords[iFiness];


        if (m_enablePrintOut == true) {
          B2INFO("CDCUnpacker : Print out CDC data block.");
          printBuffer(ibuf, nWord);
        }

        const int c_headearWords = 3;

        if (nWord < c_headearWords) {
          B2WARNING("CDCUnpacker : No CDC block header.");
          continue;
        }

        B2INFO("CDCUnpacker : RawDataBlock(CDC) : Block #  " << i);
        B2INFO("CDCUnpacker : Node ID " << iNode << ", Finness ID " << iFiness);

        setCDCPacketHeader(ibuf);

        const int dataType = getDataType();
        const int dataLength = getDataLength() / 4; // Data length in int word (4bytes).
        const int swDataLength = dataLength * 2;   // Data length in short word (2bytes).


        if (dataLength != (nWord - c_headearWords)) {
          B2ERROR("Inconsistent data size between COPPER and CDC FEE.");
          B2ERROR("data length " << dataLength << " nWord " << nWord);
          continue;
        }
        B2INFO("CDCUnpacker : Data size " << dataLength <<  " words.");

        const int board = getBoardId();
        const int trgNumber = getTriggerNumber();
        const int trgTime = getTriggerTime();

        B2INFO("CDCUnpacker : Board ID " << board <<  ", Trigger number " << trgNumber << ", Trigger time " << trgTime);

        //
        // Check the data type (raw or supressed mode?).
        //

        if (dataType == 1) { //  Raw data mode.
          B2INFO("CDCUnpacker : Raw data mode.");

          //              unsigned short swbuf[3000];

          m_buffer.clear();

          for (int it = 0; it < dataLength; ++it) {
            int index = it + c_headearWords;

            m_buffer.push_back(static_cast<unsigned short>((ibuf[index] & 0xffff0000) >> 16));
            m_buffer.push_back(static_cast<unsigned short>(ibuf[index] & 0xffff));
          }

          const int fadcTdcChannels = 48; // Total channels of FADC or TDC.
          const int nSamples = swDataLength / (2 * fadcTdcChannels); // Number of samplings.

          std::vector<unsigned short> fadcs;
          std::vector<unsigned short> tdcs;

          for (int iCh = 0; iCh < fadcTdcChannels; ++iCh) {
            const int offset = fadcTdcChannels;
            unsigned short fadcSum = 0;     // FADC sum below thereshold.
            unsigned short tdc1 = 0x7fff;   // Fastest TDC.
            unsigned short tdc2 = 0x7fff;   // 2nd fastest TDC.

            for (int iSample = 0; iSample < nSamples; ++iSample) {
              // FADC value for each sample and channel.

              unsigned short fadc = m_buffer.at(iCh + 2 * fadcTdcChannels * iSample);

              if (fadc > m_fadcThreshold) {
                fadcSum += fadc;
              }
              // TDC count for each sample and channel.
              //      unsigned short tdc = swbuf[iCh + 2 * fadcTdcChannels * iSample + offset]&0x7fff;
              unsigned short tdc = m_buffer.at(iCh + 2 * fadcTdcChannels * iSample + offset) & 0x7fff;
              //      unsigned short tdcIsValid =(swbuf[iCh + 2 * fadcTdcChannels * iSample + offset]&0x8000)>>15;
              unsigned short tdcIsValid = (m_buffer.at(iCh + 2 * fadcTdcChannels * iSample + offset) & 0x8000) >> 15;
              if (tdcIsValid == 1) { // good tdc data.
                if (tdc > 0) { // if hit timng is 0, skip.
                  if (tdc < tdc1) {
                    tdc2 = tdc1; // 2nd fastest hit
                    tdc1 = tdc;  // fastest hit.
                  }
                }
              }

              fadcs.push_back(fadc);
              tdcs.push_back(tdc);
              if (m_enableStoreCDCRawHit == true) {
                // Store to the CDCRawHitWaveForm object.
                const unsigned short status = 0;
                cdcRawHitWFs.appendNew(status, trgNumber, iNode, iFiness, board, iCh, iSample, trgTime, fadc, tdc);
              }

            }

            if (tdc1 != 0x7fff) {
              // Store to the CDCHit object.
              const WireID  wireId = getWireID(board, iCh);

              if (trgTime < tdc1) {
                tdc1 = (trgTime | 0x8000) - tdc1;
              } else {
                tdc1 = trgTime - tdc1;
              }
              const CDCHit* hit = cdcHits.appendNew(tdc1 + m_tdcOffset, fadcSum, wireId, tdc2 + m_tdcOffset);

              if (m_enableStoreCDCRawHit == true && m_setRelationRaw2Hit == true) {
                int nRaws = cdcRawHitWFs.getEntries();
                for (int k = 0; k < nRaws; ++k) {
                  hit->addRelationTo(cdcRawHitWFs[k]);
                }
              }

            }




            if (m_enablePrintOut == true) {
              //
              // Print out (for debug).
              //

              printf("FADC ch %2d : ", iCh);
              for (int iSample = 0; iSample < nSamples; ++iSample) {
                printf("%4x ", fadcs.at(iSample));
              }
              printf("\n");

              printf("TDC ch %2d  : ", iCh);
              for (int iSample = 0; iSample < nSamples; ++iSample) {
                printf("%4x ", tdcs.at(iSample));
              }
              printf("\n");
            }

          }

        } else if (dataType == 2) { // Suppressed mode.

          B2INFO("CDCUnpacker : Suppressed mode.");

          // convert int array -> short array.
          //              unsigned short swbuf[480];
          m_buffer.clear();
          for (int it = 0; it < dataLength; ++it) {
            int index = it + c_headearWords;
            m_buffer.push_back(static_cast<unsigned short>((ibuf[index] & 0xffff0000) >> 16));
            m_buffer.push_back(static_cast<unsigned short>(ibuf[index] & 0xffff));
            // for debug.
            //    printf("%4d %4x %4x %8x \n", it, swbuf[it*2],swbuf[it*2+1],ibuf[index]);
          }

          const int bufSize = static_cast<int>(m_buffer.size());
          for (int it = 0; it < bufSize;) {
            unsigned short header = m_buffer.at(it);        // Header.
            unsigned short ch = (header & 0xff00) >> 8; // Channel ID in FE.
            unsigned short length = (header & 0xff) / 2; // Data length in short word.

            //      printf("header 0x%x \n", header);
            printf("channel 0x%x \n", ch);
            printf("length 0x%x \n", length);
            if (!((length == 4) || (length == 5))) {
              B2ERROR("CDCUnpacker : data length should be 4 or 5 words.");
              it += length;
              return;
            }

            unsigned short tot = m_buffer.at(it + 1);     // Time over threshold.
            unsigned short fadcSum = m_buffer.at(it + 2);  // FADC sum.
            unsigned short tdc1 = 0;                  // TDC count.
            unsigned short tdc2 = 0;                  // 2nd TDC count.
            unsigned short tdcFlag = 0;               // Multiple hit or not (1 for multi hits, 0 for single hit).

            if (length == 4) {
              tdc1 = m_buffer.at(it + 3);
            } else if (length == 5) {
              tdc1 = m_buffer.at(it + 3);
              tdc2 = m_buffer.at(it + 4) & 0x7fff;
              tdcFlag = (m_buffer.at(it + 4) & 0x8000) >> 15;
            } else {
              B2ERROR("CDCUnpacker : Undefined data length (should be 4 or 5 short words) ");
            }

            if (m_enablePrintOut == true) {
              printf("%4x %4x %4x %4x %4x %4x %4x \n", ch, length, tot, fadcSum, tdc1, tdc2, tdcFlag);
            }
            if (length == 4 || length == 5) {

              const unsigned short status = 0;
              // Store to the CDCHit.
              const WireID  wireId = getWireID(board, ch);
              //        if(tdc1+m_tdcOffset>8191){
              //    tdc1 = 2630;
              //        }
              //              CDCHit* hit = cdcHits.appendNew(tdc1 + m_tdcOffset, fadcSum, wireId, tdc2);
              CDCHit* hit = cdcHits.appendNew(tdc1, fadcSum, wireId, tdc2);

              if (m_enableStoreCDCRawHit == true) {
                // Store to the CDCRawHit object.
                const CDCRawHit* raw = cdcRawHits.appendNew(status, trgNumber, iNode, iFiness, board, ch, trgTime, fadcSum, tdc1);
                if (m_setRelationRaw2Hit == true) {
                  hit->addRelationTo(raw);
                }
              }
            }
            it += static_cast<int>(length);
          }

        } else {
          B2WARNING("CDCUnpacker :  Undefined CDC Data Block : Block #  " << i);
        }
      }
    }
  }
}

void CDCUnpackerModule::endRun()
{
  B2INFO("CDCUnpacker : End run.");
}

void CDCUnpackerModule::terminate()
{
  B2INFO("CDCUnpacker : Terminated.");
}


const WireID CDCUnpackerModule::getWireID(int iBoard, int iCh)
{
  return m_map[iBoard][iCh];
}

void CDCUnpackerModule::loadMap()
{

  if (! FileSystem::fileExists(m_xmlMapFileName)) {
    B2ERROR("CDC unpacker can't fine a filename: " << m_xmlMapFileName);
    exit(1);
  }


  ifstream ifs;
  ifs.open(m_xmlMapFileName.c_str());
  int isl;
  int icl;
  int iw;
  int iBoard;
  int iCh;

  while (!ifs.eof()) {
    //    ifs >>  isl >> icl >> iw >> lay >> cpr >> finess >> ch;
    ifs >>  isl >> icl >> iw >> iBoard >> iCh;
    const WireID  wireId(isl, icl, iw);
    m_map[iBoard][iCh] = wireId;
  }


}


void CDCUnpackerModule::printBuffer(int* buf, int nwords)
{

  for (int j = 0; j < nwords; ++j) {
    printf(" %.8x", buf[j]);
    if ((j + 1) % 10 == 0) {
      printf("\n");
    }
  }
  printf("\n");

  return;
}
