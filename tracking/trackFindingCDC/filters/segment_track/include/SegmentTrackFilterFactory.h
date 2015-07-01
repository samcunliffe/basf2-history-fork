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

#include <tracking/trackFindingCDC/filters/segment_track/SegmentTrackFilter.h>
#include <tracking/trackFindingCDC/filters/base/FilterFactory.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Specialisation the filter factory for segment pairs
     *
     * It knows about all available filters and their parameters.
     * Can collaborate with a Module and expose these parameters to the user in steering files.
     */
    template<>
    class FilterFactory<BaseSegmentTrackFilter> :
      public FilterFactoryBase<BaseSegmentTrackFilter> {

    private:
      /// Type of the base class
      typedef FilterFactoryBase<BaseSegmentTrackFilter> Super;

    public:
      /** Fill the default filter name and parameter values*/
      FilterFactory<BaseSegmentTrackFilter>(const std::string& defaultFilterName = "simple") : Super(defaultFilterName) { }

      using Super::create;

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseSegmentTrackFilter> create(const std::string& name) const override;

      /** Getter for the valid filter names and a description for each */
      virtual std::map<std::string, std::string> getValidFilterNamesAndDescriptions() const override;
    };

    class SegmentTrackFilterFirstStepFactory : public FilterFactory<BaseSegmentTrackFilter> {

    private:
      /// Type of the base class
      typedef FilterFactory<BaseSegmentTrackFilter> Super;

    public:
      /** Fill the default filter name and parameter values*/
      SegmentTrackFilterFirstStepFactory(const std::string& defaultFilterName = "simple") : Super(defaultFilterName) { }

      using Super::create;

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const override
      {
        return "Segment track chooser to be used during the matching of segment track pairs.";
      }

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseSegmentTrackFilter> create(const std::string& name) const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override
      {
        return "SegmentTrackFilterFirstStep";
      }
    };

    class SegmentTrackFilterSecondStepFactory : public FilterFactory<BaseSegmentTrackFilter> {

    private:
      /// Type of the base class
      typedef FilterFactory<BaseSegmentTrackFilter> Super;

    public:
      /** Fill the default filter name and parameter values*/
      SegmentTrackFilterSecondStepFactory(const std::string& defaultFilterName = "simple") : Super(defaultFilterName) { }

      using Super::create;

      /** Getter for a descriptive purpose of the filter.*/
      virtual std::string getFilterPurpose() const override
      {
        return "Segment track chooser to be used during the combination of segment track pairs.";
      }

      /** Create a filter with the given name, does not set filter specific parameters. */
      virtual std::unique_ptr<BaseSegmentTrackFilter> create(const std::string& name) const override;

      /** Getter for the prefix prepended to a Module parameter.*/
      virtual std::string getModuleParamPrefix() const override
      {
        return "SegmentTrackFilterSecondStep";
      }
    };
  }
}
