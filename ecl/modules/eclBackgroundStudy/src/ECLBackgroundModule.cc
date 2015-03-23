/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Sam de Jong                                              *
 *               Alexandre Beaulieu                                       *
 *               Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclBackgroundStudy/ECLBackgroundModule.h>
#include <ecl/modules/eclBackgroundStudy/ECLCrystalData.h>

#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>

#ifdef DOARICH
#include <arich/geometry/ARICHGeometryPar.h>
#endif

#include <ecl/geometry/ECLGeometryPar.h>
#include <simulation/dataobjects/BeamBackHit.h>
#include <mdst/dataobjects/MCParticle.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//C++
#include <iostream>
#include <vector>

#define PI 3.14159265358979323846

// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TH1.h>
#include <TH2.h>


using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLBackground)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------


ECLBackgroundModule::ECLBackgroundModule() :
  HistoModule(),
  m_nEvent(0),
  h_nECLSimHits(0),
  h_CrystalRadDoseTheta(0),
  h_CrystalRadDose(0),
  h_Pileup(0),
  h_CrystalThetaID2(0),
  h_CrystalThetaID67(0),
  h_HitLocations(0),
  h_BarrelDose(0),
  h_DiodeRadDose(0),
  h_NeutronFlux(0),
  h_NeutronFluxThetaID2(0),
  h_NeutronFluxThetaID67(0),
  h_NeutronE(0),
  h_NeutronEThetaID0(0),
  h_PhotonE(0),
  h_ShowerVsTheta(0),
  h_Shower(0),
  h_ProdVert(0),
  h_ProdVertvsThetaId(0),
  T_event{0},
  T_energy{0},
  T_1us{0},
  m_arichgp(0),
  nHAPDperRing{0},
  hEMDose(0),
  hDiodeFlux(0),
  hEgamma(0),
  hEneu(0),
  hARICHDoseBB(0),
  hHAPDFlux(0),
  hEMDoseECF(0),
  hEMDoseECB(0),
  hEMDoseBAR(0),
  hEMDoseWideTID(0),
  hDiodeFluxECF(0),
  hDiodeFluxECB(0),
  hDiodeFluxBAR(0),
  hDiodeFluxWideTID(0)
{
  //Set module properties
  setDescription("Processes background campaigns and produces histograms. Requires HistoManager");

  std::vector<int> empty;
  addParam("sampleTime", m_sampleTime,     "Length of sample, in us", 1000);
  addParam("doARICH",    m_doARICH,        "If true, some ARICH plots (for shielding studies) will be produced",  false);
  addParam("crystalsOfInterest", m_CryInt, "Cell ID of crystals of interest. Dose will be printed at end of run", empty);

}

ECLBackgroundModule::~ECLBackgroundModule()
{
}

