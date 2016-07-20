/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jo-Frederik Krohn                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <reconstruction/modules/KlId/ECLBKGExpert/ECLBKGExpertModule.h>
#include <reconstruction/dataobjects/KlId.h>
#include <framework/datastore/StoreArray.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/KLMCluster.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <tracking/dataobjects/TrackClusterSeparation.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/Exception.h>
#include <cstring>

#include <boost/algorithm/string/predicate.hpp>

#include <mva/interface/Interface.h>

#include "reconstruction/modules/KlId/KLMExpert/helperFunctions.h"

using namespace KlIdHelpers;
using namespace Belle2;
using namespace std;

REG_MODULE(ECLBKGExpert);

ECLBKGExpertModule::ECLBKGExpertModule(): Module()
{
  setDescription("Use to calculate KlId for each ECLCluster.");
  setPropertyFlags(c_ParallelProcessingCertified);
}



ECLBKGExpertModule::~ECLBKGExpertModule()
{
}


// --------------------------------------Module----------------------------------------------
void ECLBKGExpertModule::initialize()
{
  StoreArray<ECLCluster>::required();

  StoreArray<KlId>::required();

  StoreArray<KlId> klids;

  //run KLMTMVA Expert first

  m_feature_variables.resize(m_nVars);


  if (not(boost::ends_with(m_identifier, ".root") or boost::ends_with(m_identifier, ".xml"))) {
    m_weightfile_representation = std::unique_ptr<DBObjPtr<DatabaseRepresentationOfWeightfile>>(new
                                  DBObjPtr<DatabaseRepresentationOfWeightfile>(m_identifier));
  }
  MVA::AbstractInterface::initSupportedInterfaces();

}


void ECLBKGExpertModule::beginRun()
{
  if (m_weightfile_representation) {
    if (m_weightfile_representation->hasChanged()) {
      std::stringstream ss((*m_weightfile_representation)->m_data);
      auto weightfile = MVA::Weightfile::loadFromStream(ss);
      init_mva(weightfile);
    }
  } else {
    auto weightfile = MVA::Weightfile::loadFromFile(m_identifier);
    init_mva(weightfile);
  }
}

void ECLBKGExpertModule::init_mva(MVA::Weightfile& weightfile)
{
  auto supported_interfaces = MVA::AbstractInterface::getSupportedInterfaces();
  MVA::GeneralOptions general_options;
  weightfile.getOptions(general_options);

  // Overwrite signal fraction from training
  if (m_signal_fraction_override > 0)
    weightfile.addSignalFraction(m_signal_fraction_override);

  m_expert = supported_interfaces[general_options.m_method]->getExpert();
  m_expert->load(weightfile);

  std::vector<float> dummy;
  dummy.resize(m_feature_variables.size(), 0);
  m_dataset = std::unique_ptr<MVA::SingleDataset>(new MVA::SingleDataset(general_options, dummy, 0));
}




void ECLBKGExpertModule::event()
{

  StoreArray<RecoTrack> genfitTracks;
  StoreArray<ECLCluster> eclClusters;
  StoreArray<KlId> klids;


  //overwritten at the end of the cluster loop
  double MVAOut;
  KlId* klid = nullptr;

  // loop thru clusters in event and classify
  for (ECLCluster& cluster : eclClusters) {

    // get various ECLCluster vars from getters
    m_ECLE              = cluster.getEnergy();
    m_ECLE9oE25         = cluster.getE9oE25();
    m_ECLTiming         = cluster.getTiming();
    m_ECLEerror         = cluster.getErrorEnergy();
    m_ECLminTrkDistance = cluster.getTemporaryMinTrkDistance();
    m_ECLdeltaL         = cluster.getTemporaryDeltaL();

    const TVector3& clusterPos = cluster.getclusterPosition();

    //find closest track
    tuple<RecoTrack*, double, const TVector3*> closestTrackAndDistance = findClosestTrack(clusterPos);
    m_ECLtrackDist = get<1>(closestTrackAndDistance);


    m_feature_variables[0] = m_ECLE;
    m_feature_variables[1] = m_ECLE9oE25;
    m_feature_variables[2] = m_ECLTiming;
    m_feature_variables[3] = m_ECLEerror;
    m_feature_variables[4] = m_ECLtrackDist;
    m_feature_variables[5] = m_ECLdeltaL;
    m_feature_variables[6] = m_ECLminTrkDistance;


    // rewrite dataset
    for (unsigned int i = 0; i < m_feature_variables.size(); ++i) {
      m_dataset->m_input[i] = m_feature_variables[i];
    }

    //classify dartaset
    MVAOut = m_expert->apply(*m_dataset)[0];

    // KlId, bkg prob, KLM, ECL
    klid = cluster.getRelatedTo<KlId>();
    klid->setBkgProb(MVAOut);
    cluster.addRelationTo(klid);

  }// for cluster in clusters
} // event











