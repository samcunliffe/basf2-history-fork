/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclHitMaker/ECLHitModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/dataobjects/Relation.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/hitecl/ECLSimHit.h>
#include <ecl/hitecl/HitECL.h>
#include <ecl/geoecl/ECLGeometryPar.h>

//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

#define PI 3.14159265358979323846


using namespace std;
using namespace boost;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLHit)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLHitModule::ECLHitModule() : Module()
{
  // Set description
//  setDescription("ECLHitMakerModule");

  setDescription("ECLDigiModuletizer");
  // Add parameters
  // I/O
  addParam("InputColName",                m_inColName, "Input collection name", string("ECLSimHits"));
  addParam("ECLHitOutColName",            m_eclHitOutColName, "Output collection name", string("ECLHits"));
  //Relations
  addParam("MCPartToECLSimHitCollectionName", m_relColNameMCToSim,
           "Name of relation collection - MCParticle to SimECLHit (if nonzero, created)", string("MCPartToECLSimHits"));
  addParam("SimHitToECLHitCollectionName", m_relColNameSimHitToHit,
           "Name of relation collection - Hit ECL to MCParticle (if nonzero, created)", string("SimHitToECLHits"));
}

ECLHitModule::~ECLHitModule()
{
}

void ECLHitModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  m_hitNum = 0;
  printModuleParams();
  m_timeCPU = clock() * Unit::us;
}

void ECLHitModule::beginRun()
{
  B2INFO("ECLHitModule: Processing run: " << m_nRun);
}

void ECLHitModule::event()
{
  StoreArray<ECLSimHit> eclArray(m_inColName);
  if (!eclArray) {
    B2ERROR("Can not find " << m_inColName << ".");
  }

  //---------------------------------------------------------------------
  // Merge the hits in the same cell and save them into ECL signal map.
  //---------------------------------------------------------------------

  // Get number of hits in this event
  int nHits = eclArray->GetEntriesFast();


  double E_cell[8736][16];
  double X_ave[8736][16];
  double T_ave[8736][16];
  double Tof_ave[8736][16];

  for (int i = 0; i < 8736; i++) {
    for (int j = 0; j < 16; j++) {
      E_cell[i][j] = 0.; X_ave[i][j] = 0. ; T_ave[i][j] = 0.; Tof_ave[i][j] = 0.;
    }
  }
  // Get instance of ecl geometry parameters
//  ECLGeometryPar * eclp = ECLGeometryPar::Instance();
  ECLGeometryPar * eclp = ECLGeometryPar::Instance();

  // Loop over all hits of steps
  for (int iHits = 0; iHits < nHits; iHits++) {
    // Get a hit
    ECLSimHit* aECLSimHit = eclArray[iHits];


    // Hit geom. info
    int hitCellId       =   aECLSimHit->getCellId();
    double hitE        = aECLSimHit->getEnergyDep() * Unit::GeV;
    double hitTOF         = aECLSimHit->getFlightTime() * Unit::ns;
    TVector3 HitInPos  =   aECLSimHit->getPosIn();
    TVector3 HitOutPos  =   aECLSimHit->getPosOut();
    int ParticleID = aECLSimHit->getPDGCode();

    eclp->Mapping(hitCellId);
    TVector3 PosCell =  eclp->GetCrystalPos();
    TVector3 VecCell =  eclp->GetCrystalVec();
    double local_pos = 15. - (0.5 * (HitInPos + HitOutPos) - PosCell) * VecCell;



    for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
      if (hitCellId == iECLCell && hitTOF < 8000) {
        int TimeIndex = (int) hitTOF / 500;
        E_cell[iECLCell][TimeIndex] = E_cell[iECLCell][TimeIndex] + hitE;
        X_ave[iECLCell][TimeIndex] = X_ave[iECLCell][TimeIndex] + hitE * local_pos;
        Tof_ave[iECLCell][TimeIndex] = Tof_ave[iECLCell][TimeIndex] + hitE * hitTOF;
      }
    } // End loop crsyal 8736

  }//for nHit



  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    for (int  TimeIndex = 0; TimeIndex < 16; TimeIndex++) {

      if (E_cell[iECLCell][TimeIndex] > 1.0e-9) {

        X_ave[iECLCell][TimeIndex] = X_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];
        T_ave[iECLCell][TimeIndex]  =  6.05 + 0.0749 * X_ave[iECLCell][TimeIndex] - 0.00112 * X_ave[iECLCell][TimeIndex] * X_ave[iECLCell][TimeIndex];
        Tof_ave[iECLCell][TimeIndex] =  Tof_ave[iECLCell][TimeIndex] / E_cell[iECLCell][TimeIndex];

        StoreArray<HitECL> eclHitArray(m_eclHitOutColName);
        m_hitNum = eclHitArray->GetLast() + 1;
        new(eclHitArray->AddrAt(m_hitNum)) HitECL();
        eclHitArray[m_hitNum]->setEventId(m_nEvent);
        eclHitArray[m_hitNum]->setCellId(iECLCell);
        eclHitArray[m_hitNum]->setEnergyDep(E_cell[iECLCell][TimeIndex]);
        eclHitArray[m_hitNum]->setTimeAve(T_ave[iECLCell][TimeIndex] + Tof_ave[iECLCell][TimeIndex]);
      }//if Energy > 0
    }//16 Time interval 16x 500 ns
  } //store  each crystal hit

  m_nEvent++;
  B2ERROR("End of events " <<  m_nEvent);
}


void ECLHitModule::endRun()
{
  m_nRun++;
}

void ECLHitModule::terminate()
{
  m_timeCPU = clock() * Unit::us - m_timeCPU;
  B2INFO("ECLHitModule finished. Time per event: " << m_timeCPU / m_nEvent / Unit::ms << " ms.");
}


void ECLHitModule::printModuleParams() const
{
  B2INFO("ECLHit parameters:")
  B2INFO("  Input collection name:  " << m_inColName)
  B2INFO("  Output collection name: " << m_eclHitOutColName)
}

