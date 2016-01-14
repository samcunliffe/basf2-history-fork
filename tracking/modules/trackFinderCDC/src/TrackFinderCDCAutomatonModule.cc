/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFinderCDC/TrackFinderCDCAutomatonModule.h>

using namespace Belle2;
using namespace TrackFindingCDC;

REG_MODULE(TrackFinderCDCAutomaton);

TrackFinderCDCAutomatonModule ::TrackFinderCDCAutomatonModule() :
  TrackFinderCDCBaseModule() // Virtual base class must be instantiated from the most derived class.
{
  this->setDescription("Performs patter recognition in the CDC based on local hit following and application of a cellular automaton in two stages.");
  ModuleParamList moduleParamList = this->getParamList();
  moduleParamList.getParameter<std::string>("SegmentOrientation").setDefaultValue("symmetric");
  moduleParamList.getParameter<std::string>("TrackOrientation").setDefaultValue("outwards");
  this->setParamList(moduleParamList);
}

void TrackFinderCDCAutomatonModule::initialize()
{
  this->SegmentFinderCDCFacetAutomatonModule::initialize();
  this->TrackFinderCDCSegmentPairAutomatonModule::initialize();
}

void TrackFinderCDCAutomatonModule::event()
{
  B2DEBUG(100, "########## TrackFinderCDCAutomaton begin ##########");

  // Stage one
  this->SegmentFinderCDCFacetAutomatonModule::event();

  // Stage two
  this->TrackFinderCDCSegmentPairAutomatonModule::event();

  B2DEBUG(100, "########## TrackFinderCDCAutomaton end ############");
}

void TrackFinderCDCAutomatonModule::terminate()
{
  this->TrackFinderCDCSegmentPairAutomatonModule::terminate();
  this->SegmentFinderCDCFacetAutomatonModule::terminate();
}
