//+
// File : DQMHistAnalysisPXDTrackCharge.h
// Description : DQM Analysis for PXD Tracked Cluster Charges
//
// Author : Bjoern Spruck, University Mainz
// Date : 2019
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <vector>
#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TGraphErrors.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Cluster Charge */

  class DQMHistAnalysisPXDTrackChargeModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor
    DQMHistAnalysisPXDTrackChargeModule();
    //! Destructor
    ~DQMHistAnalysisPXDTrackChargeModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void endRun(void) override final;
    void terminate(void) override final;

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;
    //! fit range lo edge for landau
    double m_rangeLow;
    //! fit range hi edge for landau
    double m_rangeHigh;

    //! IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    //! only one fit function for all Landaus
    TF1* m_fLandau = nullptr;
    //! Fit the Mean for all modules
    TF1* m_fMean = nullptr;
    //! Graph covering all modules
    TGraphErrors* m_gCharge = nullptr;
    //! Final Canvas
    TCanvas* m_cCharge = nullptr;

    TLine* m_line_up{}, *m_line_mean{}, *m_line_low{};
#ifdef _BELLE2_EPICS
    //! Place for EPICS PVs, Mean and maximum deviation
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

