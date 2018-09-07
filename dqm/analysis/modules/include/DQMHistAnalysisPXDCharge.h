//+
// File : DQMHistAnalysisPXDCharge.h
// Description : DAQM Analysis for PXD Common Modes
//
// Author : Bjoern Spruck, University Mainz
// Date : 2018
//-

#pragma once

#ifdef _BELLE2_EPICS
// EPICS
#include "cadef.h"
#endif

#include <framework/core/Module.h>

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <vxd/geometry/SensorInfoBase.h>

#include <TF1.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TLine.h>

namespace Belle2 {
  /*! DQM Histogram Analysis for PXD Cluster Charge */

  class DQMHistAnalysisPXDChargeModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPXDChargeModule();
  private:

    //! Module functions to be called from main process
    void initialize(void) override final;

    //! Module functions to be called from event process
    void beginRun(void) override final;
    void event(void) override final;
    void endRun(void) override final;
    void terminate(void) override final;

    // Data members
    std::string m_histogramDirectoryName;
    std::string m_pvPrefix;
    double m_rangeLow;
    double m_rangeHigh;

    //IDs of all PXD Modules to iterate over
    std::vector<VxdID> m_PXDModules;

    TF1* m_fLandau;// only one fit function for all Landaus
    TF1* m_fMean;// Fit the Mean for all modules
    TH1F* m_hCharge;
    TCanvas* m_cCharge;
//     TLine* m_line1, *m_line2, *m_line3;

    TH1* findHistLocal(TString& a);

#ifdef _BELLE2_EPICS
    chid  mychid[2];// two PVs, Mean and maximum deviation
#endif
  };
} // end namespace Belle2

