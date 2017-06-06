#include "TProfile.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TH1D.h"
#include "Math/ChebyshevPol.h"
#include "iostream"

using namespace std;

Double_t pol5pol1(Double_t* x, Double_t* par)
{
  Double_t xx = x[0];
  Double_t x6, x2;
  Double_t f, ctp;
  if (xx < par[6]) {
    f = par[0] + par[1] * xx + par[2] * xx * xx + par[3] * xx * xx * xx + par[4] * xx * xx * xx * xx + par[5] * xx * xx * xx * xx * xx;
  } else {
    x6 = par[6];
    x2 = xx - x6;
    ctp = par[0] + par[1] * x6 + par[2] * x6 * x6 + par[3] * x6 * x6 * x6 + par[4] * x6 * x6 * x6 * x6 + par[5] * x6 * x6 * x6 * x6 *
          x6;
    f = par[7] * x2 + ctp;
  }
  return f;
}

Double_t Cheb5pol1(Double_t* x, Double_t* par)
{
  Double_t xx = x[0];
  Double_t x6, x2;
  Double_t f, ctp;

  if (xx < par[6]) {
    f = ROOT::Math::Chebyshev5(xx, par[0], par[1], par[2], par[3], par[4], par[5]);
  } else {
    x6 = par[6];
    x2 = xx - x6;
    ctp = ROOT::Math::Chebyshev5(x6, par[0], par[1], par[2], par[3], par[4], par[5]);
    f = par[7] * x2 + ctp;
  }
  return f;
}

class XT {
public:
  TProfile* m_h1;
  TF1* xtpol5 = new TF1("xtpol5", pol5pol1, 0.0, 400, 8);
  TF1* xtCheb5 = new TF1("xtCheb5", Cheb5pol1, 0.0, 400, 8);
  //  TF1* xtCheb5 = (TF1*)gROOT->GetFunction("Chebyshev5");

  XT(TProfile* h1)
  {
    m_h1 = (TProfile*)h1->Clone();
    m_h1->SetDirectory(0);
  }


  XT(TProfile* h1, int mode)
  {
    m_h1 = (TProfile*)h1->Clone();
    m_h1->SetDirectory(0);
    m_mode = mode;
  }

  XT(TH1D* h1, int mode)
  {
    m_h1 = (TProfile*)h1->Clone();
    m_h1->SetDirectory(0);
    m_mode = mode;
  }

  /*Set Parameter 6 for polynomia fit*/
  void  setP6(double p6)
  {
    m_XTParam[6] = p6;
  }
  void setMode(int mode)
  {
    m_mode = mode;
  }
  /*SetParamerters for fit*/
  void  setXTParams(double p[8])
  {
    for (int i = 0; i < 8; ++i) {m_XTParam[i] = p[i];}
    m_tmax = p[6] + 50;
  }

  void setXTParams(double p0, double p1, double p2, double p3,
                   double p4, double p5, double p6, double p7)
  {
    m_XTParam[0] = p0;     m_XTParam[1] = p1;     m_XTParam[2] = p2;
    m_XTParam[3] = p3;     m_XTParam[4] = p4;     m_XTParam[5] = p5;
    m_XTParam[6] = p6;     m_XTParam[7] = p7;
    m_tmax = p6 + 50;
  }

  /*Set Fit range*/
  void  setFitRange(double tmin, double tmax)
  {
    m_tmin = tmin;
    m_tmax = tmax;
  }
  /*Set minimum number of entry required for fit*/
  void setSmallestEntryRequired(int min)
  {
    m_minRequiredEntry = min;
  }
  /*Set Debug*/
  void setDebug(bool debug)
  {
    m_debug = debug;
  }
  /* get fitted flag*/
  int getFitStatus()
  {
    return m_fitflag;
  }
  double getProb()
  {
    return m_Prob;
  }
  /*get fit params*/
  void  getFittedXTParams(double pa[8])
  {
    for (int i = 0; i < 8; ++i) {pa[i] = m_FittedXTParams[i];}
  }
  /*getXTFunction*/
  TF1* getXTFunction(int mode)
  {
    if (mode == 0) return xtpol5;
    else return xtCheb5;
  };
  TF1* getXTFunction()
  {
    if (m_mode == 0) return xtpol5;
    else return xtCheb5;
  };
  /*get histo*/
  TProfile* getFittedHisto() {return m_h1;}
  /*do fitting*/
  void FitXT()
  {
    if (m_mode == 0) FitPol5();
    else FitChebyshev();
  }
  void FitXT(int mode)
  {
    if (mode == 0) FitPol5();
    else FitChebyshev();
  }
  void FitPol5();
  void FitChebyshev();
private:
  int m_mode = 1; //0: Poly5, 1:Chebyshev
  bool m_debug = true;
  bool m_draw = false;
  int m_minRequiredEntry = 800;
  double m_XTParam[8];
  double m_FittedXTParams[8];
  int m_fitflag = 0;
  double m_Prob = 0;;
  double m_tmin = 20;
  double m_tmax = m_XTParam[6] + 50;
};

