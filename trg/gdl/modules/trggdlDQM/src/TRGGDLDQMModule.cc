//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TRGGDLModule.cc
// Section  : TRG GDL
// Owner    :
// Email    :
//---------------------------------------------------------------
// Description : A trigger module for TRG GDL
//---------------------------------------------------------------
// 1.00 : 2017/05/08 : First version
//---------------------------------------------------------------
#include <trg/gdl/modules/trggdlDQM/TRGGDLDQMModule.h>
#include <trg/gdl/modules/trggdlUnpacker/trggdlUnpackerModule.h>
#include <mdst/dataobjects/SoftwareTriggerResult.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <TDirectory.h>
#include <TPostScript.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>
#include <fstream>
#include <framework/logging/Logger.h>
#include <boost/algorithm/string.hpp>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/HitPatternCDC.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include "trg/ecl/dataobjects/TRGECLCluster.h"

using namespace std;
using namespace Belle2;
using namespace GDL;

REG_MODULE(TRGGDLDQM);

/*
   Fired data in TrgBit not available because
   of absence of TRGSummary class.
   TrgBit class only for bit configuration.
*/


TRGGDLDQMModule::TRGGDLDQMModule() : HistoModule()
{

  setDescription("DQM for GDL Trigger system");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("eventByEventTimingHistRecord", m_eventByEventTimingHistRecord,
           "Recording event by event timing distribution histogram or not",
           false);
  addParam("dumpVcdFile", m_dumpVcdFile,
           "Dumping vcd file or not",
           false);
  addParam("bitConditionToDumpVcd", m_bitConditionToDumpVcd,
           "Condition for vcd. alg format with '!' and '+'.",
           string(""));
  addParam("vcdEventStart", m_vcdEventStart,
           "Start equential event number",
           unsigned(0));
  addParam("vcdNumberOfEvents", m_vcdNumberOfEvents,
           "Number of events to dump vcd file",
           unsigned(10));
  addParam("bitNameOnBinLabel", m_bitNameOnBinLabel,
           "Put bitname on BinLabel",
           true);
  addParam("generatePostscript", m_generatePostscript,
           "Genarete postscript file or not",
           false);
  addParam("postScriptName", m_postScriptName,
           "postscript file name",
           string("gdldqm.ps"));
  addParam("skim", m_skim,
           "use skim information or not",
           int(-1));
  B2DEBUG(20, "eventByEventTimingFlag(" << m_eventByEventTimingHistRecord
          << "), m_dumpVcdFile(" << m_dumpVcdFile
          << "), m_bitConditionToDumpVcd(" << m_bitConditionToDumpVcd
          << "), m_vcdEventStart(" << m_vcdEventStart
          << "), m_vcdNumberOfEvents(" << m_vcdNumberOfEvents);


}

