#include <TFile.h>
#include <TTree.h>
#include <TCut.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <topcaf/dataobjects/TOPCAFDigit.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TMultiGraph.h>
#include <TH2F.h>
#include <iostream>
#include <string>
#include <utility>

using namespace std;
using namespace Belle2;

bool analyzeHits(const char* filename)
{

  topcaf_channel_id_t chan = 4203030700;

  TFile fileIn(filename);

  TTree* t;

  fileIn.GetObject("tree", t);

  TFile fout("hits.root", "RECREATE");

  TH1D* times = new TH1D("times", "times", 3000, -1500., 1500.);
  TH1D* tdc_bins = new TH1D("tdc_bins", "tdc bins", 3000, -1500., 1500.);
  TH1D* time_diffs = new TH1D("time_diffs", "time differences", 15000, 0., 1500.);
  TH1D* widths = new TH1D("widths", "widths", 1000, 0., 100.);
  TH2F* width_time = new TH2F("width_time", "widths vs times", 1500, 0., 1500., 1000, 0., 100.);
  TH2F* ch_time = new TH2F("ch_time", "time vs channel", 512, 0., 512., 3000, -1500., 1500.);
  TH2F* ch_tdcbin = new TH2F("ch_tdcbin", "tdc bin vs channel", 512, 0., 512., 1500, 0., 1500.);

  TCut channel_cut(Form("TOPCAFDigits.m_channel_id==%lld", chan));
  TCut width_cut("TOPCAFDigits.m_width>3.5");

  t->Draw("TOPCAFDigits.m_tdc_bin>>tdc_bin", channel_cut && width_cut);
  t->Draw("TOPCAFDigits.m_time>>times", channel_cut && width_cut);
  t->Draw("TOPCAFDigits.m_width>>widths", channel_cut);
  t->Draw("TOPCAFDigits.m_width:TOPCAFDigits.m_tdc_bin>>width_time", channel_cut);
  t->Draw("TOPCAFDigits.m_tdc_bin:TOPCAFDigits.m_pixel_id>>ch_tdcbin", width_cut);
  t->Draw("TOPCAFDigits.m_time:TOPCAFDigits.m_pixel_id>>ch_time", width_cut);


  fout.cd();
  tdc_bins->Write();
  times->Write();
  widths->Write();
  width_time->Write();
  ch_time->Write();
  ch_tdcbin->Write();

  TTreeReader theReader("tree", &fileIn);
  TTreeReaderArray<Belle2::TOPCAFDigit> hitsRV(theReader, "TOPCAFDigits");
  while (theReader.Next()) {
    //    unsigned int thissize = hitsRV.GetSize();


    for (unsigned int c = 0; c < hitsRV.GetSize(); c++) {
      if (hitsRV[c].GetChannelID() == chan && hitsRV[c].GetWidth() > 2.5) {
        for (unsigned int d = c + 1; d < hitsRV.GetSize(); d++) {

          if (hitsRV[d].GetChannelID() == chan && hitsRV[c].GetWidth() > 2.5) {
            double diff = hitsRV[c].GetTDCBin() - hitsRV[d].GetTDCBin();
            diff = TMath::Abs(diff);

            time_diffs->Fill(diff);
          }
        }
      }
    }
  }
  time_diffs->Write();
  fout.Close();
  return true;

}

int main(int argc, char* argv[])
{
  if (argc == 2) {
    analyzeHits(argv[1]);
  }

  return 0;
}
