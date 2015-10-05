
/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ami Rostomyan, Torben Ferber                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/fragmentation/FragmentationModule.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/particledb/EvtGenDatabasePDG.h>
#include <framework/utilities/FileSystem.h>

#include <boost/format.hpp>
#include <stdio.h>

#include <TDatabasePDG.h>
#include <TRandom3.h>

#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <mdst/dataobjects/MCParticleGraph.h>

#include <string>
#include <queue>

#include <EvtGenExternal/EvtExternalGenList.hh>
#include <EvtGenBase/EvtAbsRadCorr.hh>
#include <EvtGenBase/EvtDecayBase.hh>

using namespace std;
using namespace Belle2;
using namespace Pythia8;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(Fragmentation)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
FragmentationModule::FragmentationModule() : Module()
{
  //Set module properties
  setDescription("Fragmention of (u/d/s/c) quarks using PYTHIA8");

  //Parameter definition
  addParam("ParameterFile", m_parameterfile, "Input parameter file for PYTHIA",
           std::string("../modules/fragmentation/data/pythia_default.dat"));
  addParam("ListPYTHIAEvent", m_listEvent, "List event record of PYTHIA after hadronization", 0);
  addParam("UseEvtGen", m_useEvtGen, "Use EvtGen for specific decays", 1);
  addParam("EvtPdl", m_EvtPdl, "Deprecated", std::string(""));
  addParam("DecFile", m_DecFile, "EvtGen decay file (DECAY.DEC)", std::string(""));
  addParam("UserDecFile", m_UserDecFile, "User EvtGen decay file", std::string(""));

  //initialize member variables
  evtgen  = 0;
  nAdded  = 0;
  nQuarks = 0;
  nVpho   = 0;
  nAll    = 0;
  nGood   = 0;

  pythia = nullptr;
  PythiaEvent = nullptr;

}


FragmentationModule::~FragmentationModule()
{
}

void FragmentationModule::terminate()
{

  // print internal pythia error statistics
  pythia->stat();

  double ratio = 0.; //ratio of good over all events
  if (nAll) ratio = 100.0 * nGood / nAll;
  B2RESULT("Total number of events: " << nAll << ", of these fragmented: " << nGood << ", ratio: " << ratio << "%");
}

//-----------------------------------------------------------------
//                 Initialize
//-----------------------------------------------------------------
void FragmentationModule::initialize()
{
  B2INFO("Initialize PYTHIA8");
  if (getParam<std::string>("EvtPdl").isSetInSteering()) {
    B2ERROR("The 'pdlFile' parameter is deprecated and will be ignored. Use \"import pdg; pdg.read('pdlFile')\" instead.")
  }

  // Generator and the shorthand PythiaEvent = pythia->event are declared in .h file
  // A simple way to collect all the changes is to store the parameter values in a separate file,
  // with one line per change. This should be done between the creation of the Pythia object
  // and the init call for it.
  pythia = new Pythia;
  PythiaEvent = &pythia->event;
  (*PythiaEvent) = 0;

  // Switch off ProcessLevel
  pythia->readString("ProcessLevel:all = off");

  // Read the PYTHIA input file, overrides parameters
  pythia->readFile(m_parameterfile);

  // Set framework generator
  FragmentationRndm* fragRndm = new FragmentationRndm();
  pythia->setRndmEnginePtr(fragRndm);

  // Initialize PYTHIA
  pythia->init();

  // Set EvtGen (after pythia->init())
  evtgen = 0;
  EvtExternalGenList* genlist = new EvtExternalGenList();
  EvtAbsRadCorr* radCorrEngine = genlist->getPhotosModel();

  if (m_useEvtGen) {
    B2INFO("Using PYTHIA EvtGen Interface");
    FileSystem::TemporaryFile tmp;
    EvtGenDatabasePDG::Instance()->WriteEvtGenTable(tmp);
    evtgen = new EvtGenDecays(pythia, m_DecFile, tmp.getName(), genlist, radCorrEngine);
    evtgen->readDecayFile(m_UserDecFile);
  }

  // List variable(s) that differ from their defaults
  pythia->settings.listChanged();
}

