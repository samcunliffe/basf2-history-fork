/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Wenlong Yuan                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <top/modules/TOPLaserCalibrator/LaserCalibratorFit.h>

#include <framework/logging/Logger.h>

#include <Math/PdfFuncMathCore.h>
#include <Math/MinimizerOptions.h>
#include <TFile.h>
#include <TTree.h>

namespace Belle2 {
  namespace TOP {
//Crystal Ball function
    double fcnCB(double* x, double* par)
    {
      double m1 = par[1];
      double s1 = par[2];
      double a1 = par[3];
      double n1 = par[4];
      double f1 = ROOT::Math::crystalball_pdf(x[0], a1, n1, s1, m1);
      return par[0] * f1;
    }

//double Crystal Ball function
    double fcnCB2(double* x, double* par)
    {
      double m1 = par[1];
      double s1 = par[2];
      double a = par[3];
      double n = par[4];
      double m2 = m1 - par[6];
      double s2 = par[7];

      double f1 = ROOT::Math::crystalball_pdf(x[0], a, n, s1, m1);
      double f2 = ROOT::Math::crystalball_pdf(x[0], a, n, s2, m2);
      return par[0] * f1 + par[5] * f2;
    }

    LaserCalibratorFit::LaserCalibratorFit(unsigned moduleID):
      m_moduleID(moduleID)
    {
    }

    LaserCalibratorFit::~LaserCalibratorFit()
    {
    }

//initial TF1 of all channels in one slot
    void LaserCalibratorFit::setHist(std::vector<TH1F*> hist)
    {
      m_hist = hist;
      for (unsigned i = 0; i < hist.size(); i++) {
        m_func.push_back(0);
      }
    }

    double LaserCalibratorFit::getFitChisq(unsigned channel)
    {
      if (m_func[channel]) {
        return m_func[channel]->GetChisquare();
      } else {
        return -9;
      };
    }

    int LaserCalibratorFit::fitChannel(unsigned channel)
    {
      if (channel > m_hist.size()) {
        B2WARNING("Wrong channel!");
        return 0;
      }
      int hid = channel;
      if (!m_hist[hid]) {
        B2WARNING("No Evts. in channel " << channel);
        return 0;
      }
      //require at least 10 evts for each fit
      if (m_hist[hid]->GetEntries() < 10) {
        B2WARNING("Too little Evts. for fitting (Evts. < 10)!");
        return 0;
      }
      if (m_fitMethod == "gauss") {
        m_func[hid] = makeGFit(m_hist[hid]);
      } else if (m_fitMethod == "cb") {
        m_func[hid] = makeCBFit(m_hist[hid]);
      } else if (m_fitMethod == "cb2") {
        m_func[hid] = makeCB2Fit(m_hist[hid], channel, 0);
      } else {
        B2WARNING("No matched fit method!");
        return 0;
      }

      return 1;
    }

