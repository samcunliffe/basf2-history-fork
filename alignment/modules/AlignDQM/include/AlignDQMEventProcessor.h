#pragma once

#include <tracking/dqmUtils/DQMEventProcessorBase.h>
#include <alignment/modules/AlignDQM/AlignDQMModule.h>

using namespace std;

namespace Belle2 {

  class AlignDQMEventProcessor : public DQMEventProcessorBase {

  public:
    AlignDQMEventProcessor(AlignDQMModule* histoModule, string tracksStoreArrayName, string recoTracksStoreArrayName) :
      DQMEventProcessorBase(histoModule, recoTracksStoreArrayName, tracksStoreArrayName) { }

  private:

    virtual void FillCommonHistograms() override;
  };
}