void XT::FitPol5()
{
  /*
  if(m_h1->GetEntries() <m_minRequiredEntry){
    m_fitflag = -1;
    return;
  }
  */
  double max_dif = 0.12;
  double max_dif2 = 0.05;
  double par[8];
  m_h1->Fit("pol1", "MQ", "", m_tmin, 50);
  TF1* f1 = (TF1*)m_h1->GetFunction("pol1");
  double p0 = f1->GetParameter(0);
  double p1 = f1->GetParameter(1);
  double f10 = f1->Eval(10);
  /****************************/
  int in = 0; /*how many time inner part change fit limit*/
  int out = 0; /*how many time outer part change fit limit*/
  xtpol5->SetParameters(p0, p1, 0, 0, 0, 0, m_XTParam[6], 0);
  double p6default = m_XTParam[6];
  xtpol5->SetParLimits(7, 0.0, 0.001);
  xtpol5->SetParLimits(1, 0.0, 0.006);
  //  xtpol5->SetParLimits(6, p6default - 30,  p6default + 30);
  for (int i = 0; i < 10; ++i) {
    m_fitflag = 1;
    cout << "Fitting" << endl;
    double stat = m_h1->Fit(xtpol5, "M", "0", m_tmin, m_tmax);
    xtpol5->GetParameters(par);

    /*Eval outer region,*/
    double fp6 = xtpol5->Eval(par[6]);
    double fbehindp6 = xtpol5->Eval(par[6] - 12) - 0.01;
    if (fp6 < fbehindp6 || fp6 > 1) { /*may be change to good value*/
      m_fitflag = 2;
      out += 1;
      xtpol5->SetParameters(p0, p1, 0, 0, 0, 0, p6default, 0);
      xtpol5->SetParLimits(6, p6default - 10,  p6default + 10 - out / 2);
      m_tmax += 2;
      if (m_tmax < p6default + 30) {
        m_tmax = p6default + 30;
      }
    }
    /* EVal inner region*/
    /* compare p0 of linear fit vs p0 of xtfun fit and eval point t=10)*/
    if (fabs(par[0] - p0) > max_dif || fabs(f10 - xtpol5->Eval(10)) > max_dif2) {
      m_fitflag = 3;
      if (i == 9) cout << "ERROR XT FIT inner part" << endl;
      in += 1;
      xtpol5->SetParameters(p0, p1, 0, 0, 0, 0, p6default, 0);
      xtpol5->SetParLimits(1, 0, 0.08);
      m_tmin -= 0.5;

      if (m_tmin < 14) {
        m_tmin = 14; cout << "ERROR: tmin so small, fit iter: " << endl;
      }
    }

    if (stat == 0) {
      m_fitflag = 0;
      if (m_debug) {cout << "FIT Failure; Layer: " << endl;}
    }

    if (m_debug) {printf("P6default= %3.2f, p6 fit = %3.2f", p6default, par[6]);}

    if (m_fitflag == 1) {
      if (m_debug) cout << "Fit success" << endl;
      xtpol5->GetParameters(m_FittedXTParams);
      m_Prob = xtpol5->GetProb();
      break;
    }

  } //end loop of fitting
  if (m_draw) {
    TString hname = m_h1->GetName();
    TString name = hname + ".pdf";
    TCanvas* c1 = new TCanvas("c1", "", 800, 600);
    m_h1->Draw();
    xtpol5->SetLineColor(kBlack);
    xtpol5->DrawF1(0, 400, "same");
    c1->SaveAs(name);

  }
}

void XT::FitChebyshev()
{
  if (m_h1->GetEntries() < m_minRequiredEntry) {
    m_fitflag = -1;
    return;
  }
  m_tmax = m_XTParam[6] + 100;
  xtCheb5->SetParameters(-0.07, 0.005, 0., 0., 0., 0., m_XTParam[6], 0.001);
  xtCheb5->SetParLimits(7, 0., 0.01);
  double stat;
  for (int i = 0; i < 10; ++i) {
    stat = m_h1->Fit("xtCheb5", "M Q", "0", m_tmin, m_tmax);
    if (stat != 0) break;
  }
  xtCheb5->GetParameters(m_FittedXTParams);
  m_Prob = xtCheb5->GetProb();
  if (stat == 0)
    m_fitflag = 0;
  else
    m_fitflag = 1;
  //  xtCheb5->DrawF1(0,400,"same");
  if (m_draw) {
    TString hname = m_h1->GetName();
    TString name = hname + ".pdf";
    TCanvas* c1 = new TCanvas("c1", "", 800, 600);
    m_h1->Draw();
    xtCheb5->SetLineColor(kBlack);
    xtCheb5->DrawF1(0, 400, "same");
    c1->SaveAs(name);
  }
}
