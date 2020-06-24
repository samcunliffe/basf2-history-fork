/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jarek Wiechczynski                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "svd/modules/svdDQM/SVDUnpackerDQMModule.h"
#include <svd/online/SVDOnlineToOfflineMap.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <svd/dataobjects/SVDDAQDiagnostic.h>

#include <TDirectory.h>
#include <TLine.h>
#include <TStyle.h>

#include <algorithm>
#include <string>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpackerDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

std::string SVDUnpackerDQMModule::m_xmlFileName = std::string("SVDChannelMapping.xml");

SVDUnpackerDQMModule::SVDUnpackerDQMModule() : HistoModule(), m_mapping(m_xmlFileName), FADCs(NULL)
{
  //Set module properties
  setDescription("DQM Histogram for the SVD Unpacker");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDUnpacker"));
  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
  addParam("DiagnosticsName", m_SVDDAQDiagnosticsName, "Name of DAQDiagnostics Store Array.", std::string(""));

  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
}


SVDUnpackerDQMModule::~SVDUnpackerDQMModule()
{
}

//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------

void SVDUnpackerDQMModule::defineHisto()
{

  // Create a separate histogram directories and cd into it.
  TDirectory* oldDir = gDirectory;
  if (m_histogramDirectoryName != "") {
    oldDir->mkdir(m_histogramDirectoryName.c_str());// do not use return value with ->cd(), its ZERO if dir already exists
    oldDir->cd(m_histogramDirectoryName.c_str());
  }

  const unsigned short Bins_FTBFlags = 5;
  const unsigned short Bins_FTBError = 4;
  const unsigned short Bins_APVError = 4;
  const unsigned short Bins_APVMatch = 1;
  const unsigned short Bins_FADCMatch = 1;
  const unsigned short Bins_UpsetAPV = 1;
  const unsigned short Bins_BadMapping = 1;
  const unsigned short Bins_BadHeader = 1;
  const unsigned short Bins_MissedTrailer = 1;
  const unsigned short Bins_MissedHeader = 1;


  const unsigned short nBits = Bins_FTBFlags + Bins_FTBError + Bins_APVError + Bins_APVMatch + Bins_FADCMatch + Bins_UpsetAPV +
                               Bins_BadMapping + Bins_BadHeader + Bins_MissedTrailer + Bins_MissedHeader;

  DQMUnpackerHisto = new TH2S("DQMUnpackerHisto", "SVD Data Format Monitor", nBits, 1, nBits + 1, 52, 1, 53);
  DQMEventFractionHisto = new TH1S("DQMEventFractionHisto", "SVD Error Fraction Event Counter", 2, 0, 2);

  DQMUnpackerHisto->GetYaxis()->SetTitle("FADC board");
  DQMUnpackerHisto->GetYaxis()->SetTitleOffset(1.2);

  DQMEventFractionHisto->GetYaxis()->SetTitle("# of Events");
  DQMEventFractionHisto->GetYaxis()->SetTitleOffset(1.5);
  DQMEventFractionHisto->SetMinimum(0);

  TString Xlabels[nBits] = {"EvTooLong", "TimeOut", "doubleHead", "badEvt", "errCRC", "badFADC", "badTTD", "badFTB", "badALL", "errAPV", "errDET", "errFrame", "errFIFO", "APVmatch", "FADCmatch", "upsetAPV", "EVTmatch", "missHead", "missTrail", "badMapping"};


  //preparing X axis of the histograms
  for (unsigned short i = 0; i < nBits; i++) DQMUnpackerHisto->GetXaxis()->SetBinLabel(i + 1, Xlabels[i].Data());

  DQMEventFractionHisto->GetXaxis()->SetBinLabel(1, "OK");
  DQMEventFractionHisto->GetXaxis()->SetBinLabel(2, "Error(s)");


  oldDir->cd();
}

void SVDUnpackerDQMModule::initialize()
{
  m_eventMetaData.isRequired();
  m_svdDAQDiagnostics.isOptional(m_SVDDAQDiagnosticsName);

  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}


void SVDUnpackerDQMModule::beginRun()
{

  StoreObjPtr<EventMetaData> evtMetaData;
  expNumber = evtMetaData->getExperiment();
  runNumber = evtMetaData->getRun();
  errorFraction = 0;

  TString histoTitle = TString::Format("SVD Data Format Monitor, Exp %d Run %d", expNumber, runNumber);

  if (DQMUnpackerHisto != NULL) {
    DQMUnpackerHisto->Reset();
    DQMUnpackerHisto->SetTitle(histoTitle.Data());
  }

  if (DQMEventFractionHisto != NULL) {
    DQMEventFractionHisto->Reset();
  }

  shutUpNoData = false;

  if (m_mapping.hasChanged()) { m_map = std::make_unique<SVDOnlineToOfflineMap>(m_mapping->getFileName()); }

  changeFADCaxis = false;

  //getting fadc numbers from the mapping
  FADCs = &(m_map->FADCnumbers);

  //copy FADC numbers to vector and sort them
  vec_fadc.insert(vec_fadc.end(), FADCs->begin(), FADCs->end());
  std::sort(vec_fadc.begin(), vec_fadc.end());

  unsigned short ifadc = 0;
  for (const auto& fadc : vec_fadc) {
    fadc_map.insert(make_pair(fadc, ++ifadc));
    DQMUnpackerHisto->GetYaxis()->SetBinLabel(ifadc, to_string(fadc).c_str());
  }

  nEvents = 0;
  nBadEvents = 0;
}