void TRGGDLDQMModule::defineHisto()
{
  oldDir = gDirectory;
  dirDQM = gDirectory;
  oldDir->mkdir("TRGGDL");
  dirDQM->cd("TRGGDL");

  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    h_c8_gdlL1TocomL1[iskim]  = new TH1I(Form("hGDL_gdlL1TocomL1_%s", skim_smap[iskim].c_str()),  "comL1 - gdlL1 [clk8ns]", 100, 0,
                                         100);
    h_c8_gdlL1TocomL1[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c8_topTogdlL1[iskim]    = new TH1I(Form("hGDL_topTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - top_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_topTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_eclTogdlL1[iskim]    = new TH1I(Form("hGDL_eclTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - ecl_timing [clk8ns]", 500, 0,
                                         500);
    h_c8_eclTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_cdcTogdlL1[iskim]    = new TH1I(Form("hGDL_cdcTogdlL1_%s", skim_smap[iskim].c_str()),    "gdlL1 - cdc_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_cdcTogdlL1[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c8_ecl8mToGDL[iskim]    = new TH1I(Form("hGDL_ecl8mToGDL_%s", skim_smap[iskim].c_str()),    "gdlIn^{8MHz} - ecl_timing [clk8ns]",
                                         500, 0, 500);
    h_c8_ecl8mToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_topToGDL[iskim]      = new TH1I(Form("hGDL_topToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - top_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_topToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_eclToGDL[iskim]      = new TH1I(Form("hGDL_eclToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - ecl_timing [clk8ns]", 500, 0,
                                         500);
    h_c8_eclToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");
    h_c8_cdcToGDL[iskim]      = new TH1I(Form("hGDL_cdcToGDL_%s", skim_smap[iskim].c_str()),      "gdlIn - cdc_timing [clk8ns]", 700, 0,
                                         700);
    h_c8_cdcToGDL[iskim]->GetXaxis()->SetTitle("clk8ns");

    h_c2_cdcTocomL1[iskim] = new TH1I(Form("hGDL_cdcTocomL1_%s", skim_smap[iskim].c_str()), "comL1 - cdc_timing [clk2ns]", 520, 0,
                                      5200);
    h_c2_cdcTocomL1[iskim]->GetXaxis()->SetTitle("clk2ns");
    h_ns_cdcTocomL1[iskim] = new TH1D(Form("hGDL_ns_cdcTocomL1_%s", skim_smap[iskim].c_str()), "comL1 - cdc_timing [ns]", 2600, 0,
                                      10400);
    h_ns_cdcTocomL1[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcTocomL1[iskim]->GetYaxis()->SetTitle("evt / 4ns");
    h_ns_cdcTogdlL1[iskim] = new TH1D(Form("hGDL_ns_cdcTogdlL1_%s", skim_smap[iskim].c_str()), "gdlL1 - cdc_timing [ns]", 2600, 0,
                                      10400);
    h_ns_cdcTogdlL1[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcTogdlL1[iskim]->GetYaxis()->SetTitle("evt / 4ns");

    h_ns_topToecl[iskim] = new TH1D(Form("hGDL_ns_topToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - top_timing [ns]", 800, 0,
                                    4000);
    h_ns_topToecl[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_topToecl[iskim]->GetYaxis()->SetTitle("evt / 5ns");
    h_ns_topTocdc[iskim] = new TH1D(Form("hGDL_ns_topTocdc_%s", skim_smap[iskim].c_str()), "cdc_timing - top_timing [ns]", 800, 0,
                                    4000);
    h_ns_topTocdc[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_topTocdc[iskim]->GetYaxis()->SetTitle("evt / 5ns");
    h_ns_cdcToecl[iskim] = new TH1D(Form("hGDL_ns_cdcToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - cdc_timing [ns]", 2000, 0,
                                    4000);
    h_ns_cdcToecl[iskim]->GetXaxis()->SetTitle("ns");
    h_ns_cdcToecl[iskim]->GetYaxis()->SetTitle("evt / 2ns");

    h_c2_cdcToecl[iskim] = new TH1I(Form("hGDL_cdcToecl_%s", skim_smap[iskim].c_str()), "ecl_timing - cdc_timing [clk2ns]", 1000, 0,
                                    2000);
    h_c2_cdcToecl[iskim]->GetXaxis()->SetTitle("clk2ns");

    h_timtype[iskim] = new TH1I(Form("hGDL_timtype_%s", skim_smap[iskim].c_str()), "timtype", 7, 0, 7);

    h_itd[iskim] = new TH1I(Form("hGDL_itd_%s", skim_smap[iskim].c_str()), "itd", n_inbit + 1, -1, n_inbit);
    h_ftd[iskim] = new TH1I(Form("hGDL_ftd_%s", skim_smap[iskim].c_str()), "ftd", n_outbit + 1, -1, n_outbit);
    h_psn[iskim] = new TH1I(Form("hGDL_psn_%s", skim_smap[iskim].c_str()), "psn", n_outbit + 1, -1, n_outbit);
    // output extra
    h_psn_extra[iskim] = new TH1I(Form("hGDL_psn_extra_%s", skim_smap[iskim].c_str()), "psn extra", n_output_extra, 0, n_output_extra);
    for (int i = 0; i < n_output_extra; i++) {
      h_psn_extra[iskim]->GetXaxis()->SetBinLabel(i + 1, output_extra[i]);
    }
    // output pure extra
    h_psn_pure_extra[iskim] = new TH1I(Form("hGDL_psn_pure_extra_%s", skim_smap[iskim].c_str()), "psn pure extra", n_output_pure_extra,
                                       0, n_output_pure_extra);
    for (int i = 0; i < n_output_pure_extra; i++) {
      h_psn_pure_extra[iskim]->GetXaxis()->SetBinLabel(i + 1, output_pure_extra[i]);
    }
    // efficiency
    h_eff[iskim]       = new TH1D(Form("hGDL_eff_%s", skim_smap[iskim].c_str()), "efficiency", n_eff, 0, n_eff);
    for (int i = 0; i < n_eff; i++) {
      h_eff[iskim]->GetXaxis()->SetBinLabel(i + 1, c_eff[i]);
    }
    h_pure_eff[iskim]  = new TH1D(Form("hGDL_pure_eff_%s", skim_smap[iskim].c_str()), "efficiency wrt. offline", n_pure_eff, 0,
                                  n_pure_eff);
    for (int i = 0; i < n_pure_eff; i++) {
      h_pure_eff[iskim]->GetXaxis()->SetBinLabel(i + 1, c_pure_eff[i]);
      h_pure_eff[iskim]->GetXaxis()->SetLabelSize(0.05);
    }
    for (unsigned i = 0; i < n_inbit; i++) {
      if (m_bitNameOnBinLabel) {
        h_itd[iskim]->GetXaxis()->SetBinLabel(h_itd[iskim]->GetXaxis()->FindBin(i + 0.5), inbitname[i]);
        h_ftd[iskim]->GetXaxis()->SetBinLabel(h_ftd[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
        h_psn[iskim]->GetXaxis()->SetBinLabel(h_psn[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
      }
    }

    //reduce #plot
    if (iskim != 0)continue;

    // rise/fall
    for (unsigned i = 0; i < n_inbit; i++) {
      h_itd_rise[i][iskim] = new TH1I(Form("hGDL_itd_%s_rise_%s", inbitname[i], skim_smap[iskim].c_str()),
                                      Form("itd%d(%s) rising", i, inbitname[i]), 48, 0, 48);
      h_itd_rise[i][iskim]->SetLineColor(kRed);
      h_itd_fall[i][iskim] = new TH1I(Form("hGDL_itd_%s_fall_%s", inbitname[i], skim_smap[iskim].c_str()),
                                      Form("itd%d(%s) falling", i, inbitname[i]), 48, 0, 48);
      h_itd_fall[i][iskim]->SetLineColor(kGreen);
    }
    for (unsigned i = 0; i < n_outbit; i++) {
      h_ftd[iskim]->GetXaxis()->SetBinLabel(h_ftd[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
      h_ftd_rise[i][iskim] = new TH1I(Form("hGDL_ftd_%s_rise_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("ftd%d(%s) rising", i, outbitname[i]), 48, 0, 48);
      h_ftd_rise[i][iskim]->SetLineColor(kRed);
      h_ftd_fall[i][iskim] = new TH1I(Form("hGDL_ftd_%s_fall_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("ftd%d(%s) falling", i, outbitname[i]), 48, 0, 48);
      h_ftd_fall[i][iskim]->SetLineColor(kGreen);
      h_psn[iskim]->GetXaxis()->SetBinLabel(h_psn[iskim]->GetXaxis()->FindBin(i + 0.5), outbitname[i]);
      h_psn_rise[i][iskim] = new TH1I(Form("hGDL_psn_%s_rise_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("psn%d(%s) rising", i, outbitname[i]), 48, 0, 48);
      h_psn_rise[i][iskim]->SetLineColor(kRed);
      h_psn_fall[i][iskim] = new TH1I(Form("hGDL_psn_%s_fall_%s", outbitname[i], skim_smap[iskim].c_str()),
                                      Form("psn%d(%s) falling", i, outbitname[i]), 48, 0, 48);
      h_psn_fall[i][iskim]->SetLineColor(kGreen);
    }
  }

  h_eff_shifter    = new TH1D(Form("hGDL_eff_shifter"), "efficiency", n_eff_shifter, 0, n_eff_shifter);
  for (int i = 0; i < n_eff_shifter; i++) {
    h_eff_shifter->GetXaxis()->SetBinLabel(i + 1, c_eff_shifter[i]);
  }
  h_eff_shifter->SetMaximum(1.2);
  h_eff_shifter->SetMinimum(0);
  h_eff_shifter->GetXaxis()->SetLabelSize(0.05);

  oldDir->cd();
}

void TRGGDLDQMModule::beginRun()
{

  dirDQM->cd();

  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    h_c8_gdlL1TocomL1[iskim]->Reset();
    h_c8_topTogdlL1[iskim]->Reset();
    h_c8_eclTogdlL1[iskim]->Reset();
    h_c8_cdcTogdlL1[iskim]->Reset();
    h_c8_ecl8mToGDL[iskim]->Reset();
    h_c8_topToGDL[iskim]->Reset();
    h_c8_eclToGDL[iskim]->Reset();
    h_c8_cdcToGDL[iskim]->Reset();
    h_c2_cdcTocomL1[iskim]->Reset();
    h_ns_cdcTocomL1[iskim]->Reset();
    h_ns_cdcTogdlL1[iskim]->Reset();
    h_ns_topToecl[iskim]->Reset();
    h_ns_topTocdc[iskim]->Reset();
    h_c2_cdcToecl[iskim]->Reset();
    h_ns_cdcToecl[iskim]->Reset();
    h_itd[iskim]->Reset();
    h_ftd[iskim]->Reset();
    h_psn[iskim]->Reset();
    h_psn_extra[iskim]->Reset();
    h_psn_pure_extra[iskim]->Reset();
    h_eff[iskim]->Reset();
    h_pure_eff[iskim]->Reset();
    h_timtype[iskim]->Reset();
  }
  h_eff_shifter->Reset();


  oldDir->cd();
}

void TRGGDLDQMModule::initialize()
{

  if (m_skim == 0) { //no skim
    start_skim_gdldqm = 0;
    end_skim_gdldqm = 1;
  } else if (m_skim == 1) { //skim
    start_skim_gdldqm = 1;
    end_skim_gdldqm = nskim_gdldqm;
  } else { //no skim + skim
    start_skim_gdldqm = 0;
    end_skim_gdldqm = nskim_gdldqm;
  }

  StoreObjPtr<EventMetaData> bevt;
  _exp = bevt->getExperiment();
  _run = bevt->getRun();

  m_ECLCalDigitData.registerInDataStore();
  m_ECLDigitData.registerInDataStore();
  trgeclmap = new TrgEclMapping();

  // calls back the defineHisto() function, but the HistoManager module has to be in the path
  REG_HISTOGRAM

  for (int i = 0; i < 320; i++) {
    LeafBitMap[i] = m_unpacker->getLeafMap(i);
  }
  for (int i = 0; i < 320; i++) {
    strcpy(LeafNames[i], m_unpacker->getLeafnames(i));
  }
  _e_timtype = 0;
  _e_gdll1rvc = 0;
  _e_coml1rvc = 0;
  _e_toptiming = 0;
  _e_ecltiming = 0;
  _e_cdctiming = 0;
  _e_toprvc  = 0;
  _e_eclrvc  = 0;
  _e_cdcrvc  = 0;
  for (int i = 0; i < 10; i++) {
    ee_psn[i] = {0};
    ee_ftd[i] = {0};
    ee_itd[i] = {0};
  }
  for (int i = 0; i < 320; i++) {
    if (strcmp(LeafNames[i], "timtype") == 0)  _e_timtype  = LeafBitMap[i];
    if (strcmp(LeafNames[i], "gdll1rvc") == 0) _e_gdll1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "coml1rvc") == 0) _e_coml1rvc = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toptiming") == 0)_e_toptiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ecltiming") == 0)_e_ecltiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdctiming") == 0)_e_cdctiming = LeafBitMap[i];
    if (strcmp(LeafNames[i], "toprvc") == 0)   _e_toprvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "eclrvc") == 0)   _e_eclrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "cdcrvc") == 0)   _e_cdcrvc   = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn0") == 0)       ee_psn[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn1") == 0)       ee_psn[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn2") == 0)       ee_psn[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn3") == 0)       ee_psn[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn4") == 0)       ee_psn[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn5") == 0)       ee_psn[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn6") == 0)       ee_psn[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn7") == 0)       ee_psn[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn8") == 0)       ee_psn[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "psn9") == 0)       ee_psn[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd0") == 0)       ee_ftd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd1") == 0)       ee_ftd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd2") == 0)       ee_ftd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd3") == 0)       ee_ftd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd4") == 0)       ee_ftd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd5") == 0)       ee_ftd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd6") == 0)       ee_ftd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd7") == 0)       ee_ftd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd8") == 0)       ee_ftd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "ftd9") == 0)       ee_ftd[9] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd0") == 0)       ee_itd[0] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd1") == 0)       ee_itd[1] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd2") == 0)       ee_itd[2] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd3") == 0)       ee_itd[3] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd4") == 0)       ee_itd[4] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd5") == 0)       ee_itd[5] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd6") == 0)       ee_itd[6] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd7") == 0)       ee_itd[7] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd8") == 0)       ee_itd[8] = LeafBitMap[i];
    if (strcmp(LeafNames[i], "itd9") == 0)       ee_itd[9] = LeafBitMap[i];
  }

  n_inbit = m_dbinput->getninbit();
  n_outbit = m_dbftdl ->getnoutbit();
  for (int i = 0; i < 320; i++) {
    strcpy(inbitname[i], m_dbinput->getinbitname(i));
  }
  for (int i = 0; i < 320; i++) {
    strcpy(outbitname[i], m_dbftdl->getoutbitname(i));
  }

}

