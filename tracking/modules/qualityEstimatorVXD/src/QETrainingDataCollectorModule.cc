/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jonas Wagner                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/qualityEstimatorVXD/QETrainingDataCollectorModule.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRiemannHelixFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorMC.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorCircleFit.h>
#include <tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorRandom.h>
#include <geometry/bfieldmap/BFieldMap.h>

using namespace Belle2;


REG_MODULE(QETrainingDataCollector)

QETrainingDataCollectorModule::QETrainingDataCollectorModule() : Module()
{
  //Set module properties
  setDescription("Module to collect training data for a specified qualityEstimator and store it in a root file.");
  setPropertyFlags(c_ParallelProcessingCertified | c_TerminateInAllProcesses);

  addParam("EstimationMethod", m_EstimationMethod,
           "Identifier which estimation method to use. Valid identifiers are: [circleFit, tripletFit, helixFit]", std::string(""));
  addParam("SpacePointTrackCandsStoreArrayName", m_SpacePointTrackCandsStoreArrayName,
           "Name of StoreArray containing the SpacePointTrackCandidates to be estimated.", std::string(""));

  addParam("MCRecoTracksStoreArrayName", m_MCRecoTracksStoreArrayName,
           "Name of StoreArray containing MCRecoTracks. Only required for MCInfo method", std::string("MCRecoTracks"));

  addParam("MCStrictQualityEstimator", m_MCStrictQualityEstimator,
           "Only required for MCInfo method. If false combining several MCTracks is allowed.", bool(true));

  addParam("TrainingDataOutputName", m_TrainingDataOutputName, "Name of the output rootfile.", std::string("QETrainingOutput.root"));

  addParam("ClusterInformation", m_ClusterInformation, "Wether to use cluster infos or not", std::string(""));
}

void QETrainingDataCollectorModule::initialize()
{
  m_spacePointTrackCands.isRequired(m_SpacePointTrackCandsStoreArrayName);

  m_qeResultsExtractor = std::make_unique<QEResultsExtractor>(m_EstimationMethod, m_variableSet);

  m_variableSet.emplace_back("NSpacePoints", &m_nSpacePoints);

  m_variableSet.emplace_back("truth", &m_truth);

  if (m_ClusterInformation == "Average") {
    m_clusterInfoExtractor = std::make_unique<ClusterInfoExtractor>(m_variableSet);
  }

  m_recorder = std::make_unique<SimpleVariableRecorder>(m_variableSet, m_TrainingDataOutputName, "tree");

  // create pointer to chosen estimator
  if (m_EstimationMethod == "tripletFit") {
    m_estimator = std::make_unique<QualityEstimatorTripletFit>();
  } else if (m_EstimationMethod == "circleFit") {
    m_estimator = std::make_unique<QualityEstimatorCircleFit>();
  } else if (m_EstimationMethod == "helixFit") {
    m_estimator = std::make_unique<QualityEstimatorRiemannHelixFit>();
  }
  B2ASSERT("Not all QualityEstimators could be initialized!", m_estimator);
}

void QETrainingDataCollectorModule::beginRun()
{
  // BField is required by all QualityEstimators
  double bFieldZ = BFieldMap::Instance().getBField(TVector3(0, 0, 0)).Z();
  m_estimator->setMagneticFieldStrength(bFieldZ);

  m_estimatorMC = std::make_unique<QualityEstimatorMC>(m_MCRecoTracksStoreArrayName, m_MCStrictQualityEstimator);
  m_estimatorMC->setMagneticFieldStrength(bFieldZ);
}

void QETrainingDataCollectorModule::event()
{
  for (SpacePointTrackCand& aTC : m_spacePointTrackCands) {

    if (not aTC.hasRefereeStatus(SpacePointTrackCand::c_isActive)) {
      continue;
    }

    std::vector<SpacePoint const*> const sortedHits = aTC.getSortedHits();

    if (m_ClusterInformation == "Average") {
      m_clusterInfoExtractor->extractVariables(sortedHits);
    }

    m_nSpacePoints = sortedHits.size();

    // TODO: What is the point of this? Why not bool?
    double tmp = m_estimatorMC->estimateQuality(sortedHits);
    m_truth = tmp > 0 ? 1 : 0;

    m_qeResultsExtractor->extractVariables(m_estimator->estimateQualityAndProperties(sortedHits));

    // record variables
    m_recorder->record();
  }
}

void QETrainingDataCollectorModule::terminate()
{
  m_recorder->write();
  m_recorder.reset();
}
