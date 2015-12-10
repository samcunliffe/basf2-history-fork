/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/facet_relation/MCFacetRelationFilter.h>

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

void MCFacetRelationFilter::beginEvent()
{
  m_mcFacetFilter.beginEvent();
  Super::beginEvent();
}



void MCFacetRelationFilter::initialize()
{
  Super::initialize();
  m_mcFacetFilter.initialize();
}



void MCFacetRelationFilter::terminate()
{
  m_mcFacetFilter.terminate();
  Super::terminate();
}



NeighborWeight MCFacetRelationFilter::operator()(const CDCFacet& fromFacet,
                                                 const CDCFacet& toFacet)
{
  // the last wire of the neighbor should not be the same as the start wire of the facet
  if (fromFacet.getStartWire() == toFacet.getEndWire()) {
    return NOT_A_NEIGHBOR;
  }
  // Despite of that two facets are neighbors if both are true facets
  // That also implies the correct tof alignment of the hits not common to both facets
  CellWeight fromFacetWeight = m_mcFacetFilter(fromFacet);
  CellWeight toFacetWeight = m_mcFacetFilter(toFacet);

  bool mcDecision = (not isNotACell(fromFacetWeight)) and (not isNotACell(toFacetWeight));

  // the weight must be -2 because the overlap of the facets is two points
  // so the amount of two facets is 4 points hence the cellular automat
  // must calculate 3 + (-2) + 3 = 4 as cellstate
  // this can of course be adjusted for a more realistic information measure
  // ( together with the facet creator filter)
  return mcDecision ? -2.0 : NOT_A_NEIGHBOR;
}
