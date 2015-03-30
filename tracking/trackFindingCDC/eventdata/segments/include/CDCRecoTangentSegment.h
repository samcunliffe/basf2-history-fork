/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#ifndef CDCRECOTANGENTSEGMENT_H
#define CDCRECOTANGENTSEGMENT_H

#include <tracking/trackFindingCDC/eventdata/collections/CDCRecoTangentVector.h>
#include <tracking/trackFindingCDC/eventdata/entities/CDCRecoFacet.h>
#include <vector>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// A segment consisting of adjacent reconstructed tangents.
    class  CDCRecoTangentSegment : public CDCRecoTangentVector {

    public:
      /// Default constructor for ROOT compatibility.
      CDCRecoTangentSegment() {;}

      /// Empty deconstructor
      ~CDCRecoTangentSegment() {;}

      /// Takes all distinct tangents from the facets in the path - Note! there is no particular order of the tangents in the segment.
      static CDCRecoTangentSegment condense(const std::vector<const Belle2::TrackFindingCDC::CDCRecoFacet* >& recoFacetPath);


    private:
      /// ROOT Macro to make CDCRecoTangentSegment a ROOT class.
      TRACKFINDINGCDC_SwitchableClassDef(CDCRecoTangentSegment, 1);


    };

  }
}


#endif // CDCRECOTANGENTSEGMENT



