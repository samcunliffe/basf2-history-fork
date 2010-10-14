//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerLayer.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire layer.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef CDCTriggerLayer_FLAG_
#define CDCTriggerLayer_FLAG_

#include <string>
#include <vector>

#ifdef CDCTRIGGER_SHORT_NAMES
#define CTLayer CDCTriggerLayer
#endif

namespace Belle2 {

class CDCTriggerWire;

/// A class to represent a wire layer.
class CDCTriggerLayer : public std::vector<CDCTriggerWire *> {

  public:
    /// Constructor.
    CDCTriggerLayer(unsigned id,
		    unsigned superLayerId,
		    unsigned localLayerId,
		    unsigned axialStereoLayerId,
		    unsigned axialStereoSuperLayerId,
		    float offset,
		    int nShifts,
		    float cellSize,
		    unsigned nWires,
		    float innerRadius,
		    float outerRadius);

    /// Constructor for track segments.
    CDCTriggerLayer(unsigned id,
		    const CDCTriggerWire & w);

    /// Destructor
    virtual ~CDCTriggerLayer();

  public:// Selectors
    /// returns id.
    unsigned id(void) const;

    /// returns super layer id.
    unsigned superLayerId(void) const;

    /// returns local layer id in a super layer.
    unsigned localLayerId(void) const;

    /// returns id of axial or stereo id.
    unsigned axialStereoLayerId(void) const;

    /// returns id of axial or stereo super layer id.
    unsigned axialStereoSuperLayerId(void) const;

    /// returns \# of wires.
    unsigned nWires(void) const;

    /// returns a pointer to a wire. 'id' can be negative or 'id' can be greater than 'nWires()'.
    const CDCTriggerWire * const wire(int id) const;

    /// returns true if this is an axial layer.
    bool axial(void) const;

    /// returns true if this is a stereo layer.
    bool stereo(void) const;

    /// returns offset.
    float offset(void) const;

    /// returns shifts. (non-zero for stereo layers)
    int nShifts(void) const;

    /// returns cell size.
    float cellSize(void) const;

    /// sets and returns inner radius.
    float innerRadius(void) const;

    /// sets and returns outer radius.
    float outerRadius(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
	      const std::string & prefix = std::string("")) const;

    /// returns "A" or "U" or "V" depending on stereo type.
    const std::string stereoType(void) const;

  private:
    const unsigned _id;
    const unsigned _superLayerId;
    const unsigned _localLayerId;
    const unsigned _axialStereoLayerId;
    const unsigned _axialStereoSuperLayerId;
    const float _offset;
    const int _nShifts;
    const float _cellSize;
    const unsigned _nWires;
    float _innerRadius;
    float _outerRadius;
};

//-----------------------------------------------------------------------------

#ifdef CDCTriggerLayer_NO_INLINE
#define inline
#else
#undef inline
#define CDCTriggerLayer_INLINE_DEFINE_HERE
#endif

#ifdef CDCTriggerLayer_INLINE_DEFINE_HERE

inline
unsigned
CDCTriggerLayer::id(void) const {
    return _id;
}

inline
unsigned
CDCTriggerLayer::superLayerId(void) const {
    return _superLayerId;
}

inline
unsigned
CDCTriggerLayer::localLayerId(void) const {
    return _localLayerId;
}

inline
float
CDCTriggerLayer::offset(void) const {
    return _offset;
}

inline
int
CDCTriggerLayer::nShifts(void) const {
    return _nShifts;
}

inline
unsigned
CDCTriggerLayer::nWires(void) const {
    return _nWires;
}

inline
bool
CDCTriggerLayer::axial(void) const {
    if (! _nShifts) return true;
    return false;
}

inline
bool
CDCTriggerLayer::stereo(void) const {
    if (_nShifts) return true;
    return false;
}

inline
unsigned
CDCTriggerLayer::axialStereoLayerId(void) const {
    return _axialStereoLayerId;
}

inline
unsigned
CDCTriggerLayer::axialStereoSuperLayerId(void) const {
    return _axialStereoSuperLayerId;
}

inline
float
CDCTriggerLayer::cellSize(void) const {
    return _cellSize;
}

inline
const std::string
CDCTriggerLayer::stereoType(void) const {
    if (axial())
	return "A";
    else if (nShifts() > 0)
	return "U";
    else
	return "V";
}

#endif

#undef inline

} // namespace Belle2

#endif /* CDCTriggerLayer_FLAG_ */
