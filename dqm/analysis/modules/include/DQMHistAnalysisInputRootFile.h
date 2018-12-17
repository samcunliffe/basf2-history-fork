//+
// File : DQMHistAnalysisInputRootFile.h
// Description : Input module for DQM Histogram analysis
//
// Author : Boqun Wang
// Date : Jun - 2018
//-

#pragma once

#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <daq/dqm/DqmMemFile.h>
#include <dqm/analysis/modules/DQMHistAnalysis.h>

#include <TFile.h>
#include <TCanvas.h>

#include <string>
#include <map>
#include <vector>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisInputRootFileModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisInputRootFileModule();

    //! Module functions to be called from main process
    void initialize() override final;

    //! Module functions to be called from event process
    void beginRun() override final;
    void event() override final;
    void endRun() override final;
    void terminate() override final;

    // Data members
  private:
    /**
     * Pattern match for histogram name
     * @param pattern Pattern used for matchng the histogram name. Wildcards (* and ?) are supported
     * @param text    Histogram name to be matched
     * @return true, if the pattern matches the text
     */
    bool hname_pattern_match(std::string pattern, std::string text);
    //int m_interval;
    std::string m_input_name;
    TFile* m_file = nullptr;

    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;
    std::map<std::string, TCanvas*> m_cs;
    std::vector<std::string> m_folders;
    std::vector<std::string> m_histograms;

    //! Exp number, Run number
    unsigned int m_expno = 0;
    unsigned int m_runno = 0;
    unsigned int m_count = 0;
  };
} // end namespace Belle2