void TRGGDLDQMModule::endRun()
{
  if (m_generatePostscript) {
    TPostScript* ps = new TPostScript(m_postScriptName.c_str(), 112);
    gStyle->SetOptStat(0);
    TCanvas c1("c1", "", 0, 0, 500, 300);
    c1.cd();

    for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {

      h_itd[iskim]->GetXaxis()->SetRange(h_itd[iskim]->GetXaxis()->FindBin(0.5),
                                         h_itd[iskim]->GetXaxis()->FindBin(n_inbit - 0.5));
      h_itd[iskim]->Draw();
      c1.Update();
      h_ftd[iskim]->GetXaxis()->SetRange(h_ftd[iskim]->GetXaxis()->FindBin(0.5),
                                         h_ftd[iskim]->GetXaxis()->FindBin(n_outbit - 0.5));
      h_ftd[iskim]->Draw();
      c1.Update();
      h_psn[iskim]->GetXaxis()->SetRange(h_psn[iskim]->GetXaxis()->FindBin(0.5),
                                         h_psn[iskim]->GetXaxis()->FindBin(n_outbit - 0.5));
      h_psn[iskim]->Draw();
      c1.Update();
      h_ftd[iskim]->SetTitle("ftd(green), psnm(red)");
      h_ftd[iskim]->SetFillColor(kGreen);
      h_ftd[iskim]->SetBarWidth(0.4);
      h_ftd[iskim]->Draw("bar");
      h_psn[iskim]->SetFillColor(kRed);
      h_psn[iskim]->SetBarWidth(0.4);
      h_psn[iskim]->SetBarOffset(0.5);
      h_psn[iskim]->Draw("bar,same");

      c1.Update();
      h_timtype[iskim]->Draw();
      c1.Update();
      h_c8_gdlL1TocomL1[iskim]->Draw();
      c1.Update();
      h_c8_topTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_eclTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_cdcTogdlL1[iskim]->Draw();
      c1.Update();
      h_c8_ecl8mToGDL[iskim]->Draw();
      c1.Update();
      h_c8_topToGDL[iskim]->Draw();
      c1.Update();
      h_c8_eclToGDL[iskim]->Draw();
      c1.Update();
      h_c8_cdcToGDL[iskim]->Draw();
      c1.Update();
      h_c2_cdcTocomL1[iskim]->Draw();
      c1.Update();
      h_ns_cdcTocomL1[iskim]->Draw();
      c1.Update();
      h_ns_cdcTogdlL1[iskim]->Draw();
      c1.Update();
      h_ns_topToecl[iskim]->Draw();
      c1.Update();
      h_ns_topTocdc[iskim]->Draw();
      c1.Update();
      h_c2_cdcToecl[iskim]->Draw();
      c1.Update();
      h_ns_cdcToecl[iskim]->Draw();
      c1.Update();

      //reduce #plot
      if (iskim != 0)continue;
      for (unsigned i = 0; i < n_inbit; i++) {

        h_itd_rise[i][iskim]->SetTitle(Form("itd%d(%s) rising(red), falling(green)",
                                            i, inbitname[i]));
        h_itd_rise[i][iskim]->Draw();
        h_itd_fall[i][iskim]->Draw("same");
        c1.Update();
      }

    }

    ps->Close();
  }
}

