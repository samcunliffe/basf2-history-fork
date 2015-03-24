/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bastian Kronenbitter, Thomas Hauth                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCLegendreTrackingModule.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreFastHough.h>
#include <tracking/trackFindingCDC/legendre/CDCLegendreTrackDrawer.h>
#include <genfit/TrackCand.h>
#include <cdc/dataobjects/CDCHit.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

//ROOT macro
REG_MODULE(CDCLegendreTracking)

CDCLegendreTrackingModule::CDCLegendreTrackingModule() :
  Module(), m_cdcLegendreQuadTree(0, m_nbinsTheta, m_rMin, m_rMax, 0, nullptr),
  m_cdcLegendreTrackProcessor(),
  m_cdcLegendreFastHough(nullptr), m_cdcLegendreTrackDrawer(nullptr)
{
  setDescription(
    "Performs the pattern recognition in the CDC with the conformal finder: digitized CDCHits are combined to track candidates (genfit::TrackCand)");

  addParam("CDCHitsColName", m_param_cdcHitsColumnName,
           "Input CDCHits collection (should be created by CDCDigi module)",
           string("CDCHits"));

  addParam("GFTrackCandidatesColName", m_param_trackCandidatesColumnName,
           "Output GFTrackCandidates collection",
           string("TrackCands"));

  addParam("Threshold", m_param_threshold, "Threshold for peak finder", static_cast<unsigned int>(10));

  addParam("StepScale", m_stepScale, "Scale size for Stepped Hough", 0.75);

  addParam("MaxLevel", m_maxLevel,
           "Maximal level of recursive calling of FastHough algorithm", 12);

  addParam("ReconstructCurler", m_reconstructCurler,
           "Flag, whether curlers should be reconstructed", false);

  addParam("DeleteHitsInTheEnd", m_deleteHitsInTheEnd,
           "Try to delete bad hits from track candidate in the end.", true);
  addParam("MergeTracksInTheEnd", m_mergeTracksInTheEnd,
           "Try to merge tracks in the end.", true);
  addParam("AppendHitsInTheEnd", m_appendHitsInTheEnd,
           "Try to append new hits to track candidate in the end.", true);

  addParam("DeleteHitsWhileFinding", m_deleteHitsWhileFinding,
           "Try to delete bad hits from track candidate", true);
  addParam("MergeTracksWhileFinding", m_mergeTracksWhileFinding,
           "Try to merge tracks while finding.", false);
  addParam("AppendHitsWhileFinding", m_appendHitsWhileFinding,
           "Try to append new hits to track candidate while finding.", false);
  addParam("DoPostprocessingOften", m_doPostprocessingOften,
           "Repeat the postprocessing mode after every tree search.", true);
  addParam("TreeFindingNumber", m_treeFindingNumber,
           "Repeat the whole process that many times.", 3);

  addParam("DrawCandidates", m_drawCandidates,
           "Draw candidate after finding", false);

  addParam("EnableDrawing", m_drawCandInfo,
           "Enable in-module drawing", false);

  addParam("EnableBatchMode", m_batchMode,
           "Enable batch mode for track drawer. (Done with gROOT->SetBatch())", false);
}

void CDCLegendreTrackingModule::initialize()
{
  //StoreArray for genfit::TrackCandidates
  StoreArray<genfit::TrackCand>::registerPersistent(m_param_trackCandidatesColumnName);

  // initialize track drawer
  m_cdcLegendreTrackDrawer = new TrackDrawer(m_drawCandInfo, m_drawCandidates, m_batchMode);
  m_cdcLegendreTrackDrawer->initialize();

  // initialize fast hough
  m_cdcLegendreFastHough = new FastHough(m_reconstructCurler, m_maxLevel, m_nbinsTheta, m_rMax);

  // set parameters of track processor
  m_cdcLegendreTrackProcessor.setTrackDrawer(m_cdcLegendreTrackDrawer);
}

void CDCLegendreTrackingModule::event()
{
  startNewEvent();
  findTracks();
  outputObjects();
  clearVectors();
}

