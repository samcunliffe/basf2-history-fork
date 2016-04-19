/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/microtpc/modules/MicrotpcDailyReportModule.h>
#include <beast/microtpc/dataobjects/MicrotpcSimHit.h>
#include <beast/microtpc/dataobjects/MicrotpcHit.h>
#include <beast/microtpc/dataobjects/MicrotpcMetaHit.h>
#include <beast/microtpc/dataobjects/MicrotpcDataHit.h>
#include <beast/microtpc/dataobjects/MicrotpcRecoTrack.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Gearbox.h>
#include <framework/gearbox/GearDir.h>
#include <cmath>
#include <boost/foreach.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TH3.h>
#include <TFile.h>
#include <TMath.h>
#include <TDatime.h>

using namespace std;

using namespace Belle2;
using namespace microtpc;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MicrotpcDailyReport)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MicrotpcDailyReportModule::MicrotpcDailyReportModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Microtpcs (BEAST)");

  //Default values are set here.
  addParam("inputTPCNumber", m_inputTPCNumber,
           "TPC number readout", 3);
  //Date of the report to offset time stamp to zero.
  addParam("inputReportDate", m_inputReportDate, "Date of the report", 20160201);

}

MicrotpcDailyReportModule::~MicrotpcDailyReportModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void MicrotpcDailyReportModule::defineHisto()
{
  for (int i = 0; i < 2; i ++) {
    h_tpc_uptime[i] = new TH1F(TString::Format("h_tpc_uptime_%d", i), "", 3, 0., 3.);
  }
  for (int i = 0; i < 9; i ++) {
    h_tpc_rate[i] = new TH1F(TString::Format("h_tpc_rate_%d", i), "", 5000, 0., 24.);
    h_tpc_gain[i] = new TH2F(TString::Format("h_tpc_gain_%d", i), "", 1000, 0., 24., 200, 0., 2000.);
    h_tpc_triglength[i] = new TH1F(TString::Format("h_tpc_triglength_%d", i), "", 256, 0., 256.);
    h_tpc_phivtheta[i] = new TH2F(TString::Format("h_tpc_phivtheta_%d", i), "", 360, -180., 180., 180, 0., 180.);
    h_tpc_phivtheta_w[i] = new TH2F(TString::Format("h_tpc_phivtheta_w_%d", i), "",  360, -180., 180., 180, 0., 180.);
    h_tpc_edepvtrl[i] = new TH2F(TString::Format(" h_tpc_edepvtrl_%d", i), "", 2000, 0., 4000., 1000, 0., 3.);
    h_tpc_rate[i]->Sumw2();
    h_tpc_gain[i]->Sumw2();
    h_tpc_triglength[i]->Sumw2();
    h_tpc_phivtheta[i]->Sumw2();
    h_tpc_phivtheta_w[i]->Sumw2();
    h_tpc_edepvtrl[i]->Sumw2();
  }
}


void MicrotpcDailyReportModule::initialize()
{
  B2INFO("MicrotpcDailyReportModule: Initialize");

  REG_HISTOGRAM

}

void MicrotpcDailyReportModule::beginRun()
{
}

void MicrotpcDailyReportModule::event()
{
  //Here comes the actual event processing

  StoreArray<MicrotpcMetaHit> MetaHits;
  StoreArray<MicrotpcRecoTrack> Tracks;


  double TimeStamp = 0;
  for (const auto& MetaHit : MetaHits) {
    TimeStamp = MetaHit.getts_start()[0];
  }
  if (TimeStamp > 1) {
    TimeStamp -= TDatime(m_inputReportDate, 0).Convert();
    TimeStamp /= (60. * 60.);
  } else {
    TimeStamp = 0;
  }

  for (const auto& aTrack : Tracks) { // start loop over all Tracks
    //int detNb = aTrack->getdetNb();
    const double phi = aTrack.getphi();
    const double theta = aTrack.gettheta();
    const double trl = aTrack.gettrl();
    const double esum = aTrack.getesum();
    const int time_range = aTrack.gettime_range();
    int partID[7];
    partID[0] = 1; //[0] for all events
    for (int j = 0; j < 6; j++) partID[j + 1] = aTrack.getpartID()[j];
    h_tpc_uptime[0]->Fill(1);
    if (TimeStamp > 0.) {
      h_tpc_uptime[1]->Fill(1);
      for (int j = 0; j < 7; j++) {
        if (partID[j] == 1) {
          h_tpc_rate[j]->Fill(TimeStamp);
          h_tpc_gain[j]->Fill(TimeStamp, esum);
          h_tpc_triglength[j]->Fill(time_range);
          h_tpc_phivtheta[j]->Fill(phi, theta);
          h_tpc_phivtheta_w[j]->Fill(phi, theta, esum);
          h_tpc_edepvtrl[j]->Fill(esum, trl);
        }
      }
    }
    //h_tpc_rate[8]->Fill(TimeStamp);
    //h_tpc_gain[8]->Fill(TimeStamp, esum);
    h_tpc_triglength[8]->Fill(time_range);
    h_tpc_phivtheta[8]->Fill(phi, theta);
    h_tpc_phivtheta_w[8]->Fill(phi, theta, esum);
    h_tpc_edepvtrl[8]->Fill(esum, trl);
  }// end loop over all Tracks

}
void MicrotpcDailyReportModule::endRun()
{
}

void MicrotpcDailyReportModule::terminate()
{
  cout << "terminate section" << endl;
  double LifeTime = h_tpc_uptime[1]->GetMaximum();
  for (int i = 0; i < 7; i++) {
    int Nbin = h_tpc_rate[i]->GetNbinsX();
    double von = h_tpc_rate[i]->GetXaxis()->GetXmin();
    double bis = h_tpc_rate[i]->GetXaxis()->GetXmax();
    double rate = (bis - von) / (double)Nbin;
    h_tpc_rate[i]->Scale(1. / rate / 60. / 60.);
    Nbin = h_tpc_gain[i]->GetNbinsX();
    von = h_tpc_gain[i]->GetXaxis()->GetXmin();
    bis = h_tpc_gain[i]->GetXaxis()->GetXmax();
    rate = (bis - von) / (double)Nbin;
    h_tpc_gain[i]->Scale(1. / rate / 60. / 60.);

    h_tpc_triglength[i]->Scale(1. / LifeTime);
    h_tpc_phivtheta[i]->Scale(1. / LifeTime);
    h_tpc_phivtheta_w[i]->Scale(1. / LifeTime);
    h_tpc_edepvtrl[i]->Scale(1. / LifeTime);
  }
  cout << "TPC #" << m_inputTPCNumber << " daily report: event with time stamp " << h_tpc_uptime[1]->GetMaximum() /
       h_tpc_uptime[0]->GetMaximum() * 100 <<
       " % of the time " << endl;
  cout << "TPC #" << m_inputTPCNumber << " daily report: detector was up " << LifeTime / 60. / 60. / 24. / 24. * 100. <<
       " % of the time" << endl;
}


