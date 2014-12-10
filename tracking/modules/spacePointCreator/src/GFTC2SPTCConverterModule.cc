/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/GFTC2SPTCConverterModule.h>
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

// #include <testbeam/vxd/dataobjects/TelCluster.h> // for completeness (not yet needed)

#include <genfit/TrackCand.h>
#include <genfit/TrackCandHit.h>

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Const.h>

#include <algorithm> // count, sort, unique, etc..
#include <limits>

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/concept_check.hpp>

// more debugging purposes, these won't be used in the final version!
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

using namespace std;
using namespace Belle2;

REG_MODULE(GFTC2SPTCConverter)

// TODO: at the moment I am redesigning the whole thing (outsourcing some of SpacePointTrackCand functionality to this module)
// this means that this here is probably getting very messy -> REFACTOR and write nicely if everything works as desired!
// I tried to actually mark stuff which is probably no longer needed during the process of redesign, but this does not guarantee anything
// TODO: throw out all unneccessary stuff (i.e. all the vector<HitInfo<SpacePoint> >) which is here at the moment for possible future debug purposes

GFTC2SPTCConverterModule::GFTC2SPTCConverterModule() :
  Module()
{
  setDescription("Module for converting genfit::TrackCands (e.g. from TrackFinderMCTruth) to SpacePointTrackCands.");

  addParam("PXDClusters", m_PXDClusterName, "PXDCluster collection name. WARNING: it is only checked if these exist, they are not actually used at the moment!", string(""));
  addParam("SVDClusters", m_SVDClusterName, "SVDCluster collection name", string(""));
  addParam("genfitTCName", m_genfitTCName, "Name of container of genfit::TrackCands", string(""));
  addParam("SpacePointTCName", m_SPTCName, "Name of the container under which SpacePoints will be stored in the DataStore", string(""));

  addParam("SingleClusterSVDSP", m_SingleClusterSVDSPName, "Single Cluster SVD SpacePoints collection name. This StoreArray will be searched for SpacePoints", string(""));
  addParam("NoSingleClusterSVDSP", m_NoSingleClusterSVDSPName, "Non Single Cluster SVD SpacePoints collection name. This StoreArray will be searched for SpacePoints", string(""));
  addParam("PXDClusterSP", m_PXDClusterSPName, "PXD Cluster SpacePoints collection name.", string(""));

  std::vector<double> defaultOrigin = {0., 0., 0.};
  addParam("setOrigin", m_PARAMsetOrigin, "Reset origin to given Point. Used for decision if track is curling or not. (e.g. testbeam: origin is not at (0,0,0))", defaultOrigin);

  addParam("splitCurlers", m_PARAMsplitCurlers, "Split curling tracks into tracklets, where each tracklet is then non-curling", false);
  addParam("NTracklets", m_NTracklets, "Maximum number of tracklets to be saved if a curling track is split up into tracklets. Set to 1 if only the first part of a curling track is needed.", 1);
}

// ------------------------------ INITIALIZE ---------------------------------------
void GFTC2SPTCConverterModule::initialize()
{
  B2INFO("GFTC2SPTCConverter -------------- initialize() ---------------------")
  // initialize Counters
  initializeCounters();

  // check if all required StoreArrays are here
  StoreArray<PXDCluster>::required(m_PXDClusterName);
  StoreArray<SVDCluster>::required(m_SVDClusterName);
  StoreArray<SpacePoint>::required(m_SingleClusterSVDSPName);
  StoreArray<SpacePoint>::required(m_NoSingleClusterSVDSPName);
  StoreArray<SpacePoint>::required(m_PXDClusterSPName);

  StoreArray<genfit::TrackCand> gfTrackCand(m_genfitTCName);
  gfTrackCand.required(m_genfitTCName);;

  // registering StoreArray for SpacePointTrackCand (is this done correctly like this?)
  StoreArray<SpacePointTrackCand> spTrackCand(m_SPTCName);
  spTrackCand.registerPersistent(m_SPTCName);

  // register Relation to genfit::TrackCand
  spTrackCand.registerRelationTo(gfTrackCand);

  // check other input parameters
  if (m_PARAMsetOrigin.size() != 3) {
    B2WARNING("SPTC2GFTCConverter::initialize: origin is set wrong. Please provide 3 values (x,y,z). Rejecting user defined value. Resetting origin to (0,0,0)");
    m_PARAMsetOrigin.clear();
    m_PARAMsetOrigin.assign(3, 0);
  }
  m_origin.SetXYZ(m_PARAMsetOrigin.at(0), m_PARAMsetOrigin.at(1), m_PARAMsetOrigin.at(2));
  B2DEBUG(150, "Set origin to: (" << m_origin.X() << "," << m_origin.Y() << "," << m_origin.Z() << ")");

  if (m_NTracklets < 1) {
    B2WARNING("Maximum number of tracklets is set to a value lower than one. Please provide a value larger than one. Resetting to 1 now");
    m_NTracklets = 1;
  }
}

