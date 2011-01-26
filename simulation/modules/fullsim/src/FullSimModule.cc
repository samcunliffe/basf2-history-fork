/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Guofu Cao                                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <simulation/modules/fullsim/FullSimModule.h>
#include <simulation/kernel/RunManager.h>
#include <simulation/kernel/DetectorConstruction.h>
#include <simulation/kernel/PhysicsList.h>
#include <simulation/kernel/MagneticField.h>
#include <simulation/kernel/PrimaryGeneratorAction.h>
#include <simulation/kernel/EventAction.h>
#include <simulation/kernel/TrackingAction.h>
#include <simulation/kernel/SteppingAction.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/EventMetaData.h>
#include <framework/gearbox/Unit.h>

#include <G4TransportationManager.hh>
#include <G4Transportation.hh>
#include <G4ParticleTable.hh>
#include <G4DecayTable.hh>
#include <QGSP_BERT.hh>
#include <G4EventManager.hh>
#include <G4RunManager.hh>

#include <TGeoManager.h>
#include <TG4RootNavMgr.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::Simulation;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(FullSim)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

FullSimModule::FullSimModule() : Module()
{
  //Set module properties and the description
  setDescription("Performs the full Geant4 detector simulation. Requires a valid geometry in memory.");

  //Parameter definition
  addParam("InputMCParticleCollection", m_mcParticleInputColName, "The name of the input MCParticle collection.", string(DEFAULT_MCPARTICLES));
  addParam("OutputMCParticleCollection", m_mcParticleOutputColName, "The name of the output MCParticle collection.", string(DEFAULT_MCPARTICLES));
  addParam("OutputRelationCollection", m_relationOutputColName, "The name of the output Relation (Hit -> MCParticle) collection.", string("RelationHitMCP"));
  addParam("ThresholdImportantEnergy", m_thresholdImportantEnergy, "[GeV] A particle which got 'stuck' and has less than this energy will be killed after 'ThresholdTrials' trials.", 0.250);
  addParam("ThresholdTrials", m_thresholdTrials, "Geant4 will try 'ThresholdTrials' times to move a particle which got 'stuck' and has an energy less than 'ThresholdImportantEnergy'.", 10);
  addParam("TrackingVerbosity", m_trackingVerbosity, "Tracking verbosity: 0=Silent; 1=Min info per step; 2=sec particles; 3=pre/post step info; 4=like 3 but more info; 5=proposed step length info.", 0);
  addParam("CreateRelations", m_createRelations, "Set to true to create relations between Hits and MCParticles.", true);
  addParam("PhysicsList", m_physicsList, "The name of the physics list which is used for the simulation.", string("QGSP_BERT"));
  addParam("RegisterOptics", m_optics, "If true, G4OpticalPhysics is registered in Geant4 PhysicsList.", false);
  addParam("ProductionCut", m_productionCut, "Apply continuous energy loss to primary particle which has no longer enough energy to produce secondaries which travel at least the specified productionCut distance.", 0.07);
  addParam("MaxNumberSteps", m_maxNumberSteps, "The maximum number of steps before the track transportation is stopped and the track is killed.", 100000);
}


FullSimModule::~FullSimModule()
{

}


