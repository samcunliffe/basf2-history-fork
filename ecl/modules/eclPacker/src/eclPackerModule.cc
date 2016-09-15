#include <ecl/modules/eclPacker/eclPackerModule.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

REG_MODULE(ECLPacker)

ECLPackerModule::ECLPackerModule() :
  m_compressMode(false),
  m_eclRawCOPPERs("", DataStore::c_Event)
{
  setDescription("");
  addParam("InitFileName", m_eclMapperInitFileName, "Initialization file", string(""));
  addParam("RawCOPPERsName", m_eclRawCOPPERsName, "Name of the RawECL container", string("RawECL"));
  addParam("CompressMode", m_compressMode, "compress mode for ADC samples", false);
  addParam("AmpThreshold", m_ampThreshold, "Amplitude threshold", 50);

  m_EvtNum = 0;

  iEclDigIndices = new int[ECL_TOTAL_CHANNELS];
  iEclWfIndices  = new int[ECL_TOTAL_CHANNELS];

}

ECLPackerModule::~ECLPackerModule()
{

  delete m_eclMapper;
  delete[] iEclDigIndices;
  delete[] iEclWfIndices;
}

void ECLPackerModule::initialize()
{

  // require input data
  StoreArray<ECLDigit>::required();
  StoreArray<ECLDsp>::optional();

  // register output container in data store
  m_eclRawCOPPERs.registerInDataStore(m_eclRawCOPPERsName);

  // initialize channel mapper from file (temporary)
  m_eclMapper = new ECLChannelMapper();
  m_eclMapper->initFromFile(m_eclMapperInitFileName.c_str());

  // of initialize if from DB TODO

}

void ECLPackerModule::beginRun()
{
  //TODO
}

