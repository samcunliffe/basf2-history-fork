/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Eliachevitch                                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/mcTrackCurlerCloneClassifier/MCTrackCurlerCloneClassifierModule.h>

#include <tracking/trackFindingCDC/eventdata/tracks/CDCTrack.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TFCDC_MCTrackCurlerCloneClassifier);

TFCDC_MCTrackCurlerCloneClassifierModule::TFCDC_MCTrackCurlerCloneClassifierModule()
  : Super( {"CDCTrackVector"})
{
}