void FullSimModule::initialize()
{
  //Perform some initial checks
  if (gGeoManager == NULL) {
    B2ERROR("No geometry was found in memory (gGeoManager is NULL) !")
    return;
  }

  if (!gGeoManager->IsClosed()) {
    B2ERROR("The geometry was not closed ! Make sure gGeoManager->CloseGeometry() was called.")
    return;
  }

  //Create an instance of the G4Root Navigation manager.
  TG4RootNavMgr* g4rootNavMgr = TG4RootNavMgr::GetInstance(gGeoManager);

  if (g4rootNavMgr == NULL) {
    B2ERROR("Could not retrieve an instance of the TG4RootNavMgr !")
    return;
  }

  //Get the instance of the run manager.
  RunManager& runManager = RunManager::Instance();

  //Convert the TGeo volumes/materials to Geant4 volumes/materials and then
  //call the DetectorConstruction class to allow the user based modification
  //of the created geant4 volumes/materials.
  g4rootNavMgr->Initialize(new DetectorConstruction());
  g4rootNavMgr->ConnectToG4();

  //Create the Physics list
  PhysicsList* physicsList = new PhysicsList(m_physicsList);
  physicsList->setProductionCutValue(m_productionCut);
  if (m_optics) physicsList->registerOpticalPhysicsList();

  runManager.SetUserInitialization(physicsList);


  //Create the magnetic field for the Geant4 simulation
  MagneticField* magneticField = new MagneticField();
  G4FieldManager* fieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
  fieldManager->SetDetectorField(magneticField);
  fieldManager->CreateChordFinder(magneticField);

  //Create the generator action which takes the MCParticle list and converts it to Geant4 primary vertices.
  G4VUserPrimaryGeneratorAction* generatorAction = new PrimaryGeneratorAction(m_mcParticleInputColName, m_mcParticleGraph);
  runManager.SetUserAction(generatorAction);

  //Add the event action which creates the final MCParticle list and the Relation list.
  EventAction* eventAction = new EventAction(m_mcParticleOutputColName, m_relationOutputColName, m_mcParticleGraph, m_createRelations);
  runManager.SetUserAction(eventAction);

  //Add the tracking action which handles the secondary particles created by Geant4.
  TrackingAction* trackingAction = new TrackingAction(m_mcParticleGraph);
  runManager.SetUserAction(trackingAction);

  SteppingAction* steppingAction = new SteppingAction();
  steppingAction->setMaxNumberSteps(m_maxNumberSteps);
  runManager.SetUserAction(steppingAction);

  //Initialize G4 kernel
  runManager.Initialize();

  //Set the parameters for the G4Transportation system.
  //To make sure we really change all G4Transportation classes, we loop over all particles
  //even if the pointer to the G4Transportation object seems to be the same for all particles.
  G4ParticleTable::G4PTblDicIterator* partIter = G4ParticleTable::GetParticleTable()->GetIterator();
  partIter->reset();
  while ((*partIter)()) {
    G4ParticleDefinition* currParticle = partIter->value();
    G4ProcessVector& currProcList = *currParticle->GetProcessManager()->GetProcessList();
    for (int iProcess = 0; iProcess < currProcList.size(); ++iProcess) {
      G4Transportation* transport = dynamic_cast<G4Transportation*>(currProcList[iProcess]);
      if (transport != NULL) {
        transport->SetThresholdImportantEnergy(m_thresholdImportantEnergy / Unit::MeV); //Geant4 energy unit is MeV
        transport->SetThresholdTrials(m_thresholdTrials);
        break;
      }
    }
  }

  //Set the verbosity level of Geant4 according to the logging settings of the module
  int g4VerboseLevel = 0;
  switch (LogSystem::Instance().getCurrentLogLevel()) {
    case LogConfig::c_Debug : g4VerboseLevel = 2;
      break;
    case LogConfig::c_Info  : g4VerboseLevel = 1;
      break;
    default: g4VerboseLevel = 0;
  }
  G4EventManager::GetEventManager()->SetVerboseLevel(g4VerboseLevel);
  G4RunManager::GetRunManager()->SetVerboseLevel(g4VerboseLevel);
  G4EventManager::GetEventManager()->GetTrackingManager()->SetVerboseLevel(m_trackingVerbosity); //turned out to be more useful as a parameter.
}


void FullSimModule::beginRun()
{
  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  //Begin the Geant4 run
  RunManager::Instance().beginRun(eventMetaDataPtr->getRun());
}


void FullSimModule::event()
{
  //Get the event meta data
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  //Process the event
  RunManager::Instance().processEvent(eventMetaDataPtr->getEvent());
}


void FullSimModule::endRun()
{
  //Terminate the Geant4 run
  RunManager::Instance().endRun();
}


void FullSimModule::terminate()
{
  RunManager::Instance().destroy();
}
