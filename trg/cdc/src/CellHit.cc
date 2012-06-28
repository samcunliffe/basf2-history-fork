//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CellHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "framework/datastore/StoreArray.h"
#include "generators/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/dataobjects/CDCHit.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/CellHit.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/WireHitMC.h"    // to be removed

using namespace std;

namespace Belle2 {

TRGCDCCellHit::TRGCDCCellHit(const TRGCDCCell & w,
			     unsigned indexCDCHit,
			     unsigned indexCDCSimHit,
			     unsigned indexMCParticle,
			     float driftLeft,
			     float driftLeftError,
			     float driftRight,
			     float driftRightError,
			     float fudgeFactor)
    :  _state(0),
       _cell(w),
       _xyPosition(w.xyPosition()),
       _track(0),
       _iCDCHit(indexCDCHit),
       _iCDCSimHit(indexCDCSimHit),
       _iMCParticle(indexMCParticle) {
//  w.hit(this); // set by TRGCDC
    _drift[0] = driftLeft;
    _drift[1] = driftRight;
    _driftError[0] = driftLeftError * fudgeFactor;
    _driftError[1] = driftRightError * fudgeFactor;
    if (w.axial()) _state |= CellHitAxial;
    else           _state |= CellHitStereo;
}

TRGCDCCellHit::~TRGCDCCellHit() {
}

void
TRGCDCCellHit::dump(const std::string & msg,
		    const std::string & pre) const {
    std::cout << pre;
    std::cout << _cell.name();
    if (msg.find("state") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        std::cout << ",state";
        TRGUtil::bitDisplay(_state);
        if (track()) std::cout << ",trk ";
    }
    if (msg.find("position") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
	cout << ",xy=" << _xyPosition;
    }
    if (msg.find("drift") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        if (_state & CellHitLeftMask) std::cout << ", L";
        if (_state & CellHitRightMask) std::cout << ", R";
        std::cout << ",dl " << _drift[0] << "+-" << _driftError[0];
        std::cout << ",dr " << _drift[1] << "+-" << _driftError[1];
    }
    if (msg.find("mc") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
        std::cout << ",hep ";
//         if (mc()) {
//             if (mc()->hep()) std::cout << mc()->hep()->id();
//             else std::cout << "0";
//         }
//         else {
//             std::cout << "0";
//         }
    }
    std::cout << std::endl;
}

Point3D
TRGCDCCellHit::position(unsigned lr) const {
    static const HepGeom::Vector3D<double> HepXHat(1.0, 0.0, 0.0);
    static const HepGeom::Vector3D<double> HepYHat(0.0, 1.0, 0.0);
    static const HepGeom::Vector3D<double> HepZHat(0.0, 0.0, 1.0);

    //...Left...
    if (lr == CellHitLeft) {
        return _xyPosition
            - _drift[CellHitLeft] * HepZHat.cross(_xyPosition.unit());
    }

    //...Right case...
    else {
        return _xyPosition
            + _drift[CellHitRight] * HepZHat.cross(_xyPosition.unit());
    }
}

int
TRGCDCCellHit::sortById(const TRGCDCCellHit ** a, const TRGCDCCellHit ** b) {
    if ((* a)->_cell.id() > (* b)->_cell.id())
        return 1;
    else if ((* a)->_cell.id() == (* b)->_cell.id())
        return 0;
    else
        return -1;
}

const CDCHit *
TRGCDCCellHit::hit(void) const {
    StoreArray<CDCHit> CDCHits("CDCHits");
    return CDCHits[_iCDCHit];
}

const CDCSimHit *
TRGCDCCellHit::simHit(void) const {
    StoreArray<CDCSimHit> CDCHits("CDCSimHits");
    return CDCHits[_iCDCSimHit];
}

const MCParticle *
TRGCDCCellHit::mcParticle(void) const {
    StoreArray<MCParticle> mcParticles("MCParticles");
    return mcParticles[_iMCParticle];
}

} // namespace Belle2
