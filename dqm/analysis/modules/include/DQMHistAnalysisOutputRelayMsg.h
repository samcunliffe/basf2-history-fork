//+
// File : DQMHistAnalysisOutputRelayMsg.h
// Description : Output module for DQM Histogram analysis
//
// Author : B. Spruck
// Date : 25 - Mar - 2017
// based on wrok from Tomoyuki Konno, Tokyo Metropolitan Univerisity
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TSocket.h>
#include <string>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisOutputRelayMsgModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisOutputRelayMsgModule();
    virtual ~DQMHistAnalysisOutputRelayMsgModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    // Data members
  private:
    TSocket* m_sock = nullptr;
    int m_port;
    std::string m_hostname;

  };
} // end namespace Belle2

