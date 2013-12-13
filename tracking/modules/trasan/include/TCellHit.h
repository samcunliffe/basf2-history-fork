//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TCellHit.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a cell hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TCellHit_FLAG_
#define TCellHit_FLAG_

#include <string>
#include "CLHEP/Geometry/Point3D.h"

#define CellHitLeft                0
#define CellHitRight               1

//...Masks for state()...

#define CellHitTimeValid      1048576
#define CellHitChargeValid    2097152
#define CellHitFindingValid   4194304
#define CellHitFittingValid   0x40000000
#define CellHitAxial                8
#define CellHitStereo              48

#define CellHitPatternLeft             256
#define CellHitPatternRight            512
#define CellHitIsolated               1024
#define CellHitContinuous             2048
#define CellHitNeighborHit              12
#define CellHitNeighborPatternMask 0x7f000
#define CellHitNeighborHit0           4096
#define CellHitNeighborHit1           8192
#define CellHitNeighborHit2          16384
#define CellHitNeighborHit3          32768
#define CellHitNeighborHit4          65536
#define CellHitNeighborHit5         131072
#define CellHitNeighborHit6         262144

#define CellHitLocked               128
#define CellHitUsed                  64
#define CellHitLeftMask               1
#define CellHitRightMask              2
#define CellHitMultiTrack       8388608
#define CellHitConformalFinder 16777216
#define CellHitCurlFinder      33554432
#define CellHitClustFinder     67108864
#define CellHitHoughFinder     67108864
#define CellHitTrackManager   134217728
#define CellHitInvalidForFit  268435456
#define CellHitFastFinder     536870912

namespace Belle2 {
class CDCHit;
class CDCSimHit;
class MCParticle;
}

namespace Belle {

class TCell;

/// A class to represent a wire hit in CDC.
class TCellHit {

  public:

    /// Constructor.
    TCellHit(const TCell &,
             unsigned indexCDCHit = 0,
             unsigned indexCDCSimHit = 0,
             unsigned indexMCParticle = 0,
             float driftLeft = 0,
             float driftLeftError = 0,
             float driftRight = 0,
             float driftRightError = 0,
             int mcLRflag=0,
             float fudgeFacgtor = 1);

    /// Destructor
    virtual ~TCellHit();

  public:// Selectors

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

    /// returns mc left/right information
    int mcLR(void) const;

    /// returns a pointer to a TWire.
    virtual const TCell & cell(void) const;

    /// returns state.
    unsigned state(void) const;

    /// returns drift distance.
    float drift(unsigned) const;

    /// returns drift distance error.
    float dDrift(unsigned) const;

    /// returns drift distance.
    float drift(void) const;

    /// returns drift distance error.
    float dDrift(void) const;

    /// returns position in the middle of wire. z is always zero, however.
    const HepGeom::Point3D<double>  & xyPosition(void) const;

    /// returns left position. z is always zero.
    HepGeom::Point3D<double>  position(unsigned) const;

    /// assigns a pointer to a TTrack.
    const void * track(void) const;

    /// returns sequential Length in one segment : this parameter is
    /// used in TCurlFinder now.
    unsigned sequence(void) const;

    /// returns an index to CDCHit.
    unsigned iCDCHit(void) const;

    /// returns an index to CDCSimHit.
    unsigned iCDCSimHit(void) const;

    /// returns an index to MCParticle.
    unsigned iMCParticle(void) const;

    /// Access to CDCHit.
    const Belle2::CDCHit * hit(void) const;

    /// Access to CDCSimHit.
    const Belle2::CDCSimHit * simHit(void) const;

    /// Access to MCParticle.
    const Belle2::MCParticle * mcParticle(void) const;

  public:// Modifiers
    /// sets state. Meaning of bits are written below.
    unsigned state(unsigned newState);

    /// sets state. Meaning of bits are written below. (tmp)
    unsigned state(unsigned newState) const;

    /// assigns a pointer to a TTrack.
    const void * track(const void *);

