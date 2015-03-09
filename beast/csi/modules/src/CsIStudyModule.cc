/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: beaulieu                                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <beast/csi/modules/CsiModule.h>
#include <beast/csi/dataobjects/CsiSimHit.h>
#include <beast/csi/dataobjects/CsiHit.h>
#include <beast/csi/dataobjects/CsiDigiHit.h>
#include <beast/csi/modules/CsIStudyModule.h>



// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
using namespace csi;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CsIStudy)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

CsIStudyModule::CsIStudyModule() : HistoModule(),
  h_CrystalEdep(NULL),
  h_CrystalSpectrum(NULL),
  h_CrystalRadDose(NULL),
  h_CrystalRadDoseSH(NULL),
  h_NhitCrystal(NULL),
  h_LightYieldCrystal(NULL),
  h_Waveform(NULL),
  h_Gate(NULL),
  h_Charge(NULL),
  h_TrueEdep(NULL)

{
  // Set module properties
  setDescription("Analyze simulations of CsI readings in BEAST. Requires HistoManager module.");

  // Parameter definitions
  addParam("nWFSamples", m_nWFSamples, "Number of samples in the saved waveforms", 8000);
  addParam("paramTemplate", m_paramTemplate, "Template of an input parameter. Noop for now.", 0.0);

  string rootfilepath = "output/waveforms.root";
  addParam("waveformFilename", m_waveformFilename, "Path to the root file to save waveforms", rootfilepath);

}

CsIStudyModule::~CsIStudyModule()
{
}

void CsIStudyModule::defineHisto()
{
  h_CrystalEdep         = new TH1F("Crystal_Edep",  "Energy distribution in each crystal;CellID", 16, -0.5, 15.5);
  h_CrystalSpectrum     = new TH1F("Crystal_Edist",  "Photon energy distribution in all crystals;", 100, 0, 0.1);
  h_NhitCrystal         = new TH1F("Crystal_HitRate", "Number of hit per crystal;CellID; hit/s", 16, -0.5, 15.5);
  h_CrystalRadDose      = new TH1F("Crystal_RadDose", "Crystal Radiation Dose;CellID;Gy/yr", 16, -0.5, 15.5);
  h_CrystalRadDoseSH    = new TH1F("Crystal_RadDose_SH", "Crystal Radiation Dose from SimHits;CellID;Gy/yr", 16, -0.5, 15.5);
  h_LightYieldCrystal   = new TH1F("Crystal_g_yield", "Light yield each crystal;CellID;gamma/sample", 16, -0.5, 15.5);
  h_Waveform            = new TH1S("Waveform", "Recorded waveform;Time index;ADC bits", m_nWFSamples, 0, m_nWFSamples - 1);
  h_Gate                = new TH1C("Gate", "Recorded gate;Time index;ADC bits", m_nWFSamples, 0, m_nWFSamples - 1);
  h_Charge              = new TH1I("Charge", "Integrated Charge", 200, 0, 6);
  h_TrueEdep            = new TH1F("TrueEdep", "True deposited energy", 200, -5, 1);

}

void CsIStudyModule::initialize()
{
  REG_HISTOGRAM   // required to register histograms to HistoManager

  m_hits.isOptional();
  m_simhits.isRequired();
  m_digihits.isOptional();

  fWF = new TFile(m_waveformFilename.c_str(), "recreate");
}

void CsIStudyModule::beginRun()
{
}

void CsIStudyModule::endRun()
{
  //  h_LightYieldCrystal->Divide( h_CrystalEdep );
  //  h_LightYieldCrystal->Scale( 1e-6 );
}


void CsIStudyModule::event()
{
  //Loop over CsiHits
  if (m_hits.getEntries() > 0) {
    int hitNum = m_hits.getEntries(); /**< Number of Crystal hits */

    double E_tmp[16] = {0};       /**< Sum energy deposited in each cell */

    double Mass = 5;             /**< Mass of the crystal (to be replaced with data from the xml once actual mass is known)**/
    double edeptodose = GeVtoJ / Mass * usInYr / Sampletime; /**< Get dose in Gy/yr from Edep */

    /// Actual looping over CsIHits
    for (int i = 0; i < hitNum; i++) {
      CsiHit* aCsIHit = m_hits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */
      //double hitTime = aCsIHit->getTimeAve();    /**< Time of the hit*/

      E_tmp[m_cellID] += edep;

      // Fill histograms
      h_CrystalSpectrum->Fill(edep);
      h_CrystalEdep->Fill(m_cellID, edep);
      h_CrystalRadDose->Fill(m_cellID, edep * edeptodose);

      //Number of hits per second
      h_NhitCrystal->Fill(m_cellID, 1.0e9 / Sampletime);
    }
  }

  //Loop over CsiSimHits
  if (m_simhits.getEntries() > 0) {
    int hitNum = m_simhits.getEntries(); /**< Number of Crystal hits */

    double Mass = 5;  /**< Mass of the crystal **/
    double  edeptodose = GeVtoJ / Mass * usInYr / Sampletime;/**< Get dose in Gy/yr from Edep */

    /// Actual looping over CsISimHits
    for (int i = 0; i < hitNum; i++) {
      CsiSimHit* aCsIHit = m_simhits[i];
      int m_cellID = aCsIHit->getCellId();       /**< Index of the Cell*/
      double edep = aCsIHit->getEnergyDep();     /**< Energy deposited in the current hit */

      // Fill histograms
      h_CrystalRadDoseSH->Fill(m_cellID, edep * edeptodose);

      // To get the Number of photons per GeV (divide by total edep before plotting)
      if (22 == aCsIHit->getPDGCode()) {
        h_LightYieldCrystal->Fill(m_cellID);
      }
    }
  }

  //Loop over Digihits
  if (m_digihits.getEntries() > 0) {
    int hitNum = m_digihits.getEntries(); /**< Number of Crystal hits */

    /// Actual looping over CsIDigiHits
    for (int i = 0; i < hitNum; i++) {
      CsiDigiHit* aCsIDigiHit = m_digihits[i];

      int cellID = aCsIDigiHit->getCellId();
      uint32_t charge = aCsIDigiHit->getCharge();
      double trueEdep = aCsIDigiHit->getTrueEdep();
      vector<uint16_t>* waveform = aCsIDigiHit->getWaveform();
      vector<uint8_t>* status = aCsIDigiHit->getStatusBits();

      h_TrueEdep->Fill(log10(trueEdep));
      h_Charge->Fill(log10(charge));

      char histoTitle[80];
      sprintf(histoTitle, "Waveform Hit No. %i, Cell No %i", i, cellID);
      h_Waveform->SetTitle(histoTitle);

      // Write the wavforms. All have the same number in the TFile, but
      // we can access them by their cycle number:
      // root[] TFile *f1 = new TFile("filename.root")
      // root[] TH1F *Waveform4; f1->GetObject("h_Waveform;4",Waveform4);


      if (waveform->size()) {
        for (uint iBin = 0; iBin < waveform->size(); iBin++) {
          bool* statusBits = readDPPStatusBits(status->at(iBin));
          h_Gate->Fill(iBin + 1, (int) statusBits[1]);
          h_Waveform->Fill(iBin + 1, waveform->at(iBin));
        }

        fWF->cd();
        h_Gate->Write();
        h_Waveform->Write();
      }
    }
  }
}


void CsIStudyModule::terminate()
{
  fWF->Close();
}


bool*  CsIStudyModule::readDPPStatusBits(char data)
{

  bool* bit  = new bool[8];

  // fill data
  for (int i = 0; i < 8; i++) {
    bit[i] = ((data >> i) & 0x01);
  }

  return bit;
}
