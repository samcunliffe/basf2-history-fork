/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/fitting/CDCObservations2D.h>
#include <tracking/trackFindingCDC/fitting/EFitVariance.h>
#include <tracking/trackFindingCDC/fitting/EFitPos.h>

#include <tracking/trackFindingCDC/eventdata/trajectories/CDCTrajectory2D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    class CDCSegment2D;
    class CDCWireHitSegment;
    class CDCWireHit;
    class CDCWire;

    /// Class implementing a fit for two dimensional trajectory circle using a generic fitting backend.
    template<class AFitMethod>
    class CDCFitter2D : public AFitMethod {

    public:
      /// Empty constructor
      CDCFitter2D() = default;

      /// Fits a collection of observation drift circles.
      CDCTrajectory2D fit(CDCObservations2D observations2D) const
      {
        CDCTrajectory2D result;
        update(result, observations2D);
        return result;
      }

      /// Fits a collection of hit typs which are convertable to observation circles.
      template<class AHits>
      CDCTrajectory2D fit(const AHits& hits) const
      {
        CDCTrajectory2D result;
        update(result, hits);
        return result;
      }

      /// Fits together two collections of hit types which are convertable to observation circles.
      template<class AStartHits, class AEndHits>
      CDCTrajectory2D fit(const AStartHits& startHits, const AEndHits& endHits) const
      {
        CDCTrajectory2D result;
        update(result, startHits, endHits);
        return result;
      }

      /// Fits the segment
      CDCTrajectory2D fit(const CDCSegment2D& segment) const
      {
        CDCTrajectory2D result;
        update(result, segment);
        return result;
      }

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const std::vector<const CDCWire*>& wires) const
      {
        CDCTrajectory2D result;
        update(result, wires);
        return result;
      }

      /// Fits to the wire positions. Explicit specialisation to be used from python.
      CDCTrajectory2D fit(const CDCWireHitSegment& wireHits) const
      {
        CDCTrajectory2D result;
        update(result, wireHits);
        return result;
      }

      /**
       *  Updates a given trajectory with a fit to two collection of hit types,
       *  which are convertable to observation circles.
       */
      template <class AStartHits, class AEndHits>
      void update(CDCTrajectory2D& trajectory2D, const AStartHits& startHits, const AEndHits& endHits) const
      {
        CDCObservations2D observations2D;
        observations2D.setFitVariance(m_fitVariance);

        if (m_usePosition) {
          observations2D.setFitPos(EFitPos::c_RecoPos);
          observations2D.appendRange(startHits);
        }
        if (m_useOrientation) {
          observations2D.setFitPos(EFitPos::c_RLDriftCircle);
          observations2D.appendRange(startHits);
        }

        if (m_usePosition) {
          observations2D.setFitPos(EFitPos::c_RecoPos);
          observations2D.appendRange(endHits);
        }
        if (m_useOrientation) {
          observations2D.setFitPos(EFitPos::c_RLDriftCircle);
          observations2D.appendRange(endHits);
        }

        if (observations2D.size() < 4) {
          trajectory2D.clear();
        } else {
          AFitMethod::update(trajectory2D, observations2D);
        }
      }

      /**
       *  Updates a given trajectory with a fit to a collection of hits types,
       *  which are convertable to observation circles.
       */
      template <class AHits>
      void update(CDCTrajectory2D& trajectory2D, const AHits& hits) const
      {
        CDCObservations2D observations2D;
        observations2D.setFitVariance(m_fitVariance);

        if (m_usePosition) {
          observations2D.setFitPos(EFitPos::c_RecoPos);
          observations2D.appendRange(hits);
        }
        if (m_useOrientation) {
          observations2D.setFitPos(EFitPos::c_RLDriftCircle);
          observations2D.appendRange(hits);
        }

        if (observations2D.size() < 4) {
          trajectory2D.clear();
        } else {
          AFitMethod::update(trajectory2D, observations2D);
        }
      }

      /// Update the trajectory with a fit to the observations.
      void update(CDCTrajectory2D& trajectory2D, CDCObservations2D& observations2D) const
      {
        AFitMethod::update(trajectory2D, observations2D);
      }

      //set which information should be used from the recohits
      //useOnlyPosition is standard

      /// Setup the fitter to use only the reconstructed positions of the hits
      void useOnlyPosition()
      {
        m_usePosition = true;
        m_useOrientation = false;
      }

      /// Setup the fitter to use only reference position and the drift length with right left orientation
      void useOnlyOrientation()
      {
        m_usePosition = false;
        m_useOrientation = true;
      }

      /// Setup the fitter to use both the reconstructed position and the reference position and the drift length with right left orientation.
      void usePositionAndOrientation()
      {
        m_usePosition = true;
        m_useOrientation = true;
      }

      /// Setup the fitter to use the given variance measure by default.
      void setFitVariance(EFitVariance fitVariance)
      {
        m_fitVariance = fitVariance;
      }

    private:
      /// Flag indicating the reconstructed position shall be used in the fit.
      bool m_usePosition = true;

      /// Flag indicating the reference position and drift length with right left orientation shall be used in the fit.
      bool m_useOrientation = false;

      /// Default variance to be used in the fit.
      EFitVariance m_fitVariance = EFitVariance::c_Proper;
    };
  }
}
