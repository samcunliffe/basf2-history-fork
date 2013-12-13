//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWire.cc
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "tracking/modules/trasan/TCDC.h"
#include "tracking/modules/trasan/TDebug.h"
#include "tracking/modules/trasan/TWire.h"
#include "tracking/modules/trasan/TWireHitMC.h"

#define P3D HepGeom::Point3D<double>

using namespace std;

namespace Belle {

TWire::TWire(unsigned id,
	     unsigned localId,
	     const TLayer & l,
	     const P3D & fp,
	     const P3D & bp)
    : TCell(id, localId, l, fp, bp),
      _mcHits() {
}

TWire::~TWire() {
}

void
TWire::dump(const string & msg, const string & pre) const {
    cout << pre;
    cout << "w " << id();
    cout << ",local " << localId();
    cout << ",layer " << layerId();
    cout << ",super layer " << superLayerId();
    cout << ",local layer " << localLayerId();
    cout << endl;
    if (msg.find("neighbor") != string::npos) {
        for (unsigned i = 0; i < 7; i++)
            if (neighbor(i))
                neighbor(i)->dump("",
				  pre + TDebugUtilities::itostring(i) + "   ");
    }
}
  
const TWire *
TWire::neighbor(unsigned i) const {
    static bool first = false;
    if (first)
        cout << "TWire::neighbor !!! "
	     << "this function is not tested yet"
	     << endl;

    const TCDC & cdc = * TCDC::getTCDC();
    const unsigned layerId = layer().id();
    const unsigned superLayerId = layer().superLayerId();
    const unsigned localLayerId = layer().localLayerId();
    const unsigned nLayers = cdc.superLayer(superLayerId)->size();
    const int local = int(localId());

    if (i == WireInnerLeft || i == WireInnerRight) {
        if (localLayerId == 0)
            return 0;
        if (layer().offset() != 0) {
            if (i == WireInnerLeft)
                return cdc.wire(layerId - 1, local);
            else
                return cdc.wire(layerId - 1, local + 1);
        }
        else {
            if (i == WireInnerLeft)
                return cdc.wire(layerId - 1, local - 1);
            else
                return cdc.wire(layerId - 1, local);
        }
    }
    else if (i == WireLeft || i == WireRight) {
        if (i == WireLeft)
            return cdc.wire(layerId, local - 1);
        else
            return cdc.wire(layerId, local + 1);
    }
    else if (i == WireOuterLeft || i == WireOuterRight) {
        if (localLayerId == (nLayers - 1))
            return 0;
        if (layer().offset() != 0) {
            if (i == WireOuterLeft)
                return cdc.wire(layerId + 1, local);
            else
                return cdc.wire(layerId + 1, local + 1);
        }
        else {
            if (i == WireOuterLeft)
                return cdc.wire(layerId + 1, local - 1);
            else
                return cdc.wire(layerId + 1, local);
        }
    }
    return 0;
}

// int
// TWire::localIdForPlus(void) const {

//     const unsigned li = layerId();

//   //
//   // used in curl finder
//   //
//   // new version by NK
//   //
//   const TCLayer &l = *layer();
//   const int nw = l.nWires();
//   if(_localId+1==nw) return -1;
//   else return _localId;
// #if 0
//     if (ms_smallcell) {
//       if ((li == 1) || (li == 2)) {
//         if (_localId == 127) return -1;
//         else return _localId;
//       }
//       if(_localId == 63) return -1;
//       else return _localId;
//     }
//     else {
//       if (li <= 5) {
//         if(_localId == 63)
//           return -1;
//         else return _localId;
//       }
//     }

//   if(li >= 6 && li <= 8){
//     if(_localId == 79)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 9 && li <= 14){
//     if(_localId == 95)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 15 && li <= 17){
//     if(_localId == 127)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 18 && li <= 22){
//     if(_localId == 143)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 23 && li <= 26){
//     if(_localId == 159)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 27 && li <= 31){
//     if(_localId == 191)
//       return -1;
//     else return _localId;
//   }
  
//   if(li >= 32 && li <= 35){
//     if(_localId == 207)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 36 && li <= 40){
//     if(_localId == 239)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 41 && li <= 44){
//     if(_localId == 255)
//       return -1;
//     else return _localId;
//   }

//   if(li >= 45){
//     if(_localId == 287)
//       return -1;
//     else return _localId;
//   }
// #endif
  
//   return -1;
// }

// int
// TWire::localIdForMinus(void) const {

//     const unsigned li = layerId();

//   //
//   // used in curl finder
//   //
//   // new version ny NK
//   //
//   const TCLayer &l = *layer();
//   const int nw = l.nWires();
//   if(0==_localId) return nw;
//   else return _localId;
// #if 0
//     if (ms_smallcell) {
//       if ((li == 1) || (li == 2)) {
//         if (_localId == 0) return 128;
//         else return _localId;
//       }
//       if(_localId == 63) return -1;
//       else return _localId;
//     }
//     else {
//       if (li <= 5){
//         if(_localId == 0)
//           return 64;
//         else return _localId;
//       }
//     }

//   if(li >= 6 && li <= 8){
//     if(_localId == 0)
//       return 80;
//     else return _localId;
//   }

//   if(li >= 9 && li <= 14){
//     if(_localId == 0)
//       return 96;
//     else return _localId;
//   }
  
//   if(li >= 15 && li <= 17){
//     if(_localId == 0)
//       return 128;
//     else return _localId;
//   }

//   if(li >= 18 && li <= 22){
//     if(_localId == 0)
//       return 144;
//     else return _localId;
//   }

//   if(li >= 23 && li <= 26){
//     if(_localId == 0)
//       return 160;
//     else return _localId;
//   }
  
//   if(li >= 27 && li <= 31){
//     if(_localId == 0)
//       return 192;
//     else return _localId;
//   }
  
//   if(li >= 32 && li <= 35){
//     if(_localId == 0)
//       return 208;
//     else return _localId;
//   }

//   if(li >= 36 && li <= 40){
//     if(_localId == 0)
//       return 240;
//     else return _localId;
//   }

//   if(li >= 41 && li <= 44){
//     if(_localId == 0)
//       return 256;
//     else return _localId;
//   }

//   if(li >= 45){
//     if(_localId == 0)
//       return 288;
//     else return _localId;
//   }
// #endif
  
//   return -1;
// }

// void
// TWire::wirePosition(float z,
//                        HepGeom::Point3D<double> & xy,
//                        HepGeom::Point3D<double> & back,
//                        Vector3D & dir) const {
//     cout << "TWire::wirePosition !!! this function is not test yet"
//               << endl;

//     back = _backwardPosition;

//     //...Check z position...
//     if (! (z > _backwardPosition.z() && z < _forwardPosition.z())) {
//         xy = _xyPosition;
//         dir = _direction;
//     }

//     //...Setup...
//     int wireID = id();
//     float wirePosition[3] = {0., 0., 0.};
//     float dydz = 0;
//     float ybSag = 0;
//     float yfSag = 0;

//     //...Get corrections...
//     if(ms_superb) {
//       double dz = _forwardPosition.z() - _backwardPosition.z();
//       double dzb = (z - _backwardPosition.z()) / dz;
//       double dzf = (_forwardPosition.z() - z) / dz;

//       xy = dzb * _forwardPosition + dzf * _backwardPosition;
//       cout << "f=" << _forwardPosition << endl;
//       cout << "b=" << _backwardPosition << endl;
//       cout << "p" << xy << endl;
//       cout << "z=" << z << " dz=" << dz << " dzb=" << dzb << " dzf=" << dzf << endl;
//       dir = _direction;
//       return;
//     }
//     calcdc_sag3_(& wireID, & z, wirePosition, & dydz, & ybSag, & yfSag);
    
//     //...Wire position...
//     xy.setX((double) wirePosition[0]);
//     xy.setY((double) wirePosition[1]);
//     xy.setZ((double) wirePosition[2]);
//     back.setY((double) ybSag);
//     Vector3D v_aux(_forwardPosition.x() - _backwardPosition.x(),
//                    yfSag - ybSag,
//                    _forwardPosition.z() - _backwardPosition.z());
//     dir = v_aux.unit();

//     return;
// }

void
TWire::clear(void) {
    TCell::clear();

    for (unsigned i = 0; i < _mcHits.size(); i++)
        delete _mcHits[i];
    _mcHits.clear();
}

string
TWire::name(void) const {
    if (axial())
        return string("w") +
            TDebugUtilities::itostring(layerId()) +
            string("-") +
            TDebugUtilities::itostring(localId());
    return string("w") + 
        TDebugUtilities::itostring(layerId()) +
        string("=") +
        TDebugUtilities::itostring(localId());
}

} // namespace Belle
