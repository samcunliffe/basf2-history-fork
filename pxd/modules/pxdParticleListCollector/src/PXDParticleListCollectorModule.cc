/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <pxd/modules/pxdParticleListCollector/PXDParticleListCollectorModule.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/RelationArray.h>
#include <mdst/dbobjects/BeamSpot.h>

#include <TTree.h>
#include <TH1I.h>
#include <TH2F.h>

#include <boost/format.hpp>

using namespace std;
using boost::format;
using namespace Belle2;
using namespace Belle2::PXD;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(PXDParticleListCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

PXDParticleListCollectorModule::PXDParticleListCollectorModule() : CalibrationCollectorModule()
  , m_selected4Eff(false)
  , m_deltaD0oSqrt2(0.0), m_deltaZ0oSqrt2(0.0)
  , m_signal(0), m_estimated(0.0)
  , m_run(0), m_exp(0)
{
  // Set module properties
  setDescription("Calibration collector module for CDST data.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("minClusterCharge", m_minClusterCharge, "Minimum cluster charge cut", int(0));
  addParam("minClusterSize", m_minClusterSize, "Minimum cluster size cut ", int(2));
  addParam("maxClusterSize", m_maxClusterSize, "Maximum cluster size cut ", int(6));
  addParam("nBinsU", m_nBinsU, "Number of gain corrections per sensor along u side", int(4));
  addParam("nBinsV", m_nBinsV, "Number of gain corrections per sensor along v side", int(6));
  addParam("gainPayloadName", m_gainName, "Payload name for Gain to be read from DB", string(""));
  addParam("fillChargeRatioHistogram", m_fillChargeRatioHistogram,
           "Flag to fill Ratio (cluster charge to the expected MPV) histograms", bool(true));
  addParam("fillChargeTree", m_fillChargeTree, "Flag to fill cluster charge with the estimated MPV to TTree", bool(false));

  // Particle list names
  addParam("PList4GainName", m_PList4GainName, "Name of the particle list for gain calibration and efficiency study",
           std::string("e+:gain"));
  addParam("PList4EffName", m_PList4EffName, "Name of the particle list for event selection in efficiency study",
           std::string("vpho:eff"));
  addParam("PList4ResName", m_PList4ResName, "Name of the particle list for resolution study", std::string("vpho:res"));

}

void PXDParticleListCollectorModule::prepare() // Do your initialise() stuff here
{

  if (m_nBinsU == 0) {
    B2WARNING("Number of bins along u side incremented from 0->1");
    m_nBinsU = 1;
  }

  if (m_nBinsV == 0) {
    B2WARNING("Number of bins along v side incremented from 0->1");
    m_nBinsV = 1;
  }

  auto gTools = VXD::GeoCache::getInstance().getGeoTools();
  int nPXDSensors = gTools->getNumberOfPXDSensors();

  //-------------------------------------------------------------------------------------
  // PXDClusterCounter: Count the number of PXDClusters and store charge for each uBin/vBin pair
  //-------------------------------------------------------------------------------------

  auto hPXDClusterCounter = new TH1I("hPXDClusterCounter", "Number of clusters found in data sample",
                                     m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                     m_nBinsU * m_nBinsV * nPXDSensors);
  hPXDClusterCounter->GetXaxis()->SetTitle("bin id");
  hPXDClusterCounter->GetYaxis()->SetTitle("Number of clusters");
  auto hPXDClusterChargeRatio = new TH2F("hPXDClusterChargeRatio", "Charge ratio of clusters found in data sample",
                                         m_nBinsU * m_nBinsV * nPXDSensors, 0,
                                         m_nBinsU * m_nBinsV * nPXDSensors,
                                         400, 0., 4.);
  hPXDClusterChargeRatio->GetXaxis()->SetTitle("bin id");
  hPXDClusterChargeRatio->GetYaxis()->SetTitle("Cluster charge ratio (relative to expected MPV)");
  for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
    for (int uBin = 0; uBin < m_nBinsU; uBin++) {
      for (int vBin = 0; vBin < m_nBinsV; vBin++) {
        VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
        string sensorDescr = id;
        hPXDClusterCounter->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                    str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
        if (m_fillChargeRatioHistogram)
          hPXDClusterChargeRatio->GetXaxis()->SetBinLabel(iSensor * m_nBinsU * m_nBinsV + uBin * m_nBinsV + vBin + 1,
                                                          str(format("%1%_%2%_%3%") % sensorDescr % uBin % vBin).c_str());
      }
    }
  }

  registerObject<TH1I>("PXDClusterCounter", hPXDClusterCounter);
  if (m_fillChargeRatioHistogram)
    registerObject<TH2F>("PXDClusterChargeRatio", hPXDClusterChargeRatio);

  //-------------------------------------------------------------------------------------
  // PXDTrackClusterCounter: Count the number of PXDClustersFrom tracks (the same track selection as for track points)
  //-------------------------------------------------------------------------------------
  auto hPXDTrackClusterCounter = (TH1I*)hPXDClusterCounter->Clone("hPXDTrackClusterCounter");
  hPXDTrackClusterCounter->SetTitle("Number of track clusters");
  hPXDTrackClusterCounter->GetYaxis()->SetTitle("Number of track clusters");
  registerObject<TH1I>("PXDTrackClusterCounter", hPXDTrackClusterCounter);

  //-------------------------------------------------------------------------------------
  // PXDTrackPointCounter: Count the number of PXDClustersFrom tracks (the same track selection as for track points)
  //-------------------------------------------------------------------------------------
  auto hPXDTrackPointCounter = (TH1I*)hPXDClusterCounter->Clone("hPXDTrackPointCounter");
  hPXDTrackPointCounter->SetTitle("Number of track points");
  hPXDTrackPointCounter->GetYaxis()->SetTitle("Number of track points");
  registerObject<TH1I>("PXDTrackPointCounter", hPXDTrackPointCounter);

  //----------------------------------------------------------------------
  // PXDTrees for gain calibration: One tree to store the calibration data for each grid bin
  //----------------------------------------------------------------------

  if (m_fillChargeTree) // only fill the tree when required
    for (int iSensor = 0; iSensor < nPXDSensors; iSensor++) {
      for (int uBin = 0; uBin < m_nBinsU; uBin++) {
        for (int vBin = 0; vBin < m_nBinsV; vBin++) {
          VxdID id = gTools->getSensorIDFromPXDIndex(iSensor);
          auto layerNumber = id.getLayerNumber();
          auto ladderNumber = id.getLadderNumber();
          auto sensorNumber = id.getSensorNumber();
          string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
          auto tree = new TTree(treename.c_str(), treename.c_str());
          tree->Branch<int>("signal", &m_signal);
          tree->Branch<float>("estimated", &m_estimated);
          registerObject<TTree>(treename, tree);
        }
      }
    }

  auto hTotalHitsLayer1  = new TH2F("hTotalHitsLayer1",  "Total number of hits from layer 1;#phi;z [cm]",  730, -M_PI, M_PI, 400,
                                    -3.2, 6.2);
  auto hPassedHitsLayer1 = new TH2F("hPassedHitsLayer1", "Passed number of hits from layer 1;#phi;z [cm]", 730, -M_PI, M_PI, 400,
                                    -3.2, 6.2);
  auto hTotalHitsLayer2  = new TH2F("hTotalHitsLayer2",  "Total number of hits from layer 2;#phi;z [cm]",  128,   1.4,  2.5, 400,
                                    -4.2, 8.2);
  auto hPassedHitsLayer2 = new TH2F("hPassedHitsLayer2", "Passed number of hits from layer 2;#phi;z [cm]", 128,   1.4,  2.5, 400,
                                    -4.2, 8.2);
  registerObject<TH2F>("hTotalHitsLayer1", hTotalHitsLayer1);
  registerObject<TH2F>("hPassedHitsLayer1", hPassedHitsLayer1);
  registerObject<TH2F>("hTotalHitsLayer2", hTotalHitsLayer2);
  registerObject<TH2F>("hPassedHitsLayer2", hPassedHitsLayer2);

  // trees for correctd d0 and z0 to the IP
  auto treeD0Z0 = new TTree("tree_d0z0", "TTree of corrected d0 and z0");
  treeD0Z0->Branch<float>("d0", &m_deltaD0oSqrt2);
  treeD0Z0->Branch<float>("z0", &m_deltaZ0oSqrt2);
  registerObject<TTree>("tree_d0z0", treeD0Z0);

  // dbtree
  auto dbtree = new TTree("dbtree", "dbtree");
  dbtree->Branch<int>("run", &m_run);
  dbtree->Branch<int>("exp", &m_exp);
  dbtree->Branch<PXDGainMapPar>("gainMap", &m_gainMap);
  registerObject<TTree>("dbtree", dbtree);

}

void PXDParticleListCollectorModule::startRun() // Do your beginRun() stuff here
{
  m_run = m_evtMetaData->getRun();
  m_exp = m_evtMetaData->getExperiment();
  if (m_gainName.length()) {
    DBObjPtr<PXDGainMapPar> gainMap(m_gainName);
    m_gainMap = *gainMap;
  } else {
    m_gainMap = PXDGainMapPar();
  }
  getObjectPtr<TTree>("dbtree")->Fill();
}

void PXDParticleListCollectorModule::collect() // Do your event() stuff here
{
  // Update booleans for even selection
  m_selected4Eff = false;
  StoreObjPtr<ParticleList> particles4Eff(m_PList4EffName);
  if (particles4Eff.isValid() && particles4Eff->getListSize() == 1)
    m_selected4Eff = true;

  collectDeltaIP(); // using ParticleList(m_PList4ResName)

  StoreObjPtr<ParticleList> particles(m_PList4GainName);
  // Do nothing if the list is empty
  if (!particles.isValid() || particles->getListSize() < 1)
    return;

  for (auto const& particle : *particles) {
    const Track* trackPtr = particle.getTrack();
    if (!trackPtr) return;
    auto recoTrackPtr = trackPtr->getRelated<RecoTrack>("");
    if (!recoTrackPtr) return;
    auto pxdIntercepts = recoTrackPtr->getRelationsTo<PXDIntercept>("");
    for (auto const& pxdIntercept : pxdIntercepts) {
      TrackCluster_t trackCluster;
      // Function setValues() also returns a recoTrack pointer
      if (!trackCluster.setValues(pxdIntercept, "", "PXDClustersFromTracks"))
        continue;

      auto const& cluster = trackCluster.cluster;
      auto const& intersection = trackCluster.intersection;
      auto const& usedInTrack = trackCluster.usedInTrack;


      // Collect info for efficiency study
      if (m_selected4Eff)
        collectEfficiencyVariables(trackCluster);

      // Collect info for gain calibration
      // Check for valid cluster and intersection
      if (!usedInTrack || intersection.chargeMPV <= 0)
        continue;
      // Apply cluster selection cuts
      if (cluster.charge < m_minClusterCharge || cluster.size < m_minClusterSize || cluster.size > m_maxClusterSize)
        continue;
      collectGainVariables(trackCluster);

    } // end of pxdIntercepts loop
  } // end of particles loop
}

void PXDParticleListCollectorModule::collectDeltaIP()
{
  StoreObjPtr<ParticleList> particles(m_PList4ResName);
  // Requiring vpho -> l+l-
  if (!particles.isValid() || particles->getListSize() != 1)
    return;

  const Particle* mother = particles->getParticle(0);
  // Use the vertex of the mother particle (vph0 -> l+l-) instead of IP for impact parameter correction
  // This leads to very small sigma_z0
  //TVector3 vertex = mother->getVertex();

  // Use beam spot info for the interaction point
  DBObjPtr<BeamSpot> beamSpotDB;
  auto ip = beamSpotDB->getIPPosition();
  auto vertex = ip;

  const Particle* part0 = mother->getDaughter(0);
  const Particle* part1 = mother->getDaughter(1);
  //const TrackFitResult* tr0 = part0->getTrackFitResult();
  //const TrackFitResult* tr1 = part1->getTrackFitResult();
  const TrackFitResult* tr0 = part0->getTrack()->getTrackFitResultWithClosestMass(Const::pion);
  const TrackFitResult* tr1 = part1->getTrack()->getTrackFitResultWithClosestMass(Const::pion);

  track_struct.setTrackVariables(tr0, vertex);
  auto d0p_0 = track_struct.d0p;
  auto z0p_0 = track_struct.z0p;
  track_struct.setTrackVariables(tr1, vertex);
  auto d0p_1 = track_struct.d0p;
  auto z0p_1 = track_struct.z0p;

  m_deltaD0oSqrt2 = (d0p_0 + d0p_1) / sqrt(2.);
  m_deltaZ0oSqrt2 = (z0p_0 - z0p_1) / sqrt(2.);

  // Fill the tree of impact parameters
  getObjectPtr<TTree>("tree_d0z0")->Fill();
}

void PXDParticleListCollectorModule::collectGainVariables(const TrackCluster_t& trackCluster)
{
  auto cluster = trackCluster.cluster;
  auto intersection = trackCluster.intersection;
  // Compute variables from cluster needed for gain estimation
  m_signal = cluster.charge;
  m_estimated = intersection.chargeMPV;

  int uBin(-1), vBin(-1);
  auto binID = getBinID(trackCluster, uBin, vBin);
  VxdID sensorID = getVxdIDFromPXDModuleID(cluster.pxdID);
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();

  // Increment the counter
  getObjectPtr<TH1I>("PXDClusterCounter")->Fill(binID);

  // Fill variabels into tree
  if (m_fillChargeTree) {
    string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);
    getObjectPtr<TTree>(treename)->Fill();
  }

  // Fill cluster charge ratio histogram if enabled
  if (m_fillChargeRatioHistogram) {
    double ratio = m_signal / m_estimated;
    auto axis = getObjectPtr<TH2F>("PXDClusterChargeRatio")->GetYaxis();
    double maxY  = axis->GetBinCenter(axis->GetNbins());
    // Manipulate too large ratio for better estimation on median.
    getObjectPtr<TH2F>("PXDClusterChargeRatio")->Fill(binID, TMath::Min(ratio, maxY));
  }
}

