//+
// File : RawCDC.h
// Description : pseudo raw data of CDC for test
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef RAWCDC_H
#define RAWCDC_H

#include <framework/datastore/DataStore.h>
#include <daq/dataobjects/RawCOPPER.h>

#include <TObject.h>

namespace Belle2 {

  class RawCDC : public RawCOPPER {
  public:
    //! Default constructor
    RawCDC() {};
    //! Constructor using existing pointer to raw data buffer
    RawCDC(int* buf) :
      RawCOPPER(buf) {};
    //! Destructor
    ~RawCDC() {};

    ClassDef(RawCDC, 1);
  };
}

#endif
