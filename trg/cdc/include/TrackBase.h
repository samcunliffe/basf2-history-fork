//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackBase.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track object in TRGCDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackBase_FLAG_
#define TRGCDCTrackBase_FLAG_

#include <vector>
#include "CLHEP/Vector/LorentzVector.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCTBase TRGCDCTrackBase
#endif

#define TRGCDCTrackBaseType            1
#define TRGCDCCircleType               2
#define TRGCDCLineType                 4
#define TRGCDCTrackType                8
#define TRGCDCSegmentType             16

namespace Belle2 {

class TRGCDCLink;
class TRGCDCFitter;

/// A class to represent a track object in TRGCDC.
class TRGCDCTrackBase {

  public:

    /// Copy constructor
    TRGCDCTrackBase(const TRGCDCTrackBase &);

    /// Constructor
    TRGCDCTrackBase(const std::string & name, double charge);

    /// Destructor
    virtual ~TRGCDCTrackBase();

  public:

    /// returns name.
    std::string name(void) const;

    /// returns status.
    int status(void) const;

    /// returns charge.
    double charge(void) const;

    /// returns momentum vector.
    virtual const CLHEP::Hep3Vector & p(void) const;

    /// returns Pt.
    virtual double pt(void) const;

    /// returns position vector.
    virtual const CLHEP::Hep3Vector & x(void) const;

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
                      const std::string & prefix = std::string("")) const;

    /// returns a vector to track segments.
//     const std::vector<const TRGCDCTrackSegment *> &
//         trackSegments(void) const;
    const std::vector<TRGCDCLink *> & links(void) const;

    /// returns a vector to track segments.
//     const std::vector<const TRGCDCTrackSegment *> &
//         trackSegments(unsigned layerId) const;
    const std::vector<TRGCDCLink *> & links(unsigned layerId) const;

    /// returns true if fitted.
    bool fitted(void) const;

    /// returns object type.
    virtual unsigned objectType(void) const;

    /// returns a pointer to a default fitter.
    const TRGCDCFitter * const fitter(void) const;

  public: // Modifiers

    /// sets and returns name.
    std::string name(const std::string & newName);

    /// appends a link.
    void append(TRGCDCLink *);

    /// appends links.
    void append(const std::vector<TRGCDCLink *> & links);

    /// sets and returns charge.
    double charge(double c);

    /// fits itself by a default fitter. Error was happened if return value is not zero.
    virtual int fit(void);

    /// sets a default fitter.
    const TRGCDCFitter * const fitter(const TRGCDCFitter *);

  public: // Utility functions

    /// calculate closest approach. Error was happened if return value is not zero.
    virtual int approach2D(TRGCDCLink &) const;

  private:

    /// Name.
    std::string _name;

    /// Status.
    int _status;

    /// Charge.
    double _charge;

    /// Momentum.
    CLHEP::Hep3Vector _p;

    /// Position.
    CLHEP::Hep3Vector _x;

    /// Links for each super layer
//  std::vector<const TRGCDCLink *> * _ts;
    std::vector<TRGCDCLink *> * _ts;

    /// Links for all super layers
    std::vector<TRGCDCLink *> _tsAll;

    /// Size of _ts.
    const unsigned _nTs;

    /// Fitter.
    const TRGCDCFitter * _fitter;

    /// Fitting status.
    mutable bool _fitted;

    friend class TRGCDCFitter;
    friend class TRGCDCCircleFitter;
    friend class TRGCDCHelixFitter;
    friend class TRGCDCCircle;
    friend class TRGCDCTrack;
};

//-----------------------------------------------------------------------------

inline
std::string
TRGCDCTrackBase::name(void) const {
    return _name;
}

inline
std::string
TRGCDCTrackBase::name(const std::string & a) {
    return _name = a;
}

inline
int
TRGCDCTrackBase::status(void) const {
    return _status;
}

inline
const CLHEP::Hep3Vector &
TRGCDCTrackBase::p(void) const {
    return _p;
}

inline
const CLHEP::Hep3Vector &
TRGCDCTrackBase::x(void) const {
    return _x;
}

inline
double
TRGCDCTrackBase::pt(void) const {
    return _p.perp();
}

inline
bool
TRGCDCTrackBase::fitted(void) const {
    return _fitted;
}

inline
unsigned
TRGCDCTrackBase::objectType(void) const {
    return TRGCDCTrackBaseType;
}

inline
double
TRGCDCTrackBase::charge(void) const {
    return _charge;
}

inline
double
TRGCDCTrackBase::charge(double a) {
    return _charge = a;
}

inline
const TRGCDCFitter * const
TRGCDCTrackBase::fitter(void) const {
    return _fitter;
}

inline
const TRGCDCFitter * const
TRGCDCTrackBase::fitter(const TRGCDCFitter * a) {
    _fitted = false;
    return _fitter = a;
}

} // namespace Belle2

#endif /* TRGCDCTrackBase_FLAG_ */
