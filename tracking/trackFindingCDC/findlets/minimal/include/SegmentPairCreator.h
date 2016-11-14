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

#include <tracking/trackFindingCDC/eventdata/tracks/CDCSegmentPair.h>
#include <tracking/trackFindingCDC/eventdata/segments/CDCRecoSegment2D.h>

#include <tracking/trackFindingCDC/topology/ISuperLayer.h>

#include <framework/core/ModuleParamList.h>

#include <vector>
#include <array>
#include <algorithm>
#include <string>

namespace Belle2 {
  class ModuleParamList;

  namespace TrackFindingCDC {
    /// Class providing construction combinatorics for the axial stereo segment pairs.
    template<class ASegmentPairFilter>
    class SegmentPairCreator
      : public Findlet<const CDCRecoSegment2D, CDCSegmentPair> {

    private:
      /// Type of the base class
      using Super = Findlet<const CDCRecoSegment2D, CDCSegmentPair>;

    public:
      /// Constructor adding the filter as a subordinary processing signal listener.
      SegmentPairCreator()
      {
        this->addProcessingSignalListener(&m_segmentPairFilter);
      }

      /// Short description of the findlet
      std::string getDescription() final {
        return "Creates axial stereo segment pairs from a set of segments filtered by some acceptance criterion";
      }

      /// Expose the parameters to a module
      void exposeParameters(ModuleParamList* moduleParamList, const std::string& prefix) final {
        m_segmentPairFilter.exposeParameters(moduleParamList, prefix);
      }

      /// Main method constructing pairs in adjacent super layers
      void apply(const std::vector<CDCRecoSegment2D>& inputSegments,
                 std::vector<CDCSegmentPair>& segmentPairs) final {
        // Group the segments by their super layer id
        for (std::vector<const CDCRecoSegment2D*>& segmentsInSuperLayer : m_segmentsBySuperLayer)
        {
          segmentsInSuperLayer.clear();
        }

        for (const CDCRecoSegment2D& segment : inputSegments)
        {
          ISuperLayer iSuperLayer = segment.getISuperLayer();
          if (not ISuperLayerUtil::isInvalid(iSuperLayer)) {
            const CDCRecoSegment2D* ptrSegment = &segment;
            m_segmentsBySuperLayer[iSuperLayer].push_back(ptrSegment);
          }
        }

        // Make pairs of closeby superlayers
        for (ISuperLayer iSuperLayer = 0; iSuperLayer < ISuperLayerUtil::c_N; ++iSuperLayer)
        {
          const std::vector<const CDCRecoSegment2D*>& fromSegments = m_segmentsBySuperLayer[iSuperLayer];

          // Make pairs of this superlayer and the superlayer more to the inside
          ISuperLayer iSuperLayerIn = ISuperLayerUtil::getNextInwards(iSuperLayer);
          if (ISuperLayerUtil::isInCDC(iSuperLayerIn)) {
            const std::vector<const CDCRecoSegment2D*>& toSegments = m_segmentsBySuperLayer[iSuperLayerIn];
            create(fromSegments, toSegments, segmentPairs);
          }

          // Make pairs of this superlayer and the superlayer more to the outside
          ISuperLayer iSuperLayerOut = ISuperLayerUtil::getNextOutwards(iSuperLayer);
          if (ISuperLayerUtil::isInCDC(iSuperLayerOut)) {
            const std::vector<const CDCRecoSegment2D*>& toSegments = m_segmentsBySuperLayer[iSuperLayerOut];
            create(fromSegments, toSegments, segmentPairs);
          }
        }
        std::sort(segmentPairs.begin(), segmentPairs.end());
      }

    private:
      /// Creates segment pairs from a combination of from segments and to segments.
      void create(const std::vector<const CDCRecoSegment2D*>& fromSegments,
                  const std::vector<const CDCRecoSegment2D*>& toSegments,
                  std::vector<CDCSegmentPair>& segmentPairs)
      {
        CDCSegmentPair segmentPair;
        for (const CDCRecoSegment2D* ptrFromSegment : fromSegments) {
          for (const CDCRecoSegment2D* ptrToSegment : toSegments) {

            if (ptrFromSegment == ptrToSegment) continue; // Just for safety
            segmentPair.setSegments(ptrFromSegment, ptrToSegment);
            segmentPair.clearTrajectory3D();

            if (segmentPair.getTrajectory3D().isFitted()) {
              B2ERROR("CDCSegmentPair still fitted after clearing.");
              continue;
            }

            if (not segmentPair.checkSegments()) {
              B2ERROR("CDCAxialSegmentPair containing nullptr encountered in SegmentPairCreator");
              continue;
            }

            Weight pairWeight = m_segmentPairFilter(segmentPair);
            if (not isNotACell(pairWeight)) {
              segmentPair.getAutomatonCell().setCellWeight(pairWeight);
              segmentPairs.push_back(segmentPair);
            }
          }
        }
      }

    private:
      /// Structure for the segments grouped by super layer id.
      std::array<std::vector<const CDCRecoSegment2D*>, ISuperLayerUtil::c_N> m_segmentsBySuperLayer;

      /// The filter to be used for the segment pair generation.
      ASegmentPairFilter m_segmentPairFilter;
    };
  }
}
