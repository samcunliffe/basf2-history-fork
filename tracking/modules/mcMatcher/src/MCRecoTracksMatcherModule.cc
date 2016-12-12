/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/mcMatcher/MCRecoTracksMatcherModule.h>

#include <framework/dataobjects/EventMetaData.h>

//datastore types
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>

#include <framework/gearbox/Const.h>

#include <tracking/dataobjects/RecoTrack.h>
#include <mdst/dataobjects/MCParticle.h>

//hit types
#include <pxd/dataobjects/PXDTrueHit.h>
#include <pxd/dataobjects/PXDCluster.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCSimHit.h>

#include <time.h>
#include <map>

#include <Eigen/Dense>

using namespace std;
using namespace Belle2;

REG_MODULE(MCRecoTracksMatcher);

namespace {
  //small anonymous helper construct making converting a pair of iterators usable
  //with range based for
  template<class Iter>
  struct iter_pair_range : std::pair<Iter, Iter> {
    iter_pair_range(std::pair<Iter, Iter> const& x) : std::pair<Iter, Iter>(x) {}
    Iter begin() const {return this->first;}
    Iter end()   const {return this->second;}
  };

  template<class Iter>
  inline iter_pair_range<Iter> as_range(std::pair<Iter, Iter> const& x)
  {
    return iter_pair_range<Iter>(x);
  }

  typedef int DetId;
  typedef int HitId;
  typedef int TrackCandId;

  typedef std::pair<DetId, HitId> DetHitIdPair;

  struct CompDetHitIdPair {

    bool operator()(const std::pair<DetId, HitId>& lhs,
                    const std::pair<std::pair<DetId, HitId>, TrackCandId>& rhs)
    { return lhs < rhs.first; }

    bool operator()(const std::pair<std::pair<DetId, HitId>, TrackCandId>& lhs,
                    const std::pair<DetId, HitId>& rhs)
    { return lhs.first < rhs; }
  };

  // anonymous helper function to fill a set or a map with the hit IDs and det IDs (we need both a set or a map in the following).
  template <class AMapOrSet>
  void fillIDsFromStoreArray(AMapOrSet& trackCandID_by_hitID,
                             const StoreArray<RecoTrack>& storedRecoTracks)
  {
    TrackCandId recoTrackId = -1;
    typename AMapOrSet::iterator itInsertHint = trackCandID_by_hitID.end();

    for (const RecoTrack& recoTrack : storedRecoTracks) {
      ++recoTrackId;

      for (const RecoHitInformation::UsedCDCHit* cdcHit : recoTrack.getCDCHitList()) {
        itInsertHint =
          trackCandID_by_hitID.insert(itInsertHint,
                                      make_pair(pair<DetId, HitId>(Const::CDC, cdcHit->getArrayIndex()),
                                                recoTrackId));
      }
      for (const RecoHitInformation::UsedSVDHit* svdHit : recoTrack.getSVDHitList()) {
        itInsertHint =
          trackCandID_by_hitID.insert(itInsertHint,
                                      make_pair(pair<DetId, HitId>(Const::SVD, svdHit->getArrayIndex()),
                                                recoTrackId));
      }
      for (const RecoHitInformation::UsedPXDHit* pxdHit : recoTrack.getPXDHitList()) {
        itInsertHint =
          trackCandID_by_hitID.insert(itInsertHint,
                                      make_pair(pair<DetId, HitId>(Const::PXD, pxdHit->getArrayIndex()),
                                                recoTrackId));
      }
    }
  }
}

