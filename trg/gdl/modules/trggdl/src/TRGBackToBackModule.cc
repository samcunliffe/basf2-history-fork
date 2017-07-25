#include "trg/gdl/modules/trggdl/TRGBackToBackModule.h"

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>

using namespace std;
using namespace Belle2;

//this line registers the module with the framework and actually makes it available
//in steering files or the the module list (basf2 -m).
REG_MODULE(TRGBackToBack);

TRGBackToBackModule::TRGBackToBackModule() : Module::Module()
{
  setDescription(
    "Module that returns true, if the back-to-back condition "
    "of track segments in SL 2 is fulfilled (for 2017 cosmic test).\n"
  );

  addParam("hitCollectionName", m_hitCollectionName,
           "Name of the input StoreArray of CDCTriggerSegmentHits.",
           string(""));
}

void
TRGBackToBackModule::initialize()
{
  StoreArray<CDCTriggerSegmentHit>::required(m_hitCollectionName);
}

void
TRGBackToBackModule::event()
{
  StoreArray<CDCTriggerSegmentHit> hits(m_hitCollectionName);
  bool TSinMerger[12] = {false};
  for (int ihit = 0; ihit < hits.getEntries(); ++ihit) {
    if (hits[ihit]->getISuperLayer() == 2) {
      // SegmentID in SuperLayer 2 starts at 320
      // One merger corresponds to 16 segments
      unsigned mergerID = (hits[ihit]->getSegmentID() - 320) / 16;
      TSinMerger[mergerID] = true;
    }
  }
  bool BackToBack = false;
  for (unsigned i = 0; i < 6; ++i) {
    BackToBack |= (TSinMerger[i] && TSinMerger[i + 6]);
  }

  setReturnValue(BackToBack);
}