void CDCLegendreTrackingModule::startNewEvent()
{
  B2INFO("**********   CDCTrackingModule  ************");

  eventNumber++;
  m_cdcLegendreTrackDrawer->event();

  B2DEBUG(100, "Initializing hits");
  StoreArray<CDCHit> cdcHits(m_param_cdcHitsColumnName);
  m_cdcLegendreTrackProcessor.initializeHitList(cdcHits);
}

void CDCLegendreTrackingModule::findTracks()
{
  B2DEBUG(100, "Perform track finding");

  // The first case is somewhat special
  doTreeTrackFinding(50, 0.07, false);
  if (m_treeFindingNumber == 1 || m_doPostprocessingOften)
    postprocessTracks();

  for (int counter = 1; counter < m_treeFindingNumber; counter++) {
    doTreeTrackFinding(20, 0.15, true);
    if (counter == m_treeFindingNumber - 1 || m_doPostprocessingOften)
      postprocessTracks();
  }
}

void CDCLegendreTrackingModule::outputObjects()
{
  for (TrackCandidate* cand : m_cdcLegendreTrackProcessor.getTrackList()) {
    B2DEBUG(100, "R value: " << cand->getR() << "; theta: " << cand->getTheta() << "; radius: " << cand->getRadius() << "; phi: "
            << cand->getPhi() << "; charge: " << cand->getChargeSign() << "; Xc = " << cand->getXc() << "; Yc = " << cand->getYc() <<
            "Hitsize: " << cand->getNHits());
  }
  //create GenFit Track candidates
  m_cdcLegendreTrackProcessor.createGFTrackCandidates(m_param_trackCandidatesColumnName);
  m_cdcLegendreTrackDrawer->finalizeFile();
}


void CDCLegendreTrackingModule::processNodes(std::vector<QuadTreeLegendre*>& listOfCandidates,
                                             QuadTreeLegendre::CandidateProcessorLambda& lmdFunct, unsigned int limit)
{
  if (listOfCandidates.size() == 0) return;
  QuadTreeProcessor qtProcessor(m_maxLevel);

  do {
    //sort nodes by number of hits
    std::sort(listOfCandidates.begin(), listOfCandidates.end(), [](const QuadTreeLegendre * a, const QuadTreeLegendre * b) {
      return static_cast <bool>(a->getNItems() > b->getNItems());
    });

    lmdFunct(listOfCandidates.front());

    for (QuadTreeLegendre* qt : listOfCandidates) qt->cleanUpItems(qtProcessor);

    listOfCandidates.erase(std::remove_if(listOfCandidates.begin(), listOfCandidates.end(),
    [&](QuadTreeLegendre * qt) { return qt->getNItems() < limit; }),
    listOfCandidates.end());

  } while (listOfCandidates.size() > 0);
}

