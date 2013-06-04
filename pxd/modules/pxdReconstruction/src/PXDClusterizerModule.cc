/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdReconstruction/PXDClusterizerModule.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/logging/Logger.h>

#include <vxd/geometry/GeoCache.h>
#include <pxd/geometry/SensorInfo.h>

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDDigit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <boost/foreach.hpp>

using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDClusterizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDClusterizerModule::PXDClusterizerModule() :
  Module(), m_elNoise(200.0), m_cutSeed(5.0), m_cutAdjacent(3.0), m_cutCluster(
    8.0), m_sizeHeadTail(3)
{
  //Set module properties
  setDescription("Cluster PXDHits");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("ElectronicNoise", m_elNoise,
           "Noise added by the electronics, set in ENC", m_elNoise);
  addParam("NoiseSN", m_cutAdjacent,
           "SN for digits to be considered for clustering", m_cutAdjacent);
  addParam("SeedSN", m_cutSeed, "SN for digits to be considered as seed",
           m_cutSeed);
  addParam("ClusterSN", m_cutCluster, "Minimum SN for clusters", m_cutCluster);
  addParam("HeadTailSize", m_sizeHeadTail,
           "Minimum cluster size to switch to Analog head tail algorithm for cluster center",
           m_sizeHeadTail);

  addParam("Digits", m_storeDigitsName, "Digits collection name", string(""));
  addParam("Clusters", m_storeClustersName, "Cluster collection name",
           string(""));
  addParam("TrueHits", m_storeTrueHitsName, "TrueHit collection name",
           string(""));
  addParam("MCParticles", m_storeMCParticlesName, "MCParticles collection name",
           string(""));

  addParam("TanLorentz", m_tanLorentzAngle, "Tangent of the Lorentz angle",
           double(0.25));
  addParam("AssumeSorted", m_assumeSorted,
           "Assume Digits in Collection are orderd by sensor,row,column in ascending order",
           true);
}

