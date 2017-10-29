/******************************************************************************
* BASF2 (Belle Analysis Framework 2)                                          *
* Copyright(C) 2010 - Belle II Collaboration                                  *
*                                                                             *
* Author: The Belle II Collaboration                                          *
* Contributors: Jarek Wiechczynski, Peter Kvasnicka                           *
*               Giulia Casarosa, Eugenio Paoloni                              *
*                                                                             *
* This software is provided "as is" without any warranty.                     *
* Beware! Do not expose to open flames it can explode                         *
******************************************************************************/

#include <svd/modules/svdUnpacker/SVDUnpackerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

#include <arpa/inet.h>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::augmented_crc
#include <boost/cstdint.hpp>  // for boost::uint16_t
#define CRC16POLYREV 0x8005   // CRC-16 polynomial, normal representation 

#include <sstream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <set>
#include <map>

using namespace std;
using namespace Belle2;
using namespace Belle2::SVD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpacker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDUnpackerModule::SVDUnpackerModule() : Module(),
  m_generateShaperDigts(false),
  m_shutUpFTBError(0),
  m_FADCTriggerNumberOffset(0)
{
  //Set module properties
  setDescription("Produce SVDDigits from RawSVD. NOTE: only zero-suppressed mode is currently supported!");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rawSVDListName", m_rawSVDListName, "Name of the raw SVD List", string(""));
  addParam("svdDigitListName", m_svdDigitListName, "Name of the SVD Digits List", string(""));
  addParam("GenerateShaperDigts", m_generateShaperDigts, "Generate SVDShaperDigits", bool(false));
  addParam("svdShaperDigitListName", m_svdShaperDigitListName, "Name of the SVDShaperDigits list", string(""));
  addParam("xmlMapFileName", m_xmlMapFileName, "path+name of the xml file", FileSystem::findFile("data/svd/svd_mapping.xml"));
  addParam("shutUpFTBError", m_shutUpFTBError,
           "if >0 is the number of reported FTB header ERRORs before quiet operations. If <0 full log produced.", -1);
  addParam("FADCTriggerNumberOffset", m_FADCTriggerNumberOffset,
           "number to be added to the FADC trigger number to match the main trigger number", 0);
  addParam("svdDAQDiagnosticsListName", m_svdDAQDiagnosticsListName, "Name of the DAQDiagnostics  list", string(""));

}

SVDUnpackerModule::~SVDUnpackerModule()
{
}

void SVDUnpackerModule::initialize()
{
  m_eventMetaDataPtr.required();
  StoreArray<RawSVD>::required(m_rawSVDListName);
  StoreArray<SVDDigit>::registerPersistent(m_svdDigitListName);
  //StoreArray<SVDDAQDiagnostic>::registerPersistent(m_svdDAQDiagnosticsListName);
  StoreArray<SVDDAQDiagnostic> storeDAQDiagnostics(m_svdDAQDiagnosticsListName);
  storeDAQDiagnostics.registerInDataStore();
  m_svdDAQDiagnosticsListName = storeDAQDiagnostics.getName();

  if (m_generateShaperDigts) {
    StoreArray<SVDShaperDigit> storeShaperDigits(m_svdShaperDigitListName);
    storeShaperDigits.registerInDataStore();
    storeShaperDigits.registerRelationTo(storeDAQDiagnostics);
    m_svdShaperDigitListName = storeShaperDigits.getName();
  }

  loadMap();
}

void SVDUnpackerModule::beginRun()
{
  m_wrongFTBcrc = 0;

}

