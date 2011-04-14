/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Zbynek Drasal, Benjamin Schwenker,         *
 *               Peter Kvasnicka                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// History:
//- original version - ILCsoft VTXDigitizer by A. Raspereza
//- added procesor parameters: electronic noise, Z. Drasal Jan 2009
//- completely changed code structure, Z. Drasal Mar 2009
//- renamed to SiPxlGeom & created as standalone pckg, Z. Drasal Jul 2009
//- added geometry interface (SiPxlGeom), Z. Drasal Aug 2009
//- added parameter for cut on time to emulate the integration time, K. Prothmann Dec. 2009
//- new proper clustering implemented & noise generation updated, Z. Drasal Jan 2010
//- improved clustering procedure & corrected MCParticle weight according to deposited energy, Z. Drasal Feb 2010
//- fixed straggling function ("Landau fluctuations") in Si, Z. Drasal Mar 2010
//- implemented new Landau fluctuations (mean energy loss is calculated automatically), Z. Drasal Apr 2010
//- width of cluster parameter renamed to its proper name width of diffused cloud of e, Z. Drasal Apr 2010
//- digits map is saved (if name defined) together with hits map, TrackerPulse container used, Z. Drasal May 2010
//- corrected memory leak when not saving TrackerPulses, Z. Drasal Aug 2010
//- TrackerHit -> rawHits() contains 1 PXDSimHit, which contributed with highest weight, Z. Drasal Sep 2010
//- added effect of ganged pixels,
//- when two pixels in different rows are read-out simultaneously, Z. Drasal Sep 2010
//- implementation for basf2, Peter Kvasnicka, Feb 2011. Separated clustering,
//  different geometry, different support libraries, input and output format.

#include <pxd/modules/pxdDigitizer/PXDDigiModule.h>

// DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/Relation.h>

// Data objects
#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDSimHit.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/HitSorter.h>

// Framework utils
#include <framework/gearbox/Unit.h>
#include <pxd/modules/pxdDigitizer/PhysicalConstants.h>
#include <geometry/bfieldmap/BFieldMap.h>
#include <framework/logging/Logger.h>

// Include basic C
#include <cstdlib>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <list>
#include <cmath>
#include <time.h>

// ROOT
#include <TMath.h>
#include <TVector3.h>
#include <TRandom3.h>
#include <TGeoManager.h>

// Used namespaces
using namespace std;
using namespace boost;
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDDigi)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDDigiModule::PXDDigiModule() :
    Module(),
    m_currentLayerID(0), m_currentLadderID(0), m_currentSensorID(0),
    m_sensorThick(-1), m_sensorWidth(-1), m_sensorLength(-1),
    m_geometry(NULL),
    m_random(new TRandom3(0)),
    m_fluctuate(NULL)
{

  // Set description()
  setDescription(
    "PXDDigiModule: basf2 module producing PXD digits from PXDSimHits");

  // Processor parameters

  // Input and output collections
  addParam("MCPartColName", m_mcColName, "MCParticles collection name",
           string(DEFAULT_MCPARTICLES));
  addParam("SimHitsCOllectionName", m_simColName, "Input collection name", string(
             DEFAULT_PXDSIMHITS));
  addParam("MCParticlesToSimHitsColName", m_relSimName,
           "Name of MCParticles-to-PXDSimHits relation", string(DEFAULT_PXDSIMHITSREL));
  addParam("DigitsCollectionName", m_digitColName, "Digits collection name",
           string(DEFAULT_PXDDIGITS));
  addParam("MCParticlesToDigitsCollectionName", m_relDigitName,
           "Relations between MCParticles and Digits", string(DEFAULT_PXDDIGITSREL));
  addParam("ADC", m_ADC, "Simulate ADC?", bool(false));
  addParam("ADCRange", m_ADCRange,
           "Set analog-to-digital converter range 0 - ? (in e)", int(24000));
  addParam("ADCBits", m_ADCBits, "Set how many bits the ADC uses", int(8));
  addParam("Diffusion", m_diffusionCoefficient,
           "Diffusion coefficient (in mm)", double(0.00008617));
  addParam("GangedPixels", m_gangedPixels, "Use ganged pixels? (0 - no, 1 - yes, 2 - yes, special readout algorithm)",
           int(0));
  addParam("ElectronicEffects", m_electronicEffects,
           "Apply electronic effects?", bool(true));
  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", double(200.));
  addParam("ZeroSuppressionSN", m_SNAdjacent, "S/N threshold for zero suppression", double(2.5));
  addParam("IntegrationWindow", m_integrationWindow,
           "Use integration window?", bool(false));
  addParam("LandauFluct", m_landauFluct,
           "Use internal Landau fluctuations (instead of Geant4)?", bool(true));
  addParam("LandauBetaGammaCut", m_landauBetaGammaCut,
           "Below this beta*gamma factor internal Landau fluctuations not used",
           double(0.7));
  addParam("PoissonSmearing", m_PoissonSmearing,
           "Apply Poisson smearing of electrons collected on pixels?",
           bool(true));
  addParam("ProductionThreshOnDeltaRays", m_prodThreshOnDeltaRays,
           "Production threshold cut on delta electrons in keV (for Landau fluct.) - use the same as in Geant4 (80keV ~ 0.05 mm)",
           double(80));
  addParam("SegmentLength", m_segmentLength, "Segment length (in mm)",
           double(0.005));
  addParam("TanLorentz", m_tanLorentzAngle, "Tangent of the Lorentz angle",
           double(0.25));
  addParam("widthOfDiffusCload", m_widthOfDiffusCloud,
           "Integration range the e- cloud in sigmas", double(3.0));
  addParam("StartIntegrationFirstLayer", m_startIntegrationFirstLayer,
           "SimHits that appear before this time (ns) are thrown away",
           double(-10000.0));
  addParam("StopIntegrationFirstLayer", m_stopIntegrationFirstLayer,
           "SimHits that appear after this time (ns) will be thrown away",
           double(10000.0));
  addParam("StartIntegrationOtherLayer", m_startIntegrationOtherLayer,
           "SimHits that appear before this time (ns) will be thrown away",
           double(-10000.0));
  addParam("StopIntegrationOtherLayer", m_stopIntegrationOtherLayer,
           "SimHits that appear after this time (ns) will be thrown away",
           double(10000.0));
} // PXDDigiModule c'tor

