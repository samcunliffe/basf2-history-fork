//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGLink.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a serial link between trigger modules
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGLink_FLAG_
#define TRGLink_FLAG_

#include <string>
#include <vector>

namespace Belle2 {

class TRGSignal;
class TRGClock;

/// A class to represent a serial link between trigger modules
class TRGLink {

  public:
    /// Constructor.
    TRGLink(const std::string & name, const TRGClock &);

    /// Destructor
    virtual ~TRGLink();

  public:// Selectors.

    /// returns name.
    const std::string & name(void) const;

    /// returns clock.
    const TRGClock & clock(void) const;

    /// dumps contents. "message" is to select information to dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  public:// Modifiers.

    /// appends a signal to header data.
    void appendHeader(const TRGSignal *);

    /// appends a signal to trailer data.
    void appendTrailer(const TRGSignal *);

    /// appends a signal to data.
    void append(const TRGSignal *);

  private:

    /// Name of a link.
    const std::string _name;

    /// Used clock. 
    const TRGClock * _clock;

    /// Signal header.
    std::vector<const TRGSignal *> _head;

    /// Signal trailer.
    std::vector<const TRGSignal *> _trail;

    /// Signal storage.
    std::vector<const TRGSignal *> _data;
};

//-----------------------------------------------------------------------------

#ifdef TRG_NO_INLINE
#define inline
#else
#undef inline
#define TRGLink_INLINE_DEFINE_HERE
#endif

#ifdef TRGLink_INLINE_DEFINE_HERE

inline
const std::string &
TRGLink::name(void) const {
    return _name;
}

inline
const TRGClock &
TRGLink::clock(void) const {
    return * _clock;
}

inline
void
TRGLink::appendHeader(const TRGSignal * a) {
    _head.push_back(a);
}

inline
void
TRGLink::appendTrailer(const TRGSignal * a) {
    _trail.push_back(a);
}

inline
void
TRGLink::append(const TRGSignal * a) {
    _data.push_back(a);
}

#endif

#undef inline

} // namespace Belle2

#endif /* TRGLink_FLAG_ */
