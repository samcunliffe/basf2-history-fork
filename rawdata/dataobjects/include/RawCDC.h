//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWCDC_H
#define RAWCDC_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  /**
   * The Raw CDC class
   * Class for RawCOPPER class data taken by CDC
   * Currently, this class is almost same as RawCOPPER class.
   */

  class RawCDC : public RawCOPPER {
  public:
    //! Default constructor
    RawCDC();
    //! Destructor
    virtual ~RawCDC();
    /// To derive from TObject
    ClassDef(RawCDC, 1);
  };
}

#endif
