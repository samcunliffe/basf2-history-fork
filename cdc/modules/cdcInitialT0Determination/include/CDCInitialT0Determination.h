/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once
#include <framework/core/Module.h>

#include <cdc/dataobjects/WireID.h>
#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <string>
#include <vector>
#include "TH1.h"
#include "TFile.h"
namespace Belle2 {

  /**
   * determine crude t0.
   */
  class CDCInitialT0DeterminationModule : public Module {
  public:

    /**
     * Constructor.
     */
    CDCInitialT0DeterminationModule();

    /**
     * Destructor.
     */
    virtual ~CDCInitialT0DeterminationModule();

    /**
     * Initializes the Module.
     */
    void initialize() override;

    /**
     * Event action (main routine).
     */

    void event() override;

    /**
     * Termination action, fit t0 and store histograms.
     */
    void terminate() override;

  private:
    TH1D* m_hTDC[56][400] = {{nullptr}}; /**< TDC distribution histo. */
    TH1D* m_hTDCBoard[300] = {nullptr}; /**< T0 distribution of each board. */
    TH1D* m_hT0All = nullptr;   /**< T0 distribution of all channel. */
    double m_t0b[300] = {0};    /**< T0 for each board*/
    double m_t0[56][400] = {{0}}; /**< T0 of each channel*/
    bool m_flag[56][400] = {{false}} ; /**< flag =1 for good, =0 for low statistic or bad fit */
    unsigned short m_tdcMin = 0; /**< Lower boundary TDC histogram. */
    unsigned short m_tdcMax = 8400; /**< Upper boundary TDC histogram. */
    unsigned short m_adcMin = 0;    /**< ADC cut to reject noise. */
    unsigned short m_minEntries = 100; /**< min entries per histo. */
    double m_initT0 = 3579;          /**< initial t0, use int fitting */
    double m_zOffset;               /**< z offset for calculate prop time, it is position of trigger counter, */
    bool m_cosmic;     /**< for cosmic case, tof of upper sector will be negative*/
    bool m_storeFittedHisto; /**< Store fitted histogram or not*/
    std::string m_outputFileName;  /**< output file name of t0 file. */
    std::string m_histoFileName ;  /**<  output file to store TDC histo after fit*/

  };
}