PXDDigiModule::~PXDDigiModule()
{
  if (m_random != NULL)
    delete m_random;
  if (m_fluctuate != NULL)
    delete m_fluctuate;
}

//
// Method called at the beginning of data processing
//
void PXDDigiModule::initialize()
{
  // Initialize variables
  m_nRun = 0;
  m_nEvt = 0;

  // These guys are heavily cecycled.
  m_currentLayerID = 0;
  m_currentLadderID = 0;
  m_currentSensorID = 0;

  m_sensorThick = -1.;
  m_sensorWidth = -1.;
  m_sensorLength = -1.;

  // Parameters are entered in basf2 unit system.
  m_ADCRange = static_cast<int>(m_ADCRange * Unit::e);
  m_ADCUnit = m_ADCRange / pow(2.0, static_cast<double>(m_ADCBits));
  m_prodThreshOnDeltaRays *= Unit::keV;
  m_diffusionCoefficient *= Unit::mm;
  m_elNoise *= Unit::e;
  m_segmentLength *= Unit::mm;

  m_startIntegrationFirstLayer *= Unit::ns;
  m_stopIntegrationFirstLayer *= Unit::ns;
  m_startIntegrationOtherLayer *= Unit::ns;
  m_stopIntegrationOtherLayer *= Unit::ns;

  // Print set parameters
  printModuleParams();

  // CPU time start
  m_timeCPU = clock() * Unit::ms / 1000;

  // Internal Landau fluctuations
  if (m_landauFluct)
    m_fluctuate = new SiEnergyFluct(m_prodThreshOnDeltaRays);


  //
  // ROOT variables
  //
#ifdef ROOT_OUTPUT
  m_rootFile = new TFile("BelleII_PXD_Hits.root", "recreate");

  m_rootFile->cd("");

  // Declare Tree
  m_rootTree = new TTree("Hits", "Hit info");

  // After 50MB it will flush the data
  m_rootTree->SetAutoFlush(-2000000);

  m_rootTree->Branch("Layer" , &m_rootLayerID , "Layer/I");
  m_rootTree->Branch("Ladder" , &m_rootLadderID , "Ladder/I");
  m_rootTree->Branch("Sensor" , &m_rootSensorID , "Sensor/I");
  m_rootTree->Branch("RPhiSim" , &m_rootSimRPhi , "SimRPhi/D");
  m_rootTree->Branch("ZSim" , &m_rootSimZ , "SimZ/D");
  m_rootTree->Branch("MCPDG" , &m_rootMCPDG , "MCPDG/I");
  m_rootTree->Branch("IsFromMCPrim", &m_rootIsFromMCPrim, "IsFromMCPrim/I");
  m_rootTree->Branch("EvtNum" , &m_rootEvtNum , "EvtNum/I");

  // Declare Efficiency histogram
  m_rootEfficiency = new TH1F("Efficiency" , "Muon hit reconstruction efficiency" , 101, -0.005, 1.005);
  m_rootMultiplicity = new TH1F("Multiplicity", "Muon hit reconstruction multiplicity" , 501, -0.005, 5.005);
  m_rootELossG4      = new TH1F("ELossG4"     , "G4 - Energy loss in silicon"                    , 200, 0.   , 200.);
  m_rootELossDigi    = new TH1F("ELossDigi"   , "Digi - Energy loss in silicon"                  , 200, 0.   , 200.);
  m_rootDigitsMap    = new TH2F("DigitsMap"   , "Digits map"                                     , 5000, 0., 5000., 1600, 0., 1600.);
  m_rootOccupancy_0 = new TH1F("Occupancy_0" , "Occupancy_0" , 101, -0.05, 10.05);
  m_rootOccupancy_1 = new TH1F("Occupancy_1" , "Occupancy_1" , 101, -0.05, 10.05);

#endif
} //PXDDigiModule::initialize()

//
// Method called for each run
//
void PXDDigiModule::beginRun()
{
  // Print run number
  B2INFO("PXDDigi: Processing run: " << m_nRun);

  // Re-initialize geometry cache
  m_geometry = SiGeoCache::instance();
  m_geometry->refresh();

} // PXDDigiModule::beginRun()

