/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: CDC group                                                *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>

namespace Belle2 {
  /**
   * Database object for Fron-endt electronics params.
   */
  class CDCFEElectronics: public TObject {
  public:

    /**
     * Default constructor
     */
    CDCFEElectronics() {}

    /**
     * Constructor
     */
    CDCFEElectronics(short boardID, short width, short trgDelay, short aTh, short tThMV) : m_boardID(boardID),
      m_widthOfTimeWindow(width),
      m_trgDelay(trgDelay), m_adcThresh(aTh), m_tdcThreshInMV(tThMV)
    {
    }

    /** Getter for width of time window */
    short getBoardID() const
    {
      return m_boardID;
    }

    /** Getter for width of time window */
    short getWidthOfTimeWindow() const
    {
      return m_widthOfTimeWindow;
    }

    /** Getter for trigger delay */
    short getTrgDelay() const
    {
      return m_trgDelay;
    }

    /** Getter for voltage threshold (mV) for timing-signal */
    short getTDCThreshInMV() const
    {
      return m_tdcThreshInMV;
    }

    /** Getter for threshold for FADC */
    short getADCThresh() const
    {
      return m_adcThresh;
    }

  private:
    short m_boardID = 0;            /**< fee board id */
    short m_widthOfTimeWindow = 28; /**< Width of time window (in unit of 32*(TDC resol.)). N.B. This value + 1 is used on FPGA. */
    short m_trgDelay = 132;         /**< Trigger delay (in unit of 32*(TDC resol.)). This value + 1 is used on FPGA. */
    short m_adcThresh = 2;          /**< Threshold for FADC (count) */
    short m_tdcThreshInMV = 3750;   /**< Voltage threshold for timing signal (mV) */

    ClassDef(CDCFEElectronics, 1); /**< ClassDef */
  };

} // end namespace Belle2
