/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Makoto Uchida                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef CDCUnpackerModule_H
#define CDCUnpackerModule_H

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRawHit.h>
#include <cdc/dataobjects/CDCRawHitWaveForm.h>
#include <rawdata/dataobjects/RawDataBlock.h>

#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>


namespace Belle2 {

  namespace CDC {

    /**
     * CDCUnpackerModule: The CDC Raw Hits Decoder.
     */

    class CDCUnpackerModule : public Module {

    public:
      /**
       * Constructor of the module.
       */
      CDCUnpackerModule();

      /**
       * Destructor of the module.
       */
      virtual ~CDCUnpackerModule();

      /**
       * Initializes the Module.
       */
      virtual void initialize();

      /**
       * Begin run action.
       */

      virtual void beginRun();

      /**
       * Event action (main routine).
       *
       */

      virtual void event();

      /**
       * End run action.
       */
      virtual void endRun();

      /**
       * Termination action.
       */
      virtual void terminate();

      /**
       * Set CDC Packet header
       */
      void setCDCPacketHeader(int* buf)
      {

        printf("buf[0] 0x%8x", buf[0]);
        if ((buf[0] & 0xff000000) == 0x22000000) { // raw data mode.
          m_dataType = 1;
        } else if ((buf[0] & 0xff000000) == 0x20000000) { // suppressed data mode.
          m_dataType = 2;
        } else {
          B2ERROR("Undefined data type");
        }

        m_version = ((buf[0] & 0xff0000) >> 16); // Always zero.
        m_boardId = (buf[0] & 0xffff);
        m_triggerTime = ((buf[1] & 0xffff0000) >> 16);
        m_dataLength = (buf[1] & 0xffff);
        m_triggerNumber = buf[2];

      }

      /**
       * Getter for CDC data mode.
       * 1 for raw data mode, 2 for supressed mode.
       */
      int getDataType()
      {
        return m_dataType;
      }

      /**
       * Getter for trigger time in nsec.
       */

      int getTriggerTime()
      {
        return m_triggerTime;
      }

      /**
       * Getter for data length in byte.
       */

      int getDataLength()
      {
        return m_dataLength;
      }

      /**
       * Getter for trigger number.
       */

      int getTriggerNumber()
      {
        return m_triggerNumber;
      }


      /**
       * Getter for FE board ID.
       */

      int getBoardId()
      {
        return m_boardId;
      }

      /**
       * Load FE channel to cell ID map.
       */
      void loadMap();

      /**
       * Getter of Wire ID.
       */
      const WireID getWireID(int iBoard, int iCh);

      /**
       * Setter for FADC threshold.
       */
    private:

      int m_event; /// Event number.
      int m_fadcThreshold;  /// FADC threshold.
      int m_overflow; /// TDC overflow.
      int m_tdcOffset; /// TDC offset (nsec).


    private:

      /**
       * Data type of CDC data block.
       */
      int m_dataType;

      /**
       * Format version.
       */
      int m_version;

      /**
       * Frontend board ID.
       */
      int m_boardId;

      /**
       * Trigger time.
       */
      int m_triggerTime;

      /**
       * Data length of the CDC data block (in bytes).
       */
      int m_dataLength;

      /**
       * Trigger number.
       */
      int m_triggerNumber;

      /**
       * Number of good blocks.
       */
      int n_nGoodBlocks;

      /**
       * Number of error blocks.
       */
      int m_nErrorBlocks;

      /**
       * Enable/Disable to store CDCRawHit.
       */
      bool m_enableStoreCDCRawHit;

      /**
       * Enable/Disable to print out the data to the terminal.
       */

      bool m_enablePrintOut;


      /**
       * Set/unset relation between CDCHit and CDCRawHit(WaveForm).
       */

      bool m_setRelationRaw2Hit;

      /**
       * Name of the RawCDC dataobject (supressed mode).
       */
      std::string m_rawCDCName;

      /**
       * Name of the CDCRawHit dataobject (supressed mode).
       */
      std::string m_cdcRawHitName;

      /**
       * Name of the CDCRawHit dataobject (raw data mode).
       */
      std::string m_cdcRawHitWaveFormName;

      /**
       * Tree name of the CDCHit object.
       */
      std::string m_cdcHitName;

      /**
       * Name of the assignment map of FE board channel to the cell.
       */
      std::string m_xmlMapFileName;

      /**
       * Relation name between CDCRawHit and CDCHit.
       */
      std::string m_relCDCRawHitToCDCHitName;

      /**
       * Relation name between CDCRawHitWaveForm and CDCHit.
       */

      std::string m_relCDCRawHitWFToCDCHitName;

      /**
       * Short ward buffer of CDC event block.
       */
      std::vector<unsigned short> m_buffer;

      /**
       * Assignment map of FE board channel to the cell.
       * 1st column : board ID , 2nd column : FE channel (0-47).
       */
      WireID m_map[300][48];


    };//end class declaration


  } //end CDC namespace;
} // end namespace Belle2

#endif // CDCUnpackerModule_H

