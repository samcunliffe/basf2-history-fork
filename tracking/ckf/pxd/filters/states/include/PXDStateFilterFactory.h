/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include <tracking/ckf/pxd/filters/states/BasePXDStateFilter.h>

#include <tracking/trackFindingCDC/filters/base/FilterFactory.dcl.h>

namespace Belle2 {
  /// Factory that can create appropriate cluster filters from associated names.
  class PXDStateFilterFactory : public TrackFindingCDC::FilterFactory<BasePXDStateFilter> {

  private:
    /// Type of the base class
    using Super = TrackFindingCDC::FilterFactory<BasePXDStateFilter>;

  public:
    /// Constructor forwarding the default filter name
    explicit PXDStateFilterFactory(const std::string& defaultFilterName = "all");

    /// Default destructor
    ~PXDStateFilterFactory();

    /// Getter for a short identifier for the factory
    std::string getIdentifier() const override;

    /// Getter for a descriptive purpose of the constructed filters
    std::string getFilterPurpose() const override;

    /// Getter for valid filter names and a description for each
    std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;

    /// Create a filter with the given name.
    std::unique_ptr<BasePXDStateFilter> create(const std::string& filterName) const override;
  };
}
