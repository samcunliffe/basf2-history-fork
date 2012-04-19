//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"

using namespace std;

namespace Belle2 {

TRGCDCWireHit::TRGCDCWireHit(const TRGCDCWire & w,
			     unsigned indexCDCHit,
			     unsigned indexCDCSimHit,
			     float driftLeft,
			     float driftLeftError,
			     float driftRight,
			     float driftRightError,
			     float fudgeFactor)
    : TCCHit((TRGCDCCell &) w,
	     indexCDCHit,
	     indexCDCSimHit,
	     driftLeft,
	     driftLeftError,
	     driftRight,
	     driftRightError,
	     fudgeFactor) {
}

TRGCDCWireHit::~TRGCDCWireHit() {
}

const TRGCDCWire &
TRGCDCWireHit::wire(void) const {
    return dynamic_cast<const TRGCDCWire &>(cell());
}

int
TRGCDCWireHit::sortByWireId(const TRGCDCWireHit ** a,
			    const TRGCDCWireHit ** b) {
    if ((* a)->cell().id() > (* b)->cell().id())
        return 1;
    else if ((* a)->cell().id() == (* b)->cell().id())
        return 0;
    else
        return -1;
}

} // namespace Belle2
