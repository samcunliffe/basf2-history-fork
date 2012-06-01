//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : SegmentHit.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track segment hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCSegmentHit_FLAG_
#define TRGCDCSegmentHit_FLAG_

#include "trg/cdc/CellHit.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCSHit TRGCDCSegmentHit
#endif

namespace Belle2 {

class TRGCDCSegment;

/// A class to represent a track segment hit in CDC.
class TRGCDCSegmentHit : public TRGCDCCellHit {

  public:

    /// Constructor.
    TRGCDCSegmentHit(const TRGCDCSegment &);

    /// Destructor
    virtual ~TRGCDCSegmentHit();

    /// destructs all TRGCDCCellHit objects. (Called by TRGCDC)
    static void removeAll(void);

    /// new operator.
    static void * operator new(size_t);

    /// delete operator.
    static void operator delete(void *);

  public:// Selectors

    /// dumps debug information.
    virtual void dump(const std::string & message = std::string(""),
		      const std::string & prefix = std::string("")) const;

    /// returns a pointer to a track segment.
    const TRGCDCSegment & segment(void) const;

    /// returns trigger output. Null will returned if no signal.
    const TRGSignal & timing(void) const;

  public:// Modifiers

  public:// Static utility functions

    /// Sorting funnction.
    static int sortById(const TRGCDCSegmentHit ** a,
			const TRGCDCSegmentHit ** b);

  private:

    /// Keeps all TRGCDCSegmentHit created by new().
    static std::vector<TRGCDCSegmentHit *> _all;

};

//-----------------------------------------------------------------------------

inline
const TRGSignal &
TRGCDCSegmentHit::timing(void) const {
    return cell().timing();
}

} // namespace Belle2

#endif /* TRGCDCSegmentHit_FLAG_ */
