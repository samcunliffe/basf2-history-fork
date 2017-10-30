/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 * Contributors: Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclTrackClusterMatching/ECLTrackClusterMatchingModule.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationVector.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>
#include <set>
#include <vector>
#include <cmath>

using namespace std;
using namespace Belle2;

REG_MODULE(ECLTrackClusterMatching)

ECLTrackClusterMatchingModule::ECLTrackClusterMatchingModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_tree(0),
    m_iExperiment(0),
    m_iRun(0),
    m_iEvent(0),
    m_trackNo(0),
    m_trackMomentum(0),
    m_deltaPhi(0),
    m_phiCluster(0),
    m_errorPhi(0),
    m_deltaTheta(0),
    m_thetaCluster(0),
    m_errorTheta(0),
    m_phi_consistency(0),
    m_theta_consistency(0),
    m_quality(0),
    m_quality_best(0),
    m_hitstatus_best(0)
{
  setDescription("Match Tracks to ECLCluster");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the information in a root file named by parameter 'rootFileName'", bool(false));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclTrackClusterMatchingAnalysis.root"));
}

ECLTrackClusterMatchingModule::~ECLTrackClusterMatchingModule()
{
}

void ECLTrackClusterMatchingModule::initialize()
{
  B2INFO("[ECLTrackClusterMatching Module]: Starting initialization of ECLTrackClusterMatching Module.");

  StoreArray<Track> tracks;
  StoreArray<ECLCluster> eclClusters;
  tracks.registerRelationTo(eclClusters);
  tracks.registerRelationTo(tracks);

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Track Cluster Matching Analysis tree");

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");
  // m_tree->Branch("trackNo", &m_trackNo, "trackNo/I");
  m_tree->Branch("trackNo", "std::vector<int>", &m_trackNo);

  // m_tree->Branch("trackMomentum", &m_trackMomentum, "trackMomentum/D");
  // m_tree->Branch("deltaPhi", &m_deltaPhi, "deltaPhi/D");
  // m_tree->Branch("phiCluster", &m_phiCluster, "phiCluster/D");
  // m_tree->Branch("errorPhi", &m_errorPhi, "errorPhi/D");
  // m_tree->Branch("deltaTheta", &m_deltaTheta, "deltaTheta/D");
  // m_tree->Branch("thetaCluster", &m_thetaCluster, "thetaCluster/D");
  // m_tree->Branch("errorTheta", &m_errorTheta, "errorTheta/D");
  // m_tree->Branch("quality", &m_quality, "quality/D");
  // m_tree->Branch("quality_best", &m_quality_best, "quality_best/D");
  // m_tree->Branch("hitStatus_best", &m_hitstatus_best, "hitStatus_best/I");

  m_tree->Branch("trackMomentum", "std::vector<double>", &m_trackMomentum);
  m_tree->Branch("deltaPhi", "std::vector<double>",  &m_deltaPhi);
  m_tree->Branch("phiCluster", "std::vector<double>",  &m_phiCluster);
  m_tree->Branch("errorPhi", "std::vector<double>",  &m_errorPhi);
  m_tree->Branch("deltaTheta", "std::vector<double>",  &m_deltaTheta);
  m_tree->Branch("thetaCluster", "std::vector<double>",  &m_thetaCluster);
  m_tree->Branch("errorTheta", "std::vector<double>",  &m_errorTheta);
  m_tree->Branch("phi_consistency", "std::vector<double>",  &m_phi_consistency);
  m_tree->Branch("theta_consistency", "std::vector<double>",  &m_theta_consistency);
  m_tree->Branch("quality", "std::vector<double>",  &m_quality);
  m_tree->Branch("quality_best", "std::vector<double>",  &m_quality_best);
  m_tree->Branch("hitStatus_best", "std::vector<int>",  &m_hitstatus_best);

  B2INFO("[ECLTrackClusterMatchingModule]: Initialization of ECLTrackClusterMatching Module completed.");
}

void ECLTrackClusterMatchingModule::beginRun()
{
}