//
// Method called for each event
//
void PXDDigiModule::event()
{
  //local error count variable
  static int hits_throwaway = 0;

  /** Number of digits stored in this event. */
  int m_nDigitsSaved = 0;

  /** Number of relations saved in this event. */
  int m_nRelationsSaved = 0;

  // Initialize number of found muon hits versus all hits - efficiency map, resp. dep. energy in each event
#ifdef ROOT_OUTPUT

  m_rootEvtNum = m_nEvt;

  m_rootMCGenHitMap.clear();

  m_rootDepEG4   = 0.;
  m_rootDepEDigi = 0.;

#endif

  //------------------------------------------------------
  // Get the collection of MCParticles from the DataStore.
  //------------------------------------------------------
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  if (!storeMCParticles) {
    B2ERROR("PXDDigi: Input collection " << m_mcColName << " unavailable.");
  }

  //------------------------------------------------------
  // Get the collection of PXDSimHits from the DataStore.
  //------------------------------------------------------
  StoreArray<PXDSimHit> storeSimHits(m_simColName);
  if (!storeSimHits) {
    B2ERROR("PXDDigi: Input collection " << m_simColName << " unavailable.");
  }

  // Number of SimHits on input
  StoreIndex nSimHits = storeSimHits->GetEntries();
  if (nSimHits == 0) return;

  // Create HitSorter structure and fill it
  HitRecordSet simHitSet;
  SensorSet sensorSet;
  SensorUIDManager uidCodec(0);
  for (StoreIndex iHit = 0; iHit < nSimHits; ++iHit) {
    PXDSimHit* hit = storeSimHits[iHit];
    HitRecord aHitRec;
    aHitRec.m_index = iHit;
    uidCodec.setLayerID(hit->getLayerID());
    uidCodec.setLadderID(hit->getLadderID());
    uidCodec.setSensorID(hit->getSensorID());
    aHitRec.m_sensorUID = uidCodec.getSensorUID();
    simHitSet.insert(aHitRec);
    sensorSet.insert(aHitRec.m_sensorUID);
  }

  // get the sensor-side index
  SensorSideIndex& sensorIndex = simHitSet.get<SensorUIDSide>();

  //------------------------------------------------------
  // Get the MCParticle->PXDSimHit relations from the
  // DataStore and create support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToSimHits(m_relSimName);
  if (!storeMCToSimHits) {
    B2ERROR("PXDDigi: Input collection " << m_relSimName << " unavailable.");
  }

  // Fill with relation data
  int nMCSimRels = storeMCToSimHits.GetEntries();
  for (int iRel = 0; iRel < nMCSimRels; ++iRel) {
    Relation* rel = storeMCToSimHits[iRel];
    AtomicRelation arel;
    arel.m_from = rel->getFromIndex();
    RelList toIndices = rel->getToIndices();
    for (RelListItr idx = toIndices.begin(); idx != toIndices.end(); ++idx) {
      arel.m_to = (*idx);
      arel.m_weight = 1.0; // no way to retrieve weights.
      m_relMCSim.insert(arel);
    }
  }

  // Get the "to-side" index to relations.
  ToSideIndex& simIndex = m_relMCSim.get<ToSide>();

  //------------------------------------------------------
  // Create the collection of PXDDigits in the DataStore
  //------------------------------------------------------
  StoreArray<PXDDigit> storeDigits(m_digitColName);
  if (!storeDigits) {
    B2ERROR("PXDDigi: Output collection " << m_digitColName << " unavailable.");
  }

  //------------------------------------------------------
  // Initialize the collection of  MCParticle->Digit
  // relations in the DataStore.
  // No need of support structure.
  //------------------------------------------------------
  StoreArray<Relation> storeMCToDigits(m_relDigitName);
  if (!storeMCToSimHits) {
    B2ERROR("PXDDigi: Input collection " << m_relSimName << " unavailable.");
  }

  //------------------------------------------------------
  // Loop over SimHits:
  // - first over set of sensors
  // - then over hits in individual sensors.
  //------------------------------------------------------

  for (SensorSetItr iSensor = sensorSet.begin(); iSensor != sensorSet.end(); ++iSensor) {

    //------------------------------------------------------------------------
    // Initialize sensor-related data.
    //------------------------------------------------------------------------

    m_currentSensorUID = *iSensor;
    m_currentLayerID = m_geometry->getLayerID(m_currentSensorUID);
    m_currentLadderID = m_geometry->getLadderID(m_currentSensorUID);
    m_currentSensorID = m_geometry->getSensorID(m_currentSensorUID);

    // Check if digitizing pixels
    if (m_geometry->getLayerType(m_currentLayerID) != SiGeoCache::c_pixel) {
      B2ERROR("PXDDigiModule::event() - sensor not of pixel type!!!")
      continue;
    }

    // Get current sensor thickness, width, length
    m_sensorThick = m_geometry->getSensorThickness(m_currentSensorUID);
    m_sensorWidth = m_geometry->getUSensorSize(m_currentSensorUID);
    m_sensorLength = m_geometry->getVSensorSize(m_currentSensorUID);

    // Print
    B2INFO(" LayerID: " << m_currentLayerID
           << ", LadderID: " << m_currentLadderID
           << ", SensorID: " << m_currentSensorID)

    // Initialize map<cellUID, Digit> container for digits in the sensor.
    // Notes:
    // - we use Digits, containing lists of contributing SimHits, and only
    // convert to PXDDigits (and relations) on output.
    // - the digits and relations are also saved sensor-wise to keep memory
    // profile low.
    DigitMap digits;

    //------------------------------------------------------------------------
    // Construct the set of hits that lie within this sensor
    //------------------------------------------------------------------------
    std::pair<SensorSideItr, SensorSideItr> sensorHits =
      sensorIndex.equal_range(m_currentSensorUID);

    //------------------------------------------------------------------------
    // Loop over hits in the current sensor
    //------------------------------------------------------------------------
    for (SensorSideItr hitItr = sensorHits.first; hitItr != sensorHits.second; ++ hitItr) {

      // --------------------------------------------------------------------
      // Get the current SimHit and MCParticle
      //---------------------------------------------------------------------
      StoreIndex iHit = hitItr->m_index;
      PXDSimHit * simHit = storeSimHits[iHit];
      if (!simHit) continue;

      // Get its MCParticle, if any
      MCParticle * mcPart = 0;
      // We expect no more than one MCParticle per PXDSimHit!
      ToSideItr mcItr = simIndex.find(iHit);
      if (mcItr != simIndex.end())
        mcPart = storeMCParticles[mcItr->m_from];


#ifdef ROOT_OUTPUT
      //---------------------------------------------------------------------
      // Fill map of hits generated by primary particle
      //---------------------------------------------------------------------
      if (mcPart != 0 && mcPart->hasStatus(MCParticle::c_PrimaryParticle)) {
        m_rootMCGenHitMap[m_currentSensorUID]++;
      } // if primary particle

      //---------------------------------------------------------------------
      // Update energy deposited in Geant4 (in each event)
      //---------------------------------------------------------------------
      m_rootDepEG4 += simHit->getEnergyDep() * Unit::GeV;
#endif

      //---------------------------------------------------------------------
      // Cut on simHit creation time: simulate integration time of a sensor
      //---------------------------------------------------------------------
      if (m_integrationWindow) {

        if (m_currentLayerID == 1) {
          if (simHit->getGlobalTime() * Unit::ns < m_startIntegrationFirstLayer
              || simHit->getGlobalTime() * Unit::ns > m_stopIntegrationFirstLayer) {
            hits_throwaway++;
            continue;
          }
        } else { // other layer
          if (simHit->getGlobalTime() * Unit::ns < m_startIntegrationOtherLayer
              || simHit->getGlobalTime() * Unit::ns > m_stopIntegrationOtherLayer) {

            hits_throwaway++;
            continue;
          }
        }
      }

      //---------------------------------------------------------------------
      // Print hit data
      //---------------------------------------------------------------------
      B2INFO(" Simulated hit local[cm]: (" << simHit->getPosIn()[1]
             << ", " << simHit->getPosIn()[2] << ")")

      //---------------------------------------------------------------------
      // Produce ionization points along the track
      //---------------------------------------------------------------------
      IonizationPointVec ionizationPoints;
      produceIonizationPoints(iHit, ionizationPoints);

      //---------------------------------------------------------------------
      // Produce signal points
      //---------------------------------------------------------------------
      SignalPointVec signalPoints;
      produceSignalPoints(ionizationPoints, signalPoints);
      ionizationPoints.clear();

      //---------------------------------------------------------------------
      // Produce digits
      //---------------------------------------------------------------------
      produceDigits(signalPoints, iHit, digits);
      signalPoints.clear();

    } // Loop over PXDSimHits

    //------------------------------------------------------------------------
    // Add pixel noise (Poisson + electronic) to digits.
    //------------------------------------------------------------------------
    addPixelNoise(digits);

    //------------------------------------------------------------------------
    // Produce noise digits
    //------------------------------------------------------------------------
    addNoiseDigits(digits);

#if 0
    //------------------------------------------------------------------------
    // If ganged pixels used, simulate their effect
    //------------------------------------------------------------------------
    if (m_gangedPixels) {
      B2INFO(" Simulating effect of ganged pixels ...")
      SimulateGangedPixels(digits);
    }
#endif

    //------------------------------------------------------------------------
    // Save digits and relations to DataStore.
    //------------------------------------------------------------------------
    // We save at AddrAt(m_nDigitsSaved++) and AddrAt(m_nRelationsSaved++]
    saveDigits(digits);

    digits.clear();

  } // Loop over sensors

  // Clear the relation indexer
  m_relMCSim.clear();

  //---------------------------------------------------------------------------
  // Save deposited energy in histograms
  //---------------------------------------------------------------------------
#ifdef ROOT_OUTPUT

  m_rootELossG4->Fill(m_rootDepEG4 / Unit::keV);
  m_rootELossDigi->Fill(m_rootDepEDigi / Unit::keV);

#endif

  // Debug info
  if (hits_throwaway != 0) {
    B2DEBUG(100, "PXDSimHits outside " << hits_throwaway << " times outside of integration time of sensor")
  }
  hits_throwaway = 0;
  m_nDigitsSaved = 0;
  m_nRelationsSaved = 0;

  m_nEvt++;

  // Print event number
  if (m_nEvt % 100 == 0)B2INFO("Events processed: " << m_nEvt)
  } //PXDDigiModule::event()