#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#endif
void SVDUnpackerModule::event()
{
  StoreArray<RawSVD> rawSVDList(m_rawSVDListName);
  StoreArray<SVDDigit> svdDigits(m_svdDigitListName);
  StoreArray<SVDShaperDigit> shaperDigits(m_svdShaperDigitListName);
  StoreArray<SVDDAQDiagnostic> DAQDiagnostics(m_svdDAQDiagnosticsListName); //JW

  vector<SVDDAQDiagnostic> diagnosticVector;
  int diagnosticIndex;
  map<int, set<SVDShaperDigit>> diagnosticMap;

  if (!m_eventMetaDataPtr.isValid()) {  // give up...
    B2ERROR("Missing valid EventMetaData." << std::endl <<
            "No SVDDigit produced for this event");
    return;
  }

  svdDigits.clear();

  if (! m_map) { //give up
    B2ERROR("SVD xml map not loaded." << std::endl <<
            "No SVDDigit produced for this event");
    return;
  }

  unsigned int nEntries_rawSVD = rawSVDList.getEntries();
  for (unsigned int i = 0; i < nEntries_rawSVD; i++) {

    unsigned int numEntries_rawSVD = rawSVDList[ i ]->GetNumEntries();
    for (unsigned int j = 0; j < numEntries_rawSVD; j++) {

      unsigned short nWords[4];
      nWords[0] = rawSVDList[i]->Get1stDetectorNwords(j);
      nWords[1] = rawSVDList[i]->Get2ndDetectorNwords(j);
      nWords[2] = rawSVDList[i]->Get3rdDetectorNwords(j);
      nWords[3] = rawSVDList[i]->Get4thDetectorNwords(j);

      // i,j is only 0

      uint32_t* data32tab[4]; //vector of pointers

      data32tab[0] = (uint32_t*)rawSVDList[i]->Get1stDetectorBuffer(j); // points at the begining of the 1st buffer
      data32tab[1] = (uint32_t*)rawSVDList[i]->Get2ndDetectorBuffer(j);
      data32tab[2] = (uint32_t*)rawSVDList[i]->Get3rdDetectorBuffer(j);
      data32tab[3] = (uint32_t*)rawSVDList[i]->Get4thDetectorBuffer(j);


      unsigned short ftbError = 0; // JW - gdzie to dac?
      unsigned short trgType = 0;
      unsigned short trgNumber = 0;
      unsigned short cmc1;
      unsigned short cmc2;
      unsigned short apvErrors;
      unsigned short pipAddr;
      unsigned short ftbFlags;
      unsigned short emuPipAddr;
      unsigned short apvErrorsOR;

      for (unsigned int buf = 0; buf < 4; buf++) { // loop over 4 buffers

        //printB2Debug(data32tab[buf], data32tab[buf], &data32tab[buf][nWords[buf] - 1], nWords[buf]);

        uint32_t* data32_it = data32tab[buf];
        short fadc = 255, apv = 63, strip, sample[6];
        vector<uint32_t> crc16vec;


        for (; data32_it != &data32tab[buf][nWords[buf]]; data32_it++) {
          m_data32 = *data32_it; //put current 32-bit frame to union

          if (m_data32 == 0xffaa0000) {   // first part of FTB header
            diagnosticMap.clear(); // new set of objects for the current FTB
            diagnosticVector.clear();
            diagnosticIndex = -1;
            crc16vec.clear(); // clear the input container for crc16 calculation
            crc16vec.push_back(m_data32);
            data32_it++; // go to 2nd part of FTB header
            crc16vec.push_back(*data32_it);

            m_data32 = *data32_it; //put the second 32-bit frame to union

            ftbError = m_FTBHeader.errorsField; // JW

            if (m_FTBHeader.eventNumber !=
                (m_eventMetaDataPtr->getEvent() & 0xFFFFFF)) {
              if (m_shutUpFTBError) { //
                m_shutUpFTBError -= 1 ;
                B2ERROR(
                  "Trigger number mismatch." << std::endl <<
                  "Expected trigger number & 0xFFFFFF   = 0x" <<
                  std::hex  <<
                  (m_eventMetaDataPtr->getEvent() & 0xFFFFFF) <<
                  std::endl <<
                  "Trigger number in the FTB            = 0x" <<
                  std::hex <<
                  m_FTBHeader.eventNumber);
              }
            }

            if (m_FTBHeader.errorsField != 0) {
              if (m_shutUpFTBError) {
                m_shutUpFTBError -= 1 ;
                B2ERROR(
                  "Error on SVD FTB : 0x" << std::hex <<
                  m_FTBHeader.errorsField
                );
              }
            }
            continue;
          }


          crc16vec.push_back(m_data32);


          if (m_MainHeader.check == 6) { // FADC header
            fadc = m_MainHeader.FADCnum;
            trgType = m_MainHeader.trgType;       //JW
            trgNumber = m_MainHeader.trgNumber;   //JW
            if (
              m_MainHeader.trgNumber !=
              ((m_eventMetaDataPtr->getEvent() - m_FADCTriggerNumberOffset) & 0xFF)) {
              B2ERROR(" On event number: " << m_eventMetaDataPtr->getEvent() <<
                      std::endl <<
                      " Found a wrong FTB header of the SVD FADC " <<
                      std::endl <<
                      " FADC: " << fadc << std::endl <<
                      " Trigger number LSByte reported by the FADC: " <<
                      m_MainHeader.trgNumber << " + offset " <<
                      m_FADCTriggerNumberOffset <<
                      std::endl <<
                      " expected: " << (m_eventMetaDataPtr->getEvent() & 0xFF)
                     );
            }

            if (m_generateShaperDigts) { // create SVDModeByte object from MainHeader vars
              //B2INFO("Filling SVDModeByte object");
              m_SVDModeByte = SVDModeByte(m_MainHeader.runType, m_MainHeader.evtType, m_MainHeader.DAQMode, m_MainHeader.trgTiming);
            }

          }

          if (m_APVHeader.check == 2) { // APV header
            apv = m_APVHeader.APVnum;

            cmc1 = m_APVHeader.CMC1;  // odtad JW
            cmc2 = m_APVHeader.CMC2;
            apvErrors = m_APVHeader.apvErr;// temp - tylko 3 bity z 4-ech
            pipAddr = m_APVHeader.pipelineAddr;

            // JW. tutaj wstępnie wypełniam DAQDiagnostic, ale bez trailera
            // wrzucanie do wektora map
            diagnosticVector.push_back(
              SVDDAQDiagnostic(trgNumber, trgType, pipAddr, cmc1, cmc2, apvErrors, ftbError)
            );
            diagnosticMap[++diagnosticIndex] = set<SVDShaperDigit>();
          }

          if (m_data_A.check == 0) { // data
            strip = m_data_A.stripNum;

            sample[0] = m_data_A.sample1;
            sample[1] = m_data_A.sample2;
            sample[2] = m_data_A.sample3;

            data32_it++;
            m_data32 = *data32_it; // 2nd frame with data
            crc16vec.push_back(m_data32);

            sample[3] = m_data_B.sample4;
            sample[4] = m_data_B.sample5;
            sample[5] = m_data_B.sample6;


            for (unsigned int idat = 0; idat < 6; idat++) {
              // m_cellPosition member of the SVDDigit object is set to zero by NewDigit function
              SVDDigit* newDigit = m_map->NewDigit(fadc, apv, strip, sample[idat], idat);
              svdDigits.appendNew(*newDigit);

              delete newDigit;
            }

            if (m_generateShaperDigts) {
              //B2INFO("Generating SVDShaperDigit object");
              SVDShaperDigit* newShaperDigit = m_map->NewShaperDigit(fadc, apv, strip, sample, 0.0, m_SVDModeByte);
              diagnosticMap[diagnosticIndex].insert(*newShaperDigit);
              delete newShaperDigit;
            }

          }  //is data frame


          // JW - nowy blok dla trailera FADC
          if (m_FADCTrailer.check == 14)  { // FADC trailer

            ftbFlags = m_FADCTrailer.FTBFlags;
            emuPipAddr = m_FADCTrailer.emuPipeAddr;
            apvErrorsOR = m_FADCTrailer.apvErrOR;

            for (auto& pair : diagnosticMap) {
              SVDDAQDiagnostic& finalDAQDiagnostic = diagnosticVector[pair.first];

              finalDAQDiagnostic.setFTBFlags(ftbFlags);
              finalDAQDiagnostic.setEmuPipelineAddress(emuPipAddr);
              finalDAQDiagnostic.setApvErrorOR(apvErrorsOR);

              DAQDiagnostics.appendNew(finalDAQDiagnostic); // FIXME, causes the following error:
              /*** Error in `basf2': malloc(): memory corruption: 0x000000000eb1b960 ***
              ======= Backtrace: =========
              /lib/x86_64-linux-gnu/libc.so.6(+0x777e5)[0x7f6b2e7dd7e5]
              /lib/x86_64-linux-gnu/libc.so.6(+0x8213e)[0x7f6b2e7e813e]
              /lib/x86_64-linux-gnu/libc.so.6(__libc_malloc+0x54)[0x7f6b2e7ea184]
              /externals/v01-05-01/Linux_x86_64/common/lib64/libstdc++.so.6(_Znwm+0x18)[0x7f6b2edd49a8]
              /externals/v01-05-01/Linux_x86_64/opt/root/lib/libCore.so(_ZN8TStorage11ObjectAllocEm+0x9)[0x7f6b2e353809]
              /externals/v01-05-01/Linux_x86_64/opt/root/lib/libCore.so(_ZN12TClonesArrayixEi+0xf9)[0x7f6b2e396c19]
              /software/modules/Linux_x86_64/opt/libsvdUnpacker.so(_ZN6Belle23SVD17SVDUnpackerModule5eventEv+0x122f)[0x7f6afae0d74f]
              ...*/

              for (auto& d : pair.second) {
                shaperDigits.appendNew(d)->addRelationTo(&finalDAQDiagnostic);
              }
            }


          }// FADC trailer

          if (m_FTBTrailer.controlWord == 0xff55)  {// FTB trailer

            //check CRC16
            crc16vec.pop_back();
            unsigned short iCRC = crc16vec.size();
            //uint32_t *crc16input = new uint32_t[iCRC];
            uint32_t crc16input[iCRC];

            for (unsigned short icrc = 0; icrc < iCRC; icrc++)
              crc16input[icrc] = htonl(crc16vec.at(icrc));

            //verify CRC16
            boost::crc_basic<16> bcrc(0x8005, 0xffff, 0, false, false);
            bcrc.process_block(crc16input, crc16input + iCRC);
            unsigned int checkCRC = bcrc.checksum();

            if (checkCRC != m_FTBTrailer.crc16) {
              B2WARNING("FTB CRC16 checksum DOES NOT MATCH for FADC no. " << fadc);
              m_wrongFTBcrc++;
            }

          } // FTB trailer

        } // end loop over 32-bit frames in each buffer

      } // end iteration on 4 data buffers

      //m_runType = 0;

    } // end event loop

  }

} //end event function
#ifndef __clang__
#pragma GCC diagnostic pop
#endif

void SVDUnpackerModule::endRun()
{
  B2INFO("   m_wrongFTBcrc = " << m_wrongFTBcrc);
}


//load the sensor MAP from xml file
void SVDUnpackerModule::loadMap()
{
  m_map = unique_ptr<SVDOnlineToOfflineMap>(new SVDOnlineToOfflineMap(m_xmlMapFileName));
}


// additional printing function
void SVDUnpackerModule::printB2Debug(uint32_t* data32, uint32_t* data32_min, uint32_t* data32_max, int nWords)
{

  uint32_t* min = std::max((data32 - nWords), data32_min);
  uint32_t* max = std::min((data32 + nWords), data32_max);

  uint32_t* ptr = min;
  int counter = 0;

  char message[256] = "";
  ostringstream os;
  os << endl;

  while (ptr < max + 1) {
    char prev_message[256] = "";
    strcpy(prev_message, message);
    sprintf(message, "%s%.8x ", prev_message, *ptr);
    if (counter++ % 10 == 9) {
      os << message << endl;
      //sprintf(message,"");
      strcpy(message, "");
    }

    ptr++;
  }

  os << message << endl;
  //B2DEBUG(1, os.str());
  B2INFO(os.str());
  return;

}
