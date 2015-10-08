/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Viktor Trusov, Nils Braun                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/trackFindingCDC/fitting/CDCRiemannFitter.h>
#include <tracking/trackFindingCDC/fitting/CDCKarimakiFitter.h>
#include <tracking/trackFindingCDC/legendre/TrackFitter.h>
#include <framework/datastore/StoreArray.h>
#include <tracking/trackFindingCDC/legendre/quadtree/CDCLegendreQuadTree.h>
#include <tracking/trackFindingCDC/legendre/QuadTreeHitWrapper.h>

#include "TH1F.h"
#include "TH2F.h"

#include <list>
#include <vector>
#include <string>
#include <set>

using namespace std;

namespace Belle2 {

  class CDCHit;

  namespace TrackFindingCDC {

    class TrackDrawer;
    class TrackHit;
    class CDCTrack;
    class CDCTrajectory2D;

    class TrackProcessorNew {
    public:

      /**
       * Please note that the implemented methods do only use the axial hits!
       * We use the fitter and the drawer as a pointer to have the possibility to use different classes.
       */
      TrackProcessorNew() :
        m_cdcTracks(), m_trackFitter(),
        m_histChi2("h_chi2", "chi2", 100, 0., 300.),
        m_histChi2NDF("h_chi2", "chi2", 100, 0., 300.),
        m_histDist("h_dist", "dist", 100, 0., 2.),
        m_histADC("h_ADC", "ADC", 1000, 0., 1000),
        m_histADCpt("h_ADCpt", "ADC vs Pt", 400, 0., 1000, 40, 0., 3.) { }


      /**
       * Do not copy this class
       */
      TrackProcessorNew(const TrackProcessorNew& copy) = delete;

      /**
       * Do not copy this class
       */
      TrackProcessorNew& operator=(const TrackProcessorNew&) = delete;

      /**
       * Compile the hitList from the wire hit topology.
       */
      void initializeQuadTreeHitWrappers();

      void removeBadSLayers(CDCTrack& track);

      /*CDCTrack&*/ void createCDCTrackCandidates(std::vector<QuadTreeHitWrapper*>& trackHits);

      /** Created CDCTracks from the stored CDCLegendreTrackCandidates */
      void createCDCTracks(std::vector<Belle2::TrackFindingCDC::CDCTrack>& tracks) ;

      void updateTrack(CDCTrack& track);

      bool checkTrack(CDCTrack& track);

      void assignNewHits(CDCTrack& track);

      void assignNewHitsUsingSegments(CDCTrack& track, float fraction = 0.3);

      void assignNewHits();

      int estimateCharge(double theta, double r, std::vector<TrackHit*>& trackHits);

      void mergeTracks();

      void fillHist(CDCTrack& track);

      void saveHist();

      bool checkChi2(CDCTrack& track);

      double estimateChi2(CDCTrack& track);

      double getQuantile(double alpha, double n);

      void checkADC();

      CDCTrajectory2D fit(CDCTrack& track);

      void createCandidate(std::vector<const CDCWireHit*>& hits);

      void unmaskHitsInTrack(CDCTrack& track);

      /**
       * Get the list with currently stored tracks.
       */
      std::list<CDCTrack>& getTracks()
      {
        return m_cdcTracks;
      }

      /**
       * Get the list with currently stored tracks.
       */
      std::vector<QuadTreeHitWrapper>& getQuadTreeHitWrappers()
      {
        return m_QuadTreeHitWrappers;
      }

      /**
       * For the use in the QuadTree use this hit set.
       * @return the hit set with axial hits to use in the QuadTree-Finding.
       */
      std::vector<QuadTreeHitWrapper*> createQuadTreeHitWrappersForQT(bool useSegmentsOnly = false);



      /**
       * Postprocessing: Delete axial hits that do not "match" to the given track.
       */
      void deleteBadHitsOfOneTrack(CDCTrack& trackCandidate);




      /**
       * Reset all masked hits
       */
      void resetMaskedHits()
      {
        doForAllHits([](QuadTreeHitWrapper & hit) {
          hit.setMaskedFlag(false);
          hit.setUsedFlag(false);
        });

        doForAllTracks([](CDCTrack & track) {
          for (CDCRecoHit3D& hit : track) {
            hit.getWireHit().getAutomatonCell().setTakenFlag(true);
          }
        });
        /*

                doForAllHits([](QuadTreeHitWrapper & hit) {
                  if (hit.getMaskedFlag()) {
                    hit.setMaskedFlag(false);
                    hit.setUsedFlag(false);
                  }
                });
        */
      }

      /**
       * After each event the created hits and trackCandidates should be deleted.
       */
      void clearVectors()
      {
        m_QuadTreeHitWrappers.clear();
        m_cdcTracks.clear();
      }

      static bool isCurler(CDCTrack& track) { return fabs(track.getStartTrajectory3D().getCurvatureXY()) > 0.017; }

      /**
       * Do a certain function for each track in the track list
       */
      void doForAllTracks(std::function<void(CDCTrack& track)> function)
      {
        /*         for (CDCTrack& track : m_cdcTracks) {
                  function(track);
                }
        */
        for (std::list<CDCTrack>::iterator it = m_cdcTracks.begin(); it !=  m_cdcTracks.end(); ++it) {
          function(*it);
        }



      }

    private:
      std::list<CDCTrack> m_cdcTracks; /**< List of track candidates. */
      std::vector<QuadTreeHitWrapper> m_QuadTreeHitWrappers; /**< Vector which hold axial hits */
      CDCRiemannFitter m_trackFitter;
//      CDCKarimakiFitter m_trackFitter;

      /**
       * @brief Perform the necessary operations after the track candidate has been constructed
       * @param track The constructed track candidate
       * @param trackHitList list of all hits, which are used for track finding. Hits belonging to the track candidate will be deleted from it.
       * This function leaves room for other operations like further quality checks or even the actual fitting of the track candidate.
       */
      void processTrack(CDCTrack& track);


      /**
       * Do a certain function for each track in the track list
       */
      void doForAllHits(std::function<void(QuadTreeHitWrapper& hit)> function)
      {
        for (QuadTreeHitWrapper& hit : m_QuadTreeHitWrappers) {
          function(hit);
        }
      }

      TH1F m_histChi2;
      TH1F m_histChi2NDF;
      TH1F m_histDist;

      TH1F m_histADC;
      TH2F m_histADCpt;
    };
  }
}

