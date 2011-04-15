/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <generators/modules/TouschekTURTLEInputModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/EventMetaData.h>

#include <framework/core/ModuleUtils.h>

#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoNode.h>

#include <fstream>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TouschekTURTLEInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TouschekTURTLEInputModule::TouschekTURTLEInputModule() : Module()
{
  //Set module properties
  setDescription("Reads the Touschek data from a TURTLE file and stores it into the MCParticle collection.");
  setPropertyFlags(c_Input);

  //Parameter definition
  addParam("FilenameHER",  m_filenameHER, "The filename of the HER TURTLE input file.");
  addParam("FilenameLER",  m_filenameLER, "The filename of the LER TURTLE input file.");
  addParam("ReadHER", m_readHER, "Set to false to skip reading the HER data.", true);
  addParam("ReadLER", m_readLER, "Set to false to skip reading the LER data.", true);
  addParam("zCoordinate", m_zPos, "Indicates the z-coordinate of the TURTLE particles in the file. (default is 50cm, 400cm is also implemented.)", 50.0);
  addParam("MaxParticles", m_maxParticles, "The maximum number of particles per event that should be read. -1 means all of the particles are read.", -1);

  //Create and initialize member variables
  m_herPipePartMatrix = new TGeoHMatrix("TouschekPlaneHER");
  m_lerPipePartMatrix = new TGeoHMatrix("TouschekPlaneLER");
  m_readerHER = new TouschekReaderTURTLE(m_herPipePartMatrix, 11);  //HER: electrons
  m_readerLER = new TouschekReaderTURTLE(m_herPipePartMatrix, -11); //LER: positrons
}


TouschekTURTLEInputModule::~TouschekTURTLEInputModule()
{
  delete m_readerHER;
  delete m_readerLER;
}


void TouschekTURTLEInputModule::initialize()
{
  //Check parameters
  if ((m_readHER) && (!ModuleUtils::fileNameExists(m_filenameHER))) {
    B2ERROR("Parameter <FilenameHER>: Could not open the file. The filename " << m_filenameHER << " does not exist !")
  } else m_readerHER->open(m_filenameHER);

  if ((m_readLER) && (!ModuleUtils::fileNameExists(m_filenameLER))) {
    B2ERROR("Parameter <FilenameLER>: Could not open the file. The filename " << m_filenameLER << " does not exist !")
  } else m_readerLER->open(m_filenameLER);

  //Get the transformation from local Touschek plane space to global geant4 space
  //For the HER
  double herTouschekPlaneTrans[3] = {0.0, 0.0, 0.0};

  gGeoManager->CdTop();

  if (m_zPos == 50) {
    gGeoManager->cd("/IR_1/HERUpstreamFlange1_1Tube_1");
    herTouschekPlaneTrans[2] = gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 50.0; //TURTLE data is at -50 cm
  } else if (m_zPos == 400) {
    gGeoManager->cd("/IR_1/D1wal1");
    herTouschekPlaneTrans[2] = gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 400.0; //TURTLE data is at -400 cm
  } else {
    B2ERROR("TouschekTURTLEInput: The zCoordinate = " << m_zPos << " is not implemented. Please use either 50cm or 400cm.");
  }

  m_herPipePartMatrix->SetTranslation(herTouschekPlaneTrans);
  m_herPipePartMatrix->MultiplyLeft(gGeoManager->GetCurrentMatrix());


  //For the LER
  double lerTouschekPlaneTrans[3] = {0.0, 0.0, 0.0};
  gGeoManager->CdTop();
  if (m_zPos == 50) {
    gGeoManager->cd("/IR_1/LERUpstreamFlange1_1Tube_1");
    lerTouschekPlaneTrans[2] = -gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 50.0; //TURTLE data is at 50 cm
  } else if (m_zPos == 400) {
    gGeoManager->cd("/IR_1/B1wal1");
    lerTouschekPlaneTrans[2] = -gGeoManager->GetCurrentMatrix()->GetTranslation()[2] + 400.0; //TURTLE data is at 400 cm
  } else {
    B2ERROR("TouschekTURTLEInput: The zCoordinate = " << m_zPos << " is not implemented. Please use either 50cm or 400cm.");
  }

  m_lerPipePartMatrix->SetTranslation(lerTouschekPlaneTrans);
  m_lerPipePartMatrix->MultiplyLeft(gGeoManager->GetCurrentMatrix());
}


void TouschekTURTLEInputModule::event()
{
  int readHERParticles = 0;
  int readLERParticles = 0;

  try {
    //Read the data
    MCParticleGraph mpg;
    if (m_readHER) readHERParticles = m_readerHER->getParticles(m_maxParticles, mpg); //HER: electrons
    if (m_readLER) readLERParticles = m_readerLER->getParticles(m_maxParticles, mpg); //LER: positrons

    if ((readHERParticles > 0) || (readLERParticles > 0)) {
      //Generate MCParticle list
      mpg.generateList(DEFAULT_MCPARTICLES, MCParticleGraph::c_setDecayInfo | MCParticleGraph::c_checkCyclic);

      B2INFO("Read " << readHERParticles << " e- particles (HER).")
      B2INFO("Read " << readLERParticles << " e+ particles (LER).")
    }
  } catch (runtime_error &exc) {
    B2ERROR(exc.what());
  }
}
