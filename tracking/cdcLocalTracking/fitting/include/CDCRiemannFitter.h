/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRIEMANNFITTER_H
#define CDCRIEMANNFITTER_H

#include <tracking/cdcLocalTracking/mockroot/MockRoot.h>
#include <tracking/cdcLocalTracking/typedefs/BasicTypes.h>
#include <tracking/cdcLocalTracking/eventdata/CDCEventData.h>

#include <tracking/cdcLocalTracking/fitting/CDCObservations2D.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Class implementing the Riemann fit for two dimensional trajectory circle
    class CDCRiemannFitter : public CDCLocalTracking::UsedTObject {

    public:

      ///Static getter for a general riemann fitter
      static const CDCRiemannFitter& getFitter();

      ///Static getter for a line fitter
      static const CDCRiemannFitter& getLineFitter();

      ///Static getter for an origin circle fitter
      static const CDCRiemannFitter& getOriginCircleFitter();

    public:

      ///Empty constructor
      CDCRiemannFitter();

      ///Empty destructor
      ~CDCRiemannFitter();

      CDCTrajectory2D fit(const CDCRLWireHitTriple& rlWireHitTriple) const;
      CDCTrajectory2D fit(const CDCRecoSegment2D& recoSegment2D) const;
      CDCTrajectory2D fit(const CDCSegmentTriple& segmentTriple) const;

      void update(CDCTrajectory2D& fit, const CDCRLWireHitTriple& rlWireHitTriple) const;
      void update(CDCTrajectory2D& fit, const CDCRecoSegment2D& recoSegment2D) const;
      void update(CDCTrajectory2D& fit, const CDCRecoSegment2D& firstRecoSegment2D, const CDCRecoSegment2D& secondRecoSegment2D) const;
      void update(CDCTrajectory2D& fit, const CDCSegmentTriple& segmentTriple) const;

      void update(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;
      void updateWithOutRightLeft(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;
      void updateWithRightLeft(CDCTrajectory2D& fit, CDCObservations2D& observations2D) const;

      bool isLineConstrained() const { return m_lineConstrained; }
      bool isOriginConstrained() const { return m_originConstrained; }

      void setLineConstrained(bool constrained = true) { m_lineConstrained = constrained; }
      void setOriginConstrained(bool constrained = true) { m_originConstrained = constrained; }


      //set which information should be used from the recohits
      //usePositionAndOrientation is standard
      void useOnlyPosition() { m_usePosition = true; m_useOrientation = false;}
      void useOnlyOrientation() { m_usePosition = false; m_useOrientation = true;}
      void usePositionAndOrientation() { m_usePosition = true; m_useOrientation = true;}

      bool m_usePosition;
      bool m_useOrientation;

      bool m_lineConstrained;
      bool m_originConstrained;

    public:

      /** ROOT Macro to make CDCRiemannFitter a ROOT class.*/
      ClassDefInCDCLocalTracking(CDCRiemannFitter, 1);




    }; //class

  } // end namespace CDCLocalTracking
} // namespace Belle2
#endif // CDCRIEMANNFITTER
