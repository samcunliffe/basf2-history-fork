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

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <svd/dataobjects/SVDDAQDiagnostic.h>
#include <svd/dataobjects/SVDShaperDigit.h>
#include <svd/dataobjects/SVDCluster.h>

#include <vxd/geometry/SensorInfoBase.h>
#include <vxd/geometry/GeoTools.h>

#include <boost/format.hpp>
#include <string>
#include "TDirectory.h"

#include <TROOT.h>
#include <TStyle.h>
#include <TLine.h>

using namespace std;
using boost::format;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDUnpackerDQM)


//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDUnpackerDQMModule::SVDUnpackerDQMModule() : Module()
{
  //Set module properties
  setDescription("DQM Histograms");

  addParam("histogramDirectoryName", m_histogramDirectoryName, "Name of the directory where histograms will be placed",
           std::string("SVDDQM"));
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDDQMHisto.root"));
  addParam("ShaperDigitsName", m_ShaperDigitName, "Name of ShaperDigit Store Array.", std::string(""));
  addParam("DiagnosticsName", m_SVDDAQDiagnosticsName, "Name of DAQDiagnostics Store Array.", std::string(""));

}


SVDUnpackerDQMModule::~SVDUnpackerDQMModule()
{
}


void SVDUnpackerDQMModule::initialize()
{
  m_eventMetaData.isRequired();
  m_svdShapers.isRequired(m_ShaperDigitName);
  m_svdDAQDiagnostics.isRequired(m_SVDDAQDiagnosticsName);

  //B2INFO("    ShaperDigits: " << m_ShaperDigitName);

  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

}


void SVDUnpackerDQMModule::beginRun()
{

  unsigned short Bins_FTBFlags = 5;
  unsigned short Bins_FTBError = 4;
  unsigned short Bins_APVError = 4;
  unsigned short Bins_APVMatch = 1;
  unsigned short Bins_FADCMatch = 1;
  unsigned short Bins_UpsetAPV = 1;

  nBits = Bins_FTBFlags + Bins_FTBError + Bins_APVError + Bins_APVMatch + Bins_FADCMatch + Bins_UpsetAPV;

  DQMUnpackerHisto = new TH2F("DQMUnpackerHisto", "Monitor SVD Histo", nBits, 1, nBits + 1, 48, 1, 49);

  DQMUnpackerHisto->GetYaxis()->SetTitle("FADC board");
  DQMUnpackerHisto->GetYaxis()->SetTitleOffset(1.2);


  TString Xlables[nBits] = {"EvTooLong", "TimeOut", "doubleHead", "badEvt", "errCRC", "badFADC", "badTTD", "badFTB", "badALL", "errAPV", "errDET", "errFrame", "errFIFO", "APVmatch", "FADCmatch", "upsetAPV"};

  for (unsigned short i = 0; i < nBits; i++) DQMUnpackerHisto->GetXaxis()->SetBinLabel(i + 1, Xlables[i].Data());

  // Just to make sure, reset all the histograms.
  if (DQMUnpackerHisto != NULL) DQMUnpackerHisto->Reset();

}


void SVDUnpackerDQMModule::event()
{

  if (!m_svdDAQDiagnostics || !m_svdDAQDiagnostics.getEntries()) {
    cout << "straszny syff - nie ma Diagnostics" << endl;
    return;
  }

  if (m_eventMetaDataPtr->getEvent() % 1000 == 0) B2INFO("event number: " << m_eventMetaDataPtr->getEvent());

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

    fadcNo = m_svdDAQDiagnostics[i]->getFADCNumber();
    apvNo = m_svdDAQDiagnostics[i]->getAPVNumber();

    // insert FADCnumber into the map (if not already there) and assign the next bin to it.
    if (fadc_map.find(fadcNo) == fadc_map.end())   fadc_map.insert(make_pair(fadcNo, ++bin_no));

    if (ftbFlags != 0 or ftbError != 240 or apvError != 0 or !apvMatch or !fadcMatch or upsetAPV) {
      auto ybin = fadc_map.find(fadcNo);
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
  }

  for (auto& iFADC : fadc_map)  DQMUnpackerHisto->GetYaxis()->SetBinLabel(iFADC.second, to_string(iFADC.first).c_str());

}


void SVDUnpackerDQMModule::endRun()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();

    m_rootFilePtr->Write();
    m_rootFilePtr->Close();
  }
}

void SVDUnpackerDQMModule::terminate()
{
}

