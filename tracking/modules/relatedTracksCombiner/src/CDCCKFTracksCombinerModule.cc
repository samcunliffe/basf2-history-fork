/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/relatedTracksCombiner/CDCCKFTracksCombinerModule.h>

#include <framework/dataobjects/Helix.h>
#include <framework/geometry/BFieldManager.h>

using namespace Belle2;

REG_MODULE(CDCCKFTracksCombiner);

CDCCKFTracksCombinerModule::CDCCKFTracksCombinerModule() :
  Module()
{
  setDescription("Combine related tracks from CDC, determined in SVD->CDC CKF, and VXD (and VXD+CDC) into a single track by copying the hit "
                 "information and combining the seed information. The sign of the weight defines, "
                 "if the hits go before (-1) or after (+1) the CDC track.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("CDCRecoTracksStoreArrayName", m_cdcRecoTracksStoreArrayName , "Name of the input CDC StoreArray.",
           m_cdcRecoTracksStoreArrayName);
  addParam("VXDRecoTracksStoreArrayName", m_vxdRecoTracksStoreArrayName , "Name of the input VXD (and+CDC) StoreArray.",
           m_vxdRecoTracksStoreArrayName);
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the output StoreArray.", m_recoTracksStoreArrayName);
}

void CDCCKFTracksCombinerModule::initialize()
{
  m_vxdRecoTracks.isRequired(m_vxdRecoTracksStoreArrayName);
  m_cdcRecoTracks.isRequired(m_cdcRecoTracksStoreArrayName);

  m_recoTracks.registerInDataStore(m_recoTracksStoreArrayName, DataStore::c_ErrorIfAlreadyRegistered);
  RecoTrack::registerRequiredRelations(m_recoTracks);

  m_recoTracks.registerRelationTo(m_vxdRecoTracks);
  m_recoTracks.registerRelationTo(m_cdcRecoTracks);
}

void CDCCKFTracksCombinerModule::event()
{
  std::set <RecoTrack*> mergedTracks;
  // Loop over all CDC reco tracks and add them to the store array of they do not have a match or combined them with
  // their VXD partner if they do.
  // For this, the fitted or seed state of the tracks is used - if they are already fitted or not.
  for (RecoTrack& cdcRecoTrack : m_cdcRecoTracks) {
    const RelationVector<RecoTrack>& relatedVXDRecoTracks = cdcRecoTrack.getRelationsWith<RecoTrack>(m_vxdRecoTracksStoreArrayName);

    B2ASSERT("Can not handle more than 2 relations!", relatedVXDRecoTracks.size() <= 2);

    RecoTrack* vxdTrackBefore = nullptr;
    RecoTrack* vxdTrackAfter = nullptr;

    for (unsigned int index = 0; index < relatedVXDRecoTracks.size(); ++index) {
      const double weight = relatedVXDRecoTracks.weight(index);
      if (weight < 0) {
        vxdTrackBefore = relatedVXDRecoTracks[index];
      } else if (weight > 0) {
        vxdTrackAfter = relatedVXDRecoTracks[index];
      }
    }

    // Do not output non-fittable tracks
    if (not vxdTrackAfter and not vxdTrackBefore) {
      continue;
    }

    RecoTrack* newMergedTrack = nullptr;

    if (vxdTrackBefore) {
      mergedTracks.insert(vxdTrackBefore);
      std::cout << " save before\n";
      newMergedTrack = vxdTrackBefore->copyToStoreArray(m_recoTracks);
      newMergedTrack->addHitsFromRecoTrack(vxdTrackBefore, newMergedTrack->getNumberOfTotalHits());
      newMergedTrack->addRelationTo(vxdTrackBefore);
    } else {
      newMergedTrack = cdcRecoTrack.copyToStoreArray(m_recoTracks);
    }

    newMergedTrack->addHitsFromRecoTrack(&cdcRecoTrack, newMergedTrack->getNumberOfTotalHits());
    newMergedTrack->addRelationTo(&cdcRecoTrack);

    if (vxdTrackAfter) {
      mergedTracks.insert(vxdTrackAfter);
      std::cout << " save after\n";
      newMergedTrack->addHitsFromRecoTrack(vxdTrackAfter, newMergedTrack->getNumberOfTotalHits(), true);
      newMergedTrack->addRelationTo(vxdTrackAfter);
    }
  }

  // Now we have to add remaining tracks
  for (RecoTrack& vxdRecoTrack : m_vxdRecoTracks) {
    auto alreadyInclded = mergedTracks.count(&vxdRecoTrack) ;

    if (alreadyInclded) {
      std::cout << " already found \n";
    }

    if (not alreadyInclded) {
      RecoTrack* newTrack = vxdRecoTrack.copyToStoreArray(m_recoTracks);
      newTrack->addHitsFromRecoTrack(&vxdRecoTrack);
      newTrack->addRelationTo(&vxdRecoTrack);
    }
  }
}