void PXDClusterizerModule::initialize()
{
  //Register output collections
  StoreArray<PXDCluster>::registerPersistent(m_storeClustersName);
  RelationArray::registerPersistent<PXDCluster, MCParticle>(m_storeClustersName,
                                                            m_storeMCParticlesName);
  RelationArray::registerPersistent<PXDCluster, PXDDigit>(m_storeClustersName,
                                                          m_storeDigitsName);
  RelationArray::registerPersistent<PXDCluster, PXDTrueHit>(m_storeClustersName,
                                                            m_storeTrueHitsName);

  //Set names in case default was used. We need these for the RelationIndices.
  m_relDigitMCParticleName = DataStore::relationName(
                               DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                               DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relClusterMCParticleName = DataStore::relationName(
                                 DataStore::arrayName<PXDCluster>(m_storeClustersName),
                                 DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  m_relClusterDigitName = DataStore::relationName(
                            DataStore::arrayName<PXDCluster>(m_storeClustersName),
                            DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  m_relDigitTrueHitName = DataStore::relationName(
                            DataStore::arrayName<PXDDigit>(m_storeDigitsName),
                            DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  m_relClusterTrueHitName = DataStore::relationName(
                              DataStore::arrayName<PXDCluster>(m_storeClustersName),
                              DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));

  B2INFO(
    "PXDClusterizer Parameters (in default system units, *=cannot be set directly):");
  B2INFO(" -->  ElectronicNoise:    " << m_elNoise);
  B2INFO(" -->  NoiseSN:            " << m_cutAdjacent);
  B2INFO(" -->  SeedSN:             " << m_cutSeed);
  B2INFO(" -->  ClusterSN:          " << m_cutCluster);
  B2INFO(
    " -->  MCParticles:        " << DataStore::arrayName<MCParticle>(m_storeMCParticlesName));
  B2INFO(
    " -->  Digits:             " << DataStore::arrayName<PXDDigit>(m_storeDigitsName));
  B2INFO(
    " -->  Clusters:           " << DataStore::arrayName<PXDCluster>(m_storeClustersName));
  B2INFO(
    " -->  TrueHits:           " << DataStore::arrayName<PXDTrueHit>(m_storeTrueHitsName));
  B2INFO(" -->  DigitMCRel:         " << m_relDigitMCParticleName);
  B2INFO(" -->  ClusterMCRel:       " << m_relClusterMCParticleName);
  B2INFO(" -->  ClusterDigitRel:    " << m_relClusterDigitName);
  B2INFO(" -->  DigitTrueRel:       " << m_relDigitTrueHitName);
  B2INFO(" -->  ClusterTrueRel:     " << m_relClusterTrueHitName);
  B2INFO(" -->  AssumeSorted:       " << (m_assumeSorted ? "true" : "false"));
  B2INFO(" -->  TanLorentz:         " << m_tanLorentzAngle);

  //This is still static noise for all pixels, should be done more sophisticated in the future
  m_noiseMap.setNoiseLevel(m_elNoise);
}

inline void PXDClusterizerModule::findCluster(const Pixel& px)
{
  ClusterCandidate* prev = m_cache.findCluster(px.getU(), px.getV());
  if (!prev) {
    m_clusters.push_back(ClusterCandidate());
    prev = &m_clusters.back();
  }
  prev->add(px);
  m_cache.setLast(px.getU(), px.getV(), prev);
}

void PXDClusterizerModule::event()
{
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);

  if (!storeClusters.isValid())
    storeClusters.create();
  else
    storeClusters.getPtr()->Clear();

  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  relClusterMCParticle.clear();

  RelationArray relClusterDigit(storeClusters, storeDigits,
                                m_relClusterDigitName);
  relClusterDigit.clear();

  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);
  relClusterTrueHit.clear();

  int nPixels = storeDigits.getEntries();
  if (nPixels == 0)
    return;

  m_clusters.clear();
  m_cache.clear();

  if (!m_assumeSorted) {
    //If the pixels are in random order, we have to sort them first before we can cluster them
    std::map<VxdID, Sensor> sensors;
    //Fill sensors
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
      VxdID sensorID = px.get()->getSensorID();
      std::pair<Sensor::iterator, bool> it = sensors[sensorID].insert(px);
      if (!it.second)
        B2ERROR(
          "Pixel (" << px.getU() << "," << px.getV() << ") in sensor " << (string)sensorID << " is already set, ignoring second occurence");
    }

    //Now we loop over sensors and cluster each sensor in turn
    for (map<VxdID, Sensor>::iterator it = sensors.begin(); it != sensors.end();
         ++it) {
      m_noiseMap.setSensorID(it->first);
      BOOST_FOREACH(const PXD::Pixel & px, it->second) {
        if (!m_noiseMap(px, m_cutAdjacent)) continue;
        findCluster(px);
      }
      writeClusters(it->first);
      it->second.clear();
    }
  } else {
    //If we can assume that all pixels are already sorted we can skip the
    //reordering and directly cluster them once the sensorID changes, we write
    //out all existing clusters and continue
    VxdID sensorID;
    unsigned int lastU(0), lastV(0);
    for (int i = 0; i < nPixels; i++) {
      Pixel px(storeDigits[i], i);
      //Load the correct noise map for the first pixel
      if (i == 0)
        m_noiseMap.setSensorID(px.getSensorID());
      //Ignore digits with not enough signal
      if (!m_noiseMap(px, m_cutAdjacent))
        continue;

      //New sensor, write clusters
      if (sensorID != px.getSensorID()) {
        writeClusters(sensorID);
        sensorID = px.getSensorID();
        //Load the correct noise map for the new sensor
        m_noiseMap.setSensorID(sensorID);
      } else if (lastV > px.getV()
                 || (lastV == px.getV() && lastU > px.getU())) {
        //Check for sorting as precaution
        B2FATAL(
          "Pixels are not sorted correctly, please change the assumeSorted parameter " << "to false or fix the input to be ordered by v,u in ascending order");
      }
      lastU = px.getU();
      lastV = px.getV();
      // Find correct cluster and add pixel to cluster
      findCluster(px);
    }
    writeClusters(sensorID);
  }
}