void ECLTrackClusterMatchingModule::event()
{
  m_deltaPhi->clear();
  m_phiCluster->clear();
  m_errorPhi->clear();
  m_deltaTheta->clear();
  m_thetaCluster->clear();
  m_errorTheta->clear();
  m_phi_consistency->clear();
  m_theta_consistency->clear();
  m_quality->clear();
  m_quality_best->clear();
  m_trackNo->clear();
  m_trackMomentum->clear();
  m_hitstatus_best->clear();

  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_iExperiment = eventmetadata->getExperiment();
    m_iRun = eventmetadata->getRun();
    m_iEvent = eventmetadata->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }

  StoreArray<Track> tracks;
  StoreArray<ECLCluster> eclClusters;

  int i = 0;

  for (const Track& track : tracks) {

    ECLCluster* cluster_best = nullptr;
    double quality_best = 0;
    double momentum = track.getTrackFitResult(Const::pion)->getMomentum().Mag();
    // m_trackNo = i;
    // m_trackMomentum = momentum;
    ExtHitStatus hitStatus = EXT_FIRST;
    i++;
    // Find extrapolated track hits in the ECL, considering only hit points
    // that either are on the sphere, closest to or on radial direction of an
    // ECLCluster.
    for (const auto& extHit : track.getRelationsTo<ExtHit>()) {
      if (!isECLHit(extHit)) continue;
      ECLCluster* eclCluster = extHit.getRelatedFrom<ECLCluster>();
      if (eclCluster != nullptr) {
        if (eclCluster->getHypothesisId() != 5) continue;
        // m_errorPhi = extHit.getErrorPhi();
        // m_errorTheta = extHit.getErrorTheta();
        m_errorPhi->push_back(extHit.getErrorPhi());
        m_errorTheta->push_back(extHit.getErrorTheta());
        // double errorPhi = extHit.getErrorPhi();
        // if (errorPhi > 2 * M_PI) continue;
        // m_errorPhi = errorPhi);
        // double errorTheta = extHit.getErrorTheta();
        // if (errorTheta > M_PI) continue;
        // m_errorTheta = errorTheta);
        double deltaPhi = acos(cos(extHit.getPosition().Phi() - eclCluster->getPhi()));
        // m_deltaPhi = deltaPhi;
        // m_phiCluster = eclCluster->getPhi();
        m_deltaPhi->push_back(deltaPhi);
        m_phiCluster->push_back(eclCluster->getPhi());
        double deltaTheta = extHit.getPosition().Theta() - eclCluster->getTheta();
        // m_deltaTheta = deltaTheta;
        // m_thetaCluster = eclCluster->getTheta();
        m_deltaTheta->push_back(deltaTheta);
        m_thetaCluster->push_back(eclCluster->getTheta());
        double quality = clusterQuality(deltaPhi, deltaTheta, momentum);
        // m_quality = quality;
        m_quality->push_back(quality);
        m_phi_consistency->push_back(phiConsistency(deltaPhi, momentum));
        m_theta_consistency->push_back(thetaConsistency(deltaTheta, momentum));
        if (quality > quality_best) {
          quality_best = quality;
          cluster_best = eclCluster;
          hitStatus = extHit.getStatus();
        }
        // m_tree->Fill();
      }
    } // end loop on ExtHits related to Track
    // m_quality_best = quality_best;
    // m_hitstatus_best = hitStatus;
    m_trackNo->push_back(i);
    m_trackMomentum->push_back(momentum);
    m_quality_best->push_back(quality_best);
    m_hitstatus_best->push_back(hitStatus);
    if (cluster_best != nullptr && quality_best > 1e-8) {
      cluster_best->setIsTrack(true);
      track.addRelationTo(cluster_best);
    }
  } // end loop on Tracks
  m_tree->Fill();
}

void ECLTrackClusterMatchingModule::endRun()
{
}

void ECLTrackClusterMatchingModule::terminate()
{
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd();
    m_tree->Write();
    m_rootFilePtr->Close();
  }
}

bool ECLTrackClusterMatchingModule::isECLHit(const ExtHit& extHit) const
{
  if ((extHit.getDetectorID() != Const::EDetector::ECL)) return false;
  ExtHitStatus extHitStatus = extHit.getStatus();
  if (extHitStatus == EXT_ECLCROSS || extHitStatus == EXT_ECLDL || extHitStatus == EXT_ECLNEAR) return true;
  else return false;
}

double ECLTrackClusterMatchingModule::clusterQuality(double deltaPhi, double deltaTheta, double momentum) const
{
  double phi_RMS = 0.0094;
  double theta_RMS = 0.0094 + 0.0377 * exp(-4.5 * momentum);
  double phi_consistency = erfc(abs(deltaPhi) / phi_RMS);
  double theta_consistency = erfc(abs(deltaTheta) / theta_RMS);
  return phi_consistency * theta_consistency * (1 - log(phi_consistency * theta_consistency));
}

double ECLTrackClusterMatchingModule::phiConsistency(double deltaPhi, double momentum) const
{
  double phi_RMS = 0.010595 + 0.0000114 * exp(9.55 - 4.05 * momentum);
  return erfc(abs(deltaPhi) / phi_RMS);
}

double ECLTrackClusterMatchingModule::thetaConsistency(double deltaTheta, double momentum) const
{
  double theta_RMS = 0.0094 + 0.0377 * exp(-4.5 * momentum);
  return erfc(abs(deltaTheta) / theta_RMS);
}