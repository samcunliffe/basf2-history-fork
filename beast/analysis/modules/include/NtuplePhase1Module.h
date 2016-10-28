/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - 2016 Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NTUPLEPHASE1MODULE_H
#define NTUPLEPHASE1MODULE_H

#include <framework/core/Module.h>
#include <string>
#include <beast/analysis/modules/SuperKEKBTree.h>
#include <beast/analysis/modules/BEASTTree.h>
#include "TFile.h"
#include "TChain.h"
#include "Riostream.h"

namespace Belle2 {

  /**
   * Read SKB PVs, simulated measurements of BEAST sensors, and write scaled simulated Ntuple in BEAST phase 1 data format
   */
  class NtuplePhase1Module : public Module {

  public:

    /**
     * Constructor
     */
    NtuplePhase1Module();

    /**
     * Destructor
     */
    virtual ~NtuplePhase1Module();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize();

    /**
     * Called when entering a new run.
     * Set run dependent things like run header parameters, alignment, etc.
     */
    virtual void beginRun();

    /**
     * Event processor.
     */
    virtual void event();

    /**
     * End-of-run action.
     * Save run-related stuff, such as statistics.
     */
    virtual void endRun();

    /**
     * Termination action.
     * Clean-up, close files, summarize statistics, etc.
     */
    virtual void terminate();

    /**
     * Prints module parameters.
     */
    void printModuleParams() const;

  private:

    std::vector<Int_t> m_input_ts; /**< time stamp start and stop */

    std::vector<std::string> m_inputFileNames; /**< list of file names */
    std::vector<Float_t> m_input_LT_DIA_dose; /**< list of DIA LT dose */
    std::vector<Float_t> m_input_HT_DIA_dose; /**< list of DIA HT dose */
    std::vector<Float_t> m_input_LC_DIA_dose; /**< list of DIA LC dose */
    std::vector<Float_t> m_input_HC_DIA_dose; /**< list of DIA HC dose */
    std::vector<Float_t> m_input_LB_DIA_dose; /**< list of DIA LB dose */
    std::vector<Float_t> m_input_HB_DIA_dose; /**< list of DIA HB dose */

    std::vector<Float_t> m_input_LT_PIN_dose; /**< list of PIN LT dose */
    std::vector<Float_t> m_input_HT_PIN_dose; /**< list of PIN HT dose */
    std::vector<Float_t> m_input_LC_PIN_dose; /**< list of PIN LC dose */
    std::vector<Float_t> m_input_HC_PIN_dose; /**< list of PIN HC dose */
    std::vector<Float_t> m_input_LB_PIN_dose; /**< list of PIN LB dose */
    std::vector<Float_t> m_input_HB_PIN_dose; /**< list of PIN HB dose */

    std::vector<Float_t> m_input_LT_BGO_dose; /**< list of BGO LT dose */
    std::vector<Float_t> m_input_HT_BGO_dose; /**< list of BGO HT dose */
    std::vector<Float_t> m_input_LC_BGO_dose; /**< list of BGO LC dose */
    std::vector<Float_t> m_input_HC_BGO_dose; /**< list of BGO HC dose */
    std::vector<Float_t> m_input_LB_BGO_dose; /**< list of BGO LB dose */
    std::vector<Float_t> m_input_HB_BGO_dose; /**< list of BGO HB dose */

    std::vector<Float_t> m_input_LT_HE3_rate; /**< list of HE3 LT rate */
    std::vector<Float_t> m_input_HT_HE3_rate; /**< list of HE3 HT rate */
    std::vector<Float_t> m_input_LC_HE3_rate; /**< list of HE3 LC rate */
    std::vector<Float_t> m_input_HC_HE3_rate; /**< list of HE3 HC rate */
    std::vector<Float_t> m_input_LB_HE3_rate; /**< list of HE3 LB rate */
    std::vector<Float_t> m_input_HB_HE3_rate; /**< list of HE3 HB rate */

    std::vector<Float_t> m_input_LT_CSI_dose; /**< list of CSI LT dose */
    std::vector<Float_t> m_input_HT_CSI_dose; /**< list of CSI HT dose */
    std::vector<Float_t> m_input_LC_CSI_dose; /**< list of CSI LC dose */
    std::vector<Float_t> m_input_HC_CSI_dose; /**< list of CSI HC dose */
    std::vector<Float_t> m_input_LB_CSI_dose; /**< list of CSI LB dose */
    std::vector<Float_t> m_input_HB_CSI_dose; /**< list of CSI HB dose */

    std::vector<Float_t> m_input_LT_CSI_rate; /**< list of CSI LT rate */
    std::vector<Float_t> m_input_HT_CSI_rate; /**< list of CSI HT rate */
    std::vector<Float_t> m_input_LC_CSI_rate; /**< list of CSI LC rate */
    std::vector<Float_t> m_input_HC_CSI_rate; /**< list of CSI HC rate */
    std::vector<Float_t> m_input_LB_CSI_rate; /**< list of CSI LB rate */
    std::vector<Float_t> m_input_HB_CSI_rate; /**< list of CSI HB rate */

    std::vector<Float_t> m_input_I_HER; /**< HER current and error */
    std::vector<Float_t> m_input_I_LER; /**< LER current and error */

    std::vector<Float_t> m_input_P_LER; /**< LER pressure and error */
    std::vector<Float_t> m_input_P_HER; /**< HER pressure and error */

    std::vector<Float_t> m_input_bunchNb_LER; /**< LER bunch number and error */
    std::vector<Float_t> m_input_bunchNb_HER; /**< HER bunch number and error */

    std::vector<Float_t> m_input_sigma_LER; /**< LER beam size and errors */
    std::vector<Float_t> m_input_sigma_HER; /**< HER beam size and errors */

    Double_t m_input_data_bunchNb_LER; /**< LER bunch number and error */
    Double_t m_input_data_bunchNb_HER; /**< HER bunch number and error */

    TTree* m_treeBEAST = 0;   /**< BEAST tree pointer */
    TChain* m_tree = 0;            /**< tree pointer */
    std::string m_inputFileName;  /**< input file name */
    std::string m_outputFileName; /**< output file name */
    TFile* m_file;        /**< TFile */
    SuperKEKB::SuperKEKBTree m_skb;   /**< SKB tree structure */
    BEAST::BEASTTree m_beast;

    int m_numEntries;     /**< number of ntuple entries */
    int m_entryCounter;   /**< entry counter */
    unsigned m_numEvents = 0;      /**< number of events (tree entries) in the sample */
    unsigned m_eventCount = 0;     /**< current event (tree entry) */
    unsigned m_exp = 0;            /**< Date of the day */
  };

} // Belle2 namespace

#endif