//
// Method called after each run to check the data processed
//
void PXDDigiModule::endRun()
{
  m_nRun++;
} //PXDDigiModule::endRun()

//
// Method called after all data processing
//
void PXDDigiModule::terminate()
{

  //---------------------------------------------------------------------------
  // CPU time end
  //---------------------------------------------------------------------------
  m_timeCPU = clock() * Unit::ms / 1000 - m_timeCPU;

  //---------------------------------------------------------------------------
  // Print message
  //---------------------------------------------------------------------------
  B2INFO(" Time per event: "
         << std::setiosflags(std::ios::fixed | std::ios::internal)
         << std::setprecision(3)
         << m_timeCPU / m_nEvt / Unit::ms
         << " ms"
         << std::endl
         << " Processor successfully finished!"
        )

  //---------------------------------------------------------------------------
  // Clear things out of memory.
  //---------------------------------------------------------------------------

#ifdef ROOT_OUTPUT

  // Close file
  m_rootFile->cd("");
  m_rootFile->Write();
  m_rootFile->Close();

#endif

} //PXDDigiModule::terminate()



//---------------------------------------------------------------------
// Produce ionization points along the track
//---------------------------------------------------------------------
//
void PXDDigiModule::produceIonizationPoints(StoreIndex iHit,
                                            IonizationPointVec & ionizationPoints)
{
  //---------------------------------------------------------------------------
  // Initialize collections, get the current SimHit and MCParticle.
  //---------------------------------------------------------------------------
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  StoreArray<PXDSimHit> storeSimHits(m_simColName);

  PXDSimHit * simHit = storeSimHits[iHit];

  // Get the "to-side" index to relations.
  ToSideIndex& simIndex = m_relMCSim.get<ToSide>();

  // Get the hit's MCParticle, if any
  MCParticle * mcPart = 0;
  // We expect no more than one MCParticle per PXDSimHit!
  ToSideItr mcItr = simIndex.find(iHit);
  if (mcItr != simIndex.end())
    mcPart = storeMCParticles[mcItr->m_from];

  //---------------------------------------------------------------------------
  // Initialize particle/track/hit data
  //---------------------------------------------------------------------------
  TVector3 partMomentum(simHit->getMomIn() * Unit::GeV);
  double partMomentumMag = partMomentum.Mag();
  double partMass = 0.;
  double betaGamma = 0.;
  if (mcPart != 0)
    partMass = mcPart->getMass() * Unit::GeV;
  if (mcPart != 0 && partMass != 0.)
    betaGamma = partMomentumMag / partMass;
  double partdEMean = simHit->getEnergyDep() * Unit::GeV;

  // Decide if particle is low-energy --> Landau fluctuations in low energy regime
  bool lowEnergyPart = false;
  if (betaGamma < m_landauBetaGammaCut)
    lowEnergyPart = true;

  // Zero momentum --> nothing to do ...
  if (partMomentumMag == 0) {
    B2INFO(" Hit dismissed - particle momentum = 0!!!")
    return;
  }

  // Print info
  B2INFO(" Producing ionization points ...")

  //---------------------------------------------------------------------------
  // Calculate track segment parameters.
  //---------------------------------------------------------------------------
  const TVector3 entryPoint = simHit->getPosIn();
  const TVector3 exitPoint = simHit->getPosOut();

  double trackLength = (exitPoint - entryPoint).Mag();
  TVector3 direction = 1.0 / trackLength * (exitPoint - entryPoint);

  //---------------------------------------------------------------------------
  // Calculate number of segments and recalculate segment length
  //---------------------------------------------------------------------------
  int numberOfSegments = int(trackLength / m_segmentLength) + 1;
  double segmentLength = trackLength / numberOfSegments;

  // Calculate original mean energy in each segment
  partdEMean = partdEMean / numberOfSegments;

  //---------------------------------------------------------------------------
  // Set ionization points
  //---------------------------------------------------------------------------
  ionizationPoints.resize(numberOfSegments);

  // Print
  B2INFO("  Ionization points: ")

  for (int iPoint = 0; iPoint < numberOfSegments; iPoint++) {

    ionizationPoints.at(iPoint).position =
      entryPoint + direction * segmentLength * (iPoint + 0.5);
#if 0
    if (m_landauFluct && !lowEnergyPart && mcPart != 0)
      // High energy regime --> use internal Landau fluctuation (if defined)
      ionizationPoints.at(iPoint).eLoss =
        m_fluctuate->SampleFluctuations(mcPart, segmentLength / Unit::mm) * Unit::keV;
    else
      // Low energy regime or Geant4 information unavailable:
      // Use Geant4 info and distribute energy deposition uniformly
#endif
      ionizationPoints.at(iPoint).eLoss = partdEMean;

#ifdef ROOT_OUTPUT
    // Update info
    m_rootDepEDigi += ionizationPoints.at(iPoint).eLoss;
#endif

    //------------------------------------------------------------------------
    // Print
    //------------------------------------------------------------------------
    B2INFO("   Pos [mm]: ( " << ionizationPoints.at(iPoint).position.X() / Unit::mm << ", "
           << ionizationPoints.at(iPoint).position.Y() / Unit::mm << ", "
           << ionizationPoints.at(iPoint).position.Z() / Unit::mm << " )"
           << " , dE [keV]: " << ionizationPoints.at(iPoint).eLoss / Unit::keV)

  } // for i
} // generate Ionization points


