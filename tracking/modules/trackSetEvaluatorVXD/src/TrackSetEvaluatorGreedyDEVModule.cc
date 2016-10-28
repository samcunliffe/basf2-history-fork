/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/modules/trackSetEvaluatorVXD/TrackSetEvaluatorGreedyDEVModule.h"

//The following include is from Jakob's original Greedy, which employs a
//recursive greedy algorithm, that might be faster...
//#include <tracking/spacePointCreation/sptcNetwork/TrackSetEvaluatorGreedy.h>
#include <tracking/trackFindingVXD/trackSetEvaluator/Scrooge.h>

#include <vector>

//------------------------------------------------------------------------------------------------
//TMPTMPTMP:::
#include <iostream>
//------------------------------------------------------------------------------------------------

using namespace std;
using namespace Belle2;


REG_MODULE(TrackSetEvaluatorGreedyDEV)

TrackSetEvaluatorGreedyDEVModule::TrackSetEvaluatorGreedyDEVModule() : Module()
{
  setDescription("Expects a container of SpacePointTrackCandidates,\
 selects a subset of non-overlapping TCs using the Greedy algorithm.");

  addParam("NameSpacePointTrackCands", m_nameSpacePointTrackCands, "Name of expected StoreArray.", string(""));
}

void TrackSetEvaluatorGreedyDEVModule::event()
{
  //Create an empty object of the type,
  //that needs to be given to Scrooge.
  vector<Scrooge::QITrackOverlap> qiTrackOverlap;
  unsigned int const nSpacePointTrackCands = m_spacePointTrackCands.getEntries();
  qiTrackOverlap.reserve(nSpacePointTrackCands);

  //fill this object with the necessary information:
  for (auto && spacePointTrackCand : m_spacePointTrackCands) {
    qiTrackOverlap.emplace_back(spacePointTrackCand.getQualityIndex(), spacePointTrackCand.getArrayIndex(),
                                m_overlapNetworks[0]->getOverlapForTrackIndex(spacePointTrackCand.getArrayIndex()),
                                true);
  }

  //give it make a Scrooge with this input and
  Scrooge scrooge(qiTrackOverlap);
  scrooge.performSelection();

  //-----------------------------------------------------------------------------------------------
  //TMPTMPTMP:::
  for (auto && track : m_spacePointTrackCands) {
    if (track.getRefereeStatus(SpacePointTrackCand::c_isActive)) {
      cout << track.getRefereeStatusString() << ", Array Index: " << track.getArrayIndex()
           << ", QI: " << track.getQualityIndex() << endl;
    }
  }

  for (auto && track : qiTrackOverlap) {
    if (track.isActive) {
      cout << "Array Index: " << track.trackIndex  << ", QI: " << track.qualityIndex << endl;
    }
  }


  //-----------------------------------------------------------------------------------------------
}