void CDCLegendreTrackingModule::doTreeTrackFinding(unsigned int limitInitial, double rThreshold, bool increaseThreshold)
{
  B2DEBUG(100, "Performing tree track finding");
  m_treeFinder++;

  unsigned int limit = limitInitial;

  double rCDC = 113.;

  std::set<TrackHit*> hits_set = m_cdcLegendreTrackProcessor.createHitSet();

  QuadTreeProcessor qtProcessor(m_maxLevel);
  m_cdcLegendreQuadTree.clearTree();
  m_cdcLegendreQuadTree.provideItemsSet(qtProcessor, hits_set);
  int nSteps = 0;

  std::vector<QuadTreeLegendre*> listOfCandidates;


  // this lambda function will forward the found candidates to the CandidateCreate for further processing
  // hits belonging to found candidates will be marked as used and ignored for further
  // filling iterations

  QuadTreeLegendre::CandidateProcessorLambda lmdCandidateProcessingFinal = [&](QuadTreeLegendre * qt) -> void {
    TrackCandidate* trackCandidate = m_cdcLegendreTrackProcessor.createLegendreTrackCandidateFromQuadNode(qt);

    unsigned int numberOfUsedHits = 0;
    for (TrackHit* hit : hits_set)
    {
      if (hit->getHitUsage() == TrackHit::used_in_track)
        numberOfUsedHits++;
    }

    B2DEBUG(90, "Number of used hits: " << numberOfUsedHits)

    // Postprocessing of one track candidate
    m_cdcLegendreTrackProcessor.fitOneTrack(trackCandidate);

    if (m_deleteHitsWhileFinding)
    {
      m_cdcLegendreTrackProcessor.deleteBadHitsOfOneTrack(trackCandidate);
    }

    if (m_mergeTracksWhileFinding)
    {
      m_cdcLegendreTrackProcessor.mergeOneTrack(trackCandidate);
    }

    if (m_appendHitsWhileFinding)
    {
      m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
    }

    m_cdcLegendreTrackProcessor.deleteTracksWithASmallNumberOfHits();


  };

  QuadTreeLegendre::CandidateProcessorLambda lmdCandidateProcessing = [&](QuadTreeLegendre * qt) -> void {
    if (TrackCandidate::convertRhoToPt(fabs(qt->getYMean())) > 0.4) lmdCandidateProcessingFinal(qt);
    else listOfCandidates.push_back(qt);
  };


  listOfCandidates.clear();
  //find high-pt tracks (not-curlers: diameter of the track higher than radius of CDC -- 2*Rtrk > rCDC => Rtrk < 2./rCDC, r(legendre) = 1/Rtrk =>  r(legendre) < 2./rCDC)
  qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, 50, 2. / rCDC);
  if (listOfCandidates.size() > 0) processNodes(listOfCandidates, lmdCandidateProcessingFinal, 50);

  listOfCandidates.clear();
  //find curlers with diameter higher than half of radius of CDC (see calculations above)
  qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, 70, 4. / rCDC);
  if (listOfCandidates.size() > 0) processNodes(listOfCandidates, lmdCandidateProcessingFinal, 70);


  // Start loop, where tracks are searched for
  do {
    listOfCandidates.clear();
    qtProcessor.fillGivenTree(&m_cdcLegendreQuadTree, lmdCandidateProcessing, limit, rThreshold);

    if (listOfCandidates.size() > 0) {
      processNodes(listOfCandidates, lmdCandidateProcessingFinal, limit);
    } else {

      limit = limit * m_stepScale;

      if (increaseThreshold) {
        rThreshold *= 2.;
        if (rThreshold > 0.15) rThreshold = 0.15;
      }
    }
    nSteps++;
    //perform search until found track has too few hits or threshold is too small and no tracks are found
  } while (limit >= m_param_threshold && hits_set.size() >= m_param_threshold);

  //sort tracks by value of curvature
  m_cdcLegendreTrackProcessor.getTrackList().sort([](const TrackCandidate * a, const TrackCandidate * b) {
    return static_cast <bool>(a->getRadius() > b->getRadius());
  });

  // ?????
  std::vector<TrackHit*> hits_vector; //temporary array;
  m_cdcLegendreTrackDrawer->finalizeROOTFile(hits_vector);

  B2DEBUG(90, "Number of steps in tree track finding: " << nSteps);
  B2DEBUG(90, "Threshold on number of hits: " << limit);
  B2DEBUG(90, "Threshold on r: " << rThreshold);
}

void CDCLegendreTrackingModule::postprocessTracks()
{
  m_cdcLegendreTrackProcessor.fitAllTracks();

  if (m_deleteHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.deleteHitsOfAllBadTracks();
  }

  if (m_mergeTracksInTheEnd) {
    m_cdcLegendreTrackProcessor.mergeAllTracks();
  }

  if (m_appendHitsInTheEnd) {
    m_cdcLegendreTrackProcessor.appendHitsOfAllTracks();
  }
}

void CDCLegendreTrackingModule::endRun()
{
  B2INFO("Tree finder had " << m_treeFinder << " calls");
  B2INFO("Stepped finder had " << m_steppedFinder << " calls");
}

void CDCLegendreTrackingModule::terminate()
{
  delete m_cdcLegendreTrackDrawer;
  delete m_cdcLegendreFastHough;
}

void CDCLegendreTrackingModule::clearVectors()
{
  m_cdcLegendreTrackProcessor.clearVectors();
}
