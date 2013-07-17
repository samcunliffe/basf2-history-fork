//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCFrontEnd_FLAG_
#define TRGCDCFrontEnd_FLAG_

#include <fstream>
#include <string>
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCFrontEnd TRGCDCFrontEnd
#endif

namespace Belle2 {

class TRGCDCWire;

/// A class to represent a CDC front-end board
class TRGCDCFrontEnd
    : public TRGBoard,
      public std::vector<const TRGCDCWire *> {
    
  public:// enum
    enum boardType {
        innerInside  = 0,       // inner of the inner-most super layer
        innerOutside = 1,       // outer of the inner-most super layer
	outerInside  = 2,       // inner of the ouer super layer
	outerOutside = 3,       // outer of the ouer super layer
        unknown = 999
    };

  public:

    /// Constructor.
    TRGCDCFrontEnd(const std::string & name,
		   boardType type,
		   const TRGClock & systemClock,
		   const TRGClock & dataClock,
		   const TRGClock & userClock);

    /// Destructor
    virtual ~TRGCDCFrontEnd();

  public:// Selectors

    /// returns version.
    static std::string version(void);

    /// returns type.
    boardType type(void) const;

  public:// Modifiers

    /// simulates firmware.
    void simulate(void);

  public:// VHDL utilities

    /// Makes bit pattern using input bit pattern for the inner FE.
    static TRGState packerInnerInside(const TRGState & input);

    /// Makes bit pattern using input bit pattern for the outer FE.
    static TRGState packerInnerOutside(const TRGState & input);

    /// Makes bit pattern using input bit pattern for the inner FE.
    static TRGState packerOuterInside(const TRGState & input);

    /// Makes bit pattern using input bit pattern for the outer FE.
    static TRGState packerOuterOutside(const TRGState & input);

    /// Unpacks TRGState.
    static void unpackerInnerInside(const TRGState & input,
				    const TRGState & output);

    /// Unpacks TRGState.
    static void unpackerOuterInside(const TRGState & input,
				    const TRGState & output);

    /// Unpacks TRGState.
    static void unpackerOuterOutside(const TRGState & input,
				     const TRGState & output);

    /// make a VHDL component file. Non-zero value will be returned if
    /// errors occured.
    static int implementation(const boardType & type, std::ofstream &);

    /// writes a port map.
    static int implementationPort(const boardType & type, std::ofstream &);

  public:// Configuration

    void push_back(const TRGCDCWire *);

    /// dumps contents. "message" is to select information to
    /// dump. "pre" will be printed in head of each line.
    void dump(const std::string & message = "",
	      const std::string & pre = "") const;

  private:

    /// Board type.
    boardType _type;

    /// Input signal bundle.
    TRGSignalBundle * _isb;

    /// Output signal bundle.
    TRGSignalBundle * _osb;
};

//-----------------------------------------------------------------------------

} // namespace Belle2

#endif /* TRGCDCFrontEnd_FLAG_ */
