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

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <vxd/dataobjects/VxdID.h>

#include <vector>
#include <TF1.h>
#include <TCanvas.h>
#include <TLine.h>
#include <TGraphErrors.h>
#include <TFile.h>

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

    /**
     * Get histogram by its name.
     * @param histoname The name of the histogram.
     * @return The pointer to the histogram, or nullptr if not found.
     */
    TH1* GetHisto(TString histoname);

    // Data members
    //! name of histogram directory
    std::string m_histogramDirectoryName;
    //! prefix for EPICS PVs
    std::string m_pvPrefix;
    //! fit range lo edge for landau
    double m_rangeLow;
    //! fit range hi edge for landau
    double m_rangeHigh;
    //! fit range before peak
    double m_peakBefore;
    //! fit range after peak
    double m_peakAfter;

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
    //! Final Canvases for Fit and Ref
    std::map<VxdID, TCanvas*> m_cChargeMod {};
    //! Histogram for TrackedClusters
    TH1F* m_hTrackedClusters = nullptr;
    //! Final Canvas for TrackedClusters
    TCanvas* m_cTrackedClusters = nullptr;

    TLine* m_line_up{}, *m_line_mean{}, *m_line_low{};

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;
    /** Whether to use the color code for warnings and errors. */
    bool m_color = true;

#ifdef _BELLE2_EPICS
    //! Place for EPICS PVs, Mean and maximum deviation
    std::vector <chid> mychid;
#endif
  };
} // end namespace Belle2