MCRecoTracksMatcherModule::MCRecoTracksMatcherModule() : Module()
{
  setDescription("This module compares track candidates generated by some pattern recognition algorithm for PXD, SVD and/or CDC "
                 "to ideal Monte Carlo tracks and performs a matching from the former to the underlying MCParticles.");

  setPropertyFlags(c_ParallelProcessingCertified);

  //Parameter definition
  // Inputs
  addParam("prRecoTracksStoreArrayName",
           m_param_prRecoTracksStoreArrayName,
           "Name of the collection containing the tracks as generate a patter recognition algorithm to be evaluated ",
           string(""));

  addParam("mcRecoTracksStoreArrayName",
           m_param_mcRecoTracksStoreArrayName,
           "Name of the collection containing the reference tracks as generate by a Monte-Carlo-Tracker (e.g. MCTrackFinder)",
           string("MCGFTrackCands"));

  // Hit content to be evaluated
  //  Choose which hits to use, all hits assigned to the track candidate will be used in the fit
  addParam("UsePXDHits",
           m_param_usePXDHits,
           "Set true if PXDHits or PXDClusters should be used in the matching in case they are present",
           true);

  addParam("UseSVDHits",
           m_param_useSVDHits,
           "Set true if SVDHits or SVDClusters should be used in the matching in case they are present",
           true);

  addParam("UseCDCHits",
           m_param_useCDCHits,
           "Set true if CDCHits should be used in the matching in case they are present",
           true);

  addParam("UseOnlyAxialCDCHits",
           m_param_useOnlyAxialCDCHits,
           "Set true if only the axial CDCHits should be used",
           false);

  addParam("AbandonDiscardedCDCHits",
           m_param_abandonDiscardedCDCHits,
           "Set true if discarded CDC hits (in MCTrack) are not taken into account",
           true);

  addParam("MinimalPurity",
           m_param_minimalPurity,
           "Minimal purity of a PRTrack to be considered matchable to a MCTrack. "
           "This number encodes how many correct hits are minimally need to compensate for a false hits. "
           "The default 2.0/3.0 suggests that for each background hit can be compensated by two correct hits.",
           2.0 / 3.0);

  addParam("MinimalEfficiency",
           m_param_minimalEfficiency,
           "Minimal efficiency of a MCTrack to be considered matchable to a PRTrack. "
           "This number encodes which fraction of the true hits must at least be in the reconstructed track. "
           "The default 0.05 suggests that at least 20% of the true hits should have been picked up.",
           0.05);

  // PRTracks purity minimal constrains
  /*addParam("MinimalExcessNDF",
     m_param_minimalExcessNdf,
     "Minimum number of correct of degrees of freedom in the PRtrack. 2D hits count as 2. "
     "The number can be given to insure that a minimum of correct degress of freedom "
     "is in the track for the fit as well as the rest of the correct hits is able to "
     "compensate for false hits. Essentially this number is subtracted from the number "
     "of signal hits before calculating the purity.",
     5);*/

  addParam("RelateClonesToMCParticles",
           m_param_relateClonesToMCParticles,
           "Indicates whether the mc matching to MCParticles should also govern clone tracks",
           bool(true));
}

void MCRecoTracksMatcherModule::initialize()
{
  // Check if there are MC Particles
  StoreArray<MCParticle> storeMCParticles;

  if (storeMCParticles.isOptional()) {
    m_mcParticlesPresent = true;

    // Actually retrieve the StoreArrays
    StoreArray<RecoTrack> storePRRecoTracks(m_param_prRecoTracksStoreArrayName);
    StoreArray<RecoTrack> storeMCRecoTracks(m_param_mcRecoTracksStoreArrayName);

    // Require both GFTrackCand arrays and the MCParticles to be present in the DataStore
    storePRRecoTracks.isRequired();
    storeMCRecoTracks.isRequired();
    storeMCParticles.isRequired();

    // Extract the default names for the case empty stings were given
    m_param_prRecoTracksStoreArrayName = storePRRecoTracks.getName();
    m_param_mcRecoTracksStoreArrayName = storeMCRecoTracks.getName();

    // Purity relation - for each PRTrack to store the purest MCTrack
    storeMCRecoTracks.registerRelationTo(storePRRecoTracks);
    // Efficiency relation - for each MCTrack to store the most efficient PRTrack
    storePRRecoTracks.registerRelationTo(storeMCRecoTracks);
    // MC matching relation
    storePRRecoTracks.registerRelationTo(storeMCParticles);

    // Announce optional store arrays to the hits or clusters in case they should be used
    // We make them optional in case of limited detector setup.
    // PXD
    if (m_param_usePXDHits) {
      StoreArray<PXDCluster>::optional();
    }

    // SVD
    if (m_param_useSVDHits) {
      StoreArray<SVDCluster>::optional();
    }

    // CDC
    if (m_param_useCDCHits) {
      StoreArray<CDCHit>::optional();
    }
  }
}