    void LaserCalibratorFit::writeFile(std::string outfile = "laserfit.root")
    {
      auto file = new TFile(outfile.c_str(), "RECREATE");
      auto otree = new TTree("fits", "fitted times");

      unsigned channel = 0;

      if (m_fitMethod == "gauss") {
        double parms[3];

        otree->Branch("channel", &channel, "channel/I");
        otree->Branch("norm", &(parms[0]), "norm/D");
        otree->Branch("time", &(parms[1]), "time/D");
        otree->Branch("reso", &(parms[2]), "reso/D");

        for (auto& f : m_func) {
          channel++;
          if (f) {
            f->GetParameters(parms);
            otree->Fill();
          }
        }
      } else if (m_fitMethod == "cb2") {
        double parms[8];

        double time2 = 0;
        otree->Branch("channel", &channel, "channel/I");
        otree->Branch("norm1", &(parms[0]), "norm1/D");
        otree->Branch("time1", &(parms[1]), "time1/D");
        otree->Branch("reso1", &(parms[2]), "reso1/D");
        otree->Branch("alpha_CB", &(parms[3]), "alpha_CB/D");
        otree->Branch("n_CB", &(parms[4]), "n_CB/D");
        otree->Branch("norm2", &(parms[5]), "norm2/D");
        otree->Branch("dt12", &(parms[6]), "dt12/D");
        otree->Branch("reso2", &(parms[7]), "reso2/D");
        otree->Branch("time2", &time2, "time2/D");

        for (auto& f : m_func) {
          channel++;
          if (f) {
            f->GetParameters(parms);
            otree->Fill();
          }
        }
      } else if (m_fitMethod == "cb") {
        double parms[5];

        otree->Branch("channel", &channel, "channel/I");
        otree->Branch("norm", &(parms[0]), "norm/D");
        otree->Branch("time", &(parms[1]), "time/D");
        otree->Branch("reso", &(parms[2]), "reso/D");
        otree->Branch("alpha_CB", &(parms[3]), "alpha_CB/D");
        otree->Branch("n_CB", &(parms[4]), "n_CB/D");

        for (auto& f : m_func) {
          channel++;
          if (f) {
            f->GetParameters(parms);
            otree->Fill();
          }
        }
      } else {
        B2WARNING("Nothing be written to files!");
        return;
      }
      for (auto& h : m_hist) {
        if (h && h->GetEntries() >= 10) {
          h->Write();
        }
      }
      otree->Write();
      file->Close();
    }

// single Gaussian fit
    TF1* LaserCalibratorFit::makeGFit(TH1F* h)
    {
      h->SetAxisRange(m_xmin, m_xmax);
      double m = h->GetMean();
      double w = h->GetRMS();
      h->Fit("gaus", "Q", "", m - 2.*w, m + 4.*w);
      double parms[3];
      TF1* func = h->GetFunction("gaus");
      func->GetParameters(parms);
      func->SetNpx(1000);
      h->Fit("gaus", "Q", "", m - w, m + w);
      func = h->GetFunction("gaus");
      func->GetParameters(parms);

      return func;
    }

// single Crystal Ball fit
    TF1* LaserCalibratorFit::makeCBFit(TH1F* h)
    {
      h->SetAxisRange(m_xmin, m_xmax);
      auto func = new TF1("fcnCB", fcnCB, m_xmin, m_xmax, 5);

      double parms[5];
      parms[0] = 1;
      parms[1] = h->GetMean();
      parms[2] = h->GetRMS();
      parms[3] = -0.5;
      parms[4] = 4;

      func->SetParameters(parms);
      func->SetNpx(1000);

      func->SetParName(0, "norm");
      func->SetParName(1, "time");
      func->SetParName(2, "sigma");
      func->SetParName(3, "alpha_CB");
      func->SetParName(4, "n_CB");

      //func->SetParLimits(1,par[1]-0.2,par[1]+0.2);
      //func->SetParLimits(2,par[2]-0.02,par[2]+0.08);
      func->SetParLimits(3, -5, 0);
      func->SetParLimits(4, 0, 10);

      h->Fit(func, "Q", "");
      func->GetParameters(parms);
      return func;
    }

// double Crystal Ball fit
    TF1* LaserCalibratorFit::makeCB2Fit(TH1F* h, unsigned channel, bool minOut)
    {
      h->SetAxisRange(m_xmin, m_xmax);
      auto func = new TF1("fcnCB2", fcnCB2, m_xmin, m_xmax, 8);

      double vdt[8] = {0.272, 0.242, 0.208, 0.178, 0.113, 0.082, 0.0485, 0.017}; //input para. from MC study, need further studies
      double parms[8];
      //the input para. need further studies
      parms[0] = 1;
      parms[1] = 0.6;
      parms[2] = 0.1;
      parms[3] = -0.5;
      parms[4] = 4;
      parms[5] = 1;
      parms[6] = vdt[(channel) / 64]; //refers to a typical time separation of two main peaks
      //parms[6] = 0.3;
      parms[7] = 0.1;

      func->SetParameters(parms);
      func->SetNpx(1000);

      func->SetParName(0, "norm1");
      func->SetParName(1, "time1");
      func->SetParName(2, "sigma1");
      func->SetParName(3, "alpha_CB");
      func->SetParName(4, "n_CB");
      func->SetParName(5, "norm2");
      func->SetParName(6, "dt12");
      func->SetParName(7, "sigma2");

      func->SetParLimits(1, parms[1] - 0.25, parms[1] + 0.15);
      //func->SetParLimits(2,par[2]-0.02,par[2]+0.08);
      func->SetParLimits(3, -5, 0);
      func->SetParLimits(4, 0, 10);
      func->SetParLimits(6, parms[6] * 0.7, parms[6] * 1.5);

      ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(50000); //set to a large number for a test
      if (minOut) {
        h->Fit(func, "Q", "");
      } else {
        h->Fit(func);
      }
      func->GetParameters(parms);
      return func;
    }
  } // TOP namespace
}//  Belle2 namespace
