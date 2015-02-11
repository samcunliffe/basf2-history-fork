/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "../include/MCSegmentTripleNeighborChooser.h"

using namespace std;
using namespace Belle2;
using namespace TrackFindingCDC;

MCSegmentTripleNeighborChooser::MCSegmentTripleNeighborChooser(bool allowReverse) :
  m_mcSegmentTripleFilter(allowReverse)
{
}

void MCSegmentTripleNeighborChooser::clear()
{
  m_mcSegmentTripleFilter.clear();
}

void MCSegmentTripleNeighborChooser::initialize()
{
  m_mcSegmentTripleFilter.initialize();
}

void MCSegmentTripleNeighborChooser::terminate()
{
  m_mcSegmentTripleFilter.terminate();
}


