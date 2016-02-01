/********************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                           *
 * Copyright(C) 2015 - Belle II Collaboration                                   *
 *                                                                              *
 * Author: The Belle II Collaboration                                           *
 * Contributors: Eugenio Paoloni                                                *
 *                                                                              *
 * This software is provided "as is" without any warranty.                      *
 *******************************************************************************/

#ifndef SECTORMAP_HH
#define SECTORMAP_HH

#include <TObject.h>
#include <string>
#include "tracking/trackFindingVXD/environment/VXDTFFilters.h"
#include "tracking/trackFindingVXD/environment/FilterSetTypes.h"
#include <unordered_map>
#include <string>

namespace Belle2 {


  /** This class contains everything needed by the VXDTF that is not going
  * to change during a RUN, i.e. IP position, B field @ IP, static sectors,
  * 2 space points filters, 3 space points filters,  etc. etc.
  * It will be put in the datastore with duration RUN.
  * This class owns her members and she is in charge for their deletions. */
  template<class point_t, class TwoHitFilterSet_t>
  class SectorMap : public TObject {
  public:

    /// typedef for better readability. Index is name of the setup.
    using setupNameToFilters_t = std::unordered_map< std::string, Belle2::VXDTFFilters<point_t, TwoHitFilterSet_t>*>;

  private:

    /** Contains all the Filters and configurations indexed by their setupNames. */
    setupNameToFilters_t* m_allSetupsFilters;


    /// I do not want to copy this class which is a singleton so:
    SectorMap(const SectorMap& sectorMap)  = delete;

  public:


    /// Constructor.
    SectorMap() : m_allSetupsFilters(NULL)
    {
      m_allSetupsFilters = new setupNameToFilters_t;
    }


    /// Destructor deleting all filters stored.
    virtual ~SectorMap()
    {
      for (auto& filter : * m_allSetupsFilters)
        delete filter.second;
    }


    /** returns filters. */
    const VXDTFFilters<point_t, TwoHitFilterSet_t>* getFilters(const std::string& setupName)
    {
      auto  result = m_allSetupsFilters->find(setupName);
      if (result == m_allSetupsFilters->end())
        return NULL;
      return result->second;
    }


    /** returns setups. */
    const setupNameToFilters_t& getAllSetups(void)
    {
      return * m_allSetupsFilters;
    }


    /** assigns filters. */
    void assignFilters(const std::string& setupName ,
                       VXDTFFilters<point_t, TwoHitFilterSet_t>* filters)
    {
      (*m_allSetupsFilters)[ setupName ] = filters;
    }


    /** root class definition. */
    ClassDef(SectorMap , 13);
  };
}
#endif