void MCRecoTracksMatcherModule::event()
{
  // Skip in the case there are no MC particles present.
  if (not m_mcParticlesPresent) {
    B2DEBUG(100, "Skipping MC Track Matcher as there are no MC Particles registered in the DataStore.");
    return;
  }

  B2DEBUG(100, "########## MCRecoTracksMatcherModule ############");

  //Fetch store array
  StoreArray<RecoTrack> prRecoTracks(m_param_prRecoTracksStoreArrayName);
  StoreArray<RecoTrack> mcRecoTracks(m_param_mcRecoTracksStoreArrayName);
  StoreArray<MCParticle> mcParticles("");

  int nMCRecoTracks = mcRecoTracks.getEntries();
  int nPRRecoTracks = prRecoTracks.getEntries();

  B2DEBUG(100, "Number patter recognition tracks is " << nPRRecoTracks);
  B2DEBUG(100, "Number Monte-Carlo tracks is " << nMCRecoTracks);

  if (not nMCRecoTracks or not nPRRecoTracks) {
    // Neither no pattern recognition tracks
    // or no Monte Carlo tracks are present in this event
    // Cannot perform matching.
    return;
  }

  //### Build a detector_id hit_id to trackcand map for easier lookup later ###
  multimap< pair< DetId, HitId>, TrackCandId > mcRecoTrackId_by_hitId;
  fillIDsFromStoreArray(mcRecoTrackId_by_hitId, mcRecoTracks);

  //  Use set instead of multimap to handel to following situation
  //  * One hit may be assigned to multiple tracks should contribute to the efficiency of both tracks
  //  * One hit assigned twice or more to the same track should not contribute to the purity multiple times
  //  The first part is handled well by the multimap. But to enforce that one hit is also only assigned
  //  once to a track we use a set.
  set< pair< pair< DetId, HitId >, TrackCandId> > prRecoTrackId_by_hitId;
  fillIDsFromStoreArray(prRecoTrackId_by_hitId, prRecoTracks);

  // ### Get the number of relevant hits for each detector ###
  // Since we are mostly dealing with indices in this module, this is all we need from the StoreArray
  // Silently skip store arrays that are not present in reduced detector setups.
  map< DetId, int>  nHits_by_detId;

  // PXD
  if (m_param_usePXDHits) {
    StoreArray<PXDCluster> pxdClusters;
    if (pxdClusters.isOptional()) {
      int nHits = pxdClusters.getEntries();
      nHits_by_detId[Const::PXD] = nHits;
    }
  }

  // SVD
  if (m_param_useSVDHits) {
    StoreArray<SVDCluster> svdClusters;
    if (svdClusters.isOptional()) {
      int nHits = svdClusters.getEntries();
      nHits_by_detId[Const::SVD] = nHits;
    }
  }

  // CDC
  if (m_param_useCDCHits) {
    StoreArray<CDCHit> cdcHits;
    if (cdcHits.isOptional()) {
      nHits_by_detId[Const::CDC] = cdcHits.getEntries();
    }
  }

  //### Build the confusion matrix ###

  // Reserve enough space for the confusion matrix
  // The last row is meant for hits not assigned to a mcRecoTrack (aka background hits)
  Eigen::MatrixXi confusionMatrix = Eigen::MatrixXi::Zero(nPRRecoTracks, nMCRecoTracks + 1);

  // Accumulated the total number of hits/ndf for each Monte-Carlo track seperatly to avoid double counting,
  // in case patter recognition tracks share hits.
  Eigen::RowVectorXi totalNDF_by_mcRecoTrackId = Eigen::RowVectorXi::Zero(nMCRecoTracks + 1);

  // Accumulated the total number of hits/ndf for each pattern recognition track seperatly to avoid double counting,
  // in case Monte-Carlo tracks share hits.
  Eigen::VectorXi totalNDF_by_prRecoTrackId = Eigen::VectorXi::Zero(nPRRecoTracks + 1);

  // Column index for the hits not assigned to any MCTrackCand
  const int mcBkgId = nMCRecoTracks;

  // for each detector examine every hit to which mcRecoTrack and prRecoTrack it belongs
  // if the hit is not part of any mcRecoTrack put the hit in the background column.
  for (const pair<const DetId, NDF>& detId_nHits_pair : nHits_by_detId) {

    const DetId& detId = detId_nHits_pair.first;
    const int& nHits = detId_nHits_pair.second;
    const NDF& ndfForOneHit = m_ndf_by_detId[detId];

    for (HitId hitId = 0; hitId < nHits; ++hitId) {
      pair<DetId, HitId> detId_hitId_pair(detId, hitId);

      if (m_param_useOnlyAxialCDCHits and detId == Const::CDC) {
        StoreArray<CDCHit> cdcHits;
        const CDCHit* cdcHit = cdcHits[hitId];
        if (cdcHit->getISuperLayer() % 2 != 0) {
          // Skip stereo hits
          continue;
        }
      }

      // First search the unique mcRecoTrackId for the hit.
      // If the hit is not assigned to any mcRecoTrack to Id is set iMC to the background column.
      auto range_mcRecoTrackIds_for_detId_hitId_pair = mcRecoTrackId_by_hitId.equal_range(detId_hitId_pair);

      if (range_mcRecoTrackIds_for_detId_hitId_pair.first == range_mcRecoTrackIds_for_detId_hitId_pair.second) {
        if (m_param_abandonDiscardedCDCHits and detId == Const::CDC) {
          StoreArray<CDCHit> cdcHits;
          const CDCHit* cdcHit = cdcHits[hitId];
          // check if the hit was created by BG or not. in case it is not a BG hit, but related track refers to 'mcBkgId' (background track) - skip this hit.
          const CDCSimHit* cdcSimHit = cdcHit->getRelated<CDCSimHit>();
          if (cdcSimHit) {
            if (cdcSimHit->getBackgroundTag() == CDCSimHit::bg_none) {
              continue;
            }
          }
        }
        TrackCandId mcRecoTrackId = mcBkgId;
        totalNDF_by_mcRecoTrackId(mcRecoTrackId) += ndfForOneHit;
      } else {
        for (std::pair<DetHitIdPair, TrackCandId> detId_hitId_pair_and_mcRecoTrackId :
             as_range(range_mcRecoTrackIds_for_detId_hitId_pair)) {
          TrackCandId mcRecoTrackId = detId_hitId_pair_and_mcRecoTrackId.second;
          // Assign the hits/ndf to the total ndf vector seperatly here to avoid double counting, if patter recognition track share hits.
          totalNDF_by_mcRecoTrackId(mcRecoTrackId) += ndfForOneHit;
        }
      }

      // Seek all prRecoTracks
      //  use as range adapter to convert the iterator pair form equal_range to a range base iterable struct
      auto range_prRecoTrackIds_for_detId_hitId_pair = equal_range(prRecoTrackId_by_hitId.begin(),
                                                       prRecoTrackId_by_hitId.end(),
                                                       detId_hitId_pair, CompDetHitIdPair());

      // add the degrees of freedom to every prRecoTrack that has this hit
      for (const pair<pair<DetId, HitId>, TrackCandId>& detId_hitId_pair_and_prRecoTrackId :
           as_range(range_prRecoTrackIds_for_detId_hitId_pair)) {

        TrackCandId prRecoTrackId = detId_hitId_pair_and_prRecoTrackId.second;
        // Assign the hits/ndf to the total ndf vector seperatly here to avoid double counting, if Monte-Carlo track share hits.
        totalNDF_by_prRecoTrackId(prRecoTrackId) += ndfForOneHit;

        for (std::pair<DetHitIdPair, TrackCandId> detId_hitId_pair_and_mcRecoTrackId :
             as_range(range_mcRecoTrackIds_for_detId_hitId_pair)) {
          TrackCandId mcRecoTrackId = detId_hitId_pair_and_mcRecoTrackId.second;

          B2DEBUG(200, " prRecoTrackId : " <<  prRecoTrackId  << ";  mcRecoTrackId : " <<  mcRecoTrackId);
          confusionMatrix(prRecoTrackId, mcRecoTrackId) += ndfForOneHit;
        }
      } //end for
    } //end for hitId
  } // end for decId

  B2DEBUG(200, "Confusion matrix of the event : " << endl <<  confusionMatrix);

  B2DEBUG(200, "totalNDF_by_prRecoTrackId : " << endl << totalNDF_by_prRecoTrackId);
  B2DEBUG(200, "totalNDF_by_mcRecoTrackId : " << endl << totalNDF_by_mcRecoTrackId);










  // ### Building the patter recognition track to highest purity Monte-Carlo track relation ###
  typedef float Purity;

  vector< pair< TrackCandId, Purity> > purestMCTrackCandId_by_prRecoTrackId(nPRRecoTracks);

  for (TrackCandId prRecoTrackId = 0; prRecoTrackId < nPRRecoTracks; ++prRecoTrackId) {

    NDF totalNDF_for_prRecoTrackId = totalNDF_by_prRecoTrackId(prRecoTrackId);

    Eigen::RowVectorXi prTrackRow = confusionMatrix.row(prRecoTrackId);

    Eigen::RowVectorXi::Index purestMCTrackCandId_for_prRecoTrackId;

    //Also sets the index of the highest entry in the row vector
    NDF highestNDF_for_prRecoTrackId = prTrackRow.maxCoeff(&purestMCTrackCandId_for_prRecoTrackId);

    Purity highestPurity = Purity(highestNDF_for_prRecoTrackId) / totalNDF_for_prRecoTrackId;

    purestMCTrackCandId_by_prRecoTrackId[prRecoTrackId] = pair< TrackCandId, Purity>(purestMCTrackCandId_for_prRecoTrackId,
                                                          highestPurity);
  }

  // Log the patter recognition track to highest purity Monte-Carlo track relation to debug output
  {
    TrackCandId prRecoTrackId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const pair< TrackCandId, Purity>& purestMCTrackCandId_for_prRecoTrackId : purestMCTrackCandId_by_prRecoTrackId) {
      ++prRecoTrackId;

      const Purity& purity = purestMCTrackCandId_for_prRecoTrackId.second;
      const TrackCandId& mcRecoTrackId = purestMCTrackCandId_for_prRecoTrackId.first;
      B2DEBUG(200, "prRecoTrackId : " << prRecoTrackId << " ->  mcRecoTrackId : " << mcRecoTrackId << " with purity " << purity);
    }
  }










  // ### Building the Monte-Carlo track to highest efficiency patter recognition track relation ###
  typedef float Efficiency;
  vector< pair< TrackCandId,  Efficiency> > mostEfficientPRTrackCandId_by_mcRecoTrackId(nMCRecoTracks);

  for (TrackCandId mcRecoTrackId = 0; mcRecoTrackId < nMCRecoTracks; ++mcRecoTrackId) {

    NDF totalNDF_for_mcRecoTrackId = totalNDF_by_mcRecoTrackId(mcRecoTrackId);

    Eigen::VectorXi mcRecoTrackCol = confusionMatrix.col(mcRecoTrackId);

    Eigen::VectorXi::Index highestNDFPRTrackCandId_for_mcRecoTrackId;

    //Also sets the index of the highest entry in the column vector
    NDF highestNDF_for_mcRecoTrackId = mcRecoTrackCol.maxCoeff(&highestNDFPRTrackCandId_for_mcRecoTrackId);

    Efficiency highestEfficiency = Purity(highestNDF_for_mcRecoTrackId) / totalNDF_for_mcRecoTrackId;

    mostEfficientPRTrackCandId_by_mcRecoTrackId[mcRecoTrackId] = pair< TrackCandId, Efficiency>
        (highestNDFPRTrackCandId_for_mcRecoTrackId, highestEfficiency);

  }

  // Log the  Monte-Carlo track to highest efficiency patter recognition track relation
  {
    TrackCandId mcRecoTrackId = -1;
    B2DEBUG(200, "PRTrack to highest purity MCTrack relation");
    for (const pair< TrackCandId, Efficiency>& mostEfficientPRTrackCandId_for_mcRecoTrackId :
         mostEfficientPRTrackCandId_by_mcRecoTrackId) {
      ++mcRecoTrackId;

      const Efficiency& highestEfficiency = mostEfficientPRTrackCandId_for_mcRecoTrackId.second;
      const TrackCandId& prRecoTrackId = mostEfficientPRTrackCandId_for_mcRecoTrackId.first;
      B2DEBUG(200, "mcRecoTrackId : " << mcRecoTrackId << " ->  prRecoTrackId : " << prRecoTrackId << " with efficiency " <<
              highestEfficiency);

    }
  }

  // ### Classify the patter recognition tracks ###
  // Means saving the highest purity relation to the data store
  // + setup the PRTrack to MCParticle relation
  // + save the McTrackId property
  for (TrackCandId prRecoTrackId = 0; prRecoTrackId < nPRRecoTracks; ++prRecoTrackId) {
    RecoTrack* prRecoTrack = prRecoTracks[prRecoTrackId];

    const pair<TrackCandId, Purity>& purestMCTrackCandId = purestMCTrackCandId_by_prRecoTrackId[prRecoTrackId];

    const TrackCandId& mcRecoTrackId = purestMCTrackCandId.first;
    const Purity& purity = purestMCTrackCandId.second;

    if (not(purity > 0) or not(purity >= m_param_minimalPurity)) {
      // GHOST
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_ghost);
      B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as ghost.");

    } else if (mcRecoTrackId == mcBkgId) {
      // BACKGROUND
      prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_background);
      B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as background because of too low purity.");
    } else {

      // after the classification for bad purity and background we examine, whether
      // the highest purity Monte-Carlo track has in turn the highest efficiency patter recognition track
      // equal to this track.
      // All extra patter recognition tracks are stored with negativelly signed purity

      RecoTrack* mcRecoTrack = mcRecoTracks[mcRecoTrackId];

      const pair<TrackCandId, Efficiency>& mostEfficientPRTrackCandId_for_mcRecoTrackId =
        mostEfficientPRTrackCandId_by_mcRecoTrackId[mcRecoTrackId];

      const TrackCandId& mostEfficientPRTrackCandId = mostEfficientPRTrackCandId_for_mcRecoTrackId.first;
      const Efficiency& efficiency = mostEfficientPRTrackCandId_for_mcRecoTrackId.second;

      if (prRecoTrackId != mostEfficientPRTrackCandId) {
        if (efficiency >= m_param_minimalEfficiency) {

          // CLONE
          if (m_param_relateClonesToMCParticles) {

            // Set the McTrackId to the related MCParticle
            prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_matched);
            B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as matched, although it is a clone (because the module parameter is set).");

            // Add the mc matching relation
            MCParticle* mcParticle = mcRecoTrack->getRelated<MCParticle>();
            B2ASSERT("No relation from MCRecoTrack to MCParticle.", mcParticle);
            prRecoTrack->addRelationTo(mcParticle, -purity);
            B2DEBUG(100, "MC Match prId " << prRecoTrackId << " to mcPartId " << mcParticle->getArrayIndex());


          } else {

            prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_clone);
            B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as clone.");

          }
          //Setup the relation with negative purity for this case
          prRecoTrack->addRelationTo(mcRecoTrack, -purity);
          B2DEBUG(100, "Purity rel: prId " << prRecoTrackId << " -> mcId " << mcRecoTrackId << " : " << -purity);

        } else {
          // Pattern recognition track fails the minimal efficiency requirement to be matched
          // We might want to introduce a different classification here, if we see problems
          // with too many ghosts and want to investigate the specific source of the mismatch
          //

          // GHOST
          prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_ghost);
          B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as ghost because of too low efficiency.");

        }

      } else {
        // MATCHED

        //Set the McTrackId to the related MCParticle
        prRecoTrack->setMatchingStatus(RecoTrack::MatchingStatus::c_matched);
        B2DEBUG(100, "Stored PRTrack " << prRecoTrackId << " as matched.");

        //Add the mc matching relation
        MCParticle* mcParticle = mcRecoTrack->getRelated<MCParticle>();
        B2ASSERT("No relation from MCRecoTrack to MCParticle.", mcParticle);
        prRecoTrack->addRelationTo(mcParticle, purity);
        B2DEBUG(100, "MC Match prId " << prRecoTrackId << " to mcPartId " <<  mcParticle->getArrayIndex());

        //Setup the relation with positive purity for this case
        prRecoTrack->addRelationTo(mcRecoTrack, purity);
        B2DEBUG(100, "Purity rel: prId " << prRecoTrackId << " -> mcId " << mcRecoTrackId << " : " << purity);


      }
    }

  }










  // ### Classify the Monte-Carlo tracks ###
  // Meaning save the highest efficiency relation to the data store.
  for (TrackCandId mcRecoTrackId = 0; mcRecoTrackId < nMCRecoTracks; ++mcRecoTrackId) {
    RecoTrack* mcRecoTrack = mcRecoTracks[mcRecoTrackId];

    const pair<TrackCandId, Efficiency>& mostEfficiencyPRTrackCandId = mostEfficientPRTrackCandId_by_mcRecoTrackId[mcRecoTrackId];

    const TrackCandId& prRecoTrackId = mostEfficiencyPRTrackCandId.first;
    const Efficiency& efficiency = mostEfficiencyPRTrackCandId.second;

    if (prRecoTrackId >= nPRRecoTracks or prRecoTrackId < 0) {
      B2ERROR("Index of pattern recognition tracks out of range.");
    }

    if (prRecoTracks[prRecoTrackId]->getMatchingStatus() != RecoTrack::MatchingStatus::c_matched or
        not(efficiency > 0) or
        not(efficiency >= m_param_minimalEfficiency)) {

      // MISSING
      // No related pattern recognition track
      // Do not create a relation.
      B2DEBUG(100, "mcId " << mcRecoTrackId << " is missing. No relation created.");

    } else {

      RecoTrack* prRecoTrack = prRecoTracks[prRecoTrackId];

      const pair<TrackCandId, Purity>& purestMCTrackCandId_for_prRecoTrackId = purestMCTrackCandId_by_prRecoTrackId[prRecoTrackId];
      const TrackCandId& purestMCTrackCandId = purestMCTrackCandId_for_prRecoTrackId.first;

      if (mcRecoTrackId != purestMCTrackCandId) {
        // MERGED
        // this MCTrack is in a PRTrack which in turn better describes a MCTrack different form this.

        // Setup the relation with negative efficiency for this case.
        mcRecoTrack->addRelationTo(prRecoTrack, -efficiency);
        B2DEBUG(100, "Efficiency rel: mcId " << mcRecoTrackId << " -> prId " << prRecoTrackId << " : " << -efficiency);

      } else {
        // MATCHED

        // Setup the relation with positive efficiency for this case.
        mcRecoTrack->addRelationTo(prRecoTrack, efficiency);
        B2DEBUG(100, "Efficiency rel: mcId " << mcRecoTrackId << " -> prId " << prRecoTrackId << " : " << efficiency);

      }
    }

  }

  B2DEBUG(100, "########## End MCRecoTracksMatcherModule ############");

} //end event()
