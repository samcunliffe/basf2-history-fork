/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka, Martin Ritter                              *
 *                                                                        *
 **************************************************************************/

#include <pxd/testbeam/modules/TelDigiClusterModule.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/logging/Logger.h>
#include <framework/gearbox/Unit.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/testbeam/dataobjects/TelSimHit.h>
#include <pxd/testbeam/dataobjects/TelTrueHit.h>
#include <pxd/testbeam/dataobjects/TelCluster.h>
#include <boost/foreach.hpp>
#include <cmath>

#include <TRandom3.h>
#include <TMath.h>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TelDigiCluster)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TelDigiClusterModule::TelDigiClusterModule() : Module()
{
  //Set module properties
  setDescription("Digitize/Clusterize Telescope TrueHits");

  addParam("ResolutionU", m_resolutionU,
           "Resolution (sigma) for gaussian position smearing (in um) in U direction", float(4.5));
  addParam("ResolutionV", m_resolutionV,
           "Resolution (sigma) for gaussian position smearing (in um) in V direction", float(4.5));
  addParam("MCParticles", m_storeMCParticlesName,
           "MCParticle collection name", string(""));
  addParam("TrueHits", m_storeTrueHitsName,
           "TrueHit collection name", string(""));
  addParam("Clusters", m_storeClustersName,
           "TrueHit collection name", string(""));
  addParam("MCTrueHitRel", m_relMCParticleTrueHitName,
           "Relation between MCParticles and SimHits", string(""));
  addParam("ClusterMCRel", m_relClusterMCParticleName,
           "Relation between clusters and MCParticles", string(""));
  addParam("ClusterTrueHitRel", m_relClusterTrueHitName,
           "Relation between clusters and TrueHits", string(""));
}

void TelDigiClusterModule::initialize()
{
  //Register all required collections
  StoreArray<TelCluster>::registerPersistent(m_storeClustersName);
  RelationArray::registerPersistent<MCParticle, TelTrueHit>(m_relMCParticleTrueHitName);
  RelationArray::registerPersistent<TelCluster, MCParticle>(m_relClusterMCParticleName);
  RelationArray::registerPersistent<TelCluster, TelTrueHit>(m_relClusterTrueHitName);

  //Set names in case default was used
  m_relMCParticleTrueHitName = DataStore::relationName(
                                 DataStore::arrayName<MCParticle>(m_storeMCParticlesName),
                                 DataStore::arrayName<TelTrueHit>(m_storeTrueHitsName)
                               );
  m_relClusterMCParticleName = DataStore::relationName(
                                 DataStore::arrayName<TelCluster>(m_storeClustersName),
                                 DataStore::arrayName<MCParticle>(m_storeMCParticlesName)
                               );
  m_relClusterTrueHitName    = DataStore::relationName(
                                 DataStore::arrayName<TelCluster>(m_storeClustersName),
                                 DataStore::arrayName<TelTrueHit>(m_storeTrueHitsName)
                               );;

  //Convert parameters to correct units
  m_resolutionU *= Unit::um;
  m_resolutionV *= Unit::um;

  B2INFO("TBDigiClusterModule Parameters (in default system units):");
  B2INFO(" -->  TrueHits:           " << DataStore::arrayName<TelTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  MCTrueHitRel:        " << m_relMCParticleTrueHitName);
  B2INFO(" -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(" -->  Clusters:           " << DataStore::arrayName<TelCluster>(m_storeClustersName));
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);

  // Check if the global random number generator is available.
  if (!gRandom) B2FATAL("gRandom not initialized, please set up gRandom first");
}

void TelDigiClusterModule::beginRun()
{

}


void TelDigiClusterModule::event()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<TelTrueHit> storeTrueHits(m_storeTrueHitsName);

  // FIXME: Provisional fix to ensure proper output when there are no SimHits:
  // Create empty arrays, then empty relations will be created, too.
  StoreArray<TelSimHit>  storeSimHits(""); // we don't need this here.
  if (!storeSimHits.isValid())  storeSimHits.create();
  if (!storeTrueHits.isValid()) storeTrueHits.create();
  // For the same reason, initialize the RelationArrays - though the module doesn't need them.
  RelationArray mcParticlesToSimHits(storeMCParticles, storeSimHits);
  RelationArray mcParticlesToTrueHits(storeMCParticles, storeTrueHits);
  RelationArray trueHitsToSimHits(storeTrueHits, storeSimHits);

  StoreArray<TelCluster> storeClusters(m_storeClustersName);
  if (!storeClusters.isValid())
    storeClusters.create();
  else
    storeClusters->Clear();

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles, m_relClusterMCParticleName);
  relClusterMCParticle.clear();

  RelationArray relClusterTrueHit(storeClusters, storeTrueHits, m_relClusterTrueHitName);
  relClusterTrueHit.clear();

  unsigned int nTrueHits = storeTrueHits.getEntries();
  RelationIndex<MCParticle, TelTrueHit> relIndexMCParticleTrueHit(storeMCParticles, storeTrueHits, m_relMCParticleTrueHitName);

  for (unsigned int i = 0; i < nTrueHits; i++) {
    TelTrueHit* trueHit = storeTrueHits[i];
    const RelationIndex<MCParticle, TelTrueHit>::Element* mcRel = relIndexMCParticleTrueHit.getFirstElementTo(trueHit);
    int particleID = (int)mcRel->indexFrom ;
    int sensorID = (int)trueHit->getSensorID();
    float charge = trueHit->getEnergyDep() * Unit::GeV / Unit::ehEnergy;
    float u = trueHit->getU();
    float v = trueHit->getV();
    u = gRandom->Gaus(u, m_resolutionU);
    v = gRandom->Gaus(v, m_resolutionV);

    //Store Cluster into Datastore ...
    int clusterID = storeClusters->GetLast() + 1;
    new(storeClusters->AddrAt(clusterID)) TelCluster(
      sensorID, u, v,
      charge, charge,
      0, 0, 0
    );

    //Create Relations to this Cluster
    relClusterMCParticle.add(clusterID, particleID, charge);
    relClusterTrueHit.add(clusterID, i, charge);
  }

}




void TelDigiClusterModule::terminate()
{

}