//If a histogram is initalized here, it will be saved.
void ECLBackgroundModule::defineHisto()
{
  std::ostringstream s;
  s << m_sampleTime;

  //initalize histograms
  h_nECLSimHits = new TH1F("ECL_Sim_Hits", "ECL Sim Hits", 100, 0, 100);


  //Radiation dose
  h_CrystalRadDose      = new TH1F("Crystal_Rad_Dose",        "Crystal Radiation Dose vs #theta_{ID};#theta_{ID};Gy/yr",
                                   69, -0.5, 68.5);
  h_CrystalRadDoseTheta = new TH1F("Crystal_Rad_Dose_Theta",  "Crystal Radiation Dose vs #theta;#theta (deg);Gy/yr",
                                   100,   12, 152);
  h_CrystalThetaID2     = new TH1F("Crystal_Dose_ThetaID_2",  "Crystal Radiation Dose vs #phi_{ID}, #theta_{ID}=2; #phi_{ID};Gy/yr",
                                   64, -0.5, 63.5);
  h_CrystalThetaID67    = new TH1F("Crystal_Dose_ThetaID_67", "Crystal Radiation Dose vs #phi_{ID}, #theta_{ID}=67; #phi_{ID};Gy/yr",
                                   64, -0.5, 63.5);
  h_BarrelDose          = new TH1F("Crystal_Dose_Barrel",     "Crystal Radiation Dose in Barrel, 12<#theta_{ID}<59; #phi_{ID}; Gy/yr",
                                   144, -0.5, 143.5);
  h_DiodeRadDose        = new TH1F("Diode_Rad_Dose",          "Diode Radiation Dose vs #theta_{ID};#theta_{ID};Gy/yr",
                                   69, -0.5, 68.5);

  //hit locations
  h_ProdVert            = new TH1F("MCProd_Vert",             "Production Vertex;z (cm)",                           125, -200, 300);
  h_HitLocations        = new TH2F("Hit_Locations",           "Hit locations;z (cm); r (cm)",                       250, -200, 300,
                                   80,    0, 160);
  h_ProdVertvsThetaId   = new TH2F("MCProd_Vert_vs_ThetaID",  "Production Vertex vs #theta_{ID};#theta_{ID};z (cm)", 69, -0.5, 68.5,
                                   125, -200, 300);

  h_Pileup              = new TH1F("Pile_up",                 "Estimated Pile up Noise vs #theta_{ID}; #theta_{ID}; MeV", 69, -0.5,
                                   68.5);

  //Neutrons
  h_NeutronFluxThetaID2  = new TH1F("Neutron_Flux_ThetaID_2",  "Diode Neutron Flux, #theta_{ID}=2;#phi_{ID}; yr^{-1}/cm^{-2}",   64,
                                    -0.5, 63.5);
  h_NeutronFluxThetaID67 = new TH1F("Neutron_Flux_ThetaID_67", "Diode Neutron Flux, #theta_{ID}=67;#phi_{ID}; yr^{-1}/cm^{-2}",  64,
                                    -0.5, 63.5);
  h_NeutronFlux          = new TH1F("Neutron_Flux",            "Diode Neutron Flux vs #theta_{ID};#theta_{ID}; yr^{-1}/cm^{-2}", 69,
                                    -0.5, 68.5);
  h_NeutronE             = new TH1F("Neutron_Energy",          "Neutron Energy; Energy (MeV)",                           200,    0,
                                    0.5);
  h_NeutronEThetaID0     = new TH1F("Neutron_Energy_ThetaID0", "Neutron Energy, First Crystal; Energy (MeV)",             50,    0,
                                    0.5);

  h_PhotonE              = new TH1F("Photon_Energy",           "Energy of photons creating hits in ECL; Energy (MeV)",   200, 0, 10);

  //showers
  TString stime = s.str();
  h_Shower        = new TH1F("Shower_E_Dist",          "Shower Energy distribution " + stime + " #mu s;GeV;# of showers", 100, 0,
                             0.5);
  h_ShowerVsTheta = new TH2F("Shower_E_Dist_vs_theta", "Shower Energy distribution " + stime + " #mu s;GeV;#theta (deg)", 100, 0, 0.5,
                             180, 0, 180);



  //
  // Below are for the ECL shields studies
  //
  //////////////////////////////////////////

  //Doses
  hEMDose = new TH1F("hEMDose",  "Crystal Radiation Dose; Cell ID ; Gy/yr", 8736, 0, 8736);

  //Diodes
  hDiodeFlux  = new TH1F("hDiodeFlux",  "Diode Neutron Flux ; Cell ID ; 1MeV-equiv / cm^{2} yr", 8736, 0, 8736);

  //Radiation spectra
  hEgamma = new TH1F("hEgamma", "Log Spectrum of the photons hitting the crystals / 1 MeV; log_{10}(E_{#gamma}/1MeV) ", 500, -4, 3);
  hEneu   = new TH1F("hEneu",   "Log Spectrum of the neutrons hitting the diodes / 1 MeV; log_{10}(E_{n}/1MeV)",        500, -10, 2);

  //ARICH plots
  if (m_doARICH) {
    hARICHDoseBB = new TH1F("hARICHDoseBB", "Radiation dose in ARICH boards (cBB); Ring-ID; Gy/yr",                                   7,
                            -0.5, 6.5);
    hHAPDFlux    = new TH1F("hARICHnFlux",  "1-MeV equivalent neutron flux in ARICH diodes (BB) ; Ring-ID ; 1-MeV-equiv / cm^{2} yr", 7,
                            -0.5, 6.5);
  }

  hEMDoseECF     = new TH2F();
  hEMDoseECB     = new TH2F();
  hEMDoseBAR     = new TH2F();
  hEMDoseWideTID = new TH1F();

  hDiodeFluxECF     = new TH2F();
  hDiodeFluxECB     = new TH2F();
  hDiodeFluxBAR     = new TH2F();
  hDiodeFluxWideTID = new TH1F();

}

