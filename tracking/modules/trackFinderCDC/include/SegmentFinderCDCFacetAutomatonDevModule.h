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

#include <tracking/modules/trackFinderCDC/SegmentFinderCDCFacetAutomatonModule.h>

namespace Belle2 {

  /// Module for the cellular automaton tracking for the CDC on regular events
  class SegmentFinderCDCFacetAutomatonDevModule: public Belle2::TrackFindingCDC::SegmentFinderCDCFacetAutomatonImplModule<> {

  public:
    /// Constructor of the module. Setting up parameters and description.
    SegmentFinderCDCFacetAutomatonDevModule();

    /// Initialize the Module before event processing
    virtual void initialize() override;

    /// Event method exectured for each event.
    virtual void event() override;

  private:
    /** Parameter: Cluster filter to be used during the construction of clusters.
    Valid values are:
    + "all" (all hits are valid),
    + "tmva"
    + "recording"
    */
    std::string m_param_clusterFilter;

    /** Parameter: Cluster filter parameters forwarded to the cluster filter
     *  Meaning of the Key - Value pairs depend on the cluster filter
     */
    std::map<std::string, std::string> m_param_clusterFilterParameters;

    /** Parameter: Facet filter to be used during the construction of facets.
    Valid values are:
    + "none" (no facet is valid, stop at cluster generation.)
    + "all" (all facets are valid)
    + "mc" (monte carlo truth)
    + "mc_symmetric" (monte carlo truth and their mirror image)
    + "fitless" (only checking the feasability of right left passage information)
    + "fitless_hard" (also exclude the boarder line feasable combinations)
    + "simple" (mc free with simple criteria)
    */
    std::string m_param_facetFilter;

    /** Parameter: Facet neighbor chooser to be used during the construction of the graph.
    Valid values are:

    + "none" (no neighbor is correct, stops segment generation)
    + "all" (all possible neighbors are valid)
    + "mc" (monte carlo truth)
    + "mc_symmetric" (monte carlo truth and the reversed version are excepted)
    + "simple" (mc free with simple criteria)
    + "realistic" (mc free with more realistic criteria)
    */
    std::string m_param_facetNeighborChooser;


  }; // end class
} // end namespace Belle2