//---------------------------------------------------------------------
// Produce signal points on the surface of the sensor.
//---------------------------------------------------------------------

void PXDDigiModule::produceSignalPoints(
  const IonizationPointVec & ionizationPoints,
  SignalPointVec & signalPoints)
{
  //---------------------------------------------------------------------------
  // Loop over ionization points and produce signal points.
  //---------------------------------------------------------------------------
  int numberOfIonPoints = ionizationPoints.size();
  signalPoints.resize(numberOfIonPoints);

  // Print
  B2INFO("  Producing signal points: ")

  for (int iPt = 0; iPt < numberOfIonPoints; iPt++) {

    // Get current ionization point
    const IonizationPoint& iPoint = ionizationPoints.at(iPt);

    // Calculate mean trajectory of charge carriers
    TVector3 finalPos(iPoint.position);

    // Drift to surface
    double distanceToPlane = 0.5 * m_sensorThick - iPoint.position.X();
    finalPos.SetX(0.5 * m_sensorThick);

    //  Apply Lorentz shift
    double onPlaneY = iPoint.position.Y() + m_tanLorentzAngle
                      * distanceToPlane;
    double onPlaneZ = iPoint.position.Z() + 0.;

    finalPos.SetY(onPlaneY);
    finalPos.SetZ(onPlaneZ);

    //  After diffusion - calculate sigma
    double driftLength = (finalPos - iPoint.position).Mag();
    double invCosLorentzAngle =
      TMath::Sqrt(1.0 + m_tanLorentzAngle * m_tanLorentzAngle);
    double sigmaDiffus = TMath::Sqrt(driftLength * m_diffusionCoefficient);
    double sigmaY = sigmaDiffus * invCosLorentzAngle;
    double sigmaZ = sigmaDiffus;

    //------------------------------------------------------------------------
    // Save info in signal point
    //------------------------------------------------------------------------
    signalPoints.at(iPt).position = finalPos;
    signalPoints.at(iPt).sigma.SetXYZ(0., sigmaY, sigmaZ);
    signalPoints.at(iPt).charge = iPoint.eLoss * Unit::GeV / Unit::energy_eh;

    // Print
    B2INFO("   Pos [mm]:( " << signalPoints.at(iPt).position.X() << ", "
           << signalPoints.at(iPt).position.Y() << ", "
           << signalPoints.at(iPt).position.Z() << " )"
           << " , q [ENC]: " << signalPoints.at(iPt).charge)

  } // for signal points
} // Generate signal points

