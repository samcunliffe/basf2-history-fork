/**************************************************************************b
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef AXIALSTEREOSEGMENTPAIRFILTERTREE_H
#define AXIALSTEREOSEGMENTPAIRFILTERTREE_H

#include <tracking/trackFindingCDC/tempro/StaticTypedTree.h>
#include <tracking/trackFindingCDC/tempro/BranchOf.h>
#include <tracking/trackFindingCDC/tempro/Named.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialStereoSegmentPair.h>

namespace Belle2 {
  namespace CDCLocalTracking {

    /// ROOT Tree structure declaration for the evaluation of axial to stereo segment pair filters.
    typedef StaticTypedTree <
    //BranchOf < Float_t, NAMED("mcWeight") > ,
    //BranchOf < Float_t, NAMED("prWeight") > ,

    BranchOf < Bool_t, NAMED("mcDecision") > ,
             BranchOf < Bool_t, NAMED("prDecision") > ,

             BranchOf < UInt_t, NAMED("startSegment_size") > ,
             BranchOf < UInt_t, NAMED("endSegment_size") > ,

             BranchOf < Int_t, NAMED("startSegment_iSuperLayer") > ,
             BranchOf < Int_t, NAMED("endSegment_iSuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_startISuperLayer") > ,
             BranchOf < Int_t, NAMED("endFit_startISuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_nextISuperLayer") > ,
             BranchOf < Int_t, NAMED("endFit_previousISuperLayer") > ,

             BranchOf < Int_t, NAMED("startFit_nextAxialISuperLayer") > ,
             BranchOf < Int_t, NAMED("endFit_previousAxialISuperLayer") > ,

             BranchOf < float, NAMED("startFit_totalPerpS_startSegment") > ,
             BranchOf < float, NAMED("endFit_totalPerpS_startSegment") > ,
             BranchOf < float, NAMED("commonFit_totalPerpS_startSegment") > ,

             BranchOf < float, NAMED("startFit_totalPerpS_endSegment") > ,
             BranchOf < float, NAMED("endFit_totalPerpS_endSegment") > ,
             BranchOf < float, NAMED("commonFit_totalPerpS_endSegment") > ,

             BranchOf < ForwardBackwardInfo, NAMED("startFit_isForwardOrBackwardTo_startSegment") > ,
             BranchOf < ForwardBackwardInfo, NAMED("endFit_isForwardOrBackwardTo_startSegment") > ,
             BranchOf < ForwardBackwardInfo, NAMED("commonFit_isForwardOrBackwardTo_startSegment") > ,

             BranchOf < ForwardBackwardInfo, NAMED("startFit_isForwardOrBackwardTo_endSegment") > ,
             BranchOf < ForwardBackwardInfo, NAMED("endFit_isForwardOrBackwardTo_endSegment") > ,
             BranchOf < ForwardBackwardInfo, NAMED("commonFit_isForwardOrBackwardTo_endSegment") > ,

             BranchOf < float, NAMED("startFit_perpSGap") > ,
             BranchOf < float, NAMED("endFit_perpSGap") > ,
             BranchOf < float, NAMED("commonFit_perpSGap") > ,

             BranchOf < float, NAMED("startFit_perpSFrontOffset") > ,
             BranchOf < float, NAMED("endFit_perpSFrontOffset") > ,
             BranchOf < float, NAMED("commonFit_perpSFrontOffset") > ,

             BranchOf < float, NAMED("startFit_perpSBackOffset") > ,
             BranchOf < float, NAMED("endFit_perpSBackOffset") > ,
             BranchOf < float, NAMED("commonFit_perpSBackOffset") > ,

             BranchOf < float, NAMED("startFit_dist2DToCenter_endSegment") > ,
             BranchOf < float, NAMED("endFit_dist2DToCenter_startSegment") > ,

             BranchOf < float, NAMED("startFit_dist2DToFront_endSegment") > ,
             BranchOf < float, NAMED("endFit_dist2DToBack_startSegment") > ,

             BranchOf < float, NAMED("startFit_absMom2D") > ,
             BranchOf < float, NAMED("endFit_absMom2D") > ,

             BranchOf < float, NAMED("momAngleAtCenter_endSegment") > ,
             BranchOf < float, NAMED("momAngleAtCenter_startSegment") > ,

             BranchOf < float, NAMED("startFit_chi2") > ,
             BranchOf < float, NAMED("endFit_chi2") > ,
             BranchOf < float, NAMED("commonFit_chi2") > ,

             BranchOf < float, NAMED("commonFit_szSlope") > , //fusion fit method
             BranchOf < float, NAMED("commonFit_szSlope_variance") > ,

             BranchOf < float, NAMED("szFit_szSlope") > , //'manual' reconstruction fit method
             BranchOf < float, NAMED("mcFit_szSlope") > ,

             BranchOf < float, NAMED("axialFit_curvatureXY") > ,
             BranchOf < float, NAMED("axialFit_curvatureXY_variance") > ,

             BranchOf < float, NAMED("commonFit_curvatureXY") > ,
             BranchOf < float, NAMED("commonFit_curvatureXY_variance") > ,

             BranchOf < float, NAMED("mcFit_curvatureXY") >

             > AxialStereoSegmentPairFilterTreeBase;

    /// Class managing the output in the evaluation of axial to stereo segment pair filters.
    class AxialStereoSegmentPairFilterTree : public AxialStereoSegmentPairFilterTreeBase {

    public:
      /// Empty constructor.
      AxialStereoSegmentPairFilterTree();

      /// Empty deconstructor.
      ~AxialStereoSegmentPairFilterTree();

    public:
      /// Fills the object with the information gathered from the given segment to segment pair instance.
      bool setValues(const CellWeight& mcWeight, const CellWeight& prWeight, const CDCAxialStereoSegmentPair& axialStereoSegmentPair);

    }; // end class AxialStereoSegmentPairFilterVars

  } //end namespace CDCLocalTracking
} //end namespace Belle2



#endif // AXIALSTEREOSEGMENTPAIRFILTERTREE_H