void ECLBackgroundModule::initialize()
{

  REG_HISTOGRAM

  if (m_doARICH)  B2INFO("ECLBackgroundModule: ARICH plots are being produced");

  // Initialize variables
#ifdef DOARICH
  if (m_doARICH) m_arichgp = ARICHGeometryPar::Instance();
#endif

  m_nEvent = 0;
  BuildECL();

}

void ECLBackgroundModule::beginRun()
{
}

void ECLBackgroundModule::event()
{

  //Input Arrays
  StoreArray<ECLSimHit>  eclArray;
  StoreArray<MCParticle> mcParticles;
  StoreArray<BeamBackHit> BeamBackArray;
  StoreArray<ECLShower> eclShowerArray;

  //ignore events with huge number of SimHits (usually a glitchy event)
  if (eclArray.getEntries() > 4000) {
    B2INFO("ECLBackgroundModule: Skipping event #" << m_nEvent << " due to large number of ECLSimHits");
    m_nEvent++;
    return;
  }

  bool isE = false;
  bool EinTheta[nECLThetaID] = {false};

  double edepSum = 0;
  double edepSumTheta[nECLThetaID] = {0};

  h_nECLSimHits->Fill(eclArray.getEntries()); //number of ECL hits in an event

  double E_tmp[nECLCrystalTot] = {0};

  //MC ID of photon hits
  vector<int> MCPhotonIDs;

  int hitNum = eclArray.getEntries();
  for (int i = 0; i < hitNum; i++) { //loop over ECLSimHits
    ECLSimHit* aECLHit = eclArray[i];
    int m_cellID       = aECLHit->getCellId() - 1; //cell ID
    double edep        = aECLHit->getEnergyDep();  //energy deposited
    TVector3 hitPosn   = aECLHit->getPosition();   //position of hit
    int pid            = aECLHit->getPDGCode();

    float Mass         = Crystal[m_cellID]->GetMass();
    int m_thetaID      = Crystal[m_cellID]->GetThetaID();
    int m_phiID        = Crystal[m_cellID]->GetPhiID();
    int NperRing       = Crystal[m_cellID]->GetNperThetaID();   //number of crystals in this theta ring
    double theta       = Crystal[m_cellID]->GetTheta();


    //get Track ID of photons which create the SimHits
    if (pid == 22) MCPhotonIDs.push_back(aECLHit->getTrackId());

    edepSum = edepSum + edep;
    E_tmp[m_cellID] = edep + E_tmp[m_cellID]; //sum energy deposited in this crystal
    edepSumTheta[m_thetaID] = edepSumTheta[m_thetaID] + edep;   //sum of energy for this thetaID value
    EinTheta[m_thetaID] = true;                                 //there is an energy deposit in this theta ring. used later
    isE = true;


    //fill histograms
    //radiation dose for this SimHit
    double dose = edep * GeVtoJ * usInYr / (m_sampleTime * Mass);

    h_CrystalRadDoseTheta->Fill(theta, dose / NperRing);
    h_CrystalRadDose->Fill(m_thetaID,  dose / NperRing);
    hEMDose->Fill(m_cellID, dose);

    //2nd thetaID ring
    if (m_thetaID == 2) {
      h_CrystalThetaID2->Fill(m_phiID,  dose);
    }
    //67th thetaID ring
    if (m_thetaID == 67) {
      h_CrystalThetaID67->Fill(m_phiID, dose);
    }
    //Barrel
    if (m_thetaID < 59 && m_thetaID > 12) {
      h_BarrelDose->Fill(m_phiID, dose / 46);
    }

    //location of the hit
    h_HitLocations->Fill(hitPosn.Z(), sqrt(hitPosn.X() * hitPosn.X() + hitPosn.Y() * hitPosn.Y()));

  }


  //for pileup noise estimation
  for (int iECLCell = 0; iECLCell < nECLCrystalTot; iECLCell++) {
    double edep = E_tmp[iECLCell];
    int    m_thetaID = Crystal[iECLCell]->GetThetaID();
    int    NperRing  = Crystal[iECLCell]->GetNperThetaID();
    if (edep > 0.000000000001) {
      T_1us[m_thetaID] += edep / NperRing / m_sampleTime;
      T_energy[m_thetaID] += edep;
      T_event[m_thetaID] += 1;
    }

  }


  //One track can create several ECLSimHits. Remove the duplicates
  sort(MCPhotonIDs.begin(), MCPhotonIDs.end());
  vector<int>::iterator it;
  it = std::unique(MCPhotonIDs.begin(), MCPhotonIDs.end());
  MCPhotonIDs.resize(std::distance(MCPhotonIDs.begin() , it));


  //loop over MCParticles to find the photons that caused the simhits
  for (int i = 0; i < (int)MCPhotonIDs.size(); i++) {
    for (int j = 0; j < mcParticles.getEntries(); j++) {
      if (mcParticles[j]->getIndex() == MCPhotonIDs[i]) {
        h_PhotonE->Fill(mcParticles[j]->getEnergy() * 1000);
        hEgamma->Fill(log10(mcParticles[j]->getEnergy() * 1000));
        break;         //once the correct MCParticle is found, stop looping over MCParticles
      }
    }
  }

  //*****************end of crystal analysis

  //start of diode analysis
  int neuHits = BeamBackArray.getEntries();
  for (int iHits = 0; iHits < neuHits; iHits++) { //loop over BeamBackArray
    BeamBackHit* aBeamBackSimHit = BeamBackArray[iHits];

    //get relevant values
    int m_cellID  = aBeamBackSimHit->getIdentifier();
    double damage = aBeamBackSimHit->getNeutronWeight();
    double edep   = aBeamBackSimHit->getEnergyDeposit();
    int pid       = aBeamBackSimHit->getPDG();
    int SubDet    = aBeamBackSimHit->getSubDet();
    double Energy = aBeamBackSimHit->getEnergy();
    TVector3 rHit = aBeamBackSimHit->getPosition();


    if (SubDet == 6) { //ECL

      int m_thetaID      = Crystal[m_cellID]->GetThetaID();
      int m_phiID        = Crystal[m_cellID]->GetPhiID();
      int NperRing       = Crystal[m_cellID]->GetNperThetaID();

      double diodeDose = edep * GeVtoJ * usInYr / (m_sampleTime * DiodeMass);
      h_DiodeRadDose->AddBinContent(m_thetaID, diodeDose / NperRing);  //diode radiation dose plot

      if (pid == 2112) {

        double weightedFlux = damage * usInYr / (m_sampleTime * DiodeArea) ;           //neutrons per cm^2 per year

        //neutron plots
        if (m_thetaID == 0) h_NeutronEThetaID0->Fill(Energy * 1000);
        h_NeutronE->Fill(Energy * 1000);
        hEneu->Fill(log10(Energy * 1000));

        h_NeutronFlux->Fill(m_thetaID, weightedFlux / NperRing);
        hDiodeFlux->Fill(m_cellID,  weightedFlux);

        if (m_thetaID == 2)  h_NeutronFluxThetaID2->Fill(m_phiID , weightedFlux);
        if (m_thetaID == 67) h_NeutronFluxThetaID67->Fill(m_phiID , weightedFlux);


      }

    } else if (SubDet == 4 && m_doARICH) { //ARICH
      FillARICHBeamBack(aBeamBackSimHit);
    }
  }

  int nShower = eclShowerArray.getEntries();
  for (int i = 0; i < nShower; i++) {
    ECLShower* aShower = eclShowerArray[i];

    double Energy = aShower->getEnergy();
    double theta = aShower->getTheta();

    //get number of background showers with energy above 20MeV
    if (Energy > 0.02) {
      h_Shower->Fill(Energy);
      h_ShowerVsTheta->Fill(Energy, theta * 180 / PI);

    }
  }


  for (int i = 0; i < nECLThetaID; i++) {
    if (EinTheta[i]) {
      //0th McParticle in an event is the origin of all particles
      h_ProdVertvsThetaId->Fill(i, mcParticles[0]->getProductionVertex().z(), edepSumTheta[i]);
    }
  }


  if (isE) {
    h_ProdVert->Fill(mcParticles[0]->getProductionVertex().z(), edepSum);
  }

  if (m_nEvent % ((int)m_sampleTime * 100) == 0) B2INFO("ECLBackgroundModule: At Event #" << m_nEvent);
  m_nEvent++;
}