void PXDParticleListCollectorModule::collectEfficiencyVariables(const TrackCluster_t& trackCluster)
{
  auto cluster = trackCluster.cluster;
  auto intersection = trackCluster.intersection;
  auto usedInTrack = trackCluster.usedInTrack;

  auto x = intersection.x;
  auto y = intersection.y;
  auto phi = atan2(y, x);
  auto z = intersection.z;

  int uBin(-1), vBin(-1);
  auto binID = getBinID(trackCluster, uBin, vBin);
  // Filling counters
  getObjectPtr<TH1I>("PXDTrackPointCounter")->Fill(binID);
  if (usedInTrack)
    getObjectPtr<TH1I>("PXDTrackClusterCounter")->Fill(binID);

  // Filling 2D histograms
  if (cluster.pxdID < 2000) {
    //getObjectPtr<TEfficiency>("PXDLayer1Efficiency")->Fill(usedInTrack,phi,z);
    getObjectPtr<TH2F>("hTotalHitsLayer1")->Fill(phi, z);
    if (usedInTrack)
      getObjectPtr<TH2F>("hPassedHitsLayer1")->Fill(phi, z);
  } else {
    //getObjectPtr<TEfficiency>("PXDLayer2Efficiency")->Fill(usedInTrack,phi,z);
    getObjectPtr<TH2F>("hTotalHitsLayer2")->Fill(phi, z);
    if (usedInTrack)
      getObjectPtr<TH2F>("hPassedHitsLayer2")->Fill(phi, z);
  }

}
