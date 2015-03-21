//+
// File : B2BIIMdstInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contributors: Anze Zupanc
//-

#include <b2bii/modules/B2BIIMdstInput/B2BIIMdstInputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>

// Belle tables
#include "belle_legacy/tables/belletdf.h"

// Belle II dataobjects
#include <framework/dataobjects/EventMetaData.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIMdstInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIMdstInputModule::B2BIIMdstInputModule() : Module()
{
  //Set module properties
  setDescription("Module to read Belle MDST files.");
  setPropertyFlags(c_Input);

  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName"  , m_inputFileName, "Belle MDST input file name.", string("PantherInput.mdst"));

  B2DEBUG(1, "B2BIIMdstInput: Constructor done.");
}


B2BIIMdstInputModule::~B2BIIMdstInputModule()
{
}

void B2BIIMdstInputModule::initialize()
{
  // Initialize Panther
  BsInit(0);

  // Initialize Belle II DataStore
  initializeDataStore();

  // Open data file
  m_fd = new Belle::Panther_FileIO(m_inputFileName.c_str(), BBS_READ);

  // Read first event (note that this is not a real event)
  m_fd->read();
  m_nevt++;

  // Process first event
  // currently nothing is done for the first event

  B2DEBUG(1, "B2BIIMdstInput: initialized.");
}

void B2BIIMdstInputModule::initializeDataStore()
{
  B2DEBUG(99, "[B2BIIMdstInputModule::initializeDataStore] initialization of DataStore started");

  // event meta data Object pointer
  StoreObjPtr<EventMetaData>::registerPersistent();

  B2DEBUG(99, "[B2BIIMdstInputModule::initializeDataStore] initialization of DataStore ended");
}


void B2BIIMdstInputModule::beginRun()
{
  B2DEBUG(99, "B2BIIMdstInput: beginRun called.");
}


void B2BIIMdstInputModule::event()
{
  m_nevt++;

  // First event is already loaded (skip it)
  if (m_nevt == 0) return;

  // Fill EventMetaData
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();

  // read event
  int rectype = -1;
  while (rectype < 0 && rectype != -2) {
    //clear all previous event data before reading!
    BsClrTab(BBS_CLEAR);
    rectype = m_fd->read();
  }
  if (rectype == -2) {   // EoF detected
    evtmetadata->setEndOfData(); // stop event processing
    B2DEBUG(99, "[B2BIIMdstInputModule::Conversion] Conversion stopped at event #" << m_nevt << ". EOF detected!");
    return;
  }

  // Convert the Belle_event -> EventMetaData
  // Get Belle_event_Manager
  Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
  Belle::Belle_event& evt = evman[0];

  // set exp/run/evt numbers
  evtmetadata->setExperiment(evt.ExpNo());
  evtmetadata->setRun(evt.RunNo());
  evtmetadata->setEvent(evt.EvtNo() & 0x0fffffff);

  // set generated weight (>0 for MC; <0 for real DATA)
  evtmetadata->setGeneratedWeight((evt.ExpMC() == 2) ? 1.0 : -1.0);

  B2DEBUG(90, "[B2BIIMdstInputModule] Convert exp/run/evt: " << evt.ExpNo() << "/" << evt.RunNo() << "/" << int(
            evt.EvtNo() & 0x0fffffff));
}

void B2BIIMdstInputModule::endRun()
{
  B2INFO("B2BIIMdstInput: endRun done.");
}


void B2BIIMdstInputModule::terminate()
{
  delete m_fd;
  B2INFO("B2BIIMdstInput: terminate called")
}

