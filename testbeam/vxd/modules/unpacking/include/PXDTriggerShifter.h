#ifndef PXDTRIGGERFIXERMODULE_H_
#define PXDTRIGGERFIXERMODULE_H_

#include <framework/core/Module.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <rawdata/dataobjects/RawPXD.h>

#include <framework/core/MRUCache.h>

namespace Belle2 {

  namespace PXD {
    /// Module to match PXD and ONSET trigger number which differ by fixed offset
    class PXDTriggerShifterModule: public Module {
    private:
      /// MRU cache size
      enum {MAX_EVENTSHIFT = 100};
      /// MRU cache with last couple of events
      MRUCache<int, TClonesArray> m_previous_events{MAX_EVENTSHIFT};
      /// The store array with RawPXDs to match
      StoreArray<RawPXD> m_storeRaw;
      /// The trigger offset
      int m_offset;

    public:
      PXDTriggerShifterModule();
      void initialize(void);
      void event(void);
      /// Get the DHH and HLT trigger number from RawPXD
      bool getTrigNr(RawPXD& px, unsigned int& innerDHH, unsigned int& outerHLT);
      /// Unpack DHE(C) frame in dataptr
      bool unpack_dhc_frame(void* data, unsigned int& innerDHH, unsigned int& outerHLT);
    };
  }
}
#endif