//-----------------------------------------------------------------
//                 Event
//-----------------------------------------------------------------
void FragmentationModule::event()
{
  m_mcparticles.required(m_particleList);
  StoreArray<MCParticle> mcParticles(m_particleList);

  // Reset the indices of the graph
  mcParticleGraph.clear();
  mcParticleGraph.loadList(m_particleList);

  // Reset PYTHIA event record to allow for new event
  PythiaEvent->reset();

  // Reset counter for added quarks and vphos
  nAdded  = 0;
  nQuarks = 0;
  nVpho   = 0;

  // Store which MCParticle index belongs to which Pythia index
  std::map<int, int> indexPYTHIA;

  // Store which Pythia index belongs to which MCParticle index
  std::map<int, int> indexMCGraph;

  // Store position of the quark (mother of hadronized final state)
  int quarkPosition = 0;

  // Loop over all particles to find the quark pair
  int nPart = mcParticles.getEntries();
  for (int iPart = 0; iPart < nPart; iPart++) {
    MCParticle* currParticle = mcParticles[iPart];

    //returns quark id if it finds a quark, zero otherwise
    //increments a counter for the number of found quarks
    int pythiaIndex = addParticleToPYTHIA(*currParticle);

    if (pythiaIndex != 0) {
      indexPYTHIA[nAdded] = iPart;
      if (pythiaIndex > 0) quarkPosition = iPart;
    }
  }

  // Check needed if virtual exchange boson and two quarks are present
  if (nQuarks != 2) {
    B2FATAL("Invalid number of quarks: " << nQuarks << " (should be 2)!");
  }

  if (nVpho != 1) {
    B2WARNING("No virtual exchange particle given, no PYTHIA FSR in Decays");
  } else {
    // Adding QCD and QED FSR
    pythia->forceTimeShower(2, 3, 20.00);
  }

  // Check needed if event is energetically possible
  // ...

  // Do the fragmentation using PYTHIA
  setReturnValue(1); //return value is 1...
  nAll = nAll + 1;
  if (!pythia->next()) {
    B2WARNING("pythia->next() failed, event generation aborted prematurely! Printing PythiaEvent.list():");
    PythiaEvent->list();
    setReturnValue(-1); //return value becomes -1 if trials were not successfull
  } else {
    nGood = nGood + 1;
  }

  // use evtgen to perform the decay
  if (m_useEvtGen) evtgen->decay();

  // Loop over the PYTHIA list and assign the mother-daughter relation
  // Might not work if the mother appear below the daughter in the event record
  for (int iPythiaPart = 0; iPythiaPart < pythia->event.size(); ++iPythiaPart) {
    auto oldindex = indexPYTHIA.find(iPythiaPart);

    //skip "system" particle generated by PYTHIA
    if (pythia->event[iPythiaPart].id() == 90) continue;

    if (oldindex == end(indexPYTHIA)) {
      // --> new particle

      // Add to particle grapg
      int position = mcParticleGraph.size();
      mcParticleGraph.addParticle();
      indexMCGraph[iPythiaPart] = position;

      MCParticleGraph::GraphParticle* p = &mcParticleGraph[position];

      // from PYTHIA manual: "<1: an empty entry, with no meaningful information and
      // therefore to be skipped unconditionally (should not occur in PYTHIA)"
      if (pythia->event[iPythiaPart].statusHepMC() < 1)  continue;

      // Set PDG code
      p->setPDG(pythia->event[iPythiaPart].id());

      // Set four vector
      TLorentzVector p4(pythia->event[iPythiaPart].px(), pythia->event[iPythiaPart].py(), pythia->event[iPythiaPart].pz(),
                        pythia->event[iPythiaPart].e());
      p->set4Vector(p4);
      p->setMass(pythia->event[iPythiaPart].m());

      // Set vertex
      p->setProductionVertex(pythia->event[iPythiaPart].xProd(), pythia->event[iPythiaPart].yProd(), pythia->event[iPythiaPart].zProd());
      p->setProductionTime(pythia->event[iPythiaPart].zProd() * Unit::mm / Const::speedOfLight);
      p->setValidVertex(true);

      // Set all(!) particles from the generator to primary
      p->addStatus(MCParticleGraph::GraphParticle::c_PrimaryParticle);

      // Set FSR flag from PYTHIA TimeShower:QEDshowerByQ
      if (pythia->event[iPythiaPart].status() == 51 && pythia->event[iPythiaPart].id() == 22) {
        p->addStatus(MCParticleGraph::GraphParticle::c_IsFSRPhoton);
      }

      // Set PHOTOS flag from PYTHIA-EvtGen
      if (pythia->event[iPythiaPart].status() == 94 && pythia->event[iPythiaPart].id() == 22) {
        p->addStatus(MCParticleGraph::GraphParticle::c_IsPHOTOSPhoton);
      }

      // Set stable at generator level
      if (pythia->event[iPythiaPart].statusHepMC() == 1) {
        p->addStatus(MCParticleGraph::GraphParticle::c_StableInGenerator);
      }

      //set mother
      const int motherid = pythia->event[iPythiaPart].mother1();

      //check if mother exists in indexMCGraph
      auto motherindex = indexMCGraph.find(motherid);

      if (motherindex != end(indexMCGraph)) {
        int motheridingraph = indexMCGraph[motherid];
        MCParticleGraph::GraphParticle* q = &mcParticleGraph[motheridingraph];
        p->comesFrom(*q);
      } else {
        // Particle has no mother from PYTHIA, add quark as mother
        MCParticleGraph::GraphParticle* q = &mcParticleGraph[quarkPosition];
        p->comesFrom(*q);
      }

    } else {
      // particle is already in the graph
      // modify here if needed
      // ...
    }
  }

  // Print original PYTHIA list
  if (m_listEvent) PythiaEvent->list();

  // Create new ParticleGraph
  mcParticleGraph.generateList(m_particleList,
                               MCParticleGraph::c_clearParticles | MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

}

//-----------------------------------------------------------------
//                 addParticleToPYTHIA
//-----------------------------------------------------------------
int FragmentationModule::addParticleToPYTHIA(MCParticle& mcParticle)
{
  //get PDG code
  const int id = mcParticle.getPDG();

  //check that this particle is a quark or a virtual photon(-like)
  bool isVPho  = false;
  bool isQuark = false;
  if (abs(id) >= 1 && abs(id) <= 4) isQuark = true;
  if (id == 23) isVPho = true;

  if (!(isVPho || isQuark)) return 0;

  //check that there is no daughter for the quarks
  if (isQuark && mcParticle.getDaughters().size()) {
    B2WARNING("Quark already has a daughter!");
    return 0;
  }

  //get some basic kinematics
  const double mass   = mcParticle.getMass();
  const TVector3& p   = mcParticle.getMomentum();
  const double energy = sqrt(mass * mass + p.Mag2());

  //add this (anti)quark to the PythiaEvent
  if (id == 23) {
    PythiaEvent->append(23, -22, 0, 0, 2, 3, 0, 0, p.X(), p.Y(), p.Z(), energy, mass);
    nVpho++;
  } else if (id > 0) {
    PythiaEvent->append(id, 23, 1, 0, 0, 0, 101, 0, p.X(), p.Y(), p.Z(), energy, mass, 20.0);
    nQuarks++;
  } else if (id < 0) {
    PythiaEvent->append(id, 23, 1, 0, 0, 0, 0, 101, p.X(), p.Y(), p.Z(), energy, mass, 20.0);
    nQuarks++;
  }

  nAdded++;

  return id;
}

//-----------------------------------------------------------------
//                 random generator for PYTHIA
//-----------------------------------------------------------------
FragmentationRndm::FragmentationRndm() : Pythia8::RndmEngine()
{

}
double FragmentationRndm::flat()
{
  double value = gRandom->Rndm();
  return value;
}
