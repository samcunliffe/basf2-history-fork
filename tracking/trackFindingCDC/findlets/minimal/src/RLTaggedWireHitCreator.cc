/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/findlets/minimal/RLTaggedWireHitCreator.h>

#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCRLWireHit.h>

#include <vector>

using namespace Belle2;
using namespace TrackFindingCDC;

std::string RLTaggedWireHitCreator::getDescription()
{
  return "Use all not-taken stereo hits and output them as right and left hypothesis.";
}

void RLTaggedWireHitCreator::apply(std::vector<CDCWireHit>& inputWireHits, std::vector<CDCRLWireHit>& outputRLWireHits)
{
  // Initialize the RL hits
  for (const CDCWireHit& wireHit : inputWireHits) {
    if (not wireHit->hasTakenFlag() and not wireHit.isAxial()) {
      wireHit->unsetAssignedFlag();

      for (ERightLeft rlInfo : {ERightLeft::c_Left, ERightLeft::c_Right}) {
        outputRLWireHits.emplace_back(&wireHit, rlInfo);
      }
    }
  }
}
