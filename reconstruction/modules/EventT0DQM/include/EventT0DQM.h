/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ewan Hill  (ehill@mail.ubc.ca)                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <reconstruction/dataobjects/CDCDedxTrack.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreObjPtr.h>
#include <mdst/dataobjects/TRGSummary.h>

#include "TH1F.h"
#include "TString.h"
#include "TDirectory.h"
using std::vector;
using std::string;

#include <framework/dataobjects/EventT0.h>

namespace Belle2 {

  /**
   * This module to design collect the event t0 values base on different detectors and physics processes.
   */

  class EventT0DQMModule : public HistoModule {

  public:

    /** Default constructor */
    EventT0DQMModule();

    /** Destructor */
    virtual ~EventT0DQMModule();

    /** Defination of histograms */
    virtual void defineHisto() override;

    /** Initialize the module */
    virtual void initialize() override;

    /** This method is called for each run */
    virtual void beginRun() override;

    /** This method is called for each event. All processing of the event
     * takes place in this method. */
    virtual void event() override;

    /** This method is called at the end of each run */
    virtual void endRun() override;

    /** End of the event processing. */
    virtual void terminate() override;



  private:


    StoreObjPtr<TRGSummary>    obj_trg_summary;   /**< Trigger Summary data object */
    // L1 timing source from getTimeType() in TRGSummary
    // See ETimingTYpe in mdst/dataobjects/include/TRGSummary.h (but information is obsolete...)
    // (2019/11/11) release-04-00-02, return values from getTimType() are
    // 0=ecltrg, 3=cdctrg, 5=delayed Bhabha, 7=random trigger from gdl, (toptrg is undefined yet)
    int m_L1_timing_src;

    StoreObjPtr<SoftwareTriggerResult> m_TrgResult; /**< Store array for Trigger selection */
    StoreObjPtr<EventT0> m_eventT0 ;  /**< Store array for event t0 */

    TH1F* h_eventT0_ECL_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for ECL and bha bha events wrt the ECL trigger time*/
    TH1F* h_eventT0_CDC_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for CDC and bha bha events wrt the ECL trigger time*/
    TH1F* h_eventT0_TOP_bhabha_L1_ECLTRG{nullptr}; /**< event t0 histogram for TOP and bha bha events wrt the ECL trigger time*/
    TH1F* h_eventT0_ECL_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for ECL and hadronic events wrt the ECL trigger time */
    TH1F* h_eventT0_CDC_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for CDC and hadronic events wrt the ECL trigger time */
    TH1F* h_eventT0_TOP_hadron_L1_ECLTRG{nullptr}; /**< event t0 histogram for TOP and hadronic events wrt the ECL trigger time */
    TH1F* h_eventT0_ECL_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for ECL and mu mu events wrt the ECL trigger time*/
    TH1F* h_eventT0_CDC_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for CDC and mu mu events wrt the ECL trigger time*/
    TH1F* h_eventT0_TOP_mumu_L1_ECLTRG{nullptr};   /**< event t0 histogram for TOP and mu mu events wrt the ECL trigger time*/

    TH1F* h_eventT0_ECL_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for ECL and bha bha events wrt the CDC trigger time*/
    TH1F* h_eventT0_CDC_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for CDC and bha bha events wrt the CDC trigger time*/
    TH1F* h_eventT0_TOP_bhabha_L1_CDCTRG{nullptr}; /**< event t0 histogram for TOP and bha bha events wrt the CDC trigger time*/
    TH1F* h_eventT0_ECL_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for ECL and hadronic events wrt the CDC trigger time */
    TH1F* h_eventT0_CDC_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for CDC and hadronic events wrt the CDC trigger time */
    TH1F* h_eventT0_TOP_hadron_L1_CDCTRG{nullptr}; /**< event t0 histogram for TOP and hadronic events wrt the CDC trigger time */
    TH1F* h_eventT0_ECL_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for ECL and mu mu events wrt the CDC trigger time*/
    TH1F* h_eventT0_CDC_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for CDC and mu mu events wrt the CDC trigger time*/
    TH1F* h_eventT0_TOP_mumu_L1_CDCTRG{nullptr};   /**< event t0 histogram for TOP and mu mu events wrt the CDC trigger time*/

  };

} // Belle2 namespace