//---------------------------------------------------------------------
// Produce Digits from signal points.
//---------------------------------------------------------------------
void PXDDigiModule::produceDigits(const SignalPointVec & signalPoints,
                                  StoreIndex iSimHit, DigitMap & digits)
{
  // Calculate number of signal points
  int numberOfSigPoints = signalPoints.size();

  //---------------------------------------------------------------------------
  // Initialize dataStore collections and the corresponding PXDHit
  // and MCParticle to reconstruct relations.
  //---------------------------------------------------------------------------
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  StoreArray<PXDSimHit> storeSimHits(m_simColName);

  // Get the "to-side" index to relations.
  ToSideIndex& simIndex = m_relMCSim.get<ToSide>();

  // Get the hit's MCParticle, if any
  MCParticle * mcPart = 0;
  StoreIndex iMCPart = 0;
  // We expect no more than one MCParticle per PXDSimHit!
  ToSideItr mcItr = simIndex.find(iSimHit);
  if (mcItr != simIndex.end()) {
    iMCPart = mcItr->m_from;
    mcPart = storeMCParticles[iMCPart];
  }

  //---------------------------------------------------------------------------
  // Loop over all signal points and generate digits and relations.
  //---------------------------------------------------------------------------

  // Print
  B2INFO("  Producing digits: ")

  for (int iPoint = 0; iPoint < numberOfSigPoints; ++iPoint) {

    // Get sensor pitch
    double pitchZ = m_geometry->getVSensorPitch(m_currentSensorUID);
    double pitchRPhi = m_geometry->getUSensorPitch(m_currentSensorID);

    // Get current signal point
    const SignalPoint& sPoint = signalPoints[iPoint];

    // Calculate center
    double centreZ = sPoint.position.Z();
    double centreRPhi = sPoint.position.Y();

    // Calculate sigma
    double sigmaZ = sPoint.sigma.Z();
    double sigmaRPhi = sPoint.sigma.Y();

    // Calculate low and upper limit in Z
    double lowZ = centreZ - m_widthOfDiffusCloud * sigmaZ;
    double upZ = centreZ + m_widthOfDiffusCloud * sigmaZ;

    // Find corresponding cellID in Z, adjust to pixel range
    int iLowZ = max(0, m_geometry->getVSensorCellID(m_currentSensorUID, lowZ));
    int iUpZ = min(m_geometry->getVSensorCells(m_currentSensorUID) - 1,
                   m_geometry->getVSensorCellID(m_currentSensorUID, upZ));

    // Calculate low and upper limit in RPhi
    double lowRPhi = centreRPhi - m_widthOfDiffusCloud * sigmaRPhi;
    double upRPhi = centreRPhi + m_widthOfDiffusCloud * sigmaRPhi;

    // Find corresponding cellID in RPhi, adjust to pixel range
    int iLowRPhi = max(0, m_geometry->getUSensorCellID(m_currentSensorUID, lowRPhi));
    int iUpRPhi = min(m_geometry->getUSensorCells(m_currentSensorUID) - 1,
                      m_geometry->getUSensorCellID(m_currentSensorUID, upRPhi));


    // Integrate charge - effect of diffusion

    for (int iZ = iLowZ; iZ <= iUpZ; ++iZ) {

      // Find current position in Z
      double posZ = m_geometry->getVCellPosition(m_currentSensorUID, iZ);

      // Integral in Z
      double lowerTailZ = TMath::Freq((posZ - 0.5 * pitchZ - centreZ)
                                      / sigmaZ);
      double upperTailZ = TMath::Freq((posZ + 0.5 * pitchZ - centreZ)
                                      / sigmaZ);
      double integralZ = upperTailZ - lowerTailZ;

      for (int iRPhi = iLowRPhi; iRPhi <= iUpRPhi; ++iRPhi) {

        // Find current position in RPhi
        double posRPhi = m_geometry->getUCellPosition(
                           m_currentSensorID, iRPhi);

        // Integral in RPhi
        double lowerTailRPhi = TMath::Freq((posRPhi - 0.5 * pitchRPhi
                                            - centreRPhi) / sigmaRPhi);
        double upperTailRPhi = TMath::Freq((posRPhi + 0.5 * pitchRPhi
                                            - centreRPhi) / sigmaRPhi);
        double integralRPhi = upperTailRPhi - lowerTailRPhi;

        // Calculate charge collected by cell[iZ,iRPhi]
        double charge = sPoint.charge * integralRPhi * integralZ;

        // Update digit info if exists, otherwise create a new one
        int cellUID = m_geometry->getCellUID(iRPhi, iZ);
        DigitMapItr itrCell = digits.find(cellUID);
        if (itrCell == digits.end()) {
          // Insert a new digit
          Digit newdigit;
          newdigit.cellIDRPhi = iRPhi;
          newdigit.cellIDZ = iZ;
          newdigit.cellPosRPhi = posRPhi;
          newdigit.cellPosZ = posZ;
          newdigit.charge = charge;
          newdigit.pxdSimHitMap[iSimHit] = charge;
          digits[cellUID] = newdigit;
        } else {
          // Update the digit found with new data
          Digit& currentDigit = itrCell->second;
          currentDigit.charge += charge;
          currentDigit.pxdSimHitMap[iSimHit] += charge;
        }

      }
    } // Integration - effect of diffusion

  } // For signalPoints

}

//---------------------------------------------------------------------
// Add Poisson smearing and electronic noise to digits
//---------------------------------------------------------------------
void PXDDigiModule::addPixelNoise(DigitMap & digits)
{
  //---------------------------------------------------------------------------
  // NOTE_PQ: We also have to consider pedestal noise and random pixel gains.
  //---------------------------------------------------------------------------

  for (DigitMapItr iterDMap = digits.begin(); iterDMap != digits.end(); iterDMap++) {

    Digit& digit = iterDMap->second;

    //------------------------------------------------------------------------
    // Add Poisson smearing
    //------------------------------------------------------------------------
    if (m_PoissonSmearing) {

      double charge = digit.charge;

      // For big charge assume Gaussian distr.
      if (charge > (1000. * Unit::e))
        digit.charge = m_random->Gaus(charge, TMath::Sqrt(charge));
      else  // Otherwise Poisson distr.
        digit.charge = m_random->Poisson(charge);

    } // if Poisson

    //------------------------------------------------------------------------
    // Add electronics effects
    //------------------------------------------------------------------------
    if (m_electronicEffects)
      digit.charge += m_random->Gaus(0., m_elNoise);

  } // For digits

} // addPixelNoise


