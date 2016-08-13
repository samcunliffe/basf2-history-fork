/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/he3tube/modules/He3tubeStudyModule.h>
#include <beast/he3tube/dataobjects/He3tubeSimHit.h>
#include <beast/he3tube/dataobjects/He3tubeHit.h>
#include <beast/he3tube/dataobjects/He3MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>
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

int eventNum = 0;

using namespace std;

using namespace Belle2;
using namespace he3tube;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(He3tubeStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

He3tubeStudyModule::He3tubeStudyModule() : HistoModule()
{
  // Set module properties
  setDescription("Study module for He3tubes (BEAST)");

  // Parameter definitions
  addParam("sampleTime", m_sampletime, "Length of sample, in us", 10000);

}

He3tubeStudyModule::~He3tubeStudyModule()
{
}

//This module is a histomodule. Any histogram created here will be saved by the HistoManager module
void He3tubeStudyModule::defineHisto()
{
  for (int i = 0 ; i < 9 ; i++) {
    h_mche3_kinetic[i]  = new TH1F(TString::Format("h_mche3_kinetic_%d", i), "MC kin. energy [GeV]", 1000, 0., 10.);
  }

  h_NeutronHits = new TH1F("NeutronHits", "Neutron Hits;Tube ", 4, -0.5, 3.5);
  h_DefNeutronHits = new TH1F("DefNeutronHits", "Definite Neutron Hits;Tube ", 4, -0.5, 3.5);
  h_NeutronRate = new TH1F("NeutronRate", "Neutron Hits per second;Tube; Rate (Hz)", 4, -0.5, 3.5);
  h_DefNeutronRate = new TH1F("DefNeutronRate", "Neutron Hits per second;Tube; Rate (Hz)", 4, -0.5, 3.5);

  h_Edep1H3H =       new TH1F("Edep1H3H"     , "Energy deposited by Proton and Tritium; MeV", 100, 0.7, 0.8);
  h_Edep1H3H_detNB = new TH1F("Edep1H3H_tube", "Energy deposited by Proton and Tritium in each tube;Tube Num; MeV", 4, -0.5, 3.5);
  h_Edep1H =         new TH1F("Edep1H"       , "Energy deposited by Protons;MeV", 100, 0, 0.7);
  h_Edep3H =         new TH1F("Edep3H"       , "Energy deposited by Tritiums;MeV", 100, 0, 0.4);
  h_TotEdep =        new TH1F("TotEdep"      , "Total energy deposited;MeV", 100, 0, 1.5);
  h_DetN_Edep =      new TH1F("DetN_Edep"    , "Energy deposited vs detector number;Tube Num;MeV", 4, -0.5, 3.5);

  h_PulseHeights_NotNeutron = new TH1F("PulseHeights_NotNeutron", "Pulse height of waveforms from non-neutron events", 100, 0, 18000);
  h_PulseHeights_Neutron =    new TH1F("PulseHeights_Neutron"   , "Pulse height of waveforms from neutron events", 100, 0, 18000);
  h_PulseHeights_DefNeutron =    new TH1F("PulseHeights_DefNeutron"   , "Pulse height of waveforms from definite neutron events", 100,
                                          0, 18000);
  h_PulseHeights_All =        new TH1F("PulseHeights_All"   , "Pulse height of waveforms from all events", 100, 0, 18000);

}


void He3tubeStudyModule::initialize()
{
  B2INFO("He3tubeStudyModule: Initialize");

  REG_HISTOGRAM

  //convert sample time into rate in Hz
  rateCorrection = m_sampletime / 1e6;
}

void He3tubeStudyModule::beginRun()
{
}

void He3tubeStudyModule::event()
{
  //Here comes the actual event processing

  StoreArray<He3tubeSimHit>  simHits;
  StoreArray<He3tubeHit> Hits;
  StoreArray<He3MCParticle> mcparts;

  for (const auto& mcpart : mcparts) { // start loop over all Tracks
    const double energy = mcpart.getEnergy();
    const double mass = mcpart.getMass();
    double kin = energy - mass;
    const double PDG = mcpart.getPDG();
    int partID[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (PDG == 11) partID[0] = 1; //positron
    else if (PDG == -11) partID[1] = 1; //electron
    else if (PDG == 22) partID[2] = 1; //photon
    else if (PDG == 2112) partID[3] = 1; //neutron
    else if (PDG == 2212) partID[4] = 1; //proton
    else if (PDG == 1000080160) partID[5] = 1; // O
    else if (PDG == 1000060120) partID[6] = 1; // C
    else if (PDG == 1000020040) partID[7] = 1; // He
    else partID[8] = 1;
    for (int i = 0; i < 9; i++) {
      if (partID[i] == 1) {
        h_mche3_kinetic[i]->Fill(kin);
      }
    }
  }

  //initalize various counters
  double edepSum = 0;
  double edepSum_1H = 0;
  double edepSum_3H = 0;
  double totedepSum = 0;
  double totedepDet[4] = {0};

  bool ContainsP[4] = {false};
  bool Contains3H[4] = {false};

  for (int i = 0; i < simHits.getEntries(); i++) {
    He3tubeSimHit* aHit = simHits[i];
    double edep = aHit->getEnergyDep();
    int detNB = aHit->getdetNb();
    int PID = aHit->gettkPDG();

    if (detNB > 3) {
      B2WARNING("Hit registered in undefined tube, ignoring");
      continue;

    }


    h_DetN_Edep->AddBinContent(detNB + 1, edep);
    totedepSum = totedepSum + edep;
    totedepDet[detNB] = totedepDet[detNB] + edep;

    //energy deposited by protons
    if (PID == 2212) {
      ContainsP[detNB] = true;
      //nPhits++;
      h_Edep1H3H_detNB->AddBinContent(detNB + 1, edep);
      edepSum_1H = edepSum_1H + edep;
      edepSum = edepSum + edep;

    }
    //energy deposited by tritiums
    if (PID == 1000010030) {
      Contains3H[detNB] = true;
      //n3Hhits++;
      h_Edep1H3H_detNB->AddBinContent(detNB + 1, edep);
      edepSum_3H = edepSum_3H + edep;
      edepSum = edepSum + edep;
    }


  }

  if (totedepSum != 0)  h_TotEdep->Fill(totedepSum);
  if (edepSum != 0)     h_Edep1H3H->Fill(edepSum);
  if (edepSum_1H != 0)  h_Edep1H->Fill(edepSum_1H);
  if (edepSum_3H != 0)  h_Edep3H->Fill(edepSum_3H);


  int neutronStatus = 0;
  int tubeNum = -1;

  //pulse heights of digitized waveforms
  for (int i = 0; i < Hits.getEntries(); i++) {
    He3tubeHit* aHit = Hits[i];
    if (aHit->definiteNeutron()) { //if this is true, this hit was definitely caused by a neutron.
      nDefiniteNeutron++;
      h_DefNeutronHits->Fill(aHit->getdetNb());
      h_DefNeutronRate->Sumw2();
      h_DefNeutronRate->Fill(aHit->getdetNb(), 1 / rateCorrection);
      h_PulseHeights_DefNeutron->Fill(aHit->getPeakV());
      neutronStatus = 1;
      tubeNum = aHit->getdetNb();
    }

    if (ContainsP[aHit->getdetNb()] && Contains3H[aHit->getdetNb()]) {
      if (neutronStatus == 0) neutronStatus = 2;
      tubeNum = aHit->getdetNb();
      h_PulseHeights_Neutron->Fill(aHit->getPeakV());
      h_NeutronHits->Fill(aHit->getdetNb());
      h_NeutronRate->Sumw2();
      h_NeutronRate->Fill(aHit->getdetNb(), 1 / rateCorrection);
      nNeutronHits++;
    } else {
      h_PulseHeights_NotNeutron->Fill(aHit->getPeakV());
    }
    h_PulseHeights_All->Fill(aHit->getPeakV());

  }

  if (neutronStatus == 1) B2DEBUG(80, "He3tubeStudyModule: Definite Neutron in tube #" << tubeNum);
  else if (neutronStatus == 2) B2DEBUG(80, "He3tubeStudyModule: Possible Neutron in tube #" << tubeNum);

  for (int i = 0; i < 4; i++) {
    if (ContainsP[i]) nPhits++;
    if (Contains3H[i]) n3Hhits++;
  }

  eventNum++;



}

void He3tubeStudyModule::endRun()
{

  B2RESULT("He3tubeStudyModule: # of neutrons:         " << nNeutronHits);
  B2RESULT("                    # of definite neutrons " << nDefiniteNeutron);
  B2RESULT("                    # of 3H hits:          " << n3Hhits);
  B2RESULT("                    # of H hits:           " << nPhits);



}

void He3tubeStudyModule::terminate()
{
}