void ECLBackgroundModule::endRun()
{
  B2INFO("ECLBackgroundModule: Total Number of events: "  << m_nEvent)

  //fill pileup plot
  for (int idx = 0; idx < nECLThetaID; idx++) {
    if (T_event[idx] > 0) {
      double average = T_energy[idx] / T_event[idx];
      h_Pileup->Fill(idx, sqrt(T_1us[idx] / average) * average * 1000);//1000 in MeV
    }
  }

  //print doses of crystals of interest
  for (int i = 0; i < (int)m_CryInt.size(); i++) {
    if (m_CryInt[i] > 8736) {
      B2WARNING("ECLBackgroundModule: Invalid cell ID. must be less than 8736");
      continue;
    }
    double dose = hEMDose->GetBinContent(m_CryInt[i] + 1); //add 1 since bin #1 corrosponds to cell ID #0
    int thetaID = Crystal[m_CryInt[i]]->GetThetaID();
    int phiID   = Crystal[m_CryInt[i]]->GetPhiID();
    B2RESULT("Dose in Crystal " << m_CryInt[i] << ": " << dose << " ThetaID=" << thetaID << ", PhiID=" << phiID);
  }

  hEMDoseECF = BuildPosHisto(hEMDose, "forward");
  hEMDoseECB = BuildPosHisto(hEMDose, "backward");
  hEMDoseBAR = BuildPosHisto(hEMDose, "barrel");
  hEMDoseWideTID = BuildThetaIDWideHisto(hEMDose);

  hDiodeFluxECF  = BuildPosHisto(hDiodeFlux, "forward");
  hDiodeFluxECB  = BuildPosHisto(hDiodeFlux, "backward");
  hDiodeFluxBAR  = BuildPosHisto(hDiodeFlux, "barrel");
  hDiodeFluxWideTID = BuildThetaIDWideHisto(hDiodeFlux);

  hEMDose->SetTitle("Crystal Radiation Dose vs Cell ID");
  hDiodeFlux->SetTitle("Diode Neutron Flux vs Cell ID");

}

