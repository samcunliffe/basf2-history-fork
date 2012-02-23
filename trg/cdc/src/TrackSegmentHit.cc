//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegmentHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track segment hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/cdc/TrackSegmentHit.h"
#include "trg/cdc/Wire.h"

using namespace std;

namespace Belle2 {

TRGCDCTrackSegmentHit::TRGCDCTrackSegmentHit(const TRGCDCWire & w)
    : TCWHit(w),
      _state(0) {
}

TRGCDCTrackSegmentHit::TRGCDCTrackSegmentHit(const TRGCDCWire & w,
					     float driftLeft,
					     float driftLeftError,
					     float driftRight,
					     float driftRightError,
					     float fudgeFactor)
    : TCWHit(w,
	     0,
	     0,
	     driftLeft,
	     driftLeftError,
	     driftRight,
	     driftRightError,
	     fudgeFactor),
      _state(0) {
}

TRGCDCTrackSegmentHit::~TRGCDCTrackSegmentHit() {
}

int
TRGCDCTrackSegmentHit::sortByWireId(const TRGCDCTrackSegmentHit ** a,
				    const TRGCDCTrackSegmentHit ** b) {
    if ((* a)->wire().id() > (* b)->wire().id())
        return 1;
    else if ((* a)->wire().id() == (* b)->wire().id())
        return 0;
    else
        return -1;
}

void
TRGCDCTrackSegmentHit::dump(const std::string & message,
			    const std::string & prefix) const {
    cout << prefix;
    cout << "state=" << _state << endl;
    TCWHit::dump(message, prefix);
}

} // namespace Belle2
