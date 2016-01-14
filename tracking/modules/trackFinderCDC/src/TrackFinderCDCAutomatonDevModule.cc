/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonDevModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCAutomatonDev);

TrackFinderCDCAutomatonDevModule::TrackFinderCDCAutomatonDevModule()
{
  this->setDescription("Deprectated for TrackFinderCDCAutomatonModule.");
}

void TrackFinderCDCAutomatonDevModule::initialize()
{
  B2WARNING("TrackFinderCDCAutomatonModule is deprectated for TrackFinderCDCAutomatonDevModule.");
  TrackFinderCDCAutomatonModule::initialize();
}
