/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/



#include <ecl/simulation/SensitiveDetector.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/logging/Logger.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <ecl/dataobjects/ECLSim.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/geometry/ECLGeometryPar.h>


#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

#include <G4TrackingManager.hh>
#include <G4Track.hh>
#include <G4Step.hh>
#include <G4SteppingManager.hh>
#include <G4SDManager.hh>
#include <G4TransportationManager.hh>
#include <G4FieldManager.hh>
#include <G4MagneticField.hh>


//#include "CLHEP/Geometry/Vector3D.h"
//#include "CLHEP/Geometry/Point3D.h"

#include "TVector3.h"
#define PI 3.14159265358979323846

using namespace std;

namespace Belle2 {
  namespace ECL {

    SensitiveDetector::SensitiveDetector(G4String name, G4double thresholdEnergyDeposit, G4double thresholdKineticEnergy):
      Simulation::SensitiveDetectorBase(name, ECL), m_thresholdEnergyDeposit(thresholdEnergyDeposit),
      m_thresholdKineticEnergy(thresholdKineticEnergy), m_simhitNumber(0), m_trackID(-999), firstcall(0)
    {
      StoreArray<ECLSim>eclSims;
      StoreArray<MCParticle>mcParticles;

      RelationArray eclSimRel(mcParticles, eclSims);
      registerMCParticleRelation(eclSimRel);
      StoreArray<ECLSim>::registerPersistent();
      StoreArray<ECLSimHit>::registerPersistent();
      RelationArray::registerPersistent<MCParticle, ECLSim>("", "");
      RelationArray::registerPersistent<ECLSimHit, MCParticle>("", "");
    }


    SensitiveDetector::~SensitiveDetector()
    {

    }

    void SensitiveDetector::Initialize(G4HCofThisEvent*)
    {}

//-----------------------------------------------------
// Method invoked for every step in sensitive detector
//-----------------------------------------------------
//G4bool SensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
    bool SensitiveDetector::step(G4Step* aStep, G4TouchableHistory*)
    {

      const G4StepPoint& preStep  = *aStep->GetPreStepPoint();
      const G4StepPoint& postStep = * aStep->GetPostStepPoint();

      G4Track& track  = * aStep->GetTrack();
      if (m_trackID != track.GetTrackID()) {
        //TrackID changed, store track informations
        m_trackID = track.GetTrackID();
        //Get momentum
        m_momentum = preStep.GetMomentum() ;
        //Get energy
        m_startEnergy =  preStep.GetKineticEnergy() ;
        //Reset energy deposit;
        m_energyDeposit = 0;
        //Reset Wighted Time;
        m_WightedTime = 0;
        //Reset m_WightedPos;
        m_WightedPos.SetXYZ(0, 0, 0);

      }
      //Update energy deposit
      m_energyDeposit += aStep->GetTotalEnergyDeposit() ;

      m_startTime = preStep.GetGlobalTime();
      m_endTime = postStep.GetGlobalTime();
      m_WightedTime += (m_startTime + m_endTime) / 2 * (aStep->GetTotalEnergyDeposit());

      m_startPos =  preStep.GetPosition();
      m_endPos = postStep.GetPosition();
      TVector3 position((m_startPos.getX() + m_endPos.getX()) / 2 / cm, (m_startPos.getY() + m_endPos.getY()) / 2 / cm, (m_startPos.getZ() + m_endPos.getZ()) / 2 / cm);
      m_WightedPos += position * (aStep->GetTotalEnergyDeposit());

      //Save Hit if track leaves volume or is killed
      if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
        int pdgCode = track.GetDefinition()->GetPDGEncoding();

        const G4VPhysicalVolume& v = * track.GetVolume();
        G4ThreeVector posCell = v.GetTranslation();
        // Get layer ID

        if (v.GetName().find("Crystal") != string::npos) {

          m_cellID = eclp.ECLVolNameToCellID(v.GetName());
          int saveIndex = -999;
          double dTotalEnergy = 1 / m_energyDeposit; //avoid the error  no match for 'operator/'
          if (m_energyDeposit > 0.)saveIndex = saveSimHit(m_cellID, m_trackID, pdgCode, m_WightedTime / m_energyDeposit , m_energyDeposit, m_momentum, m_WightedPos * dTotalEnergy);
          if (m_energyDeposit == 0. && m_momentum.r() > 0.01 * GeV)saveIndex = saveSimHit(m_cellID, m_trackID, pdgCode, (m_startTime + m_endTime) / 2 , m_energyDeposit, m_momentum, position);
        }

        //Reset TrackID
        m_trackID = 0;
      }

      /*
                 if (track.GetNextVolume() != track.GetVolume() || track.GetTrackStatus() >= fStopAndKill) {
                    if(find(myvector.begin(), myvector.end(), track.GetParentID()) !=myvector.end()  ){
                      myvector.push_back(m_trackID);
                      cout<<"saved track "<<m_trackID<<endl;

                    }
                    else if((find(myvector.begin(), myvector.end(), track.GetParentID()) ==myvector.end()&&find(myvector.begin(), myvector.end(), track.GetParentID()->GetParentID()) ==myvector.end()  )||track.GetParentID()==0)
                    {cout<<"Myvector Size "<< myvector.size()<<"  mother "<<track.GetParentID()  <<" first track "<< m_trackID<<" "<<pdgCode<<endl;
                     myvector.clear();
                     myvector.push_back(m_trackID);}
                 }else{
                    const G4VPhysicalVolume& v1 = *track.GetNextVolume();
                   cout<< m_trackID <<" track.GetTrackStatus() "<<track.GetTrackStatus()<<" track.GetNextVolume()   "<<v1.GetName()<<" "<<v.GetName()<<endl;

                }

      */


//    cout << pdgCode << " CellID " << m_cellID  <<" track ID "<<m_trackID<< endl;
      // Ge layer ID
//  const unsigned layerId = v.GetCopyNo();
      return true;
    }


