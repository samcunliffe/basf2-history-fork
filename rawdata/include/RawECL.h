//+
// File : RawCOPPER.h
// Description : Module to handle raw data from COPPER
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef RAWECL_H
#define RAWECL_H

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <rawdata/RawCOPPER.h>
#include <rawdata/RawHeader.h>
#include <rawdata/RawTrailer.h>
#include <framework/datastore/DataStore.h>

#include <TObject.h>

namespace Belle2 {

  class RawECL : public RawCOPPER {
  public:
    //! Default constructor
    RawECL();
    //! Destructor
    virtual ~RawECL();

    ClassDef(RawECL, 1);
  };
}

#endif
