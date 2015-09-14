/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <tracking/trackFindingCDC/filters/base/FilterFactoryBase.h>

#include <map>
#include <string>

namespace Belle2 {

  namespace TrackFindingCDC {

    /**
     * Intermediate minimal class that can should specialised
     * for concrete instances of filter factories.
     *
     * The construction enables automatic collaboration with depending constructs
     * such as the UnionRecordingFilter.
     */
    template<class AFilter>
    class FilterFactory : public FilterFactoryBase<AFilter> {

    public:
      /** Fill the default filter name and parameter values*/
      FilterFactory(std::string filterName,
                    std::map<std::string, std::string> filterParameters
                    = std::map<std::string, std::string>()) :
        FilterFactoryBase<AFilter>(filterName, filterParameters)
      {
      }
    };

  } // end namespace TrackFindingCDC
} // end namespace Belle2