    void SensitiveDetector::EndOfEvent(G4HCofThisEvent*)
    {
    }

    int SensitiveDetector::saveSimHit(
      const G4int cellId,
      const G4int trackID,
      const G4int pid,
      const G4double tof,
      const G4double edep,
      G4ThreeVector mom,
      TVector3 posAve)
    {
      StoreArray<MCParticle> mcParticles;
      //change Later
      StoreArray<ECLSim> eclArray;
      if (!eclArray) eclArray.create();
      RelationArray eclSimRel(mcParticles, eclArray);
      m_simhitNumber = eclArray->GetLast() + 1;
      new(eclArray->AddrAt(m_simhitNumber)) ECLSim();
      eclArray[m_simhitNumber]->setCellId(cellId);
      eclArray[m_simhitNumber]->setTrackId(trackID);
      eclArray[m_simhitNumber]->setPDGCode(pid);
      eclArray[m_simhitNumber]->setFlightTime(tof / ns);
      eclArray[m_simhitNumber]->setEnergyDep(edep / GeV);
      TVector3 momentum(mom.getX() / GeV, mom.getY() / GeV, mom.getZ() / GeV);
      eclArray[m_simhitNumber]->setMomentum(momentum);
      eclArray[m_simhitNumber]->setPosIn(posAve);
      B2DEBUG(150, "HitNumber: " << m_simhitNumber);
      eclSimRel.add(trackID, m_simhitNumber);

      StoreArray<ECLSimHit> eclSimHitArray;
      RelationArray eclSimHitToMCPart(eclSimHitArray, mcParticles);
      StoreObjPtr<EventMetaData> eventMetaDataPtr;
      int m_currentEvnetNumber = eventMetaDataPtr->getEvent();

      if (!eclSimHitArray) eclSimHitArray.create();
      //cout<<PrimaryTrackId<<" "<<trackID<<endl;
      //cout<<"firstcall "<<firstcall<<" m_oldEvnetNumber "<<m_oldEvnetNumber<<endl;
      if (firstcall == 0 || m_currentEvnetNumber != m_oldEvnetNumber) {
        m_oldEvnetNumber = m_currentEvnetNumber;
        for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
          for (int  TimeIndex = 0; TimeIndex < 16; TimeIndex++) {
            ECLHitIndex[iECLCell][TimeIndex] = -1;
          }
        }
        firstcall++;
      }
      if (m_currentEvnetNumber == m_oldEvnetNumber) {
        if ((tof / ns) < 8000) {
          TimeIndex = (int)(tof / ns) / 500;
          double E_cell = (edep / GeV);
          if (ECLHitIndex[cellId][TimeIndex] == -1) {
            m_hitNum = eclSimHitArray->GetLast() + 1;
            new(eclSimHitArray->AddrAt(m_hitNum)) ECLSimHit();

            ECLGeometryPar* eclp = ECLGeometryPar::Instance();
            PosCell =  eclp->GetCrystalPos(cellId);
            VecCell =  eclp->GetCrystalVec(cellId);
            local_pos = (15. - (posAve  - PosCell) * VecCell);
            T_ave =  6.05 + 0.0749 * local_pos - 0.00112 * local_pos * local_pos + (tof / ns)  ;

            ECLHitIndex[cellId][TimeIndex] = m_hitNum;
            eclSimHitArray[m_hitNum]->setCellId(cellId);
            eclSimHitArray[m_hitNum]->setEnergyDep(E_cell);
            eclSimHitArray[m_hitNum]->setTimeAve(T_ave);
          } else {
            m_hitNum = ECLHitIndex[cellId][TimeIndex];
            double old_edep = eclSimHitArray[m_hitNum]->getEnergyDep();
            double old_TimeAve = eclSimHitArray[m_hitNum]->getTimeAve();

            //cout<<m_currentEvnetNumber<<" "<<m_hitNum
            //    <<" old cellId  "<<eclSimHitArray[m_hitNum]->getCellId()<<" new "<<cellId
            //    <<" oldE "<<old_edep<<" + newE "<<E_cell<<"= "<<old_edep+E_cell
            //    <<" oldT "<<old_TimeAve<<" newT  "<<T_ave
            //    <<"= "<< (old_edep*old_TimeAve+ E_cell*T_ave)/(old_edep+E_cell)<<endl;
            eclSimHitArray[m_hitNum]->setEnergyDep(old_edep + E_cell);
            eclSimHitArray[m_hitNum]->setTimeAve((old_edep * old_TimeAve + E_cell * T_ave) / (old_edep + E_cell));
          }
        }
      }//if m_oldEvnetNumber==m_oldEvnetNumber
      return (m_simhitNumber);
    }//saveSimHit



  }//namespace ecl
} //namespace Belle II
