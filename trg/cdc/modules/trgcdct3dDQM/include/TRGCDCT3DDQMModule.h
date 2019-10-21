#ifndef TRCCDCT3DDQMMODULE_h
#define TRCCDCT3DDQMMODULE_h

#include <framework/core/HistoModule.h>
#include <trg/cdc/dataobjects/TRGCDCT3DUnpackerStore.h>
#include <trg/cdc/dataobjects/CDCTriggerTrack.h>
#include <trg/cdc/dataobjects/CDCTriggerSegmentHit.h>
#include <trg/cdc/modules/trgcdct3dUnpacker/TRGCDCT3DUnpackerModule.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <TH2I.h>
#include <TH1I.h>

namespace Belle2 {

  //! DQM module for TRGCDCT3D
  class TRGCDCT3DDQMModule : public HistoModule {

  public:
    //! Costructor
    TRGCDCT3DDQMModule();
    //! Destrunctor
    virtual ~TRGCDCT3DDQMModule() {}

  public:
    //! initialize
    virtual void initialize();
    //! begin Run
    virtual void beginRun();
    //! Event
    virtual void event();
    //! End Run
    virtual void endRun();
    //! terminate
    virtual void terminate() {}
    //! Define Histogram
    virtual void defineHisto();

  protected:
    //! dz of T3D in each module
    TH1D* h_dz = nullptr;
    //! phi of T3D in each module
    TH1D* h_phi = nullptr;
    //! tanlambda of T3D in each module
    TH1D* h_tanlambda = nullptr;
    //! pt of T3D in each module
    TH1D* h_pt = nullptr;
    //! phi (from 2D) of T3D in each module
    TH1D* h_phi_2D = nullptr;
    //! pt (from 2D) of T3D in each module
    TH1D* h_pt_2D = nullptr;
    //! TSF1 ID of T3D in each module
    TH1D* h_ID_TSF1 = nullptr;
    //! TSF3 ID of T3D in each module
    TH1D* h_ID_TSF3 = nullptr;
    //! TSF5 ID of T3D in each module
    TH1D* h_ID_TSF5 = nullptr;
    //! TSF7 ID of T3D in each module
    TH1D* h_ID_TSF7 = nullptr;
    //! TSF1 priority time of T3D in each module
    TH1D* h_rt_TSF1 = nullptr;
    //! TSF3 priority time of T3D in each module
    TH1D* h_rt_TSF3 = nullptr;
    //! TSF5 priority time of T3D in each module
    TH1D* h_rt_TSF5 = nullptr;
    //! TSF7 priority time of T3D in each module
    TH1D* h_rt_TSF7 = nullptr;
    //! TSF1 validity of T3D in each module
    TH1D* h_validity_TSF1 = nullptr;
    //! TSF3 validity of T3D in each module
    TH1D* h_validity_TSF3 = nullptr;
    //! TSF5 validity of T3D in each module
    TH1D* h_validity_TSF5 = nullptr;
    //! TSF7 validity of T3D in each module
    TH1D* h_validity_TSF7 = nullptr;


    //! TDirectories for DQM histograms
    TDirectory* oldDir = nullptr;
    //! TDirectories for DQM histograms
    TDirectory* dirDQM = nullptr;

    /// flag to save ps file
    bool m_generatePostscript;

    /// name of ps file
    std::string m_postScriptName;

    /// experiment number
    unsigned _exp = 0;

    /// run number
    unsigned _run = 0;

    /// T3D module number
    int m_T3DMOD = 0;

    /// 3D data store
    StoreArray<CDCTriggerTrack> entAry;

    /// 2D data store
    StoreArray<CDCTriggerTrack> entAry_2D;

    /// Stereo TSF data store
    StoreArray<CDCTriggerSegmentHit> entAry_TSF;
  };

}

#endif
