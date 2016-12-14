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

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.h>

#include <tracking/trackFindingCDC/utilities/WeightedRelation.h>
#include <tracking/trackFindingCDC/utilities/StringManipulation.h>

#include <framework/core/ModuleParamList.h>
#include <framework/logging/Logger.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    template <class AFilterFactory, class ACollectorItem, class ACollectionItem>
    class FilterSelector :
      public Findlet<WeightedRelation<const ACollectorItem, const ACollectionItem>> {
    public:
      /// Shortcut class name for a weighted relation between a collector item and a collection item.
      typedef WeightedRelation<const ACollectorItem, const ACollectionItem> WeightedRelationItem;

      /// The parent class
      typedef Findlet<WeightedRelation<const ACollectorItem, const ACollectionItem>> Super;

      FilterSelector() : Super()
      {
        addProcessingSignalListener(m_filter);
      }

      void exposeParameters(ModuleParamList* moduleParamList,
                            const std::string& prefix) override
      {
        Super::exposeParameters(moduleParamList, prefix);
      }

      void apply(std::vector<WeightedRelationItem>& weightedRelations) override
      {
        for (auto& weightedRelation : weightedRelations) {
          Weight filterResult = m_filter({ weightedRelation.getFrom(), weightedRelation.getTo() });
          weightedRelation.setWeight(filterResult);
        }

        std::sort(weightedRelations.begin(), weightedRelations.end(), WeightedRelationsGreater<WeightedRelationItem>());

        // Erase all items with a weight of NAN
        auto firstNANItem = std::find(weightedRelations.begin(), weightedRelations.end(), NAN);
        weightedRelations.erase(firstNANItem, weightedRelations.end());
      }

    private:
      ChoosableFilter<AFilterFactory> m_filter;
    };
  }
}
