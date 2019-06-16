//+
// File : PXDReadRawBonnDAQMatched.h
// Description : Module to Load BonnDAQ file and store it as RawPXD in Data Store
// This is meant for lab use (standalone testing, debugging) without an event builder.
//
// Author : Bjoern Spruck
// Date : 16.05.2019
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawPXD.h>

#include <string>
#include <vector>
#include <map>
#include <stdlib.h>


namespace Belle2 {

  namespace PXD {

    class PXDReadRawBonnDAQMatchedModule : public Module {
      enum {MAXEVTSIZE = 4 * 1024 * 1024 + 256 * 4 + 16};
      // Public functions
    public:

      //! Constructor / Destructor
      PXDReadRawBonnDAQMatchedModule();

    private:

      ~PXDReadRawBonnDAQMatchedModule() override final;

      void initialize() override final;
      void event() override final;
      void terminate() override final;


      // Data members

      // Parallel processing parameters

      //! Event Meta Data
      StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

      //! DHH Data
      StoreArray<RawPXD> m_rawPXD;

      //! File Name
      std::string m_filename;

      std::string m_RawPXDsName;  /**< The name of the StoreArray RawPXDs to create */

      //! Compression Level
      int m_compressionLevel;

      //! No. of sent events
      int m_nread;

      //! buffer
      int* m_buffer;

      //! File handle
      FILE* fh;

      unsigned int m_expNr; ///< set by Param
      unsigned int m_runNr; ///< set by Param
      unsigned int m_subRunNr; ///< set by Param

      void endian_swapper(void* a, unsigned int len);///< swaps memory region, quick and dirty
      int readOneEvent(unsigned int& tnr); ///< Read event and store it in datastore if trigger nr matches
      int read_data(char* data, size_t len); ///< Read amount of data (len bytes) from file to ptr data

      std::map <unsigned int, off_t> m_event_offset; ///< map event nr to offsets
      off_t m_last_offset{0}; ///< last checked file offset
    };

  } // end namespace PXD
} // end namespace Belle2

