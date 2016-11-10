/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost, Nils Braun                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/backgroundSegment/BackgroundSegmentsFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {
  namespace TrackFindingCDC {

    /// Factory that can create appropriate segment filters for new tracks from associated names.
    class BackgroundSegmentsFilterFactory : public FilterFactory<BaseBackgroundSegmentsFilter> {

    private:
      /// Type of the base class
      using Super = FilterFactory<BaseBackgroundSegmentsFilter>;

    public:
      /// Constructor forwarding the default filter name
      BackgroundSegmentsFilterFactory(const std::string& defaultFilterName = "none");

      /// Getter for a short identifier for the factory
      std::string getIdentifier() const override;

      /// Getter for a descriptive purpose of the constructed filters
      std::string getFilterPurpose() const override;

      /// Getter for valid filter names and a description for each
      std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

      /// Create a filter with the given name.
      std::unique_ptr<BaseBackgroundSegmentsFilter> create(const std::string& filterName) const override;
    };
  }
}