void TRGGDLDQMModule::event()
{
  /* cppcheck-suppress variableScope */
  static unsigned nvcd = 0;
  static bool begin_run = true;

  int n_leafs = 0;
  n_leafs  = m_unpacker->getnLeafs();
  int n_leafsExtra = 0;
  n_leafsExtra = m_unpacker->getnLeafsExtra();
  n_clocks = m_unpacker->getnClks();
  int nconf = m_unpacker->getconf();
  int nword_input  = m_unpacker->get_nword_input();
  int nword_output = m_unpacker->get_nword_output();
  skim.clear();

  StoreArray<TRGGDLUnpackerStore> entAry;
  if (!entAry || !entAry.getEntries()) {
    return;
  }

  //Get skim type from SoftwareTriggerResult
  for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
    if (iskim == 0) skim.push_back(iskim);
  }
  StoreObjPtr<SoftwareTriggerResult> result_soft;
  if (result_soft.isValid()) {
    const std::map<std::string, int>& skim_map = result_soft->getResults();
    for (int iskim = start_skim_gdldqm; iskim < end_skim_gdldqm; iskim++) {
      if (iskim == 0);
      else if (skim_map.find(skim_menu[iskim]) != skim_map.end()) {
        const bool accepted = (result_soft->getResult(skim_menu[iskim]) == SoftwareTriggerCutResult::c_accept);
        if (accepted) skim.push_back(iskim);
      }
    }
  }


  //prepare entAry adress
  int clk_map = 0;
  for (int i = 0; i < 320; i++) {
    if (strcmp(entAry[0]->m_unpackername[i], "evt") == 0) evtno = entAry[0]->m_unpacker[i];
    if (strcmp(entAry[0]->m_unpackername[i], "clk") == 0) clk_map = i;
  }

  const double clkTo2ns = 1. / .508877;
  const double clkTo1ns = 0.5 / .508877;
  TH2I* h_0;

  dirDQM->cd();


  h_0 = new TH2I(Form("hgdl%08d", evtno), "", n_clocks, 0, n_clocks, n_leafs + n_leafsExtra, 0,
                 n_leafs + n_leafsExtra);
  h_p = new TH2I(Form("hpsn%08d", evtno), "", n_clocks, 0, n_clocks, n_outbit, 0, n_outbit);
  h_f = new TH2I(Form("hftd%08d", evtno), "", n_clocks, 0, n_clocks, n_outbit, 0, n_outbit);
  h_i = new TH2I(Form("hitd%08d", evtno), "", n_clocks, 0, n_clocks, n_inbit, 0, n_inbit);
  for (unsigned i = 0; i < n_inbit; i++) {
    h_i->GetYaxis()->SetBinLabel(i + 1, inbitname[i]);
  }
  for (unsigned i = 0; i < n_outbit; i++) {
    h_f->GetYaxis()->SetBinLabel(i + 1, outbitname[i]);
    h_p->GetYaxis()->SetBinLabel(i + 1, outbitname[i]);
  }


  oldDir->cd();

  // fill "bit vs clk" for the event
  for (int ii = 0; ii < entAry.getEntries(); ii++) {
    std::vector<int*> Bits(n_leafs + n_leafsExtra);
    //set pointer
    for (int i = 0; i < 320; i++) {
      if (LeafBitMap[i] != -1) {
        Bits[LeafBitMap[i]] = &(entAry[ii]->m_unpacker[i]);
      }
    }
    for (int leaf = 0; leaf < n_leafs + n_leafsExtra; leaf++) {
      h_0->SetBinContent(entAry[ii]->m_unpacker[clk_map] + 1, leaf + 1, *Bits[leaf]);
    }
  }
  for (int leaf = 0; leaf < 320; leaf++) {
    if (LeafBitMap[leaf] != -1)h_0->GetYaxis()->SetBinLabel(LeafBitMap[leaf] + 1, LeafNames[LeafBitMap[leaf]]);
  }

  int coml1rvc      = h_0->GetBinContent(1,        1 + _e_coml1rvc);
  int toprvc        = h_0->GetBinContent(1,        1 + _e_toprvc);
  int eclrvc        = h_0->GetBinContent(1,        1 + _e_eclrvc);
  int cdcrvc        = h_0->GetBinContent(1,        1 + _e_cdcrvc);
  int c1_top_timing = h_0->GetBinContent(n_clocks, 1 + _e_toptiming);
  int c1_ecl_timing = h_0->GetBinContent(n_clocks, 1 + _e_ecltiming);
  int c1_cdc_timing = h_0->GetBinContent(n_clocks, 1 + _e_cdctiming);
  int c8_top_timing = c1_top_timing >> 3;
  int c2_top_timing = c1_top_timing >> 1;
  int c8_ecl_timing = c1_ecl_timing >> 3;
  int c2_ecl_timing = c1_ecl_timing >> 1;
  int c8_cdc_timing = c1_cdc_timing >> 3;
  int c2_cdc_timing = c1_cdc_timing >> 1;

  if (begin_run) {
    B2DEBUG(20, "nconf(" << nconf
            << "), n_clocks(" << n_clocks
            << "), n_leafs(" << n_leafs
            << "), n_leafsExtra(" << n_leafsExtra
            << ")");
    begin_run = false;
  }

  int psn[5] = {0};
  int ftd[5] = {0};
  int itd[5] = {0};
  int timtype  = 0;


  int gdll1_rvc = h_0->GetBinContent(h_0->GetXaxis()->FindBin(n_clocks - 0.5), 1 + _e_gdll1rvc);

  // fill event by event timing histogram and get time integrated bit info
  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    int psn_tmp[5] = {0};
    int ftd_tmp[5] = {0};
    int itd_tmp[5] = {0};
    for (unsigned j = 0; j < (unsigned)nword_input; j++) {
      itd_tmp[j] = h_0->GetBinContent(clk, 1 + ee_itd[j]);
      itd[j] |= itd_tmp[j];
      for (int i = 0; i < 32; i++) {
        if (itd_tmp[j] & (1 << i)) h_i->SetBinContent(clk, i + 1 +  j * 32, 1);
      }
    }
    if (nconf == 0) {
      psn_tmp[0] = h_0->GetBinContent(clk, 1 + ee_psn[0]);
      ftd_tmp[0] = h_0->GetBinContent(clk, 1 + ee_ftd[0]);
      psn[0] |= psn_tmp[0];
      ftd[0] |= ftd_tmp[0];
      for (int i = 0; i < 32; i++) {
        if (psn_tmp[0] & (1 << i)) h_p->SetBinContent(clk, i + 1, 1);
        if (ftd_tmp[0] & (1 << i)) h_f->SetBinContent(clk, i + 1, 1);
      }
      psn_tmp[1] = h_0->GetBinContent(clk, 1 + ee_psn[2]) * (1 << 16) + h_0->GetBinContent(clk, 1 + ee_psn[1]);
      ftd_tmp[1] = h_0->GetBinContent(clk, 1 + ee_ftd[2]) * (1 << 16) + h_0->GetBinContent(clk, 1 + ee_ftd[1]);
      psn[1] |= psn_tmp[1];
      ftd[1] |= ftd_tmp[1];
      for (int i = 0; i < 32; i++) {
        if (psn_tmp[1] & (1 << i)) h_p->SetBinContent(clk, i + 1 + 32, 1);
        if (ftd_tmp[1] & (1 << i)) h_f->SetBinContent(clk, i + 1 + 32, 1);
      }
    } else {
      for (unsigned j = 0; j < (unsigned)nword_output; j++) {
        psn_tmp[j] = h_0->GetBinContent(clk, 1 + ee_psn[j]);
        ftd_tmp[j] = h_0->GetBinContent(clk, 1 + ee_ftd[j]);
        psn[j] |= psn_tmp[j];
        ftd[j] |= ftd_tmp[j];
        for (int i = 0; i < 32; i++) {
          if (psn_tmp[j] & (1 << i)) h_p->SetBinContent(clk, i + 1 +  j * 32, 1);
          if (ftd_tmp[j] & (1 << i)) h_f->SetBinContent(clk, i + 1 +  j * 32, 1);
        }
      }
    }
    int timtype_tmp = h_0->GetBinContent(clk, 1 + _e_timtype);
    timtype = (timtype_tmp == 0) ? timtype : timtype_tmp;

  } // clk

  // fill rising and falling edges
  fillRiseFallTimings();
  // fill Output_extra for efficiency study
  fillOutputExtra();
  // fill Output for high purity efficiency study
  if (m_skim != 1) {
    fillOutputPureExtra();
  }

  // fill summary histograms
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    h_timtype[skim[ifill]]->Fill(timtype);
    h_itd[skim[ifill]]->Fill(-0.5);
    h_ftd[skim[ifill]]->Fill(-0.5);
    h_psn[skim[ifill]]->Fill(-0.5);
    for (int i = 0; i < 32; i++) {
      for (unsigned j = 0; j < (unsigned)nword_input; j++) {
        if (itd[j] & (1 << i)) h_itd[skim[ifill]]->Fill(i + 0.5 + 32 * j);
      }
      for (unsigned j = 0; j < (unsigned)nword_output; j++) {
        if (ftd[j] & (1 << i)) h_ftd[skim[ifill]]->Fill(i + 0.5 + 32 * j);
        if (psn[j] & (1 << i)) h_psn[skim[ifill]]->Fill(i + 0.5 + 32 * j);
      }
    }

    // fill timestamp values stored in header
    int gdlL1TocomL1 = gdll1_rvc < coml1rvc ? coml1rvc - gdll1_rvc : (coml1rvc + 1280) - gdll1_rvc;
    h_c8_gdlL1TocomL1[skim[ifill]]->Fill(gdlL1TocomL1);

    int topTogdlL1 = gdll1_rvc < c8_top_timing ? (gdll1_rvc + 1280) - c8_top_timing : gdll1_rvc - c8_top_timing;
    h_c8_topTogdlL1[skim[ifill]]->Fill(topTogdlL1);

    int eclTogdlL1 = gdll1_rvc < c8_ecl_timing ? (gdll1_rvc + 1280) - c8_ecl_timing : gdll1_rvc - c8_ecl_timing;
    h_c8_eclTogdlL1[skim[ifill]]->Fill(eclTogdlL1);

    int cdcTogdlL1 = gdll1_rvc < c8_cdc_timing ? (gdll1_rvc + 1280) - c8_cdc_timing : gdll1_rvc - c8_cdc_timing;
    h_c8_cdcTogdlL1[skim[ifill]]->Fill(cdcTogdlL1);

    int c127_ecl_timing = c8_ecl_timing & (((1 << 7) - 1) << 4);
    int fit8mToGDL = c127_ecl_timing < eclrvc ? eclrvc - c127_ecl_timing : (eclrvc + 1280) - c127_ecl_timing;
    h_c8_ecl8mToGDL[skim[ifill]]->Fill(fit8mToGDL);

    int topToGDL = c8_top_timing < toprvc ? toprvc - c8_top_timing : (toprvc + 1280) - c8_top_timing;
    h_c8_topToGDL[skim[ifill]]->Fill(topToGDL);
    int eclToGDL = c8_ecl_timing < eclrvc ? eclrvc - c8_ecl_timing : (eclrvc + 1280) - c8_ecl_timing;
    h_c8_eclToGDL[skim[ifill]]->Fill(eclToGDL);
    int cdcToGDL = c8_cdc_timing < cdcrvc ? cdcrvc - c8_cdc_timing : (cdcrvc + 1280) - c8_cdc_timing;
    h_c8_cdcToGDL[skim[ifill]]->Fill(cdcToGDL);

    int c2_comL1 = coml1rvc << 2;
    int c2_gdlL1 = gdll1_rvc << 2;
    int c2_diff_cdcTogdlL1 = c2_gdlL1 > c2_cdc_timing ?
                             c2_gdlL1 - c2_cdc_timing :
                             c2_gdlL1 - c2_cdc_timing + (1280 << 2) ;
    h_ns_cdcTogdlL1[skim[ifill]]->Fill(c2_diff_cdcTogdlL1 * clkTo2ns);

    int c2_diff_cdcTocomL1 = c2_comL1 > c2_cdc_timing ?
                             c2_comL1 - c2_cdc_timing :
                             c2_comL1 - c2_cdc_timing + (1280 << 2) ;
    h_c2_cdcTocomL1[skim[ifill]]->Fill(c2_diff_cdcTocomL1);
    h_ns_cdcTocomL1[skim[ifill]]->Fill(c2_diff_cdcTocomL1 * clkTo2ns);

    int c2_diff_cdcToecl = c2_ecl_timing > c2_cdc_timing ?
                           c2_ecl_timing - c2_cdc_timing :
                           c2_ecl_timing - c2_cdc_timing + (1280 << 2);
    h_c2_cdcToecl[skim[ifill]]->Fill(c2_diff_cdcToecl);
    h_ns_cdcToecl[skim[ifill]]->Fill(c2_diff_cdcToecl * clkTo2ns);

    int c1_diff_topToecl = c1_ecl_timing > c1_top_timing ?
                           c1_ecl_timing - c1_top_timing :
                           c1_ecl_timing - c1_top_timing + (1280 << 3);
    h_ns_topToecl[skim[ifill]]->Fill(c1_diff_topToecl *  clkTo1ns);

    int c2_diff_topTocdc = c2_cdc_timing > c2_top_timing ?
                           c2_cdc_timing - c2_top_timing :
                           c2_cdc_timing - c2_top_timing + (1280 << 2);
    h_ns_topTocdc[skim[ifill]]->Fill(c2_diff_topTocdc *  clkTo2ns);
  }


  // vcd dump
  if (m_dumpVcdFile) {
    if (anaBitCondition()) {
      nvcd++;
      B2DEBUG(20, "anaBitCondition fired, evt(" << evtno << ")");
      if (m_vcdEventStart <= nvcd && nvcd < m_vcdEventStart + m_vcdNumberOfEvents) {
        genVcd();
      }
    }
  }

  // discard event by event histograms
  if (! m_eventByEventTimingHistRecord) {
    h_0->Delete();
    h_p->Delete();
    h_f->Delete();
    h_i->Delete();
  }
}

