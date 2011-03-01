//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackBase.cc
// Section  : TrackBaseing
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/TrackBase.h"
#include "trg/cdc/TrackSegment.h"

using namespace std;

namespace Belle2 {

TRGCDCTrackBase::TRGCDCTrackBase(const string & name, float charge)
    : _name(name),
      _status(0),
      _charge(charge),
      _ts(0) {
    const unsigned n = TRGCDC::getTRGCDC()->nSuperLayers();
    _ts = new vector<const TCTSegment *>[n];
}

TRGCDCTrackBase::~TRGCDCTrackBase() {
    delete[] _ts;
}

void
TRGCDCTrackBase::dump(const string & cmd, const string & pre) const {
    const unsigned n = TRGCDC::getTRGCDC()->nSuperLayers();
    cout << pre;
    if (cmd.find("detail") != string::npos)
        cout << name() << ":" << status() << ":" << _p << ":" << _v << endl;
    cout << pre << "ts:";
    for (unsigned i = 0; i < n; i++) {
        cout << i << " " << _ts[i].size();
        for (unsigned j = 0; j < _ts[i].size(); j++) {
            if (j == 0) cout << "(";
            else cout << ",";
            cout << _ts[i][j]->id();
        }
        if (_ts[i].size())
            cout << ")";
        cout << ":";
    }
    cout << endl;
}

void
TRGCDCTrackBase::append(const TRGCDCTrackSegment * a) {
    _ts[a->superLayerId()].push_back(a);
}

const std::vector<const TRGCDCTrackSegment *> &
TRGCDCTrackBase::trackSegments(unsigned layerId) const {
    return _ts[layerId];
}

} // namespace Belle2
