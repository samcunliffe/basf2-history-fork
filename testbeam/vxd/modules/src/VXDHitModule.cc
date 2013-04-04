/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#include <testbeam/vxd/modules/VXDHitModule.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>

#include <framework/datastore/StoreArray.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <testbeam/vxd/dataobjects/VXDHit.h>

#include <generators/dataobjects/MCParticle.h>

#include <boost/foreach.hpp>
#include <cmath>
#include <svd/geometry/GeoSVDCreator.h>
#include <../framework/datastore/include/RelationArray.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::VXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(VXDHit)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

VXDHitModule::VXDHitModule() : Module()
{
  //Set module properties
  setDescription("Module to analyze PXD+SVD TrueHits and primary particles");

  addParam("MCParticles", m_MCParticles,
           "MCParticle collection name", string(""));

}

void VXDHitModule::initialize()
{
  StoreArray<VXDHit>::registerPersistent();
}

void VXDHitModule::beginRun()
{

}

void VXDHitModule::event()
{
  StoreArray<MCParticle> particles;
  //const int nParticles = particles.getEntries();

  StoreArray<PXDTrueHit> pxdhits;
  StoreArray<SVDTrueHit> svdhits;
  StoreArray<VXDHit> hits;

  const unsigned int numPXDHits = pxdhits.getEntries();
  const unsigned int numSVDHits = svdhits.getEntries();

  for (unsigned int i = 0; i < numPXDHits; i++) {
    bool isFromPrimary = false;
    int pdg = 0;
    double pMomentum = 0.0;
    int nPrimary = 0;

    const PXDTrueHit* pxdhit = pxdhits[i];

    RelationVector<MCParticle> mcs = pxdhit->getRelationsFrom<MCParticle>();
    pdg = mcs[0]->getPDG();
    pMomentum = double(mcs[0]->getMomentum().Mag());
    for (unsigned int j = 0; j < mcs.size(); j++) {
      if (mcs[j]->getStatus() & MCParticle::c_PrimaryParticle) {
        //B2ERROR("PXDTrueHit "<<i<<" is from primary particle");
        isFromPrimary = true;
        nPrimary++;
      }
    }
    VXDHit* hit = hits.appendNew(
                    VXDHit(
                      pxdhit->getSensorID(),
                      pxdhit->getU(),
                      pxdhit->getV(),
                      pxdhit->getEntryU(),
                      pxdhit->getEntryV(),
                      pxdhit->getExitU(),
                      pxdhit->getExitV(),
                      pxdhit->getEnergyDep(),
                      pxdhit->getGlobalTime(),
                      pxdhit->getMomentum(),
                      pxdhit->getEntryMomentum(),
                      pxdhit->getExitMomentum()));

    hit->setFromPrimary(isFromPrimary);
    hit->setPdg(pdg);
    hit->setParticleMomentum(pMomentum);
    hit->setNumParticles(mcs.size());
    hit->setNumPrimary(nPrimary);

  }

  for (unsigned int i = 0; i < numSVDHits; i++) {
    bool isFromPrimary = false;
    int pdg = 0;
    double pMomentum = 0.0;
    int nPrimary = 0;
    const SVDTrueHit* svdhit = svdhits[i];

    RelationVector<MCParticle> mcs = svdhit->getRelationsFrom<MCParticle>();
    pdg = mcs[0]->getPDG();
    pMomentum = double(mcs[0]->getMomentum().Mag());
    for (unsigned int j = 0; j < mcs.size(); j++) {
      if (mcs[j]->getStatus() & MCParticle::c_PrimaryParticle) {
        //B2ERROR("SVDTrueHit "<<i<<" is from primary particle");
        isFromPrimary = true;
        nPrimary++;

      }
    }

    VXDHit* hit = hits.appendNew(
                    VXDHit(
                      svdhit->getSensorID(),
                      svdhit->getU(),
                      svdhit->getV(),
                      svdhit->getEntryU(),
                      svdhit->getEntryV(),
                      svdhit->getExitU(),
                      svdhit->getExitV(),
                      svdhit->getEnergyDep(),
                      svdhit->getGlobalTime(),
                      svdhit->getMomentum(),
                      svdhit->getEntryMomentum(),
                      svdhit->getExitMomentum()));

    hit->setFromPrimary(isFromPrimary);
    hit->setPdg(pdg);
    hit->setParticleMomentum(pMomentum);
    hit->setNumParticles(mcs.size());
    hit->setNumPrimary(nPrimary);
  }


}




void VXDHitModule::terminate()
{

}