void ECLPackerModule::event()
{

  B2DEBUG(50, "EclPacker:: event called ");
  // input data
  StoreArray<ECLDigit> ECLDigitData;
  StoreArray<ECLDsp>   ECLWaveformData;

  // output data
  m_eclRawCOPPERs.clear();

  B2DEBUG(50, "EclPacker:: output data arrays created");

  int nActiveChannelsWithADCData, nActiveDSPChannels;
  int triggerPhase = 0, dspMask = 0;

  // get total number of hits
  int nEclDigits   = ECLDigitData.getEntries();
  int nEclWaveform = ECLWaveformData.getEntries();


  for (int i = 0; i < ECL_CRATES; i++) {
    collectorMaskArray[i] = 0;
    for (int j = 0; j < ECL_BARREL_SHAPERS_IN_CRATE; j++) {
      shaperMaskArray[i][j]    = 0;
      shaperNHits[i][j]        = 0;
      shaperADCMaskArray[i][j] = 0;
      shaperNWaveform[i][j]    = 0;
    }
  }

  for (int j = 0; j < ECL_TOTAL_CHANNELS; j++) {
    iEclDigIndices[j] = -1;
    iEclWfIndices[j] = -1;
  }


  B2DEBUG(100, "EclPacker:: N_Digits    = " << nEclDigits);
  B2DEBUG(100, "EclPacker:: N_Waveforms = " << nEclWaveform);

  int cid = 0, i_digit = 0, i_wf = 0;
  int iCOPPER, iFINESSE, iCrate, iShaper, iChannel, nShapers;

  B2DEBUG(100, "EclPacker:: Hits ======>> ");
  // fill number of hits, masks and fill correspondance between cellID and index in container
  for (int i_digit = 0; i_digit < nEclDigits; i_digit++) {
    cid = ECLDigitData[i_digit]->getCellId();
    int amp = ECLDigitData[i_digit]->getAmp();

    if (amp < m_ampThreshold) continue;

    //  int tim = ECLDigitData[i_digit]->getTimeFit();
    //  int qua = ECLDigitData[i]->getQuality();
    //TODO: Threshold
    iCrate = m_eclMapper->getCrateID(cid);
    iShaper = m_eclMapper->getShaperPosition(cid);
    iChannel = m_eclMapper->getShaperChannel(cid);
    if (iCrate < 1 && iShaper < 1 && iChannel < 1) {
      B2ERROR("Wrong crate/shaper/channel ids: " << iCrate << " " << iShaper << " " << iChannel << " for CID " << cid);
      throw eclPacker_internal_error();
    }

    collectorMaskArray[iCrate - 1] |= (1 << (iShaper - 1));

    shaperMaskArray[iCrate - 1][iShaper - 1] |= (1 << (iChannel - 1));
    shaperNHits[iCrate - 1][iShaper - 1]++;

    iEclDigIndices[cid - 1] = i_digit;
  }

  for (int i_wf = 0; i_wf < nEclWaveform; i_wf++) {
    cid = ECLWaveformData[i_wf]->getCellId();
    iCrate = m_eclMapper->getCrateID(cid);
    iShaper = m_eclMapper->getShaperPosition(cid);
    iChannel = m_eclMapper->getShaperChannel(cid);

    shaperADCMaskArray[iCrate - 1][iShaper - 1] |= (1 << (iShaper - 1));
    shaperNWaveform[iCrate - 1][iShaper - 1]++;

    iEclWfIndices[cid - 1] = i_wf;
  }

  // fill rawCOPPERPacker data
  RawCOPPERPackerInfo rawcprpacker_info;
  rawcprpacker_info.exp_num = 0;
  rawcprpacker_info.run_subrun_num = 1; // run number : 14bits, subrun # : 8bits
  rawcprpacker_info.eve_num = m_EvtNum;
  rawcprpacker_info.tt_ctime = 0x7123456;  //??? (copy-past from CDC)
  rawcprpacker_info.tt_utime = 0xF1234567; //???
  rawcprpacker_info.b2l_ctime = 0x7654321; //???


  B2DEBUG(100, "EclPacker:: proceed COPPERs... ");
  B2DEBUG(100, "EclPacker:: ECL_COPPERS = " << ECL_COPPERS);

  //cycle over all coppers
  for (iCOPPER = 1; iCOPPER <= ECL_COPPERS; iCOPPER++) {

    std::vector <int> buff[ECL_FINESSES_IN_COPPER];

    int iCOPPERNode = (iCOPPER <= ECL_BARREL_COPPERS) ? BECL_ID + iCOPPER : EECL_ID + iCOPPER;

    //check if at least one of FINESSES have hits
    int icr1 = m_eclMapper->getCrateID(iCOPPERNode, 0);
    int icr2 = m_eclMapper->getCrateID(iCOPPERNode, 1);
    B2DEBUG(200, "iCOPPERNode = 0x" << std::hex << iCOPPERNode << std::dec << " nCrate1 = " << icr1 << " nCrate2 = " << icr2);
    if (!(collectorMaskArray[icr1 - 1] || collectorMaskArray[icr2 - 1])) continue;

    rawcprpacker_info.node_id = iCOPPERNode;
    // Create RawECL object

    int nwords[2] = {0, 0};
    const int finesseHeaderNWords = 3;


    //cycle over finesses in copper

    for (iFINESSE = 0; iFINESSE < ECL_FINESSES_IN_COPPER; iFINESSE++) {
      //  for (iCrate = 1; iCrate <= ECL_CRATES; iCrate++)

      iCrate = m_eclMapper->getCrateID(iCOPPERNode, iFINESSE);

      nShapers = m_eclMapper->getNShapersInCrate(iCrate);

      if (!shaperMaskArray[iCrate - 1]) continue;
      B2DEBUG(200, "Pack data for iCrate = " << iCrate << " nShapers = " << nShapers);

//      int type   = 0;
//      int fee_id = 0;
//      int ver    = 0;
//      const short trigTime = 0x0;

//      buff[iFINESSE].push_back((type << 24) | (ver << 16) | fee_id);
//      buff[iFINESSE].push_back((trigTime << 16) | nwords[iFINESSE]); // recalculate nwords later
//      buff[iFINESSE].push_back(m_EvtNum);

      // write EclCollector header to the buffer
      int eclCollectorHeader = 0xFFF;
      if (m_compressMode) eclCollectorHeader += (1 << 12);
      buff[iFINESSE].push_back(eclCollectorHeader);

      // cycle over shaper boards connected to finesse
      B2DEBUG(200, "Finess data writen, start cycle over shapers ");

      for (iShaper = 1; iShaper <= nShapers; iShaper++) {


        nActiveDSPChannels = shaperNHits[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "iCrate = " << iCrate << " iShaper = " << iShaper << " nActiveDSPChannels = " << nActiveDSPChannels);
        nActiveChannelsWithADCData = shaperNWaveform[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "nActiveChannelsWithADCData = " << nActiveChannelsWithADCData);

        int shaperDataLength = 4 + nActiveDSPChannels + nActiveChannelsWithADCData * ECL_ADC_SAMPLES_PER_CHANNEL;
        // fill shaperDsp header
        int shaper_header_w0 = (0x10 << 16) | shaperDataLength;
        buff[iFINESSE].push_back(shaper_header_w0);

        triggerPhase = 0; //?????
        int shaper_header_w1 = (nActiveChannelsWithADCData & 0x1F) << 24;
        shaper_header_w1 |= (ECL_ADC_SAMPLES_PER_CHANNEL & 0x7F) << 16;
        shaper_header_w1 |= (nActiveDSPChannels & 0x1F) << 8;
        shaper_header_w1 |= triggerPhase;
        buff[iFINESSE].push_back(shaper_header_w1);

        dspMask = shaperMaskArray[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "dspMask = " << std::hex << dspMask);
        int shaper_header_w2 = (dspMask & 0xFFFF) << 16;
        shaper_header_w2 |= (m_EvtNum & 0xFFFF); // trigger tag
        buff[iFINESSE].push_back(shaper_header_w2);

        int adcMask = shaperADCMaskArray[iCrate - 1][iShaper - 1];
        B2DEBUG(200, "adcMask = " << std::hex << adcMask);
        int shaper_header_w3 = (adcMask & 0xFFFF);
        buff[iFINESSE].push_back(shaper_header_w3);

        // cycle over shaper channels

        for (iChannel = 1; iChannel <= ECL_CHANNELS_IN_SHAPER; iChannel++) {


          cid = m_eclMapper->getCellId(iCrate, iShaper, iChannel);

          if (cid < 1) continue;

          i_digit = iEclDigIndices[cid - 1];
          if (i_digit < 0) continue;
          int qua = ECLDigitData[i_digit]->getQuality();
          int amp = ECLDigitData[i_digit]->getAmp();
          int tim = ECLDigitData[i_digit]->getTimeFit();
          int hit_data = ((qua & 3) << 30) & 0xC0000000;
          hit_data |= (tim & 0x1FFF) << 18;
          hit_data |= ((amp + 128) & 0x3FFFF);
          buff[iFINESSE].push_back(hit_data);

//          B2DEBUG(100,"cid = " << cid << " amp = " << amp << " tim = " << tim);

        }

        int* adcBuffer_temp = new int[nActiveChannelsWithADCData];
        resetBuffPosition();
        setBuffLength(ECL_ADC_SAMPLES_PER_CHANNEL);
        for (iChannel = 1; iChannel <= ECL_CHANNELS_IN_SHAPER; iChannel++) {
          cid = m_eclMapper->getCellId(iCrate, iShaper, iChannel);
          if (cid < 1) continue;
          i_wf   = iEclWfIndices[cid - 1];
          if (i_wf < 0) continue;
          B2DEBUG(200, "i_wf = " << i_wf);
          ECLWaveformData[i_wf]->getDspA(m_EclWaveformSamples); // Check this method in implementation of ECLDsp.h!!!
          int adc_data_base = 0;
          int adc_data_diff_width = 0;
          int adc_data_offset = 0;

          if (m_compressMode) {
            // calculate adc_data_base and adc_data_diff_width for compressed mode
            int ampMax = m_EclWaveformSamples[0];
            int ampMin = m_EclWaveformSamples[1];
            for (int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              if (m_EclWaveformSamples[iSample] > ampMax) ampMax = m_EclWaveformSamples[iSample];
              if (m_EclWaveformSamples[iSample] < ampMin) ampMin = m_EclWaveformSamples[iSample];
            }
            adc_data_base = ampMin & 0x3FFFF;
            writeNBits(adcBuffer_temp, adc_data_base, 18);
            adc_data_diff_width = int(log2((float)ampMax - (float)ampMin)) + 1;
            writeNBits(adcBuffer_temp, adc_data_diff_width, 5);

            for (int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              adc_data_offset = m_EclWaveformSamples[iSample] - adc_data_base;
              writeNBits(adcBuffer_temp, adc_data_offset, adc_data_diff_width);
            }
            for (int i = 0; i < m_bufPos; i++) {
              buff[iFINESSE].push_back(adcBuffer_temp[i]);
            }
          } else {
            for (int iSample = 0; iSample < ECL_ADC_SAMPLES_PER_CHANNEL; iSample++) {
              buff[iFINESSE].push_back(m_EclWaveformSamples[iSample]);
            }

          }
          B2DEBUG(100, "B RiShaper =  " << iShaper << " iChannel = " << iChannel);


        }
        delete[] adcBuffer_temp;


      }

    }



    //  B2DEBUG(200,"iShaper =  " << iShaper << " processing iChan   = " << iChannel);

    RawECL* newRawECL = m_eclRawCOPPERs.appendNew();


    nwords[0] = buff[0].size();
    nwords[1] = buff[1].size();

    buff[0][0] |= (nwords[0] - finesseHeaderNWords) * 4;
    buff[1][0] |= (nwords[1] - finesseHeaderNWords) * 4;

    B2DEBUG(100, "**** iEvt = " << m_EvtNum << " node= " << iCOPPERNode);
    for (unsigned int i = 0; i < 2; i++)
      for (unsigned int j = 0; j < buff[i].size(); j++) {
        //    B2DEBUG(100,">> " << std::hex << setfill('0') << setw(8) << buff[i][j]);
      }

    B2DEBUG(100, "Call PackDetectorBuf");
    newRawECL->PackDetectorBuf(buff[0].data(), nwords[0], buff[1].data(), nwords[1],
                               NULL, 0, NULL, 0, rawcprpacker_info);

  }




  m_EvtNum++;

}

