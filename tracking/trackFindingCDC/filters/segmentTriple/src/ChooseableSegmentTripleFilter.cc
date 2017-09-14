/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/filters/segmentTriple/ChooseableSegmentTripleFilter.h>

#include <tracking/trackFindingCDC/filters/segmentTriple/SegmentTripleFilterFactory.h>

#include <tracking/trackFindingCDC/filters/base/ChooseableFilter.icc.h>

using namespace Belle2 ;
using namespace TrackFindingCDC ;

template class TrackFindingCDC::ChooseableFilter<SegmentTripleFilterFactory>;