// ------------------------------------- EVENT -------------------------------------------------------
void GFTC2SPTCConverterModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "GFTC2SPTCConverter::event(). Processing event " << eventCounter << " --------");

  StoreArray<genfit::TrackCand> mcTrackCands(m_genfitTCName);
  StoreArray<SpacePointTrackCand> spacePointTrackCands(m_SPTCName);

  std::vector<HitInfo<SpacePoint> > tcSpacePoints; // collection of all SpacePoints, that will be used to create the SpacePointTrackCandidate

  int nTCs = mcTrackCands.getEntries();

  B2DEBUG(15, "Found " << nTCs << " genfit::TrackCands in StoreArray " << mcTrackCands.getName());

  for (int iTC = 0; iTC < nTCs; ++iTC) {
    genfit::TrackCand* trackCand = mcTrackCands[iTC];
    m_genfitTCCtr += 1;
//       trackCand->Print(); // debug purposes

    // for easier handling fill a taggedPair (typedef) with a boolen to indicate whether this hit has already been used or if it has still to be processed
    std::vector<flaggedPair<int> > fHitIDs;
    for (unsigned int i = 0; i < trackCand->getNHits(); ++i) {
      auto hit = trackCand->getHit(i);
      flaggedPair<int> aPair(false, hit->getDetId(), hit->getHitId());
      fHitIDs.push_back(aPair);
    }

    B2DEBUG(10, "Now processing genfit::TrackCand " << iTC << ".");
    try {
      const SpacePointTrackCand spacePointTC = createSpacePointTC(trackCand);
      // if the trackCand should not be split, do not even check if it is a curling track, simply add it to the store Array
      if (!m_PARAMsplitCurlers) {
        SpacePointTrackCand* newSPTC = spacePointTrackCands.appendNew(spacePointTC);
        m_SpacePointTCCtr += 1;
        newSPTC->addRelationTo(trackCand);
        B2DEBUG(10, "Added new SpacePointTrackCand to StoreArray " << spacePointTrackCands.getName());
      } else {
        const std::vector<int> changingIndices = checkTrackCandForCurling(spacePointTC); // ckeck the TrackCand for curling behaviour and get the indices (if it does)
        // COULDDO: define method for adding a SpacePointTrackCand to the store array
        if (changingIndices.size() == 0) { // if no indices come back, the track is not curling
          SpacePointTrackCand* newSPTC = spacePointTrackCands.appendNew(spacePointTC);
          m_SpacePointTCCtr += 1;
          newSPTC->addRelationTo(trackCand);
          B2DEBUG(10, "Added new SpacePointTrackCand to StoreArray " << spacePointTrackCands.getName());
        } else {
          // get a vector of SpacePointTrackCands and store each of them separately in the StoreArray. relate each of them to the genfitTC
          // COULDDO: store curling tracks in a separate StoreArray, where only curling tracks are (for easier handling in backconversion for example)
          // COULDDO: introduce additional counter that counts curling tracks
          const std::vector<Belle2::SpacePointTrackCand> tracklets = splitCurlingTrackCand(spacePointTC, m_NTracklets, changingIndices);
          for (auto SPTrackCand : tracklets) {
            SpacePointTrackCand* newSPTC = spacePointTrackCands.appendNew(SPTrackCand);
            m_SpacePointTCCtr += 1;
            newSPTC->addRelationTo(trackCand);
            B2DEBUG(10, "Added new SpacePointTrackCand to StoreArray " << spacePointTrackCands.getName());
          }
        }
      }
    } catch (SpacePointTrackCand::UnsupportedDetType& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (FoundNoSpacePoint& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (SpacePoint::InvalidNumberOfClusters& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (UnsuitableGFTrackCand& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (UnusedHits& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (FoundNoTrueHit& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (FoundNoCluster& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    } catch (TrueHitsNotMatching& anEx) {
      B2WARNING("Caught an exception during creation of SpacePointTrackCand: " << anEx.what() << " This TrackCandidate will not be processed");
    }
  }
}

// -------------------------------- TERMINATE --------------------------------------------------------
void GFTC2SPTCConverterModule::terminate()
{
  B2INFO("GFTC2SPTCConverter::terminate: got " << m_genfitTCCtr << " genfit::TrackCands and created " << m_SpacePointTCCtr << " SpacePointTrackCands.");
}

// ---------------------------------------- Create SpacePoint TrackCand
const Belle2::SpacePointTrackCand GFTC2SPTCConverterModule::createSpacePointTC(const genfit::TrackCand* genfitTC)
{
  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  std::vector<HitInfo<SpacePoint> > tcSpacePoints;

  int nHits = genfitTC->getNHits();
  B2DEBUG(15, "genfit::TrackCand contains " << nHits << " hits");

  // for easier handling fill a taggedPair (typedef) to distinguish whether a hit has alread been used or if is stil open for procession
  std::vector<flaggedPair<int> > fHitIDs;
  for (int i = 0; i < nHits; ++i) {
    auto aHit = genfitTC->getHit(i);
    flaggedPair<int> aPair(false, aHit->getDetId(), aHit->getHitId());
    fHitIDs.push_back(aPair);
  }

  // now loop over all hits and add them appropriately
  for (int iTCHit = 0; iTCHit < nHits; ++iTCHit) {
    genfit::TrackCandHit* aTCHit = genfitTC->getHit(iTCHit);
    int detID = aTCHit->getDetId();
    int hitID = aTCHit->getHitId();
    int planeID = aTCHit->getPlaneId(); // not used at the moment (except for debug output)
    double sortingParam = aTCHit->getSortingParameter();
    B2DEBUG(60, "TrackCandHit " << iTCHit << " has detID: " << detID << ", hitID: " << hitID << ", planeID: " << planeID);

    // check if this hit has already been used, if not process
    if (fHitIDs[iTCHit].get<0>()) {
      B2DEBUG(60, "This hit has already been added to the SpacePointTrackCand via a SpacePoint and will not be processed again");
    } else {
      // check to which detector the hit belongs and act accordingly
      if (detID == Const::PXD) {
        const PXDCluster* aCluster = pxdClusters[hitID];
        const SpacePoint* aSpacePoint = getPXDSpacePoint(aCluster, fHitIDs, iTCHit);
        tcSpacePoints.push_back({sortingParam, aSpacePoint});
        B2DEBUG(60, "Added SpacePoint " << aSpacePoint->getArrayIndex() << " from Array " << aSpacePoint->getArrayName() << " to tcSpacePoints");
      } else if (detID == Const::SVD) {
        const SVDCluster* aCluster = svdClusters[hitID];
        const SpacePoint* aSpacePoint = getSVDSpacePoint(aCluster, fHitIDs, iTCHit);
        tcSpacePoints.push_back({sortingParam, aSpacePoint});
        B2DEBUG(60, "Added SpacePoint " << aSpacePoint->getArrayIndex() << " from Array " << aSpacePoint->getArrayName() << " to tcSpacePoints");
      } else {
        throw SpacePointTrackCand::UnsupportedDetType();
      }
    }
  }

  // check if all hits have been used
  for (unsigned int i = 0; i < fHitIDs.size(); ++i) {
    flaggedPair<int> fPair = fHitIDs[i];
    B2DEBUG(200, "Hit " << i << " with (detID,hitID): (" << fPair.get<1>() << "," << fPair.get<2>() << ") has been used: " << fPair.get<0>());
    if (!fPair.get<0>()) {
      B2ERROR("There is at least one TrackCandHit that has not been marked as used");
      throw UnusedHits();
    }
  }

  // create a vector of SpacePoint* and one with sorting Parameters to add to the SpacePointTrackCand
  std::vector<const SpacePoint*> spacePoints;
  std::vector<double> sortingParams;
  for (const HitInfo<SpacePoint> aSP : tcSpacePoints) {
    spacePoints.push_back(aSP.second);
    sortingParams.push_back(aSP.first);
  }

  SpacePointTrackCand spacePointTC = SpacePointTrackCand(spacePoints, genfitTC->getPdgCode(), genfitTC->getChargeSeed(), genfitTC->getMcTrackId());

  spacePointTC.set6DSeed(genfitTC->getStateSeed());
  spacePointTC.setCovSeed(genfitTC->getCovSeed());
  spacePointTC.setSortingParameters(sortingParams);

  return spacePointTC;
}

// -------------------------------------- get PXD SpacePoint ----------------------------------------------------------------------------
const Belle2::SpacePoint* GFTC2SPTCConverterModule::getPXDSpacePoint(const PXDCluster* pxdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit)
{
  B2DEBUG(70, "Trying to find a related SpacePoint for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " << pxdCluster->getArrayName());
  const SpacePoint* spacePoint = pxdCluster->getRelatedFrom<SpacePoint>(m_PXDClusterSPName);
  if (spacePoint == NULL) {
    throw FoundNoSpacePoint();
  }
  B2DEBUG(80, "Found SpacePoint " << spacePoint->getArrayIndex() << " in StoreArray " << spacePoint->getArrayName());
  markHitAsUsed(flaggedHitIDs, iHit);
  return spacePoint;
}

// -------------------------------- get SVD Space Point --------------------------------------------------------------------------------------
const Belle2::SpacePoint* GFTC2SPTCConverterModule::getSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit)
{
  B2DEBUG(70, "Trying to find the appropriate SpacePoint for SVDCluster " << svdCluster->getArrayIndex() << " from Array " << svdCluster->getArrayName())

  // search for NoSinglCluster SpacePoints
  RelationVector<SpacePoint> spacePoints = svdCluster->getRelationsFrom<SpacePoint>(m_NoSingleClusterSVDSPName);
  B2DEBUG(60, "Found " << spacePoints.size() << " related SpacePoints for SVDCluster " << svdCluster->getArrayIndex() << " from StoreArray " << svdCluster->getArrayName());

  // if there is no relation to two cluster SpacePoint return the single cluster SpacePoint
  if (spacePoints.size() == 0) return getSingleClusterSVDSpacePoint(svdCluster, flaggedHitIDs, iHit);
  else {
    // if size != 0 there is at least on SpacePoint. Now loop over all related SpacePoints to decide which one to use
    // WARNING: It is possible that more than one SpacePoint has a valid combination of Clusters in the genfit::TrackCand -> TODO: decide which one of these is the right one!
    // If this decision cannot be made -> throw
    // How it is tried to make this decision:
    // 1) Check which SpacePoints are allowed by the indices of their Clusters and also check if one of these Clusters is already used by another SpacePoint already added to the SpacePointTrackCand
    // 2) If ONLY ONE SpacePoint is allowed and its Clusters are not used by any other SpacePoint in the SpacePointTrackCand (i.e. only one valid SpacePoint) -> use this SpacePoint
    // 3) If more than one SpacePoints are allowed, but ONLY ONE of these uses Clusters that are not yet used -> use this SpacePoint
    // 4) If more than one SpacePoints are allowed and more than one SpacePoint uses Clusters that are not yet used, find that SpacePoint which has Cluster indices that appear in consecutive order in the genfit::TrackCand -> use that SpacePoint (if consecutive order cannot be found, use the SpacePoint with the smallest difference of positions)
    // 5) If ONLY ONE SpacePoint is allowed BUT one of his Clusters is already used by another SpacePoint -> throw
    // 6) If no SpacePoint can be found that is allowed and has unused hits -> add this Cluster as singleCluster SpacePoint

    std::vector<std::pair<bool, bool> > existAndValidSP; // .first: Cluster Combination exists in the TrackCand, .second: Cluster Combination exists and has not yet been used
    std::vector<std::pair<int, int> > clusterPositions; // .first: index of SpacePoint in RelationVector, .second: Index of Cluster in the genfit::TrackCand

    // loop over all SpacePoints
    for (unsigned int iSP = 0; iSP < spacePoints.size(); ++iSP) {
      const SpacePoint* aSP = spacePoints[iSP];
      B2DEBUG(100, "Processing SpacePoint " << iSP + 1 << " of " << spacePoints.size() << " with Index " << aSP->getArrayIndex() << " in StoreArray " << aSP->getArrayName() << " related with SVDCluster " << svdCluster->getArrayIndex());

      // get all Relations from the SpacePoint to SVDClusters and look if both (there should be no more than 2) are in the genfit::TrackCand
      // COULDDO: at the moment searching in all SVDCluster StoreArrays, maybe precise this with a new Module Parameter
      RelationVector<SVDCluster> relatedClusters = aSP->getRelationsTo<SVDCluster>("ALL");
      if (relatedClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); }

      bool foundBoth = true; // change to false if the second Cluster cannot be found in the genfit::TrackCand
      bool bothValid = true; // change to false if the second Cluster can be found, but is already used

      for (const SVDCluster & aCluster : relatedClusters) {
        B2DEBUG(100, "Now checking Cluster " << aCluster.getArrayIndex() << ". VXDID is: " << aCluster.getSensorID());

        // if the original Cluster and this Cluster are the same, this Cluster does not need to be checked, because it led here the first way
        // WARNING: only comparing the indices of the Clusters here, not if they are from the same StoreArray!!!
        if (aCluster.getArrayIndex() == svdCluster->getArrayIndex()) {
          clusterPositions.push_back({iSP, iHit});
          B2DEBUG(100, "SVDCluster " << aCluster.getArrayIndex() << " will not be checked for validity, because it led to SpacePoint " << aSP->getArrayIndex() << " from Array " << aSP->getArrayName() << ", which is checked now.");
          continue;
        }

        flaggedPair<int> validID(false, Const::SVD, aCluster.getArrayIndex()); // flaggedPair for finding valid IDs: hit is in genfit::TrackCand and has not yet been used by another SpacePoint
        flaggedPair<int> existingID(true, Const::SVD, aCluster.getArrayIndex()); // flaggedPair for finding existing but used fHitIDs

        // find the positions of these two pairs in flaggedHitIDs
        unsigned int validPos =  std::find(flaggedHitIDs.begin(), flaggedHitIDs.end(), validID) - flaggedHitIDs.begin(); // position in "normal array indexing" (i.e. starting at 0, ending at vector.size() -1 )
        unsigned int existingPos = std::find(flaggedHitIDs.begin(), flaggedHitIDs.end(), existingID) - flaggedHitIDs.begin();

        // now check with these positions if a validID is still in the TrackCand, if not check if an existing but used ID is in the TrackCand (an ID is in the trackCand if it is < TrackCand.getNHits() or flaggedHitIDs.size())
        if (validPos >= flaggedHitIDs.size()) {
          bothValid = false;
          validPos = -1; // not found ?? is this intelligent?
          B2DEBUG(100, "Cluster " << aCluster.getArrayIndex() << " from StoreArray " << aCluster.getArrayName() << " related from SpacePoint " << aSP->getArrayIndex() << " from StoreArray " << aSP->getArrayName() << " has no valid counter part in genfit::TrackCand");
          if (existingPos < flaggedHitIDs.size()) {
            B2DEBUG(100, "Hit has already been used by another SpacePoint")
          } else {
            foundBoth = false;
            existingPos = -1; // not found
            B2DEBUG(100, "Hit is not contained in genfit::TrackCand")
          }
        }
        B2DEBUG(100, "validID = (" << validID.get<1>() << "," << validID.get<2>() << "), found at position " << validPos << ", existingID found at position " << existingPos  << " of " << flaggedHitIDs.size());
        clusterPositions.push_back({iSP, validPos});
      }

      // assign determined values to existAndValidSP
      existAndValidSP.push_back({foundBoth, bothValid});

      B2DEBUG(100, "Cluster Cobination of SpacePoint " << aSP->getArrayIndex() << " is contained in genfit::TrackCand: " << foundBoth << ". SpacePoint is valid: " << bothValid);
    }

    // get the number of existing but used and the number of valid SpacePoints
    int nExistingButUsedSP = std::count(existAndValidSP.begin(), existAndValidSP.end(), std::make_pair(true, false));
    int nValidSP = std::count(existAndValidSP.begin(), existAndValidSP.end(), std::make_pair(true, true));
    B2DEBUG(100, "Found " << nValidSP << " valid SpacePoints, in all SpacePoints related to SVD Cluster with Index " << svdCluster->getArrayIndex() << " in StoreArray " << svdCluster->getArrayName() << ". There were " << nExistingButUsedSP << " SpacePoints that have an allowed Cluster Combination but one of the Clusters has already been used.");

    // if there is no valid SpacePoint, but a SpacePoint with an existing but used cluster, throw, because conversion cannot be done properly then
    if (nValidSP < 1 && nExistingButUsedSP > 0) {
      B2ERROR("All of the SpacePoints that are related to Cluster " << svdCluster->getArrayIndex() << " have Cluster Combinations where one of the Clusters is already used by another SpacePoint! This genfit::TrackCand cannot be converted properly to a SpacePointTrackCand");
      throw UnsuitableGFTrackCand();
    }
    // COULDDO: merge the cases for one and for more than one valid SpacePoints (should be easily doable), since code is mostly copy paste
    // if no valid SpacePoint and no existing but used SpacePoint can be found, add this Cluster as SingleCluster SpacePoint
    else if (nValidSP < 1 && nExistingButUsedSP < 1) {
      B2DEBUG(100, "Found no valid SpacePoint and no SpacePoint with existing but used Clusters/Hits. Adding a SingleClusterSpacePoint related to SVDCluser " << svdCluster->getArrayIndex() <<  " from " << svdCluster->getArrayName());
      return getSingleClusterSVDSpacePoint(svdCluster, flaggedHitIDs, iHit);
    }
    // if only one valid SpacePoint can be found, add that SpacePoint
    else if (nValidSP == 1) {
      unsigned int iSP = std::find(existAndValidSP.begin(), existAndValidSP.end(), std::make_pair(true, true)) - existAndValidSP.begin();
      const SpacePoint* spacePoint = spacePoints[iSP];
      B2DEBUG(80, "SpacePoint " << spacePoint->getArrayIndex() << " from StoreArray " << spacePoint->getArrayName() << " is the only valid SpacePoint related to SVDCluster " << svdCluster->getArrayIndex() << " from " << svdCluster->getArrayName());

      int pos1 = clusterPositions[2 * iSP].second;
      int pos2 = clusterPositions[2 * iSP + 1].second;

      int distance = (pos1 - pos2) * (pos1 - pos2);
      if (distance != 1) {
        B2ERROR("The squared distance between the two Clusters of the only valid SpacePoint is " << distance << " This leads to wrong ordered TrackCandHits. This TrackCand will not be converted!");
        throw UnsuitableGFTrackCand();
      }

      markHitAsUsed(flaggedHitIDs, pos1);
      markHitAsUsed(flaggedHitIDs, pos2);

      return spacePoint;
    }
    // if more than one valid SpacePoint can be found, choose the one with smallest difference of Cluster positions inside the genfit::TrackCand
    else if (nValidSP > 1) {
      std::vector<fourTuple<int> > positionInfos; // 1) index of SpacePoint in RelationVector, 2) squared position difference, 3) & 4) are positions

      for (unsigned int iSP = 0 ; iSP < spacePoints.size(); ++iSP) {
        if (!existAndValidSP[iSP].second) continue; // if not valid continue with next SpacePoint from RelationVector
        int posDiff = clusterPositions[iSP * 2].second - clusterPositions[2 * iSP + 1].second; // sign does not matter here, as only squared values are compared later

        B2DEBUG(200, "Difference of positions of Clusters in genfit::TrackCand is " << posDiff << " for SpacePoint " << spacePoints[iSP]->getArrayIndex() << " (RelationVector index " << iSP << ") containing this and another compatible Cluster");
        fourTuple<int> aTuple(iSP, posDiff * posDiff, clusterPositions[2 * iSP].second, clusterPositions[2 * iSP + 1].second);
        positionInfos.push_back(aTuple);
      }

      // sort to find the smallest difference
      std::sort(positionInfos.begin(), positionInfos.end(), [](const fourTuple<int> lTuple, const fourTuple<int> rTuple) { return lTuple.get<1>() < rTuple.get<1>(); });
      int iSP = positionInfos[0].get<0>(); // SpacePoint with smallest difference of positions is on first position in positinInfo

      int distance = positionInfos[0].get<1>();
      if (distance != 1) {
        B2ERROR("The shortest squared distance between two Clusters is " << distance << ". This leads to wrong ordered TrackCandHits. This TrackCand will not be converted!")
        throw UnsuitableGFTrackCand();
      }
      const SpacePoint* spacePoint = spacePoints[iSP];

      B2DEBUG(80, "SpacePoint " << spacePoint->getArrayIndex() << " from StoreArray " << spacePoint->getArrayName() << " is the valid SpacePoint with two Clusters in consecutive order from all valid SpacePoints related to SVDCluster " << svdCluster->getArrayIndex() << " from " << svdCluster->getArrayName());

      markHitAsUsed(flaggedHitIDs, positionInfos[0].get<2>());
      markHitAsUsed(flaggedHitIDs, positionInfos[0].get<3>());

      return spacePoint;
    }
    // if a SpacePoint gets through all of the above if-clauses, throw
    else {
      B2ERROR("No appropriate SpacePoint has been found for SVD Cluster " << svdCluster->getArrayIndex() << " from StoreArray " << svdCluster->getArrayName() << ". This TrackCand will not be converted!");
      throw UnsuitableGFTrackCand();
    }
  }
  // CAUTION: momentary solution to stop compiler from complaining about -Wreturn-type
  B2FATAL("Reached end of GFTC2SPTCConverterModule::getSVDSpacePoint() without a return statement or a throw");
  return new SpacePoint;
}

// ------------------------------- get SingleCluster SVD SpacePoint
// COULDDO: Template this (almost the same as get PXD SpacePoint) (drawback: need a string argument then, to specify the StoreArray in which desired SpacePoints are located!)
const Belle2::SpacePoint* GFTC2SPTCConverterModule::getSingleClusterSVDSpacePoint(const SVDCluster* svdCluster, std::vector<flaggedPair<int> >& flaggedHitIDs, int iHit)
{
  B2DEBUG(70, "Trying to find a related SpacePoint for Cluster " << svdCluster->getArrayIndex() << " from Array " << svdCluster->getArrayName());
  const SpacePoint* spacePoint = svdCluster->getRelatedFrom<SpacePoint>(m_SingleClusterSVDSPName);
  if (spacePoint == NULL) {
    throw FoundNoSpacePoint();
  }
  B2DEBUG(80, "Found SpacePoint " << spacePoint->getArrayIndex() << " in StoreArray " << spacePoint->getArrayName());
  markHitAsUsed(flaggedHitIDs, iHit);
  return spacePoint;
}

// ---------------------------------- check track cand for curling behaviour ----------------------------------------------------------------
const std::vector<int> GFTC2SPTCConverterModule::checkTrackCandForCurling(const Belle2::SpacePointTrackCand& SPTrackCand)
{
  const std::vector<const Belle2::SpacePoint*>& tcSpacePoints = SPTrackCand.getHits();
  unsigned int nHits = SPTrackCand.getNHits();

  std::vector<int> returnVector; // fill this vector with indices, if no indices can be found, leave it empty

  std::pair<bool, bool> directions; // only store the last two directions to decide if it has changed or not. .first is always last hit, .second is present hit.
  directions.first = true; // assume that the track points outwards from the interaction point

  for (unsigned int iHit = 0; iHit < nHits; ++iHit) {
    const SpacePoint* spacePoint = tcSpacePoints[iHit];
    auto detType = spacePoint->getType();

    // get global position and momentum for every spacePoint in the SpacePointTrackCand
    std::pair<TVector3, TVector3> hitGlobalPosMom;

    if (detType == VXD::SensorInfoBase::PXD) {
      // first get PXDCluster, from that get TrueHit
      PXDCluster* pxdCluster = spacePoint->getRelatedTo<PXDCluster>(m_PXDClusterName);
      PXDTrueHit* pxdTrueHit = pxdCluster->getRelatedTo<PXDTrueHit>("ALL"); // COULDDO: search only certain PXDTrueHit arrays -> new parameter for module

      if (pxdTrueHit == NULL) {
        B2WARNING("Found no PXDTrueHit for PXDCluster " << pxdCluster->getArrayIndex() << " from Array " << pxdCluster->getArrayName() << ". This PXDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
        throw FoundNoTrueHit();
      }

      hitGlobalPosMom = getGlobalPositionAndMomentum(pxdTrueHit);

    } else if (detType == VXD::SensorInfoBase::SVD) {
      // get all related SVDClusters and do some sanity checks, before getting the SVDTrueHits and then using them to get global position and momentum
      RelationVector<SVDCluster> svdClusters = spacePoint->getRelationsTo<SVDCluster>(m_SVDClusterName);
      if (svdClusters.size() > 2) { throw SpacePoint::InvalidNumberOfClusters(); } // should never throw, since this check should already be done in SpacePoint creation!
      if (svdClusters.size() == 0) {
        B2WARNING("Found no related clusters for SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << ". With no Cluster no information if a track is curling or not can be obtained");
        throw FoundNoCluster(); // this should also never happen, as the vice versa way is used above to get to the SpacePoints in the first place!!
      } else {
        // collect the TrueHits (maximum 2), if there is more than one compare them, to see if both Clusters point to the same TrueHit
        std::vector<SVDTrueHit*> svdTrueHits;
        for (const SVDCluster & aCluster : svdClusters) {
          SVDTrueHit* svdTrueHit = aCluster.getRelatedTo<SVDTrueHit>("ALL"); // COULDDO: search only certain SVDTrueHit arrays -> new parameter for module
          if (svdTrueHit == NULL) {
            B2WARNING("Found no SVDTrueHit for SVDCluster " << aCluster.getArrayIndex() << " from Array " << aCluster.getArrayName() << ". This SVDCluster is related with SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName());
            throw FoundNoTrueHit();
          }
          svdTrueHits.push_back(svdTrueHit);
        }
        // if there were 2 clusters, check if they are both related to the same SVDTrueHit, if not throw. Else get position and momentum for this SpacePoint
        if (svdTrueHits.size() > 1) {
          if (svdTrueHits[0] != svdTrueHits[1]) {
            B2WARNING("TrueHits of SpacePoint " << spacePoint->getArrayIndex() << " from Array " << spacePoint->getArrayName() << " are not matching: TrueHit 1 has Index " << svdTrueHits[0]->getArrayIndex() << ", TrueHit 2 has Index " << svdTrueHits[1]->getArrayIndex());
            throw TrueHitsNotMatching();
          }
        }
        hitGlobalPosMom = getGlobalPositionAndMomentum(svdTrueHits[0]);
      }
    } else { // this should never be reached, because it should be caught in the creation of the SpacePointTrackCand which is passed to this function!
      throw SpacePointTrackCand::UnsupportedDetType();
    }

    // get the direction of flight for the present SpacePoint
    directions.second = getDirectionOfFlight(hitGlobalPosMom, m_origin);

    // check if the directions have changed since the last hit, if so, add the number of the SpacePoint (inside the SpacePointTrackCand) to the returnVector
    if (directions.first != directions.second) {
      B2DEBUG(75, "The direction of flight has changed for SpacePoint " << iHit << " in SpacePointTrackCand. The StoreArray index of this SpacePoint is " << spacePoint->getArrayIndex() << " in " << spacePoint->getArrayName());
      returnVector.push_back(iHit);
    }
    // assign old value to .first, for next comparison
    directions.first = directions.second;
  }
  return returnVector;
}

// get the global position of a TrueHit
template<class TrueHit>
std::pair<const TVector3, const TVector3> GFTC2SPTCConverterModule::getGlobalPositionAndMomentum(TrueHit* aTrueHit)
{
  // get sensor stuff (needed for pointToGlobal)
  VxdID aVxdId = aTrueHit->getSensorID();

  B2DEBUG(100, "Getting global position and momentum vectors for TrueHit " << aTrueHit->getArrayIndex() << " from Array " << aTrueHit->getArrayName() << ". This hit has VxdID " << aVxdId);

  const VXD::GeoCache& geometry = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensorInfoBase = geometry.getSensorInfo(aVxdId);

  // get position
  TVector3 hitGlobal = sensorInfoBase.pointToGlobal(TVector3(aTrueHit->getU(), aTrueHit->getV(), 0)); // should work like this, since local coordinates are only 2D
  B2DEBUG(100, "Global position of hit is (" << hitGlobal.X() << "," << hitGlobal.Y() << "," << hitGlobal.Z() << ")");

  // get momentum
  TVector3 pGlobal = sensorInfoBase.pointToGlobal(aTrueHit->getMomentum());
  B2DEBUG(100, "Global momentum of hit is (" << pGlobal.X() << "," << pGlobal.Y() << "," << pGlobal.Z() << ")");

  return std::make_pair(hitGlobal, pGlobal);
}

// get the direction of flight. false is inwards, true is outwards !!
bool GFTC2SPTCConverterModule::getDirectionOfFlight(const std::pair<const TVector3, const TVector3>& hitPosAndMom, const TVector3 origin)
{
  TVector3 originToHit = hitPosAndMom.first - origin;
  TVector3 momentumAtHit = hitPosAndMom.second + originToHit;

  B2DEBUG(250, "Position of hit relative to origin is (" << originToHit.X() << "," << originToHit.Y() << "," << originToHit.Z() << "). Momentum relative to hit (relative to origin) (" << momentumAtHit.X() << "," << momentumAtHit.Y() << "," << momentumAtHit.Z() << ")");

  // cylindrical coordinates (?) -> use TVector3.Perp() to get the radial component of a given vector
  // for spherical coordinates -> use TVector3.Mag() for the same purposes
  double hitRadComp = originToHit.Perp(); // radial component of hit coordinates
  double hitMomRadComp = momentumAtHit.Perp(); // radial component of the tip of the momentum vector, when its origin would be the hit position (as it is only the direction of the momentum that matters here, units are completely ignored)

  if (hitMomRadComp < hitRadComp) {
    B2DEBUG(100, "Direction of flight is inwards for this hit");
    return false;
  } else {
    B2DEBUG(100, "Direction of flight is outwards for this hit");
    return true;
  }
}

// ------------------------------------- split curling track candidates into tracklets -------------------------------------------------------
const std::vector<Belle2::SpacePointTrackCand> GFTC2SPTCConverterModule::splitCurlingTrackCand(const Belle2::SpacePointTrackCand& SPTrackCand, int NTracklets, const std::vector<int>& splitIndices)
{
  std::vector<SpacePointTrackCand> spacePointTCs;

  // first 'first Index' is 0
  int firstIndex = 0;
  // return NTracklets at most
  for (unsigned iTr = 0; iTr < uint(NTracklets) && iTr < splitIndices.size(); ++iTr) {
    int lastIndex = splitIndices[iTr];

    B2DEBUG(75, "Creating Tracklet " << iTr << " of " << splitIndices.size() << " possible Tracklets for this SpacePointTrackCand. The indices for this Tracklet are (first,last): (" << firstIndex << "," << lastIndex << ")");

    const std::vector<const SpacePoint*> trackletSpacePoints = SPTrackCand.getHitsInRange(firstIndex, lastIndex);
    const std::vector<double> trackletSortingParams = SPTrackCand.getSortingParametersInRange(firstIndex, lastIndex);

    // next first Index is lastIndex from this turn + 1
    firstIndex = lastIndex + 1;

    SpacePointTrackCand newSPTrackCand = SpacePointTrackCand(trackletSpacePoints, SPTrackCand.getPdgCode(), SPTrackCand.getChargeSeed(), SPTrackCand.getMcTrackID());
    newSPTrackCand.setSortingParameters(trackletSortingParams);

    // TODO: set state seed and cov seed for all but the first tracklets (first is just the seed of the original TrackCand)

    spacePointTCs.push_back(newSPTrackCand);
  }

  return spacePointTCs;
}

void GFTC2SPTCConverterModule::initializeCounters()
{
  m_SpacePointTCCtr = 0;
  m_genfitTCCtr = 0;
}

void GFTC2SPTCConverterModule::markHitAsUsed(std::vector<flaggedPair<int> >& flaggedHitIDs, int hitToMark)
{
  flaggedHitIDs[hitToMark].get<0>() = true;
  flaggedPair<int> fPair = flaggedHitIDs[hitToMark];
  B2DEBUG(150, "Marked Hit " << hitToMark << " as used. (detID,hitID) of this hit is (" << fPair.get<1>() << "," << fPair.get<2>() << ")");
}