void ECLPackerModule::endRun()
{
  //TODO
}

void ECLPackerModule::terminate()
{
}

void ECLPackerModule::setBuffLength(int bufLength)
{
  m_bufLength = bufLength;
}

void ECLPackerModule::resetBuffPosition()
{
  m_bufPos = 0;
  m_bitPos = 0;

}



void ECLPackerModule::writeNBits(int* buff, int value, int bitsToWrite)
{
  int tmpval = 0;
  if (value > pow(2, bitsToWrite) - 1) {
    B2ERROR("Error compressing ADC samples: tying to write to long word");
    throw Write_adc_samples_error();
  }

  if (m_bitPos + bitsToWrite > 32)
    if (m_bufPos == m_bufLength) {
      B2ERROR("Error compressing ADC samples: unexpectedly reach end of buffer");
      throw Write_adc_samples_error();
    } else {
      tmpval = pow(2, 32 - m_bitPos) - 1;
      buff[m_bufPos] &= (~tmpval);
      buff[m_bufPos] += value >> (32 - m_bitPos);
      m_bufPos++;
      buff[m_bufPos] = value << (32 - m_bitPos);
      m_bitPos += bitsToWrite;
      m_bitPos -= 32;
    }
  else {
    tmpval = pow(2, 32 - m_bitPos) - 1;
    buff[m_bufPos] &= (~tmpval);
    buff[m_bufPos] += value << (32 - m_bitPos);
    m_bitPos += bitsToWrite;
    if (m_bitPos == 32) {
      m_bufPos++;
      m_bitPos -= 32;
    }
  }

}








