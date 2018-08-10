//+
// File : Root2Raw.h
// Description : Module to convert recorded ROOT raw data into raw data file
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 4 - Jun - 2014
//-

#ifndef ROOT2RAW_H
#define ROOT2RAW_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <framework/core/Module.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <rawdata/dataobjects/RawPXD.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <rawdata/dataobjects/RawARICH.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawFTSW.h>

<<< <<< < HEAD
#include <rawdata/dataobjects/RawHeader_latest.h>
#include <rawdata/dataobjects/RawTrailer_latest.h>
== == == =
#include <rawdata/dataobjects/RawHeader_v1.h>
#include <rawdata/dataobjects/RawTrailer_v1.h>
  >>> >>> > feature / daq - restruct

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>


#define MAXEVTSIZE 4000000


  namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Root2RawModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Root2RawModule();
    virtual ~Root2RawModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();

    //!
    virtual void endRun();

    //!
    virtual void terminate();

  protected :
    //!
    int m_file;
    std::string m_filename;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
