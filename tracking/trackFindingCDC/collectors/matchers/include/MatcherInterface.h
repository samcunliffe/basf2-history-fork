/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Nils Braun                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/findlets/base/Findlet.h>
#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class ACollectorItem, class ACollectionItem>
    class MatcherInterface :
      public Findlet<const ACollectorItem,
      const ACollectionItem,
      WeightedRelation<const ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      typedef WeightedRelation<const ACollectorItem, const ACollectionItem> WeightedRelationItem;

      void apply(const std::vector<ACollectorItem>& collectorItems, const std::vector<ACollectionItem>& collectionItems,
                 std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (const ACollectorItem& collectorItem : collectorItems) {
          match(collectorItem, collectionItems, weightedRelations);
        }

        std::sort(weightedRelations.begin(), weightedRelations.end(), WeightedRelationsGreater<WeightedRelationItem>());
      }

    protected:
      virtual void match(const ACollectorItem& collectorItem, const std::vector<ACollectionItem>& collectionItems,
                         std::vector<WeightedRelationItem>& relationsForCollector)
      {
        for (const ACollectionItem& collectionItem : collectionItems) {
          Weight weight = match(collectorItem, collectionItem);
          relationsForCollector.emplace_back(&collectorItem, weight, &collectionItem);
        }
      };

      virtual Weight match(const ACollectorItem& collectorItem, const ACollectionItem& collectionItem)
      {
        return NAN;
      }
    };
  }
}
