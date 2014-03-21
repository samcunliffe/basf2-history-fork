/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AXIALAXIALSEGMENTPAIRFILTERTREE_H_
#define AXIALAXIALSEGMENTPAIRFILTERTREE_H_

#include <tracking/cdcLocalTracking/tempro/StaticTypedTree.h>
#include <tracking/cdcLocalTracking/tempro/BranchOf.h>
#include <tracking/cdcLocalTracking/tempro/Named.h>

#include <tracking/cdcLocalTracking/eventdata/tracks/CDCAxialAxialSegmentPair.h>

#include <tracking/cdcLocalTracking/typedefs/BasicConstants.h>

#include <TVector2.h>
#include <TObject.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// Filter for the constuction of axial to axial segment pairs based on simple criterions
    typedef StaticTypedTree <
    BranchOf < float, NAMED("mcWeight") > ,
             BranchOf < bool, NAMED("mcDecision") > ,

             BranchOf < UInt_t, NAMED("startSegment_size") > ,
             BranchOf < UInt_t, NAMED("endSegment_size") > ,

             BranchOf < Int_t, NAMED("startSegment_iSuperLayer") > ,
             BranchOf < Int_t, NAMED("endSegment_iSuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_startISuperLayer") > ,
             BranchOf < Int_t, NAMED("endFit_startISuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_nextISuperLayer") > ,
             //BranchOf < Int_t, NAMED("endFit_nextISuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_nextAxialISuperLayer") > ,
             //BranchOf < Int_t, NAMED("endFit_nextAxialISuperLayer") > ,


             BranchOf < bool, NAMED("startSegment_isForwardTrajectory_startFit") > ,
             BranchOf < bool, NAMED("startSegment_isForwardTrajectory_endFit") > ,
             BranchOf < bool, NAMED("startSegment_isForwardTrajectory_commonFit") > ,

             BranchOf < bool, NAMED("endSegment_isForwardTrajectory_startFit") > ,
             BranchOf < bool, NAMED("endSegment_isForwardTrajectory_endFit") > ,
             BranchOf < bool, NAMED("endSegment_isForwardTrajectory_commonFit") > ,

             BranchOf < float, NAMED("startSegment_totalPerpS_startFit") > ,
             BranchOf < float, NAMED("startSegment_totalPerpS_endFit") > ,
             BranchOf < float, NAMED("startSegment_totalPerpS_commonFit") > ,

             BranchOf < float, NAMED("endSegment_totalPerpS_startFit") > ,
             BranchOf < float, NAMED("endSegment_totalPerpS_endFit") > ,
             BranchOf < float, NAMED("endSegment_totalPerpS_commonFit") > ,

             BranchOf < float, NAMED("perpS_gap_startFit") > ,
             BranchOf < float, NAMED("perpS_gap_endFit") > ,
             BranchOf < float, NAMED("perpS_gap_commonFit") > ,

             BranchOf < float, NAMED("perpS_offset_startHits_startFit") > ,
             BranchOf < float, NAMED("perpS_offset_startHits_endFit") > ,
             BranchOf < float, NAMED("perpS_offset_startHits_commonFit") > ,

             BranchOf < float, NAMED("perpS_offset_endHits_startFit") > ,
             BranchOf < float, NAMED("perpS_offset_endHits_endFit") > ,
             BranchOf < float, NAMED("perpS_offset_endHits_commonFit") > ,

             BranchOf < float, NAMED("startSegment_absmom") > ,
             BranchOf < float, NAMED("endSegment_absmom") > ,


             BranchOf < float, NAMED("startSegment_com_x") > ,
             BranchOf < float, NAMED("startSegment_com_y") > ,
             BranchOf < float, NAMED("endSegment_com_x") > ,
             BranchOf < float, NAMED("endSegment_com_y") > ,

             BranchOf < float, NAMED("startSegment_center_x") > ,
             BranchOf < float, NAMED("startSegment_center_y") > ,
             BranchOf < float, NAMED("endSegment_center_x") > ,
             BranchOf < float, NAMED("endSegment_center_y") > ,

             BranchOf < float, NAMED("startSegment_extrapolation_x") > ,
             BranchOf < float, NAMED("startSegment_extrapolation_y") > ,
             BranchOf < float, NAMED("endSegment_extrapolation_x") > ,
             BranchOf < float, NAMED("endSegment_extrapolation_y") > ,

             BranchOf < float, NAMED("startSegment_extrapolation_to_center_distance") > ,
             BranchOf < float, NAMED("endSegment_extrapolation_to_center_distance") >

             > AxialAxialSegmentPairFilterTreeBase;


    class AxialAxialSegmentPairFilterTree : public AxialAxialSegmentPairFilterTreeBase {

    public:
      AxialAxialSegmentPairFilterTree();

      ~AxialAxialSegmentPairFilterTree();

    public:
      /// Fills the object with the information gather from the given segment to segment pair instance.
      bool setValues(const CellWeight& mcWeight, const CDCAxialAxialSegmentPair& axialAxialSegmentPair);

      /*
      TVector2 m_startSegment_com;
      TVector2 m_endSegment_com;

      Float_t m_startSegment_mom;
      Float_t m_endSegment_mom;

      TVector2 m_startSegment_center;
      TVector2 m_endSegment_center;

      TVector2 m_startSegment_extrapolation;
      TVector2 m_endSegment_extrapolation;

      TVector2 m_startSegment_centerMom;
      TVector2 m_endSegment_centerMom;

      TVector2 m_startSegment_extarpolationMom;
      TVector2 m_endSegment_extrapolationMom;

      Float_t m_distanceAtStart;
      Float_t m_distanceAtEnd;

      Float_t m_momAngleDeviationAtStart;
      Float_t m_momAngleDeviationAtEnd;
      */

    private:


    }; // end class AxialAxialSegmentPairFilterVars

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // AXIALAXIALSEGMENTPAIRFILTERTREE_H_