void SVDUnpackerDQMModule::event()
{
  if (!m_svdDAQDiagnostics.isValid() && (!shutUpNoData)) {
    B2WARNING("There are no SVDDAQDiagnostic objects saved by the Unpacker! SVD Data Format Monitoring disabled!");
    shutUpNoData = true;
    return;
  }

  badEvent = 0;
  nEvents++;

  unsigned int nDiagnostics = m_svdDAQDiagnostics.getEntries();

  unsigned short bin_no = 0;
  gStyle->SetOptStat(0);

  for (unsigned short i = 0; i < nDiagnostics; i++) {

    ftbFlags = m_svdDAQDiagnostics[i]->getFTBFlags();
    ftbError = m_svdDAQDiagnostics[i]->getFTBError();
    apvError = m_svdDAQDiagnostics[i]->getAPVError();
    apvMatch = m_svdDAQDiagnostics[i]->getAPVMatch();
    fadcMatch = m_svdDAQDiagnostics[i]->getFADCMatch();
    upsetAPV = m_svdDAQDiagnostics[i]->getUpsetAPV();
    badMapping = m_svdDAQDiagnostics[i]->getBadMapping();
    badHeader = m_svdDAQDiagnostics[i]->getBadHeader();
    badTrailer = m_svdDAQDiagnostics[i]->getBadTrailer();
    missedHeader = m_svdDAQDiagnostics[i]->getMissedHeader();
    missedTrailer = m_svdDAQDiagnostics[i]->getMissedTrailer();

    fadcNo = m_svdDAQDiagnostics[i]->getFADCNumber();
    //apvNo = m_svdDAQDiagnostics[i]->getAPVNumber();

    // insert FADCnumber into the map (if not already there) and assign the next bin to it.
    if (changeFADCaxis) {
      if (fadc_map.find(fadcNo) == fadc_map.end())   fadc_map.insert(make_pair(fadcNo, ++bin_no));
    }

    if (ftbFlags != 0 or ftbError != 240 or apvError != 0 or !apvMatch or !fadcMatch or upsetAPV or badMapping or badHeader
        or badTrailer or missedHeader or missedTrailer) {

      badEvent = 1;

      auto ybin = fadc_map.find(fadcNo);

      if (badMapping)  {
        if (!changeFADCaxis) {
          changeFADCaxis = true;
          fadc_map.clear();
          break;
        } else {
          DQMUnpackerHisto->Fill(20, ybin->second);
        }
      }

      if (badHeader) DQMUnpackerHisto->Fill(17, ybin->second);
      if (missedHeader) DQMUnpackerHisto->Fill(18, ybin->second);
      if (badTrailer or missedTrailer) DQMUnpackerHisto->Fill(19, ybin->second);

      if (ftbFlags != 0) {
        if (ftbFlags & 16) DQMUnpackerHisto->Fill(5, ybin->second);
        if (ftbFlags & 8) DQMUnpackerHisto->Fill(4, ybin->second);
        if (ftbFlags & 4) DQMUnpackerHisto->Fill(3, ybin->second);
        if (ftbFlags & 2) DQMUnpackerHisto->Fill(2, ybin->second);
        if (ftbFlags & 1) DQMUnpackerHisto->Fill(1, ybin->second);
      }
      if (ftbError != 240) {
        switch (ftbError - 240) {
          case 3:
            DQMUnpackerHisto->Fill(6, ybin->second); //badFADC
            break;
          case 5:
            DQMUnpackerHisto->Fill(7, ybin->second); //badTTD
            break;
          case 6:
            DQMUnpackerHisto->Fill(8, ybin->second); //badFTB
            break;
          case 7:
            DQMUnpackerHisto->Fill(9, ybin->second);//badALL;
            break;
        }
      }

      if (apvError != 0) {
        if (apvError & 1) DQMUnpackerHisto->Fill(10, ybin->second);
        if (apvError & 2) DQMUnpackerHisto->Fill(11, ybin->second);
        if (apvError & 4) DQMUnpackerHisto->Fill(12, ybin->second);
        if (apvError & 8) DQMUnpackerHisto->Fill(13, ybin->second);
      }

      if (!apvMatch) DQMUnpackerHisto->Fill(14, ybin->second);
      if (!fadcMatch) DQMUnpackerHisto->Fill(15, ybin->second);
      if (upsetAPV) DQMUnpackerHisto->Fill(16, ybin->second);

    }
  } //end Diagnostics loop

  if (changeFADCaxis) {
    for (auto& iFADC : fadc_map)  DQMUnpackerHisto->GetYaxis()->SetBinLabel(iFADC.second, to_string(iFADC.first).c_str());
  }
  if (badEvent) nBadEvents++;
  errorFraction = 100 * float(nBadEvents) / float(nEvents);

  if (DQMEventFractionHisto != NULL) {
    TString histoFractionTitle = TString::Format("SVD bad events fraction: %f %%,  Exp %d Run %d", errorFraction, expNumber, runNumber);
    DQMEventFractionHisto->SetTitle(histoFractionTitle.Data());
  }


  DQMEventFractionHisto->Fill(badEvent);

} // end event function


void SVDUnpackerDQMModule::endRun()
{
  // Summary report on SVD DQM monitor
  if (nBadEvents) {
    B2WARNING("=================== SVD DQM Data Format Statistics: =============");
    B2WARNING(" We found " << nBadEvents << "/" << nEvents << " corrupted events, which is " << errorFraction <<
              "%");
    B2WARNING("=================================================================");
  }

}