void ECLBackgroundModule::terminate()
{
}


//
// Methods to study performance of ECL shields
//   and potential impact on ARICH doses
/////////////////////////////////////////////
#ifdef DOARICH
int ECLBackgroundModule::FillARICHBeamBack(BeamBackHit* aBBHit)
{

  double _damage   = aBBHit->getNeutronWeight();
  double _eDep     = aBBHit->getEnergyDeposit();
  float _trlen     = aBBHit->getTrackLength();
  int _pid         = aBBHit->getPDG();
  TVector3 _posHit = aBBHit->getPosition();

  int _moduleID    = m_arichgp->getCopyNo(_posHit);

  double r = _posHit.Perp();
  int _ring = 0;

  _ring = ARICHmod2row(_moduleID);

  B2DEBUG(200, "Filling ARICH BeamBackHit");
  B2DEBUG(200, " PDG = " << _pid);
  B2DEBUG(200, " Edep = " << _eDep);
  B2DEBUG(200, " Ring = " << _ring);
  B2DEBUG(200, " Radius = " << r);
  B2DEBUG(200, " Module = " << _moduleID);

  if (2112 == _pid) {
    hHAPDFlux->Fill(_ring, _damage * _trlen / HAPDthickness  * usInYr / (m_sampleTime * HAPDarea * nHAPDperRing[_ring]));
  } else {
    hARICHDoseBB->Fill(_ring, _eDep / (HAPDmass * nHAPDperRing[_ring]) * GeVtoJ * usInYr / m_sampleTime);
  }

  return 1;
}