//---------------------------------------------------------------------
// Produce noise digits and add them to the DigitsMap.
//---------------------------------------------------------------------
void PXDDigiModule::addNoiseDigits(DigitMap & digits)
{
  // Pixels with charge above this threshold will be taken into account by
  // clustering algorithms, all other will be "zero-suppressed".

  // Don't generate random hits in a sensor without signal.
  if (digits.size() == 0) return;

  // Calculate the fraction of pixels that will be above threshold.
  double fraction = 1.0 - TMath::Freq(m_SNAdjacent);

  // Number of noise pixels is approximately Poissonian
  int fractionPixels = m_random->Poisson(fraction
                                         * m_geometry->getUSensorCells(m_currentSensorUID)
                                         * m_geometry->getVSensorCells(m_currentSensorUID));

  // Save pixel dimensions of the sensor
  short int nU = m_geometry->getUSensorCells(m_currentSensorUID);
  short int nV = m_geometry->getVSensorCells(m_currentSensorUID);

  // Generate noise digits
  for (int iNoisePixel = 0; iNoisePixel < fractionPixels; iNoisePixel++) {

    short int iPixelU = m_random->Integer(nU);
    short int iPixelV = m_random->Integer(nV);

    // Describe pixel by unique ID
    int cellUID = m_geometry->getCellUID(iPixelU, iPixelV);

    // Only set pixels that have not been already set
    if (digits.find(cellUID) != digits.end()) continue;

    // Generate random charge above the cut
    double charge =
      TMath::NormQuantile(m_random->Uniform(TMath::Freq(m_SNAdjacent), 1.0)) * m_elNoise;

    // Create new noise digit
    Digit newDigit;
    newDigit.cellIDRPhi = iPixelU;
    newDigit.cellIDZ = iPixelV;
    newDigit.cellPosRPhi = m_geometry->getUCellPosition(m_currentSensorUID, iPixelU);
    newDigit.cellPosZ = m_geometry->getVCellPosition(m_currentSensorUID, iPixelV);
    newDigit.charge = charge;
    // ... and there is no relation to a simHit.
  } // generate random pixels

} // ProduceNoiseDigits

#if 0
//---------------------------------------------------------------------
// Simulate the effect of ganged pixels.
//---------------------------------------------------------------------
void PXDDigiModule::simulateGangedPixels(DigitMap & digits)
{
  // Set-up a new DigitMap
  DigitMap gangedDigits;

  // Go through all pixels and remap pixels
  for (iterSensorMap = digitsSensorMap.begin(); iterSensorMap
       != digitsSensorMap.end(); iterSensorMap++) {

    //
    // Get digit & copy it's content to a new digit
    Digit * currentDigit = iterSensorMap->second;
    Digit * newDigit = new Digit;

    // Cell information is initialized later on
    newDigit->cellIDRPhi = 0;
    newDigit->cellIDZ = 0;
    newDigit->cellPosRPhi = 0.;
    newDigit->cellPosZ = 0.;
    newDigit->charge = currentDigit->charge;
    newDigit->simTrkHitMap = currentDigit->simTrkHitMap;

    int currentIDRPhi = currentDigit->cellIDRPhi;
    int currentIDZ = currentDigit->cellIDZ;
    int newIDRPhi = 0;
    int newIDZ = 0;

    // Upper sensor half
    if ((currentIDZ / (m_geometry->getSensorNPixelsInZ(m_currentLayerID,
                                                       m_currentSensorID) / 2)) == 0) {

      // New IDs
      if (m_gangedPixels == 1) {

        newIDRPhi = currentIDRPhi;
        newIDZ = currentIDZ + m_geometry->getSensorNPixelsInZ(
                   m_currentLayerID, m_currentSensorID) / 2;
      } else {

        newIDRPhi = currentIDRPhi;
        newIDZ = (((currentIDZ % 16) % 4) * 4 + (currentIDZ % 16) / 4
                  + 4) % 16 + (currentIDZ / 16) * 16
                 + m_geometry->getSensorNPixelsInZ(m_currentLayerID,
                                                   m_currentSensorID) / 2;
      }
    }
    // Lower sensor half
    else {

      // New IDs
      if (m_gangedPixels == 1) {

        newIDRPhi = currentIDRPhi;
        newIDZ = currentIDZ - m_geometry->getSensorNPixelsInZ(
                   m_currentLayerID, m_currentSensorID) / 2;
      } else {

        newIDRPhi = currentIDRPhi;
        newIDZ = currentIDZ - m_geometry->getSensorNPixelsInZ(
                   m_currentLayerID, m_currentSensorID) / 2;
        newIDZ = (((newIDZ % 16) % 4) * 4 + (newIDZ % 16) / 4 + 4) % 16
                 + (newIDZ / 16) * 16;
      }
    }

    // Set new digit IDs and positions
    newDigit->cellIDRPhi = newIDRPhi;
    newDigit->cellIDZ = newIDZ;
    newDigit->cellPosRPhi
    = m_geometry->getPixelPosInRPhi(m_currentLayerID,
                                    m_currentSensorID, false, newIDRPhi, newIDZ);
    newDigit->cellPosZ = m_geometry->getPixelPosInZ(m_currentLayerID,
                                                    m_currentSensorID, newIDZ);

    // New encoded sensor ID
    int currentPixelID = m_geometry->encodePixelID(m_currentLayerID,
                                                   m_currentSensorID, currentIDRPhi, currentIDZ);
    int cellUID = m_geometry->encodePixelID(m_currentLayerID,
                                            m_currentSensorID, newIDRPhi, newIDZ);

    // Add current digit
    if (gangedDigits.find(currentPixelID) != gangedDigits.end()) {

      // Update charge
      gangedDigits[currentPixelID]->charge += currentDigit->charge;

      // Update weight of SimTrackerHits
      SimTrackerHitMap gangedMap =
        gangedDigits[currentPixelID]->simTrkHitMap;
      SimTrackerHitMap::iterator iterSHM;

      for (iterSHM = currentDigit->simTrkHitMap.begin(); iterSHM
           != currentDigit->simTrkHitMap.end(); iterSHM++) {

        PXDSimHit * simHit = iterSHM->first;
        float weight = iterSHM->second;

        if (gangedMap.find(simHit) != gangedMap.end())
          gangedMap[simHit] += weight;
        else
          gangedMap[simHit] = weight;
      }

      // Clear memory
      delete currentDigit;

    } else
      gangedDigits[currentPixelID] = currentDigit;

    // Add new digit (into the ganged half of the sensor)
    if (gangedDigits.find(cellUID) != gangedDigits.end()) {

      // Update charge
      gangedDigits[cellUID]->charge += newDigit->charge;

      // Update weight of SimTrackerHits
      SimTrackerHitMap gangedMap =
        gangedDigits[cellUID]->simTrkHitMap;
      SimTrackerHitMap::iterator iterSHM;

      for (iterSHM = newDigit->simTrkHitMap.begin(); iterSHM
           != newDigit->simTrkHitMap.end(); iterSHM++) {

        PXDSimHit * simHit = iterSHM->first;
        float weight = iterSHM->second;

        if (gangedMap.find(simHit) != gangedMap.end())
          gangedMap[simHit] += weight;
        else
          gangedMap[simHit] = weight;
      }

      // Clear memory
      delete newDigit;

    }
    // Generate noise and add new digit
    else {

      // Update charge with noise (double-noise - 2 pixels are ganged)
      if (m_electronicEffects) {

        double noise = m_random->Gaus(0., m_elNoise);
        newDigit->charge += noise;
      }

      gangedDigits[cellUID] = newDigit;
    }
  } // For

  // Clear digitsSensorMap and save gangedSensorMap instead
  digitsSensorMap.clear();

  iterDigitsMap->second = gangedDigits; //PQ??: Isn't this local? Hope it survives...

} // For
#endif

