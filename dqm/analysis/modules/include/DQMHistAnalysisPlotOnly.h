//+
// File : DQMHistAnalysisPlotOnly.h
// Description : Module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 13 - Oct - 2017
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <string>
#include <TCanvas.h>

namespace Belle2 {

  /**
   * The module to plot a list of histograms into canvases.
   */
  class DQMHistAnalysisPlotOnlyModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisPlotOnlyModule();
    virtual ~DQMHistAnalysisPlotOnlyModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:

    /** Parameter list for histograms */
    std::vector< std::vector<std::string>> m_histlist;

    //TH1* findHistLocal(TString a);
    /**
     * Get histogram by its name.
     * @param a The name of the histogram.
     * @return The found histogram, nullptr if not found.
     */
    TH1* GetHisto(TString a);

    /** Parameter list for histograms */
    std::map< std::string, TCanvas*> m_canvasList;

  };
} // end namespace Belle2

