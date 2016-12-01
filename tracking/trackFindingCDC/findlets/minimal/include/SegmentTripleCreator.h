/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentTriple.h>
#include <tracking/trackFindingCDC/eventdata/tracks/CDCAxialSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCSegment2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <vector>
#include <array>
#include <string>
#include <algorithm>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the axial stereo segment pairs.
    template <class ASegmentTripleFilter>
    class SegmentTripleCreator
      : public Findlet<const CDCSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCSegment2D, const CDCAxialSegmentPair, CDCSegmentTriple>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentTripleCreator()
      {
        this->addProcessingSignalListener(&m_segmentTripleFilter);
      }

      /// Short description of the findlet
      std::string getDescription() override final
      {
        return "Creates segment triples from a set of middle segments and already combined axial "
               "segment pairs filtered by some acceptance criterion";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) override final
      {
        m_segmentTripleFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCSegment2D>& inputSegments,
                 const std::vector<CDCAxialSegmentPair>& inputAxialSegmentPairs,
                 std::vector<CDCSegmentTriple>& segmentTriples) override final
      {
        // Group the segments by their super layer id
        for (std::vector<const CDCSegment2D*>& segementsInSuperLayer : m_segmentsBySuperLayer) {
          segementsInSuperLayer.clear();
        }

        for (const CDCSegment2D& segment : inputSegments) {
          ISuperLayer iSuperLayer = segment.getISuperLayer();
          const CDCSegment2D* ptrSegment = &segment;
          m_segmentsBySuperLayer[iSuperLayer].push_back(ptrSegment);
        }

        for (const CDCAxialSegmentPair& axialSegmentPair : inputAxialSegmentPairs) {
          const CDCSegment2D* startSegment = axialSegmentPair.getStartSegment();
          const CDCSegment2D* endSegment = axialSegmentPair.getEndSegment();

          ISuperLayer startISuperLayer = startSegment->getISuperLayer();
          ISuperLayer endISuperLayer = endSegment->getISuperLayer();

          B2ASSERT("Invalid start ISuperLayer", ISuperLayerUtil::isAxial(startISuperLayer));
          B2ASSERT("Invalid end ISuperLayer", ISuperLayerUtil::isAxial(endISuperLayer));

          if (startISuperLayer != endISuperLayer) {
            ISuperLayer middleISuperLayer = (startISuperLayer + endISuperLayer) / 2;

            B2ASSERT("Middle ISuperLayer is not stereo",
                     not ISuperLayerUtil::isAxial(middleISuperLayer));

            const std::vector<const CDCSegment2D*>& middleSegments =
              m_segmentsBySuperLayer[middleISuperLayer];
            create(axialSegmentPair, middleSegments, segmentTriples);
          } else {
            // Case where start and end super layer are the same
            // Look for stereo segments in the adjacent super layer
            ISuperLayer middleISuperLayerIn = ISuperLayerUtil::getNextInwards(startISuperLayer);
            ISuperLayer middleISuperLayerOut = ISuperLayerUtil::getNextOutwards(startISuperLayer);
            for (ISuperLayer middleISuperLayer : {middleISuperLayerIn, middleISuperLayerOut}) {
              if (ISuperLayerUtil::isInCDC(middleISuperLayer)) {
                const std::vector<const CDCSegment2D*>& middleSegments
                  = m_segmentsBySuperLayer[middleISuperLayer];
                create(axialSegmentPair, middleSegments, segmentTriples);
              }
            }
          }
        }
        std::sort(segmentTriples.begin(), segmentTriples.end());
      }

    private:
      /// Creates segment triples by adding the middle segment to the axial segments and filter out the valuable combinations
      void create(const CDCAxialSegmentPair& axialSegmentPair,
                  const std::vector<const CDCSegment2D*>& middleSegments,
                  std::vector<CDCSegmentTriple>& segmentTriples)
      {
        CDCSegmentTriple segmentTriple(axialSegmentPair);
        for (const CDCSegment2D* middleSegment : middleSegments) {
          segmentTriple.setMiddleSegment(middleSegment);
          segmentTriple.clearTrajectory3D();

          // Ask the filter to assess this triple
          Weight weight = m_segmentTripleFilter(segmentTriple);

          if (not isNotACell(weight)) {
            segmentTriple.getAutomatonCell().setCellWeight(weight);
            segmentTriples.insert(segmentTriples.end(), segmentTriple);
          }
        }
      }

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment triple generation.
      ASegmentTripleFilter m_segmentTripleFilter;
    };
  }
}