#else
int ECLBackgroundModule::FillARICHBeamBack(BeamBackHit* aBBHit) { return 1;}
#endif

int ECLBackgroundModule::BuildECL()
{
  for (int i = 0; i < nECLCrystalTot; i++) {
    Crystal[i] = new ECLCrystalData(i);
  }
  return 1;
}

//Method used for debugging.
int ECLBackgroundModule::SetPosHistos(TH1F* h, TH2F* hFWD, TH2F* hBAR, TH2F* hBWD)
{
  char FWDtitle[100];
  char BWDtitle[100];
  char BARtitle[100];

  char FWDname[16];
  char BWDname[16];
  char BARname[16];

  sprintf(FWDtitle, "%s -- Forward Endcap" , h->GetTitle());
  sprintf(BWDtitle, "%s -- Backward Endcap", h->GetTitle());
  sprintf(BARtitle, "%s -- Barrel",          h->GetTitle());

  sprintf(FWDname, "%sFWD", h->GetName());
  sprintf(BWDname, "%sBWD", h->GetName());
  sprintf(BARname, "%sBAR", h->GetName());

  float value = 0;
  // Fill 2D histograms with the values in the 1D histogram
  for (int i = 0; i < nECLCrystalTot; i++)  {
    value = h->GetBinContent(i + 1);

    if (i < nECLCrystalECF) {
      hFWD->Fill(floor(Crystal[i]->GetX()), floor(Crystal[i]->GetY()), value);

    } else if (i >= (nECLCrystalBAR + nECLCrystalECF)) {
      hBWD->Fill(floor(Crystal[i]->GetX()), floor(Crystal[i]->GetY()), value);

    } else
      hBAR->Fill(floor(Crystal[i]->GetZ()), floor(Crystal[i]->GetR() * (Crystal[i]->GetPhi() - 180) * PI / 180)    , value);
  }

  return 1;
}