void PXDClusterizerModule::writeClusters(VxdID sensorID)
{
  if (m_clusters.empty())
    return;

  //Get all datastore elements
  StoreArray<MCParticle> storeMCParticles(m_storeMCParticlesName);
  StoreArray<PXDDigit> storeDigits(m_storeDigitsName);
  StoreArray<PXDTrueHit> storeTrueHits(m_storeTrueHitsName);
  StoreArray<PXDCluster> storeClusters(m_storeClustersName);
  RelationArray relClusterMCParticle(storeClusters, storeMCParticles,
                                     m_relClusterMCParticleName);
  RelationArray relClusterDigit(storeClusters, storeDigits,
                                m_relClusterDigitName);
  RelationArray relClusterTrueHit(storeClusters, storeTrueHits,
                                  m_relClusterTrueHitName);
  RelationIndex<PXDDigit, MCParticle> relDigitMCParticle(storeDigits,
                                                         storeMCParticles, m_relDigitMCParticleName);
  RelationIndex<PXDDigit, PXDTrueHit> relDigitTrueHit(storeDigits,
                                                      storeTrueHits, m_relDigitTrueHitName);

  //Get Geometry information
  const SensorInfo& info = dynamic_cast<const SensorInfo&>(VXD::GeoCache::get(
                                                             sensorID));

  BOOST_FOREACH(ClusterCandidate & cls, m_clusters) {
    //Check for noise cuts
    if (!(cls.size() > 0 && m_noiseMap(cls.getCharge(), m_cutCluster) && m_noiseMap(cls.getSeed(), m_cutSeed))) continue;

    const Pixel& seed = cls.getSeed();
    unsigned int maxU(0), minU(info.getUCells() - 1) , maxV(0), minV(info.getVCells() - 1);
    double minUCharge(0), maxUCharge(0), minVCharge(0), maxVCharge(0);
    double posU(0), posV(0);

// For head tail, we need the min/max pixels in both directions. To avoid copy
// pasting, we define a macro for checking if the ID is at the border
#define SET_CLUSTER_LIMIT(var,op,cell,charge)\
  if(var op cell){\
    var = cell;\
    var##Charge = charge;\
  }else if(var == cell){\
    var##Charge += charge;\
  }

    map<int, float> mc_relations;
    map<int, float> truehit_relations;
    vector<pair<int, float> > digit_weights;
    digit_weights.reserve(cls.size());
    BOOST_FOREACH(const PXD::Pixel & px, cls.pixels()) {
      SET_CLUSTER_LIMIT(minU, > , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(maxU, < , px.getU(), px.getCharge());
      SET_CLUSTER_LIMIT(minV, > , px.getV(), px.getCharge());
      SET_CLUSTER_LIMIT(maxV, < , px.getV(), px.getCharge());

      posU += px.getCharge() * info.getUCellPosition(px.getU());
      posV += px.getCharge() * info.getVCellPosition(px.getV());

      typedef const RelationIndex<PXDDigit, MCParticle>::Element relMC_type;
      typedef const RelationIndex<PXDDigit, PXDTrueHit>::Element relTrueHit_type;

      //Fill map with MCParticle relations
      BOOST_FOREACH(relMC_type & mcRel, relDigitMCParticle.getElementsFrom(px.get())) {
        mc_relations[mcRel.indexTo] += mcRel.weight;
      };
      //Fill map with PXDTrueHit relations
      BOOST_FOREACH(relTrueHit_type & trueRel, relDigitTrueHit.getElementsFrom(px.get())) {
        truehit_relations[trueRel.indexTo] += trueRel.weight;
      };
      //Add digit to the Cluster->Digit relation list
      digit_weights.push_back(make_pair(px.getIndex(), px.getCharge()));
    }
    posU /= cls.getCharge();
    posV /= cls.getCharge();
    const int sizeU = maxU - minU + 1;
    const int sizeV = maxV - minV + 1;
    // Calculate shape correlation coefficient: only for non-trivial shapes
    double uError(0), vError(0), rho(0);
    if ((sizeU > 1) && (sizeV > 1)) {
      double posUU(0), posVV(0), posUV(0);
      BOOST_FOREACH(const PXD::Pixel & px, cls.pixels()) {
        double du = info.getUCellPosition(px.getU()) - posU;
        double dv = info.getVCellPosition(px.getV()) - posV;
        posUU += px.getCharge() * du * du;
        posVV += px.getCharge() * dv * dv;
        posUV += px.getCharge() * du * dv;
      }
      rho = posUV / sqrt(posUU * posVV);
    }
    if (sizeU >= m_sizeHeadTail) {
      //Average charge in the central area
      double centreCharge = (cls.getCharge() - minUCharge - maxUCharge) / (sizeU - 2);
      minUCharge = (minUCharge < centreCharge) ? minUCharge : centreCharge;
      maxUCharge = (maxUCharge < centreCharge) ? maxUCharge : centreCharge;
      double minUPos = info.getUCellPosition(minU);
      double maxUPos = info.getUCellPosition(maxU);
      posU = 0.5 * (minUPos + maxUPos + (maxUCharge - minUCharge) / centreCharge * info.getUPitch());
      double sn = centreCharge / m_cutAdjacent / m_elNoise;
      double landauHead = minUCharge / centreCharge;
      double landauTail = maxUCharge / centreCharge;
      uError = 0.5 * info.getUPitch() * sqrt(2.0 / sn / sn +
                                             0.5 * landauHead * landauHead + 0.5 * landauTail * landauTail);
    } else if (sizeU <= 2) { // Add a phantom charge to second strip
      double phantomCharge = m_cutAdjacent * m_elNoise;
      uError = info.getUPitch() * (sizeV + 2) * phantomCharge / (cls.getCharge() + (sizeV + 3) * phantomCharge);
    } else { // from 3 to m_sizeHeadTail
      double sn = cls.getSeedCharge() / m_elNoise;
      uError = 2 * info.getUPitch() / sn;
    }
    if (sizeV >= m_sizeHeadTail) {
      //Average charge in the central area
      double centreCharge = (cls.getCharge() - minVCharge - maxVCharge) / (sizeV - 2);
      minVCharge = (minVCharge < centreCharge) ? minVCharge : centreCharge;
      maxVCharge = (maxVCharge < centreCharge) ? maxVCharge : centreCharge;
      double minVPos = info.getVCellPosition(minV);
      double maxVPos = info.getVCellPosition(maxV);
      posV = 0.5 * (minVPos + maxVPos + (maxVCharge * info.getVPitch(maxVPos) -
                                         minVCharge * info.getVPitch(minVPos)) / centreCharge);
      double snHead = centreCharge / m_cutAdjacent / m_elNoise / info.getVPitch(minVPos);
      double snTail = centreCharge / m_cutAdjacent / m_elNoise / info.getVPitch(maxVPos);
      double landauHead = minUCharge / centreCharge * info.getVPitch(minVPos);
      double landauTail = maxUCharge / centreCharge * info.getVPitch(maxVPos);
      vError = 0.5 * sqrt(1.0 / snHead / snHead + 1.0 / snTail / snTail +
                          0.5 * landauHead * landauHead + 0.5 * landauTail * landauTail);
    } else if (sizeV <= 2) { // Add a phantom charge to second strip
      double phantomCharge = m_cutAdjacent * m_elNoise;
      vError = info.getVPitch(posV) * (sizeU + 2) * phantomCharge / (cls.getCharge() + (sizeU + 3) * phantomCharge);
    } else { // from 3 to m_sizeHeadTail
      double sn = cls.getSeedCharge() / m_elNoise;
      vError = 2 * info.getVPitch(posV) / sn;
    }

    //Lorentz shift correction FIXME: get from Bfield
    posU -= 0.5 * info.getThickness() * m_tanLorentzAngle;

    //Store Cluster into Datastore ...
    int clsIndex = storeClusters.getEntries();
    storeClusters.appendNew(PXDCluster(
                              seed.get()->getSensorID(), posU, posV, uError, vError,
                              rho, cls.getCharge(), seed.getCharge(),
                              cls.size(), sizeU, sizeV, minU, minV
                            ));

    //Create Relations to this Digit
    relClusterMCParticle.add(clsIndex, mc_relations.begin(), mc_relations.end());
    relClusterTrueHit.add(clsIndex, truehit_relations.begin(), truehit_relations.end());
    relClusterDigit.add(clsIndex, digit_weights.begin(), digit_weights.end());
  }

  m_clusters.clear();
  m_cache.clear();
}
