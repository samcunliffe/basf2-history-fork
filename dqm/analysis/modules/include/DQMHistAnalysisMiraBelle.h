/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kindo Haruki                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//DQM
#include <dqm/analysis/modules/DQMHistAnalysis.h>

// Monitoring
#include <dqm/dataobjects/DQMFileMetaData.h>
#include <dqm/dataobjects/MonitoringObject.h>

#include <vector>

#include <TCanvas.h>
#include <TLine.h>
#include <TH2Poly.h>

namespace Belle2 {

  /**
   * Make summary of data quality from reconstruction
   */
  class DQMHistAnalysisMiraBelleModule : public DQMHistAnalysisModule {

  public:

    /**
     * Constructor
     */
    DQMHistAnalysisMiraBelleModule();

    /**
     * Destructor
     */
    virtual ~DQMHistAnalysisMiraBelleModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun() override;

    /**
     * Event processor.
     */
    virtual void event() override;

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun() override;

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate() override;

    /**
    * Find canvas by name
    * @param cname Name of the canvas
    * @return The pointer to the canvas, or nullptr if not found.
    */
    TCanvas* find_canvas(TString cname);

  protected:
    bool m_debug;/**<debug*/
    bool m_enableAlert;/**<Enable alert by base color of canvases*/
    Belle2::DQMFileMetaData* dqm_data = nullptr; /**< DQM file meta data */
    Belle2::MonitoringObject* mon_mumu = nullptr; /**< MonitoringObject for mumu events */
    Belle2::MonitoringObject* mon_dst = nullptr; /**< MonitoringObject for mumu events */

    // TCanvases for mumu
    TCanvas* main = nullptr; /**< TCanvas object for main display */
    TCanvas* resolution = nullptr; /**< TCanvas object for resolution display */
    TCanvas* muon_val = nullptr; /**< TCanvas object for mumu values display */
    // TCanvases for D*
    TCanvas* dst_mass = nullptr; /** TCanvas object for D0 mass, delta mass, and pi0 mass */
    TCanvas* pi_val = nullptr; /** TCanvas object for PID distributions of pi */
    TCanvas* k_val = nullptr; /** TCanvas object for PID distributions of K */
    const double scale = 0.09375; /** Scale factor "signal region" / "sideband", assuming uniform events */

  };

} // Belle2 namespace
