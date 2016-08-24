/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/pindiode/modules/PindiodeStudyModule.h>
#include <beast/pindiode/dataobjects/PindiodeSimHit.h>
#include <beast/pindiode/dataobjects/PindiodeHit.h>
#include <generators/SAD/dataobjects/SADMetaHit.h>
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

using namespace std;

using namespace Belle2;
using namespace pindiode;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PindiodeStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PindiodeStudyModule::PindiodeStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for Pindiodes (BEAST)");

  //Default values are set here. New values can be in PINDIODE.xml.
  addParam("CrematGain", m_CrematGain, "Charge sensitive preamplifier gain [volts/C] ", 1.4);
  addParam("WorkFunction", m_WorkFunction, "Convert eV to e [e/eV] ", 1.12);
  addParam("FanoFactor", m_FanoFactor, "e resolution ", 0.1);
}

PindiodeStudyModule::~PindiodeStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void PindiodeStudyModule::defineHisto()
{
  //Default values are set here. New values can be in PINDIODE.xml.
  for (int i = 0; i < 4; i++) {
    h_pin_rate[i] = new TH1F(TString::Format("h_pin_rate_%d", i), "Energy deposited [MeV]", 100, 0., 100.);
    h_pin_rate[i]->Sumw2();
  }
  for (int i = 0; i < 100; i++) {
    h_dose1[i] = new TH1F(TString::Format("h_dose1_%d", i), "", 10000, 0., 10000.);
    h_dose2[i] = new TH1F(TString::Format("h_dose2_%d", i), "", 10000, 0., 10000.);
    h_dose1Weight[i] = new TH1F(TString::Format("h_dose1Weight_%d", i), "", 10000, 0., 10000.);
    h_dose2Weight[i] = new TH1F(TString::Format("h_dose2Weight_%d", i), "", 10000, 0., 10000.);
    h_volt[i] = new TH1F(TString::Format("h_volt_%d", i), "", 10000, 0., 100.);
    h_time[i] = new TH1F(TString::Format("h_time_%d", i), "", 1000, 0., 100.);
    h_vtime[i] = new TH1F(TString::Format("h_vtime_%d", i), "", 1000, 0., 100.);

    h_idose[i] = new TH1F(TString::Format("h_idose_%d", i), "", 10000, 0., 10000.);
    h_idoseWeight[i] = new TH1F(TString::Format("h_idoseWeight_%d", i), "", 10000, 0., 10000.);
    h_ivolt[i] = new TH1F(TString::Format("h_ivolt_%d", i), "", 10000, 0., 100.);
    h_itime[i] = new TH1F(TString::Format("h_itime_%d", i), "", 1000, 0., 100.);
    h_ivtime[i] = new TH1F(TString::Format("h_ivtime_%d", i), "", 1000, 0., 100.);

    h_dose1[i]->Sumw2();
    h_dose2[i]->Sumw2();
    h_dose1Weight[i]->Sumw2();
    h_dose2Weight[i]->Sumw2();
    h_idose[i]->Sumw2();
    h_idoseWeight[i]->Sumw2();
  }

}


void PindiodeStudyModule::initialize()
{
  B2INFO("PindiodeStudyModule: Initialize");

  //read pindiode xml file
  getXMLData();

  REG_HISTOGRAM

}

void PindiodeStudyModule::beginRun()
{
}

void PindiodeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<PindiodeSimHit>  SimHits;
  StoreArray<PindiodeHit> Hits;
  StoreArray<SADMetaHit> sadMetaHits;
  double rate = 0;
  for (const auto& sadMetaHit : sadMetaHits) {
    rate = sadMetaHit.getrate();
  }

  //Skip events with no Hits
  if (SimHits.getEntries() == 0) {
    return;
  }

  int nSimHits = SimHits.getEntries();
  for (int i = 0; i < nSimHits; i++) {
    PindiodeSimHit* aHit = SimHits[i];
    int detNb = aHit->getCellId();
    if (detNb < 64) {
      double edep = aHit->getEnergyDep();
      double time = aHit->getFlightTime();
      //int PDG = aHit->getPDGCode();
      const double meanEl = edep / m_WorkFunction * 1e9; //GeV to eV
      const double sigma = sqrt(m_FanoFactor * meanEl); //sigma in electron
      const int NbEle = (int)gRandom->Gaus(meanEl, sigma); //electron number
      double volt = NbEle * 1.602176565e-19 * m_CrematGain * 1e12; // volt
      h_dose1[detNb]->Fill(edep * 1e6); //GeV to keV
      h_dose1Weight[detNb]->Fill(edep * 1e6, rate); //GeV to keV
      if ((edep * 1e9) > m_WorkFunction) {
        h_dose2[detNb]->Fill(edep * 1e6); //GeV to keV
        h_dose2Weight[detNb]->Fill(edep * 1e6, rate); //GeV to keV
        h_volt[detNb]->Fill(volt * 1e3); //V to mV
        h_time[detNb]->Fill(time);
        h_vtime[detNb]->Fill(time, volt);
        h_pin_rate[0]->Fill(detNb);
        h_pin_rate[1]->Fill(detNb, rate);
      }
    }
  }
  int nHits = Hits.getEntries();
  for (int i = 0; i < nHits; i++) {
    PindiodeHit* aHit = Hits[i];
    int detNb = aHit->getdetNb();
    if (detNb < 64) {
      double edep = aHit->getedep();
      double volt = aHit->getV();
      double time = aHit->gettime();
      h_idose[detNb]->Fill(edep); //keV
      h_idoseWeight[detNb]->Fill(edep, rate); //keV
      h_ivolt[detNb]->Fill(volt * 1e3); //V to mV
      h_itime[detNb]->Fill(time);
      h_ivtime[detNb]->Fill(time, volt);
      h_pin_rate[2]->Fill(detNb);
      h_pin_rate[3]->Fill(detNb, rate);
    }
  }


}
//read tube centers, impulse response, and garfield drift data filename from PINDIODE.xml
void PindiodeStudyModule::getXMLData()
{
  GearDir content = GearDir("/Detector/DetectorComponent[@name=\"PINDIODE\"]/Content/");
  /*
  //get the location of the tubes
  BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

    PINCenter.push_back(TVector3(activeParams.getLength("z_pindiode"), activeParams.getLength("r_pindiode"),
                                 activeParams.getLength("Phi")));
    nPIN++;
  }
  */
  m_CrematGain = content.getDouble("CrematGain");
  m_WorkFunction = content.getDouble("WorkFunction");
  m_FanoFactor = content.getDouble("FanoFactor");

  B2INFO("PinDigitizer");

}
void PindiodeStudyModule::endRun()
{


}

void PindiodeStudyModule::terminate()
{
}