TH2F* ECLBackgroundModule::BuildPosHisto(TH1F* h, const char* sub)
{

  // Initialize variables
  char _title[100];
  char _name[16];
  TH2F* h_out;

  double value = 0;

// Forward endcap value vs (x,y)
  if (!strcmp(sub, "forward")) {
    sprintf(_name, "%sFWD", h->GetName());
    sprintf(_title, "%s -- Forward Endcap;x(cm);y(cm)" , h->GetTitle());
    h_out = new TH2F(_name, _title, 90, -150, 150, 90, -150, 150); //position in cm
    h_out->Sumw2();
    for (int i = 0; i < nECLCrystalECF; i++)  {
      value = h->GetBinContent(i + 1);
      h_out->Fill(floor(Crystal[i]->GetX()),
                  floor(Crystal[i]->GetY()),
                  value);
    }

    // Backward endcap value vs (x,y)
  } else if (!strcmp(sub, "backward")) {
    sprintf(_name, "%sBWD", h->GetName());
    sprintf(_title, "%s -- Backward Endcap;x(cm);y(cm)", h->GetTitle());
    h_out = new TH2F(_name, _title, 90, -150, 150, 90, -150, 150); //position in cm
    h_out->Sumw2();
    for (int i = (nECLCrystalBAR + nECLCrystalECF); i < nECLCrystalTot; i++) {
      value = h->GetBinContent(i + 1);
      h_out->Fill(floor(Crystal[i]->GetX()),
                  floor(Crystal[i]->GetY()),
                  value);
    }


    // The rest: barrel value vs (theta_ID, phi_ID)
  } else if (!strcmp(sub, "barrel")) {
    sprintf(_name, "%sBAR", h->GetName());
    sprintf(_title, "%s -- Barrel;#theta_{ID};#phi_{ID}",          h->GetTitle());
    h_out = new TH2F(_name, _title, 47, 12, 59, 144, 0, 144); //position in cm (along z and along r*phi)
    h_out->Sumw2();
    for (int i = nECLCrystalECF; i < (nECLCrystalBAR + nECLCrystalECF); i++) {
      value = h->GetBinContent(i + 1);
      h_out->Fill(Crystal[i]->GetThetaID(),  Crystal[i]->GetPhiID(), value);
    }

  } else {
    B2WARNING("ECLBackgroundModule: Unable to BuildPosHisto. Check Arguments.")
    h_out =  new TH2F("(empty)", "(empty)", 1, 0, 1, 1, 0, 1);
  }

  return h_out;
}


TH1F*   ECLBackgroundModule::BuildThetaIDWideHisto(TH1F* h_cry)
{

  char _title[100];
  char _name[64];

  //Define the boundaries of the bins
  static const int    _nbins = 21;
  static const double _xbins[] = { -0.5,  0.5,  4.5,  8.5, 11.5, 12.5,
                                   16.5, 20.5, 24.5, 28.5, 32.5,
                                   36.5, 40.5, 44.5, 48.5, 52.5,
                                   56.5, 58.5, 59.5, 63.5, 67.5, 68.5
                                 };


  sprintf(_title, "%s vs #theta_{ID} -- averages" , h_cry->GetTitle());
  sprintf(_name, "%svsTheWide", h_cry->GetName());

  //New pointer to the returned histogram ...
  TH1F* h_out  = new TH1F(_name, _title, 1, 0, 1);
  // ... but only temp variables to the temporary ones
  TH1F h_mass("h_mass", "Total Mass per Theta-ID", 1, 0, 1);
  TH1F h_N("h_N", "Entries (unweighted) per Theta-ID bin", 1, 0, 1);

  //Apply all the same binning
  h_out->SetBins(_nbins, _xbins);
  h_mass.SetBins(_nbins, _xbins);
  h_N.SetBins(_nbins, _xbins);

  h_out->SetTitle(_title);
  h_out->Sumw2();

  //Make histo for total mass, then divide!
  for (int i = 0; i < nECLCrystalTot; i++)  {
    h_out->Fill(Crystal[i]->GetThetaID(), h_cry->GetBinContent(i + 1) * Crystal[i]->GetMass());
    h_mass.Fill(Crystal[i]->GetThetaID(), Crystal[i]->GetMass());
    h_mass.SetBinError(Crystal[i]->GetThetaID(), 0);
    h_N.Fill(Crystal[i]->GetThetaID());
  }
  h_out->SetXTitle("#theta_{ID}");
  h_out->SetYTitle(h_cry->GetYaxis()->GetTitle());
  h_out->Divide(&h_mass);

  return h_out;
}


int ECLBackgroundModule::ARICHmod2row(int modID)
{
  if (modID <= 42) return 0;
  else if (modID <= 90) return 1;
  else if (modID <= 144) return 2;
  else if (modID <= 204) return 3;
  else if (modID <= 270) return 4;
  else if (modID <= 342) return 5;
  else if (modID <= 420) return 6;

  B2WARNING("ECLBackgroundModule: ARICHmod2row: modID out of bound; can't get ring index");
  return -1;
}