bool TRGGDLDQMModule::anaBitCondition(void)
{
  if (m_bitConditionToDumpVcd.length() == 0) return true;
  const char* cst = m_bitConditionToDumpVcd.c_str();
  bool reading_word = false;
  bool result_the_term = true; // init value must be true
  bool not_flag = false;
  unsigned begin_word = 0;
  unsigned word_length = 0;
  // notation steeing side must follow
  //  no blank between '!' and word
  for (unsigned i = 0; i < m_bitConditionToDumpVcd.length(); i++) {
    if (('a' <= cst[i] && cst[i] <= 'z') ||
        ('A' <= cst[i] && cst[i] <= 'Z') ||
        ('_' == cst[i]) || ('!' == cst[i]) ||
        ('0' <= cst[i] && cst[i] <= '9')) {
      if (reading_word) { // must not be '!'
        word_length++;
        if (i == m_bitConditionToDumpVcd.length() - 1) {
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
            return true;
          }
        }
      } else {
        // start of new word
        reading_word = true;
        if ('!' == cst[i]) {
          begin_word = i + 1;
          not_flag = true;
          word_length = 0;
        } else {
          begin_word = i;
          not_flag = false;
          word_length = 1;
          if (i == m_bitConditionToDumpVcd.length() - 1) {
            // one char bit ('f',...) comes end of conditions, 'xxx+f'
            bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
            B2DEBUG(20,
                    m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                    << "(" << fired << ")");
            if (((!not_flag && fired) || (not_flag && !fired)) && result_the_term) {
              return true;
            }
          }
        }
      }
    } else if ('+' == cst[i] || i == m_bitConditionToDumpVcd.length() - 1) {
      // End of the term.
      if (reading_word) { // 'xxx+'
        if (result_the_term) {
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if ((!not_flag && fired) || (not_flag && !fired)) {
            return true;
          } else {
            // this term is denied by the latest bit
          }
        } else {
          // already false.
        }
        reading_word = false;
      } else {
        // prior char is blank, 'xxx  +'
        if (result_the_term) {
          return true;
        } else {
          // already false
        }
      }
      result_the_term = true; //  go to next term
    } else {
      // can be blank (white space) or any delimiter.
      if (reading_word) {
        // end of a word, 'xxxx '
        reading_word = false;
        if (result_the_term) {
          // worth to try
          bool fired = isFired(m_bitConditionToDumpVcd.substr(begin_word, word_length));
          B2DEBUG(20,
                  m_bitConditionToDumpVcd.substr(begin_word, word_length).c_str()
                  << "(" << fired << ")");
          if ((!not_flag && fired) || (not_flag && !fired)) {
            // go to next word
          } else {
            result_the_term = false;
          }
        } else {
          // already false
        }
        reading_word = false;
      } else {
        // 2nd blank 'xx  ' or leading blanck '^ '
      }
    }
  }
  return false;
}