    /// assigns a pointer to a TTrack. (tmp)
    const void * track(const void *) const;

    /// sets sequential length in one segment : this parameter is used in TCurlFinder now.
    unsigned sequence(unsigned) const;

    /// Sets drift time.
    void setDriftTime(double driftTime, unsigned i);

  public:// Static utility functions

    /// Sort function.
    static int sortById(const TCellHit ** a, const TCellHit ** b);

  private:

    int _mcLR;

    mutable unsigned _state;
    float _drift[2];         // 0:left, 1:right
    float _driftError[2];
    const TCell & _cell;
    const HepGeom::Point3D<double>  & _xyPosition;
    mutable const void * _track;
    mutable unsigned _sequentialLength;

    /// Index to CDCHit array
    unsigned _iCDCHit;

    /// Index to CDCSimHit array
    unsigned _iCDCSimHit;

    /// Index to MCParticle array
    unsigned _iMCParticle;

    // _state bit definition

    // Pre-detemined
    //   20 : drift time valid
    //   21 : charge(dE/dx) valid
    //   22 : valid for finding
    //   30 : valid for fit
    //    3 : axial hit
    // 4, 5 : stereo hit

    // Hit pattern
    //    8 : hit pattern left
    //    9 : hit pattern right
    //   10 : isolated hit pattern
    //   11 : continuous hit pattern
    //   12 : neighbor hit 0
    //   13 : neighbor hit 1
    //   14 : neighbor hit 2
    //   15 : neighbor hit 3
    //   16 : neighbor hit 4
    //   17 : neighbor hit 5
    //   18 : neighbor hit 6

    // Tracking results
    //    7 : locked
    //    6 : used for tracking
    //    0 : left hit
    //    1 : right hit
    //   23 : shared by multi-track
    //   24 : found by conforaml finder
    //   25 : found by curl finder
    //   26 : found by clust finder
    //   27 : found by track manager
    //   28 : NOT valid for fitting

    // Fitting results
};

//-----------------------------------------------------------------------------

inline
int TCellHit::mcLR(void) const {
    return _mcLR;
}

inline
const TCell &
TCellHit::cell(void) const {
    return _cell;
}

inline
unsigned
TCellHit::state(void) const {
    return _state;
}

inline
float
TCellHit::drift(unsigned i) const {
    if (i) return _drift[1];
    return _drift[0];
}

inline
float
TCellHit::dDrift(unsigned i) const {
    if (i) return _driftError[1];
    return _driftError[0];
}

inline
float
TCellHit::drift(void) const {
    return (_drift[0] + _drift[1]) / 2.;
}

inline
float
TCellHit::dDrift(void) const {
    return (_driftError[0] + _driftError[1]) / 2.;
}

inline
const HepGeom::Point3D<double>  &
TCellHit::xyPosition(void) const {
    return _xyPosition;
}

inline
unsigned
TCellHit::state(unsigned i) {
    return _state = i;
}

inline
unsigned
TCellHit::state(unsigned i) const {
    return _state = i;
}

inline
const void *
TCellHit::track(void) const {
    return _track;
}

inline
const void *
TCellHit::track(const void * a) {
    return _track = a;
}

inline
const void *
TCellHit::track(const void * a) const {
    return _track = a;
}

inline
unsigned
TCellHit::sequence(void) const {
    return _sequentialLength;
}

inline
unsigned
TCellHit::sequence(unsigned a) const {
    return _sequentialLength = a;
}

inline
void
TCellHit::setDriftTime(double driftTime, unsigned i) {
    if(i) _drift[1] = driftTime;
    else _drift[0] = driftTime;
}

inline
unsigned
TCellHit::iCDCHit(void) const {
    return _iCDCHit;
}

inline
unsigned
TCellHit::iCDCSimHit(void) const {
    return _iCDCSimHit;
}

inline
unsigned
TCellHit::iMCParticle(void) const {
    return _iMCParticle;
}

} // namespace Belle

#endif /* TCellHit_FLAG_ */
