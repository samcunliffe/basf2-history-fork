/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMCmatching/ECLMCMatchingModule.h>

//framework headers
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

//ecl package headers
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/geometry/ECLGeometryPar.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLHitAssignment.h>

#include <generators/dataobjects/MCParticle.h>
#include <framework/datastore/RelationArray.h>


//root
#include <TVector3.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair



using namespace std;
using namespace boost;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLMCMatching)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLMCMatchingModule::ECLMCMatchingModule() : Module()
{
  // Set description
//  setDescription("ECLSimHitMakerModule");

  setDescription("ECLMCMatchingModule");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

}

ECLMCMatchingModule::~ECLMCMatchingModule()
{
}

void ECLMCMatchingModule::initialize()
{
  // Initialize variables
  m_nRun    = 0 ;
  m_nEvent  = 0 ;
  RelationArray::registerPersistent<ECLSimHit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLDigit, MCParticle>("", "");
  RelationArray::registerPersistent<ECLShower, MCParticle>("", "");



}

void ECLMCMatchingModule::beginRun()
{
}

void ECLMCMatchingModule::event()
{


  int HitIndex[8736][16];
  int HitOldTrack[8736][16];
  int DigiIndex[8736];
  int DigiOldTrack[8736];
  int ShowerOldTrack[8736];
  for (int i = 0; i < 8736; i++) {
    DigiIndex[i] = -1; DigiOldTrack[i] = -1; ShowerOldTrack[i] = -1;
    for (int j = 0; j < 16; j++) {  HitIndex[i][j] = -1; HitOldTrack[i][j] = -1;}
  }

  StoreArray<MCParticle> mcParticles;
  PrimaryTrackMap eclPrimaryMap;
  eclPrimaryMap.clear();
  int nMcParticles = mcParticles.getEntries();
  for (int iPart = 0; iPart < nMcParticles; ++iPart) {
    /*
        if(mcParticles[iPart]->getMother()==NULL)
               cout<<" "<<m_nEvent<<"Track "<<mcParticles[iPart]->getArrayIndex()
               <<" PDG "<<mcParticles[iPart]->getPDG()
               <<" P "<<mcParticles[iPart]->getMomentum().Mag()
               <<" vx "<< mcParticles[iPart]->getProductionVertex().Perp()<<" "<<mcParticles[iPart]->getProductionVertex().z()<<endl;
        else
               cout<<" "<<m_nEvent<<"Track "<<mcParticles[iPart]->getArrayIndex()
               <<" PDG "<<mcParticles[iPart]->getPDG()
               <<" P "<<mcParticles[iPart]->getMomentum().Mag()
               <<" vx "<< mcParticles[iPart]->getProductionVertex().Perp()<<" "<<mcParticles[iPart]->getProductionVertex().z()
               <<" Mother "<< mcParticles[iPart]->getMother()->getArrayIndex()
               <<" PDG "<<mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getPDG()
               <<" P "<<mcParticles[mcParticles[iPart]->getMother()->getArrayIndex()]->getMomentum().Mag()<<endl;
    */
    if (mcParticles[iPart]->getMother() == NULL) {

      if (mcParticles[iPart]->getArrayIndex() == -1)
      {     eclPrimaryMap.insert(pair<int, int>(iPart, iPart));}
      else {eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), mcParticles[iPart]->getArrayIndex()));}
      //cout<<"mom "<<mcParticles[iPart]->getArrayIndex() <<endl;
    } else {
      eclPrimaryMap.insert(pair<int, int>(mcParticles[iPart]->getArrayIndex(), eclPrimaryMap[mcParticles[iPart]->getMother()->getArrayIndex() ]));
      //cout<<"mom "<<mcParticles[iPart]->getMother()->getArrayIndex() <<" "<<mcParticles[iPart]->getArrayIndex()<<endl;
    }
  }

  StoreArray<ECLSimHit> eclSimArray;
  StoreArray<ECLSimHit> eclHitArray;
  RelationArray eclSimHitRel(mcParticles, eclSimArray);
  RelationArray eclHitToMCPart(eclHitArray, mcParticles);

  /*
    int hitNum = eclHitArray->GetEntriesFast();
    for (int ii = 0; ii < hitNum; ii++) {
      ECLSimHit* aECLSimHit = eclHitArray[ii];
      int hitCellId       =  aECLSimHit->getCellId();
      double hitTimeAve       =  aECLSimHit->getTimeAve()   * Unit::ns;
      int TimeIndex = (int) hitTimeAve / 500;
      HitIndex[hitCellId][TimeIndex] = ii;
    }

    for (int index = 0; index < eclSimHitRel.getEntries(); index++) {

      MCParticle* aMCParticle = mcParticles[eclSimHitRel[index].getFromIndex()];
      TVector3 McP =  aMCParticle->getMomentum();
      TVector3 Pvertex = aMCParticle->getProductionVertex();

      int PrimaryIndex = -1;
      map<int, int>::iterator iter = eclPrimaryMap.find(eclSimHitRel[index].getFromIndex());
      if (iter != eclPrimaryMap.end()) {
        PrimaryIndex = iter->second;
      } else cout << "CantFind Track in eclPrimaryMap " << eclSimHitRel[index].getFromIndex() << endl;

      for (int hit = 0; hit < (int)eclSimHitRel[index].getToIndices().size(); hit++) {
        ECLSimHit* aECLSimHit = eclSimArray[eclSimHitRel[index].getToIndex(hit)];
        int hitCellId         = aECLSimHit->getCellId();
        double hitE           = aECLSimHit->getEnergyDep() * Unit::GeV;
        //double hitTOF         = aECLSimHit->getFlightTime() * Unit::ns;
        int TimeIndex = (int) hitTOF / 500;


        if (hitTOF < 8000 && hitE > 30e-6) {
          if (PrimaryIndex != -1 && HitIndex[hitCellId][TimeIndex] != -1 && HitOldTrack[hitCellId][TimeIndex] != PrimaryIndex) {
            eclHitToMCPart.add(HitIndex[hitCellId][TimeIndex], PrimaryIndex);
            HitOldTrack[hitCellId][TimeIndex] = PrimaryIndex;
          }
        }//if hitTOF < 8000&&E_cell[hitCellId][TimeIndex] > 30e-6)
      }//for (int hit = 0
    }//for index
  */

  StoreArray<ECLDigit> eclDigiArray;
  RelationArray  eclDigiToMCPart(eclDigiArray, mcParticles);

  int hitNum1 = eclDigiArray->GetEntriesFast();
  for (int ii = 0; ii < hitNum1; ii++) {
    ECLDigit* aECLDigi = eclDigiArray[ii];
    float FitEnergy    = (aECLDigi->getAmp()) / 20000.;//ADC count to GeV
    int cId          = (aECLDigi->getCellId() - 1);
    if (FitEnergy < 0.) {continue;}
    DigiIndex[cId] = ii;
  }
  const int eclHitToMCPartn = eclHitToMCPart.getEntries() ;
  for (int index = 0; index < eclHitToMCPartn; index++) {
    ECLSimHit* aECLSimHit = eclHitArray[eclHitToMCPart[index].getFromIndex()];
    int hitCellId       =  aECLSimHit->getCellId();

    for (int iMCpart = 0; iMCpart < (int)eclHitToMCPart[index].getToIndices().size(); iMCpart++) {
      if (DigiIndex[hitCellId] != -1 && DigiOldTrack[hitCellId] != (int)eclHitToMCPart[index].getToIndex(iMCpart)) {
        eclDigiToMCPart.add(DigiIndex[hitCellId], eclHitToMCPart[index].getToIndex(iMCpart));
        DigiOldTrack[hitCellId] = eclHitToMCPart[index].getToIndex(iMCpart);
      }
    }
  }

  StoreArray<ECLShower> eclRecShowerArray;
  StoreArray<ECLHitAssignment> eclHitAssignmentArray;
  RelationArray  eclShowerToMCPart(eclRecShowerArray, mcParticles);


  HitShowerMap eclShowerMap;
  eclShowerMap.clear();

  const int ShowerNum = eclRecShowerArray->GetEntriesFast();
  const int hANum = eclHitAssignmentArray->GetEntriesFast();
  for (int iShower = 0; iShower < ShowerNum; iShower++) {
    ECLShower* aECLShower = eclRecShowerArray[iShower];
    double showerId = aECLShower->GetShowerId();
    for (int iHA = 0; iHA < hANum; iHA++) {
      ECLHitAssignment* aECLHitAssignment = eclHitAssignmentArray[iHA];
      int m_HAShowerId = aECLHitAssignment->getShowerId();
      int m_HAcellId = aECLHitAssignment->getCellId();
      if (m_HAShowerId != showerId)continue;
      if (m_HAShowerId > showerId)break;
      eclShowerMap.insert(pair<int, int>(m_HAcellId, m_HAShowerId));
    }//for HA hANum
  }//ShowerNum

  //cout<<"Total showers in Event "<<m_nEvent<<" : "<< ShowerNum<<endl;
  for (int index = 0; index < eclDigiToMCPart.getEntries(); index++) {
    ECLDigit* aECLDigi = eclDigiArray[eclDigiToMCPart[index].getFromIndex()];
    int cId          = (aECLDigi->getCellId() - 1);
    for (int iMCpart = 0; iMCpart < (int)eclDigiToMCPart[index].getToIndices().size(); iMCpart++) {

      int ShowerIndex = -1;
      HitShowerMap::size_type entries = eclShowerMap.count(cId);
      HitShowerMap::iterator iter = eclShowerMap.find(cId);
      for (HitShowerMap::size_type cnt = 0; cnt != entries; ++cnt) {
        ShowerIndex =  iter++->second ;
        if (ShowerIndex != -1 && ShowerOldTrack[ShowerIndex] != (int)eclDigiToMCPart[index].getToIndex(iMCpart)) {
          eclShowerToMCPart.add(ShowerIndex, eclDigiToMCPart[index].getToIndex(iMCpart));
          // cout << "ShowerRel" << m_nEvent << " " << cId << " shower" << ShowerIndex << " mom" << eclDigiToMCPart[index].getToIndex(iMCpart)
          //       << " PDG " << mcParticles[eclDigiToMCPart[index].getToIndex(iMCpart)]->getPDG()
          //      << endl;
          ShowerOldTrack[ShowerIndex] = eclDigiToMCPart[index].getToIndex(iMCpart);
        }
      }
    }//for iMCpart
  }//for index

  m_nEvent++;

}


void ECLMCMatchingModule::endRun()
{
  m_nRun++;
}

void ECLMCMatchingModule::terminate()
{
}