//------------------------------------------------------------------------------
// Save digits collected for a single sensor to the DataStore as PXDDigits.
// Also save relations of digits to MCParticles.
//------------------------------------------------------------------------------
void PXDDigiModule::saveDigits(DigitMap & digits)
{
  //---------------------------------------------------------------------------
  // Initialize dataStore collections and the corresponding PXDHit
  // and MCParticle to reconstruct relations.
  //---------------------------------------------------------------------------
  StoreArray<MCParticle> storeMCParticles(m_mcColName);
  StoreArray<PXDDigit> storeDigits(m_digitColName);
  StoreArray<Relation> storeDigitsRel(m_relDigitName);

  // Get the "to-side" index to relations.
  ToSideIndex& simIndex = m_relMCSim.get<ToSide>();

  //---------------------------------------------------------------------------
  // Loop over the DigitMap and generate PXDDigits and relations.
  //---------------------------------------------------------------------------

  // Print
  B2INFO("  Saving digits: ")

  for (DigitMapItr iDigit = digits.begin(); iDigit != digits.end(); ++iDigit) {

    Digit& digit = iDigit->second;

    // Add new PXDDigit to the StoreArray
    new(storeDigits->AddrAt(m_nDigitsSaved++)) PXDDigit(
      m_currentSensorUID,                    // m_sensorUID
      static_cast<short>(digit.cellIDRPhi),  // m_uCellID
      static_cast<short>(digit.cellIDZ),     // m_vCellID
      static_cast<float>(digit.cellPosRPhi), // m_uCellPosition
      static_cast<float>(digit.cellPosZ),    // m_vCellPosition
      static_cast<float>(digit.charge)       // m_charge
    );

    // Turn list of hits and weights into relations.
    for (HitRelationMapItr iLink = digit.pxdSimHitMap.begin();
         iLink != digit.pxdSimHitMap.end(); ++iLink) {

      // Get hit index and weight
      StoreIndex iHit = iLink->first;
      double weight = iLink->second;

      // Get the hit's MCParticle: should be at most one!
      StoreIndex iMCPart = 0;
      ToSideItr mcItr = simIndex.find(iHit);
      if (mcItr != simIndex.end())
        iMCPart = mcItr->m_from;

      // Save the relation
      // Note: Do we save "no relations" to MCParticles as relations to
      // MCParticle[0]?
      if (iMCPart > 0)
        new(storeDigitsRel->AddrAt(m_nRelationsSaved++)) Relation(
          storeMCParticles, storeDigits, iMCPart, m_nDigitsSaved - 1, weight
        );

    } // for HitRelationMapItr
  } // for digit
} // saveDigits


//---------------------------------------------------------------------
// Print processor parameters.
//---------------------------------------------------------------------
void PXDDigiModule::printModuleParams() const
{
  B2INFO("\nPXDDigiModule parameters: " << endl)
  B2INFO("  DiffusionCoefficient[um]:          " << m_diffusionCoefficient / Unit::um);
  B2INFO("  Ganged pixels option:              " << m_gangedPixels);
  if (m_electronicEffects) {
    B2INFO("  El. noise [fC]:                    " << m_elNoise / Unit::fC)
  }
  if (m_PoissonSmearing) {
    B2INFO("  Electronics - Poisson smearing?:   " << "   yes")
  } else {
    B2INFO("  Electronics - Poisson smearing?:   " << "    no")
  }
  if (m_integrationWindow) {
    B2INFO("  Integration window [us]:           "
           << m_startIntegrationFirstLayer / Unit::us << " ; "
           << m_stopIntegrationFirstLayer / Unit::us)
  }
  if (m_landauFluct) {
    B2INFO("  Internal Landau fluctuations?:     " << "   yes")
    B2INFO("     Prod. threhold on second. e [keV]: "
           << m_prodThreshOnDeltaRays / Unit::keV)
    B2INFO("     Beta*Gamma limit for Landau fluct: "
           << m_landauBetaGammaCut)
  } else {
    B2INFO("  Internal Landau fluctuations?:     "
           << "    no")
  }
  B2INFO("  Spatial precision [um]:            "
         << m_segmentLength / Unit::um)
  B2INFO("  Tangent of Lorentz angle:          "
         << m_tanLorentzAngle)
  B2INFO("  Width of diffused cloud:           "
         << m_widthOfDiffusCloud)
}

