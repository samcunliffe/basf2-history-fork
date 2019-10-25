//+
// File : DQMHistAnalysisSVDEfficiency.h
// Description : An example module for DQM histogram analysis
//
// Author : Giulia Casarosa (PI), Gaetano De Marino (PI)
// Date : 20190428
//-

#pragma once

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <TFile.h>
#include <TPaveText.h>
#include <TCanvas.h>

namespace Belle2 {
  /*! Class definition for the output module of Sequential ROOT I/O */

  class DQMHistAnalysisSVDEfficiencyModule : public DQMHistAnalysisModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DQMHistAnalysisSVDEfficiencyModule();
    virtual ~DQMHistAnalysisSVDEfficiencyModule();

    //! Module functions to be called from main process
    virtual void initialize() override;

    //! Module functions to be called from event process
    virtual void beginRun() override;
    virtual void event() override;
    virtual void endRun() override;
    virtual void terminate() override;

    //parameters
    bool m_printCanvas; /**< if true print the pdf of the canvases */
    float m_effError; /**<error level of the efficiency */
    float m_effWarning; /**< warning level of the efficiency */
    float m_effEmpty; /**<empty level of the efficiency */
    float m_errEffError = 1; /**<error level of the efficiency error*/
    float m_errEffWarning = 0.5; /**< warning level of the efficiency error*/

    //! Data members
  private:

    /** Reference Histogram Root file name */
    std::string m_refFileName;
    /** The pointer to the reference file */
    TFile* m_refFile = nullptr;

    TCanvas* m_cEfficiencyU = nullptr; /**< efficiency U plot canvas */
    TCanvas* m_cEfficiencyV = nullptr; /**< efficiency V plot canvas */
    SVDSummaryPlots* m_hEfficiency = nullptr; /**< efficiency  histo */
    TCanvas* m_cEfficiencyErrU = nullptr; /**<efficiency U error plot canvas */
    TCanvas* m_cEfficiencyErrV = nullptr; /**<efficiency Verror plot canvas */
    SVDSummaryPlots* m_hEfficiencyErr = nullptr; /**< efficiency error histo */

    Int_t findBinY(Int_t layer, Int_t sensor); /**< find Y bin corresponding to sensor, efficiency plot*/

    TPaveText* m_legProblem = nullptr; /**< efficiency plot legend, problem */
    TPaveText* m_legWarning = nullptr; /**< efficiency plot legend, warning */
    TPaveText* m_legNormal = nullptr; /**< efficiency plot legend, normal */
    TPaveText* m_legEmpty = nullptr; /**< efficiency plot legend, empty */

    Int_t m_effUstatus; /**< number representing the status of the efficiency U side */
    Int_t m_effVstatus;/**< number representing the status of the efficiency V side */
    Int_t m_effUErrstatus; /**< number representing the status of the efficiency error U side */
    Int_t m_effVErrstatus; /**< number representing the status of the efficiency error V side */

    //! IDs of all SVD Modules to iterate over
    std::vector<VxdID> m_SVDModules;

  };
} // end namespace Belle2

