//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : DisplayDrawingArea.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDC_SHORT_NAMES
#define TRGCDCDisplayDrawingArea_INLINE_DEFINE_HERE

#include <iostream>
#include <pangomm/init.h>
#include "trg/cdc/DisplayDrawingAreaRphi.h"

using namespace std;

namespace Belle2 {

TRGCDCDisplayDrawingArea::TRGCDCDisplayDrawingArea(int size,
                                                   double outerR)
    : _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false) {
}

TRGCDCDisplayDrawingArea::~TRGCDCDisplayDrawingArea() {
}

} // namespace Belle2

#endif
