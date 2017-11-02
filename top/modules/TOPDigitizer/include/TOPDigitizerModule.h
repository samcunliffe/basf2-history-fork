/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Petric, Marko Staric                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <top/dbobjects/TOPSampleTimes.h>
#include <top/dbobjects/TOPCalTimebase.h>
#include <framework/database/DBObjPtr.h>
#include <string>


namespace Belle2 {

  /**
   * TOP digitizer.
   * This module takes hits form G4 simulation (TOPSimHits),
   * applies TTS, T0 jitter and does spatial and time digitization.
   * (QE had been moved to the simulation: applied in SensitiveBar, SensitivePMT)
   * Output to TOPDigits.
   */
  class TOPDigitizerModule : public Module {

  public:

    /**
     * Constructor
     */
    TOPDigitizerModule();

    /**
     * Destructor
     */
    virtual ~TOPDigitizerModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

  private:

    // module steering parameters
    double m_timeZeroJitter = 0;       /**< r.m.s of T0 jitter */
    double m_electronicJitter = 0;     /**< r.m.s of electronic jitter */
    double m_darkNoise = 0;            /**< uniform dark noise (hits per bar) */
    double m_trigT0Sigma = 0;          /**< trigger T0 resolution */
    double m_bunchTimeSep = 0;         /**< time between two bunches */
    double m_ADCx0 = 0; /**< pulse height distribution parameter [ADC counts] */
    double m_ADCp1 = 0; /**< pulse height distribution parameter, must be non-negative */
    double m_ADCp2 = 0; /**< pulse height distribution parameter, must be positive */
    double m_ADCmax = 0; /**< pulse height upper bound of range [ADC counts] */
    double m_pedestalRMS = 0; /**< r.m.s of pedestals [ADC counts]*/
    int m_threshold = 0; /**< pulse height threshold [ADC counts] */
    int m_hysteresis = 0; /**< pulse height threshold hysteresis [ADC counts] */
    int m_thresholdCount = 0; /**< minimal number of samples above threshold */
    bool m_useWaveforms = false; /**< if true, use full waveform digitization */
    bool m_useDatabase = false;  /**< if true, use sample times from database */
    bool m_simulateTTS = true; /**< if true, add TTS to simulated hits */
    bool m_allChannels = false; /**< if true, always make waveforms for all channels */
    unsigned m_storageDepth = 0;           /**< ASIC analog storage depth */

    // other
    TOPSampleTimes m_sampleTimes; /**< equidistant sample times */
    DBObjPtr<TOPCalTimebase>* m_timebase = 0; /**< sample times from database */
    double m_timeMin = 0; /**< time range limit: minimal time */
    double m_timeMax = 0; /**< time range limit: maximal time */



  };

} // Belle2 namespace