void TRGGDLDQMModule::genVcd(void)
{
  int prev_i[400] = {0};
  int prev_f[400] = {0};
  int prev_p[400] = {0};
  //int prev_g[400]={0}; // Future Plan
  ofstream outf(Form("vcd/e%02dr%08de%08d.vcd", _exp, _run, evtno));
  outf << "$date" << endl;
  outf << "   Aug 20, 2018 17:53:52" << endl;
  outf << "$end" << endl;
  outf << "$version" << endl;
  outf << "   ChipScope Pro Analyzer  14.7 P.20131013 (Build 14700.13.286.464)" << endl;
  outf << "$end" << endl;
  outf << "$timescale" << endl;
  if (n_clocks == 32) {
    outf << "    32ns" << endl;
  } else if (n_clocks == 48) {
    outf << "    8ns" << endl;
  } else {
    outf << "    1ns" << endl;
  }
  outf << "$end" << endl;
  outf << "" << endl;
  outf << "$scope module gdl0067d_icn $end" << endl;
  int seqnum = 0;
  for (unsigned j = 0; j < n_inbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " " << inbitname[j] << " $end" << endl;
  }
  for (unsigned j = 0; j < n_outbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " ftd." << outbitname[j] << " $end" << endl;
  }
  for (unsigned j = 0; j < n_outbit; j++) {
    outf << "$var wire  1  n" << seqnum++ << " psn." << outbitname[j] << " $end" << endl;
  }

  outf << "$upscope $end" << endl;
  outf << "$enddefinitions $end" << endl << endl;

  for (unsigned clk = 1; clk <= n_clocks; clk++) {
    seqnum = 0;
    outf << "#" << clk - 1 << endl;
    for (unsigned k = 1; k <= n_inbit; k++) {
      if (clk == 1 || prev_i[k - 1] != h_i->GetBinContent(clk, k)) {
        prev_i[k - 1] = h_i->GetBinContent(clk, k);
        outf << h_i->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= n_outbit; k++) {
      if (clk == 1 || prev_f[k - 1] != h_f->GetBinContent(clk, k)) {
        prev_f[k - 1] = h_f->GetBinContent(clk, k);
        outf << h_f->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    for (unsigned k = 1; k <= n_outbit; k++) {
      if (clk == 1 || prev_p[k - 1] != h_p->GetBinContent(clk, k)) {
        prev_p[k - 1] = h_p->GetBinContent(clk, k);
        outf << h_p->GetBinContent(clk, k) << "n" << seqnum << endl;
      }
      seqnum++;
    }
    /* Future Plan
    for(unsigned k=1; k<=n_gdlbits; k++){
      if(clk == 1 || prev_g[k-1] != hg->GetBinContent(clk, k)){
    prev_g[k-1] = hg->GetBinContent(clk, k);
    if(EBitWidth[k-1] != 1){
    char ans[33];
    if(k-1 == e_rvc){
      dec2binstring(hg->GetBinContent(clk, k), ans, true);
    }else{
      dec2binstring(hg->GetBinContent(clk, k), ans);
    }
    outf << "b" << ans << " n" << seqnum << endl;
    }else{
    outf << hg->GetBinContent(clk, k) << "n" << seqnum << endl;
    }
      }
      seqnum++;
    }
    */
  }
  outf.close();
}

bool
TRGGDLDQMModule::isFired(std::string bitname)
{
  bool isPsnm = false;
  for (unsigned i = 0; i < bitname.length(); i++) {
    if ('A' <= bitname[i] && bitname[i] <= 'Z') {
      isPsnm = true;
    }
  }
  boost::algorithm::to_lower(bitname);
  int bn = m_dbftdl->getoutbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (isPsnm) {
        if (h_p->GetBinContent(clk + 1, bn + 1) > 0)
          return true;
      } else {
        if (h_f->GetBinContent(clk + 1, bn + 1) > 0)
          return true;
      }
    }
  }
  bn = m_dbinput->getinbitnum(bitname.c_str());
  for (unsigned clk = 0; clk < n_clocks; clk++) {
    if (bn > -1) {
      if (h_i->GetBinContent(clk + 1, bn + 1) > 0)
        return true;
    }
  }
  return false;
}

void
TRGGDLDQMModule::fillRiseFallTimings(void)
{

  //std::cout << "rise " << skim.size() << std::endl;

  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    //reduce #plot
    if (skim[ifill] != 0)continue;

    for (unsigned i = 0; i < n_inbit; i++) {
      if (n_clocks == 32) {
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
      } else {
        h_itd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_itd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
      }
      bool rising_done = false;
      bool falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_i->GetBinContent(clk + 1, i + 1) > 0) {
          if (! rising_done) {
            h_itd_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_itd_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_i->GetBinContent(clk + 1, i + 1) == 0) {
          if (rising_done && ! falling_done) {
            h_itd_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
    }
    for (unsigned i = 0; i < n_outbit; i++) {
      if (n_clocks == 32) {
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk32ns");
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetRange(1, 32);
      } else {
        h_ftd_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_psn_rise[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_ftd_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
        h_psn_fall[i][skim[ifill]]->GetXaxis()->SetTitle("clk8ns");
      }
      bool rising_done = false;
      bool falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_f->GetBinContent(clk + 1, i + 1) > 0) {
          if (! rising_done) {
            h_ftd_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_ftd_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_f->GetBinContent(clk + 1, i + 1) == 0) {
          if (rising_done && ! falling_done) {
            h_ftd_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
      rising_done = false;
      falling_done = false;
      for (unsigned clk = 0; clk < n_clocks; clk++) {
        if (h_p->GetBinContent(clk + 1, i + 1) > 0) {
          if (! rising_done) {
            h_psn_rise[i][skim[ifill]]->Fill(clk + 0.5);
            rising_done = true;
          } else if (rising_done && !falling_done && clk == n_clocks - 1) {
            h_psn_fall[i][skim[ifill]]->Fill(clk + 0.5);
          }
        } else if (h_p->GetBinContent(clk + 1, i + 1) == 0) {
          if (rising_done && ! falling_done) {
            h_psn_fall[i][skim[ifill]]->Fill(clk + 0.5);
            falling_done = true;
          }
        }
      }
    }
  }
}



void
TRGGDLDQMModule::fillOutputExtra(void)
{
  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    bool c4_fired = isFired("C4");
    bool hie_fired = isFired("HIE");
    bool lml_fired = (isFired("LML0") || isFired("LML1") || isFired("LML2") || isFired("LML3") || isFired("LML4") || isFired("LML5")
                      || isFired("LML6") || isFired("LML7") || isFired("LML8") || isFired("LML9") || isFired("LML10") || isFired("ECLMUMU"));
    bool fff_fired = isFired("FFF");
    bool ff_fired  = isFired("ff");
    bool f_fired   = isFired("f");
    bool ffo_fired = isFired("FFO");
    bool ffb_fired = isFired("FFB");
    bool ffy_fired = isFired("ffy");
    bool fyo_fired = isFired("fyo");
    bool fyb_fired = isFired("fyb");
    bool bha2D_fired = isFired("BHA");
    bool bha3D_fired = isFired("BHA3D");
    bool lml0_fired  = isFired("LML0");
    bool lml1_fired  = isFired("LML1");
    bool lml2_fired  = isFired("LML2");
    bool lml3_fired  = isFired("LML3");
    bool lml4_fired  = isFired("LML4");
    bool lml5_fired  = isFired("LML5");
    bool lml6_fired  = isFired("LML6");
    bool lml7_fired  = isFired("LML7");
    bool lml8_fired  = isFired("LML8");
    bool lml9_fired  = isFired("LML9");
    bool lml10_fired = isFired("LML10");
    bool lml12_fired = isFired("LML12");
    bool lml13_fired = isFired("LML13");
    bool eclmumu_fired = isFired("ECLMUMU");
    bool mu_b2b_fired = isFired("mu_b2b");
    bool mu_eb2b_fired = isFired("mu_eb2b");
    bool cdcklm1_fired = isFired("cdcklm1");
    bool cdcklm2_fired = isFired("cdcklm2");
    bool klm_hit_fired = isFired("klm_hit");
    bool eklm_hit_fired = isFired("eklm_hit");
    bool cdcecl1_fired = isFired("cdcecl1");
    bool cdcecl2_fired = isFired("cdcecl2");
    bool cdcecl3_fired = isFired("cdcecl3");
    bool cdcecl4_fired = isFired("cdcecl4");

    if (1) {
      h_psn_extra[skim[ifill]]->Fill(0.5);
    }
    if (fff_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(1.5);
    }
    if (ffo_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(2.5);
    }
    if (ffb_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(3.5);
    }
    if (fff_fired) {
      h_psn_extra[skim[ifill]]->Fill(4.5);
    }
    if (c4_fired || hie_fired) {
      h_psn_extra[skim[ifill]]->Fill(5.5);
    }
    if (fff_fired || ffo_fired || ffb_fired) {
      h_psn_extra[skim[ifill]]->Fill(6.5);
    }
    if ((fff_fired || ffo_fired || ffb_fired) && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(7.5);
    }
    if (bha2D_fired) {
      h_psn_extra[skim[ifill]]->Fill(8.5);
    }
    if (bha3D_fired) {
      h_psn_extra[skim[ifill]]->Fill(9.5);
    }
    if (ff_fired) {
      h_psn_extra[skim[ifill]]->Fill(10.5);
    }
    if (ff_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(11.5);
    }
    if (f_fired) {
      h_psn_extra[skim[ifill]]->Fill(12.5);
    }
    if (f_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(13.5);
    }
    if (lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(14.5);
    }
    if (fff_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(15.5);
    }
    if (ffo_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(16.5);
    }
    if (ffb_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(17.5);
    }
    if (ffy_fired) {
      h_psn_extra[skim[ifill]]->Fill(18.5);
    }
    if (ffy_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(19.5);
    }
    if (fyo_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(20.5);
    }
    if (fyb_fired && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(21.5);
    }
    if ((ffy_fired || fyo_fired || fyb_fired) && (c4_fired || hie_fired)) {
      h_psn_extra[skim[ifill]]->Fill(22.5);
    }
    if (ffy_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(23.5);
    }
    if (fyo_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(24.5);
    }
    if (fyb_fired && (lml_fired)) {
      h_psn_extra[skim[ifill]]->Fill(25.5);
    }
    if (c4_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(26.5);
    }
    if (hie_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(27.5);
    }
    if (lml0_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(28.5);
    }
    if (lml1_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(29.5);
    }
    if (lml2_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(30.5);
    }
    if (lml3_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(31.5);
    }
    if (lml4_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(32.5);
    }
    if (lml5_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(33.5);
    }
    if (lml6_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(34.5);
    }
    if (lml7_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(35.5);
    }
    if (lml8_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(36.5);
    }
    if (lml9_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(37.5);
    }
    if (lml10_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(38.5);
    }
    if (lml12_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(39.5);
    }
    if (lml13_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(40.5);
    }
    if (eclmumu_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(41.5);
    }
    if (mu_b2b_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(42.5);
    }
    if (mu_eb2b_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(43.5);
    }
    if (cdcklm1_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(44.5);
    }
    if (cdcklm2_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(45.5);
    }
    if (klm_hit_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(46.5);
    }
    if (eklm_hit_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(47.5);
    }
    if (mu_b2b_fired  && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(48.5);
    }
    if (mu_eb2b_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(49.5);
    }
    if (cdcklm1_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(50.5);
    }
    if (cdcklm2_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(51.5);
    }
    if (klm_hit_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(52.5);
    }
    if (eklm_hit_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(53.5);
    }
    if (cdcecl1_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(54.5);
    }
    if (cdcecl2_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(55.5);
    }
    if (cdcecl3_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(56.5);
    }
    if (cdcecl4_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_extra[skim[ifill]]->Fill(57.5);
    }
    if (cdcecl1_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(58.5);
    }
    if (cdcecl2_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(59.5);
    }
    if (cdcecl3_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(60.5);
    }
    if (cdcecl4_fired && lml_fired) {
      h_psn_extra[skim[ifill]]->Fill(61.5);
    }
  }

  for (unsigned ifill = 0; ifill < skim.size(); ifill++) {
    //fill efficiency values
    h_eff[skim[ifill]]->SetBinContent(1,  h_psn_extra[skim[ifill]]->GetBinContent(1 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //fff with c4|hie
    h_eff[skim[ifill]]->SetBinContent(2,  h_psn_extra[skim[ifill]]->GetBinContent(2 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //ffo with c4|hie
    h_eff[skim[ifill]]->SetBinContent(3,  h_psn_extra[skim[ifill]]->GetBinContent(3 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //ffb with c4|hie
    h_eff[skim[ifill]]->SetBinContent(4,  h_psn_extra[skim[ifill]]->GetBinContent(19 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //ffy with c4|hie
    h_eff[skim[ifill]]->SetBinContent(5,  h_psn_extra[skim[ifill]]->GetBinContent(20 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //fyo with c4|hie
    h_eff[skim[ifill]]->SetBinContent(6,  h_psn_extra[skim[ifill]]->GetBinContent(21 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        5 + 1)); //fyb with c4|hie
    h_eff[skim[ifill]]->SetBinContent(7,  h_psn_extra[skim[ifill]]->GetBinContent(27 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //hie with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(8,  h_psn_extra[skim[ifill]]->GetBinContent(26 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //c4 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(9,  h_psn_extra[skim[ifill]]->GetBinContent(28 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml0 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(10, h_psn_extra[skim[ifill]]->GetBinContent(29 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml1 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(11, h_psn_extra[skim[ifill]]->GetBinContent(30 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml2 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(12, h_psn_extra[skim[ifill]]->GetBinContent(31 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml3 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(13, h_psn_extra[skim[ifill]]->GetBinContent(32 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml4 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(14, h_psn_extra[skim[ifill]]->GetBinContent(33 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml5 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(15, h_psn_extra[skim[ifill]]->GetBinContent(34 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml6 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(16, h_psn_extra[skim[ifill]]->GetBinContent(35 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml7 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(17, h_psn_extra[skim[ifill]]->GetBinContent(36 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml8 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(18, h_psn_extra[skim[ifill]]->GetBinContent(37 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml9 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(19, h_psn_extra[skim[ifill]]->GetBinContent(38 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml10 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(20, h_psn_extra[skim[ifill]]->GetBinContent(39 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml12 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(21, h_psn_extra[skim[ifill]]->GetBinContent(40 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //lml13 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(22, h_psn_extra[skim[ifill]]->GetBinContent(9 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        0 + 1)); //bha3d with all
    h_eff[skim[ifill]]->SetBinContent(23, h_psn_extra[skim[ifill]]->GetBinContent(42 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //mu_b2b with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(24, h_psn_extra[skim[ifill]]->GetBinContent(48 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //mu_b2b with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(25, h_psn_extra[skim[ifill]]->GetBinContent(49 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //mu_eb2b with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(26, h_psn_extra[skim[ifill]]->GetBinContent(44 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //cdcklm1 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(27, h_psn_extra[skim[ifill]]->GetBinContent(45 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        6 + 1)); //cdcklm2 with fff|ffo|ffb
    h_eff[skim[ifill]]->SetBinContent(28, h_psn_extra[skim[ifill]]->GetBinContent(15 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //fff with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(29, h_psn_extra[skim[ifill]]->GetBinContent(16 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //ffo with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(30, h_psn_extra[skim[ifill]]->GetBinContent(17 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //ffb with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(31, h_psn_extra[skim[ifill]]->GetBinContent(11 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //ff with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(32, h_psn_extra[skim[ifill]]->GetBinContent(13 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //f with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(33, h_psn_extra[skim[ifill]]->GetBinContent(23 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //ffy with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(34, h_psn_extra[skim[ifill]]->GetBinContent(24 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //fyo with lml|eclmumu
    h_eff[skim[ifill]]->SetBinContent(35, h_psn_extra[skim[ifill]]->GetBinContent(25 + 1) / h_psn_extra[skim[ifill]]->GetBinContent(
                                        14 + 1)); //fyb with lml|eclmumu
  }

  if (m_skim != 0) {
    h_eff_shifter->SetBinContent(1,  h_psn_extra[1]->GetBinContent(1 + 1) / h_psn_extra[1]->GetBinContent(5 + 1)); //fff with c4|hie
    h_eff_shifter->SetBinContent(2,  h_psn_extra[1]->GetBinContent(2 + 1) / h_psn_extra[1]->GetBinContent(5 + 1)); //ffo with c4|hie
    h_eff_shifter->SetBinContent(3,  h_psn_extra[1]->GetBinContent(19 + 1) / h_psn_extra[1]->GetBinContent(5 + 1)); //ffy with c4|hie
    h_eff_shifter->SetBinContent(4,  h_psn_extra[1]->GetBinContent(20 + 1) / h_psn_extra[1]->GetBinContent(5 + 1)); //fyo with c4|hie
    h_eff_shifter->SetBinContent(5,  h_psn_extra[1]->GetBinContent(27 + 1) / h_psn_extra[1]->GetBinContent(
                                   6 + 1)); //hie with fff|ffo|ffb
    h_eff_shifter->SetBinContent(6,  h_psn_extra[1]->GetBinContent(26 + 1) / h_psn_extra[1]->GetBinContent(
                                   6 + 1)); //c4 with fff|ffo|ffb
    h_eff_shifter->SetBinContent(7,  h_psn_extra[4]->GetBinContent(48 + 1) / h_psn_extra[4]->GetBinContent(
                                   14 + 1) * 10); //mu_b2b with lml|eclmumu
    h_eff_shifter->SetBinContent(8,  h_psn_extra[4]->GetBinContent(49 + 1) / h_psn_extra[4]->GetBinContent(
                                   14 + 1) * 50); //mu_eb2b with lml|eclmumu
  }

}

const char* TRGGDLDQMModule::output_extra[n_output_extra] = {
  "all", "fff&(c4|hie)", "ffo&(c4|hie)", "ffb&(c4|hie)", "fff", "c4|hie", "fff|ffo|ffb", "(fff|ffo|ffb)&(c4|hie)", "bha2D", "bha3D",
  "ff", "ff&(lml|eclmumu)", "f", "f&(lml|eclmumu)", "lml|eclmumu", "fff&(lml|eclmumu)", "ffo&(lml|eclmumu)", "ffb&(lml|eclmumu)", "ffy", "ffy&(c4|hie)",
  "fyo&(c4|hie)", "fyb&(c4|hie)", "(ffy|ffo|ffb)&(c4|hie)", "ffy&(lml|eclmumu)", "fyo&(lml|eclmumu)", "fyb&(lml|eclmumu)", "c4&(fff|ffo|ffb)", "hie&(fff|ffo|ffb)", "lml0&(fff|ffo|ffb)", "lml1&(fff|ffo|ffb)",
  "lml2&(fff|ffo|ffb)", "lml3&(fff|ffo|ffb)", "lml4&(fff|ffo|ffb)", "lml5&(fff|ffo|ffb)", "lml6&(fff|ffo|ffb)", "lml7&(fff|ffo|ffb)", "lml8&(fff|ffo|ffb)", "lml9&(fff|ffo|ffb)", "lml10&(fff|ffo|ffb)", "lml12&(fff|ffo|ffb)",
  "lml13&(fff|ffo|ffb)", "eclmumu&(fff|ffo|ffb)", "mu_b2b&(fff|ffo|ffb)", "mu_eb2b&(fff|ffo|ffb)", "cdcklm1&(fff|ffo|ffb)", "cdcklm2&(fff|ffo|ffb)", "klm_hit&(fff|ffo|ffb)", "eklm_hit&(fff|ffo|ffb)", "mu_b2b&(lml|eclmumu)", "mu_eb2b&(lml|eclmumu)",
  "cdcklm1&(lml|eclmumu)", "cdcklm2&(lml|eclmumu)", "klm_hit&(lml|eclmumu)", "eklm_hit&(lml|eclmumu)", "cdcecl1&(fff|ffo|ffb)", "cdcecl2&(fff|ffo|ffb)", "cdcecl3&(fff|ffo|ffb)", "cdcecl4&(fff|ffo|ffb)", "cdcecl1&(lml|eclmumu)", "cdcecl2&(lml|eclmumu)",
  "cdcecl3&(lml|eclmumu)", "cdcecl4&(lml|eclmumu)"
};

const char* TRGGDLDQMModule::c_eff_shifter[n_eff_shifter] = {
  "CDC fff",
  "CDC ffo",
  "CDC ffy",
  "CDC fyo",
  "ECL hie",
  "ECL c4",
  "KLM b2b,x10",
  "EKLM eb2b,x50"
};

const char* TRGGDLDQMModule::c_eff[n_eff] = {
  "fff with c4|hie",
  "ffo with c4|hie",
  "ffb with c4|hie",
  "ffy with c4|hie",
  "fyo with c4|hie",
  "fyb with c4|hie",
  "hie with fff|ffo|ffb",
  "c4 with fff|ffo|ffb",
  "lml0 with fff|ffo|ffb",
  "lml1 with fff|ffo|ffb",
  "lml2 with fff|ffo|ffb",
  "lml3 with fff|ffo|ffb",
  "lml4 with fff|ffo|ffb",
  "lml5 with fff|ffo|ffb",
  "lml6 with fff|ffo|ffb",
  "lml7 with fff|ffo|ffb",
  "lml8 with fff|ffo|ffb",
  "lml9 with fff|ffo|ffb",
  "lml10 with fff|ffo|ffb",
  "lml12 with fff|ffo|ffb",
  "lml13 with fff|ffo|ffb",
  "bha3d with all",
  "mu_b2b with fff|ffo|ffb",
  "mu_b2b with lml|eclmumu",
  "mu_eb2b with lml|eclmumu",
  "cdcklm1 with fff|ffo|ffb",
  "cdcklm2 with fff|ffo|ffb",
  "fff with lml|eclmumu",
  "ffo with lml|eclmumu",
  "ffb with lml|eclmumu",
  "ff with lml|eclmumu",
  "f with lml|eclmumu",
  "ffy with lml|eclmumu",
  "fyo with lml|eclmumu",
  "fyb with lml|eclmumu"
};


void
TRGGDLDQMModule::fillOutputPureExtra(void)
{
  //get offline CDC track information
  StoreArray<Track> Tracks;
  int n_fulltrack = 0;
  float max_dphi = 0;
  float phi_list[100];
  for (int itrack = 0; itrack < Tracks.getEntries(); itrack++) {
    const TrackFitResult* tfr = Tracks[itrack]->getTrackFitResult(Const::pion);
    if (tfr == nullptr) continue;

    float z0     = tfr->getZ0();
    float d0     = tfr->getD0();
    float phi    = tfr->getPhi();
    //float omega  = tfr->getOmega();
    int   flayer = tfr->getHitPatternCDC().getFirstLayer();
    int   llayer = tfr->getHitPatternCDC().getLastLayer();
    float pt     = tfr->getTransverseMomentum();
    //std::cout << z0 << " " << d0 << " " << omega << " " << flayer << " " << llayer << " " << pt << std::endl;
    if (z0 > -1 && z0 < 1 && d0 > -1 && d0 < 1 && flayer < 8 && llayer > 50
        && pt > 0.3) { //select track from IP, hit SL0 and SL8, pt>0.3GeV
      phi_list[n_fulltrack] = phi;
      n_fulltrack += 1;
    }
  }
  for (int i = 0; i < n_fulltrack; i++) {
    for (int j = 0; j < n_fulltrack; j++) {
      float dphi = phi_list[i] - phi_list[j];
      if (dphi < 0)   dphi = -dphi;
      if (dphi > 3.14) dphi = 2 * 3.14 - dphi;
      if (dphi > max_dphi) max_dphi = dphi;
    }
  }

  //get offline ECL cluster information
  //StoreArray<ECLCluster> ECLClusters;
  //double total_energy = 0;
  //for (int iclst = 0; iclst < ECLClusters.getEntries(); iclst++) {
  //  total_energy += ECLClusters[iclst]->getEnergyRaw();
  //}
  //
  //
  int ecl_timing_threshold_low  = -200; // (ns)  xtal timing selection
  int ecl_timing_threshold_high =  200; // (ns)  xtal timing selection
  double ecl_xtcid_energy_sum[576] = {0};
  double total_energy = 0;
  int ncluster = 0;
  for (const auto& eclcalhit : m_ECLCalDigitData) {

    // (ecl) calibation status check and cut
    if (!eclcalhit.isCalibrated()) {continue;}
    if (eclcalhit.isFailedFit()) {continue;}
    if (eclcalhit.isTimeResolutionFailed()) {continue;}

    // (ecl) xtal-id
    int ecl_cid   = (double) eclcalhit.getCellId();

    // (ecl) fitter quality check and cut
    int ecl_quality = -1;
    for (const auto& eclhit : m_ECLDigitData) {
      if (ecl_cid == eclhit.getCellId()) {
        ecl_quality = (int) eclhit.getQuality();
      }
    }
    if (ecl_quality != 0) {continue;}

    // (ecl) xtal energy
    double ecl_xtal_energy  = eclcalhit.getEnergy(); // ECLCalDigit


    // (ecl) timing cut
    int ecl_timing = eclcalhit.getTime();
    if (ecl_timing < ecl_timing_threshold_low ||
        ecl_timing > ecl_timing_threshold_high) {continue;}

    // (ecl) tc-id for xtal-id
    int ecl_tcid  = trgeclmap->getTCIdFromXtalId(ecl_cid);
    int ecl_thetaid = trgeclmap->getTCThetaIdFromTCId(ecl_tcid);

    if (ecl_tcid >= 0 && ecl_tcid < 576 && ecl_thetaid >= 2 && ecl_thetaid <= 15) { //pick up only 2=<thetaid=<15
      ecl_xtcid_energy_sum[ecl_tcid] = ecl_xtcid_energy_sum[ecl_tcid] + ecl_xtal_energy;
      //ecltimingsum[i] = ecl_timing;
    }
  }

  for (int i = 0; i < 576; i++) {
    if (ecl_xtcid_energy_sum[i] > 0.1) {
      total_energy += ecl_xtcid_energy_sum[i];
      ncluster += 1;
    }
  }

  //get offline KLM cluster information


  //fff: require the number of CDC full tracks is more than or equal to 3
  if (n_fulltrack > 2) {
    //std::cout << "fff" << std::endl;
    bool fff_fired = isFired("fff");
    bool ffy_fired = isFired("ffy");
    bool c4_fired  = isFired("C4");
    bool hie_fired = isFired("HIE");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(0.5);
    }
    if (fff_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(1.5);
    }
    if (ffy_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(2.5);
    }
  }
  //ffo: require the number of CDC full tracks is more than or equal to 2, opening angle > 90deg
  if (n_fulltrack > 1 && max_dphi > 3.14 / 2.) {
    //std::cout << "fff" << std::endl;
    bool ffo_fired = isFired("ffo");
    bool fyo_fired = isFired("fyo");
    bool c4_fired  = isFired("C4");
    bool hie_fired = isFired("HIE");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(3.5);
    }
    if (ffo_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(4.5);
    }
    if (fyo_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(5.5);
    }
  }
  //ffo: require the number of CDC full tracks is more than or equal to 2, opening angle >150deg
  if (n_fulltrack > 1 && max_dphi > 3.14 * 5 / 6.) {
    //std::cout << "fff" << std::endl;
    bool ffb_fired = isFired("ffb");
    bool fyb_fired = isFired("fyb");
    bool c4_fired  = isFired("C4");
    bool hie_fired = isFired("HIE");
    if (c4_fired || hie_fired) {
      h_psn_pure_extra[0]->Fill(6.5);
    }
    if (ffb_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(7.5);
    }
    if (fyb_fired && (c4_fired || hie_fired)) {
      h_psn_pure_extra[0]->Fill(8.5);
    }
  }

  //hie: require the total energy of ECL cluster is more than 1GeV
  if (total_energy > 1) {
    //std::cout << "hie" << std::endl;
    bool fff_fired = isFired("FFF");
    bool ffo_fired = isFired("FFO");
    bool ffb_fired = isFired("FFB");
    bool hie_fired = isFired("hie");
    if (fff_fired || ffo_fired || ffb_fired) {
      h_psn_pure_extra[0]->Fill(9.5);
    }
    if (hie_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_pure_extra[0]->Fill(10.5);
    }
  }

  //c4: require the total number of cluster is more than 3
  if (ncluster > 3) {
    //std::cout << "hie" << std::endl;
    bool fff_fired = isFired("FFF");
    bool ffo_fired = isFired("FFO");
    bool ffb_fired = isFired("FFB");
    bool c4_fired = isFired("c4");
    if (fff_fired || ffo_fired || ffb_fired) {
      h_psn_pure_extra[0]->Fill(11.5);
    }
    if (c4_fired && (fff_fired || ffo_fired || ffb_fired)) {
      h_psn_pure_extra[0]->Fill(12.5);
    }
  }

  h_pure_eff[0]->SetBinContent(1,  h_psn_pure_extra[0]->GetBinContent(1 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 0 + 1)); //fff with c4|hie
  h_pure_eff[0]->SetBinContent(2,  h_psn_pure_extra[0]->GetBinContent(4 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 3 + 1)); //ffo with c4|hie
  h_pure_eff[0]->SetBinContent(3,  h_psn_pure_extra[0]->GetBinContent(7 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 6 + 1)); //ffb with c4|hie
  h_pure_eff[0]->SetBinContent(4,  h_psn_pure_extra[0]->GetBinContent(2 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 0 + 1)); //ffy with c4|hie
  h_pure_eff[0]->SetBinContent(5,  h_psn_pure_extra[0]->GetBinContent(5 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 3 + 1)); //fyo with c4|hie
  h_pure_eff[0]->SetBinContent(6,  h_psn_pure_extra[0]->GetBinContent(8 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 6 + 1)); //fyb with c4|hie
  h_pure_eff[0]->SetBinContent(7,  h_psn_pure_extra[0]->GetBinContent(10 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 9 + 1)); //hie with fff|ffo|ffb
  h_pure_eff[0]->SetBinContent(8,  h_psn_pure_extra[0]->GetBinContent(12 + 1) / h_psn_pure_extra[0]->GetBinContent(
                                 11 + 1)); //hie with fff|ffo|ffb

}

const char* TRGGDLDQMModule::output_pure_extra[n_output_pure_extra] = {
  "c4|hie offline_fff", "fff&(c4|hie) offline_fff", "ffy&(c4|hie) offline_fff",
  "c4|hie offline_ffo", "ffo&(c4|hie) offline_ffo", "fyo&(c4|hie) offline_ffo",
  "c4|hie offline_ffb", "ffb&(c4|hie) offline_ffb", "fyb&(c4|hie) offline_ffb",
  "fff|ffb|ffo offline_hie", "hie&(fff|ffb|ffo) offline_hie",
  "fff|ffb|ffo offline_c4", "c4&(fff|ffb|ffo) offline_c4"
};


const char* TRGGDLDQMModule::c_pure_eff[n_pure_eff] = {
  "fff with c4|hie",
  "ffo with c4|hie",
  "ffb with c4|hie",
  "ffy with c4|hie",
  "fyo with c4|hie",
  "fyb with c4|hie",
  "hie with fff|ffo|ffb",
  "c4 with fff|ffo|ffb"
};


