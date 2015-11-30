// -*- C++ -*-
//
// Package:     B2BIIFixMdst
// Module:      B2BIIFixMdst_muid
//
// Description: Probability density provider.
//
// Usage:       This is the class definition of the object that provides
//              the probability densities used in muon / hadron discrimination.
//
// Author:      Leo Piilonen
// Created:     31-Jan-2001
//
// $Id: B2BIIFixMdst_muid.cc 9944 2006-11-29 07:36:07Z katayama $
//
// Revision history
//
// $Log$
//
// Revision 2.0 2015/03/11 tkeck
// Conversion to Belle II
//
// Revision 1.1  2002/08/13 05:18:45  hitoshi
// muid_user is absorped in B2BIIFixMdst (by Piilonen).
//
// Revision 1.00  2002/08/08 14:00:00  piilonen
// Copied from muid_user
//
// my include file -- should always be first!

#include <b2bii/modules/B2BIIFixMdst/B2BIIFixMdstModule.h>
#include "belle_legacy/panther/panther.h"
#include "belle_legacy/tables/mdst.h"
#include "belle_legacy/tables/belletdf.h"
#include "belle_legacy/belleutil/belutil.h"

// system include files
#include <cmath>
#include <algorithm>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

namespace Belle2 {


  const double MuidProb::kRchisqMax = 10.0 ;
  const double MuidProb::kEEclMax = 1.0 ;
  const double MuidProb::kPTrkMax = 5.0 ;

// Start of muid routines in B2BIIFixMdst __________________________________

  void B2BIIFixMdstModule::Muid_init(void)
  {

    m_old_expno = (m_expno != 0) ? m_expno : 5;  // expt #5, by default
    m_mapped_expno = m_old_expno;
    if (m_mapped_expno > 0) {
      m_muonprob = new MuidProb("muon", m_mapped_expno);
      m_pionprob = new MuidProb("pion", m_mapped_expno);
      m_kaonprob = new MuidProb("kaon", m_mapped_expno);

      B2INFO("%B2BIIFixMdst muid is initialized by basf with"
             << std::endl << "  Endcap_MX_layer = " << m_eklm_max_layer
             << ((m_eklm_max_layer ==  0) ?
                 " ==> use value in data file" : "")
             << ((m_eklm_max_layer == 13) ? " (if possible)" : "")
             << std::endl << "           UseECL = "
             << ((m_use_ecl == 0) ? "false" : "true")
             << std::endl << "            ExpNo = " << m_expno
             << ((m_expno == 0) ?
                 " ==> use value in data file" : ""));
    } else {
      B2INFO("%B2BIIFixMdst muid is disabled by user (ExpNo = "
             << m_expno << " is negative)");
    }

    return;

  }

  void B2BIIFixMdstModule::Muid_term(void)
  {

    if (m_mapped_expno > 0) {
      delete m_muonprob;
      delete m_pionprob;
      delete m_kaonprob;
    }

    return;

  }

  void B2BIIFixMdstModule::Muid_begin_run(int expmc, int exp, int run)
  {

    if (m_mapped_expno > 0) {
      int expno = (m_expno != 0) ? m_expno : exp;
      if (expno != m_old_expno) {
        B2INFO("Reading DB constants for "
               << ((expmc == 1) ? "data" : "MC") << ", Exp #" << expno
               << ", Run #" << run);
        m_old_expno = expno;
        m_mapped_expno = m_old_expno;
        m_muonprob->readDB("muon", m_mapped_expno);
        m_pionprob->readDB("pion", m_mapped_expno);
        m_kaonprob->readDB("kaon", m_mapped_expno);
      }
    }

    return;

  }

  void B2BIIFixMdstModule::Muid_end_run(void)
  {

    return;

  }

  void B2BIIFixMdstModule::Muid_event(void)
  {

    Mdst_klm_mu_ex_Manager& muexMgr = Mdst_klm_mu_ex_Manager::get_manager();
    //if (muexMgr.count()) {
    // TODO: this always evaluates to false!
    if (&muexMgr == NULL) {
      B2ERROR("%B2BIIFixMdst muid did not find Mdst_klm_mu_ex table "
              << "in this event, which implies" << std::endl
              << "   that muid_set and muid_dec were not run yet. "
              << "Required order of modules is " << std::endl
              << "   ... muid_set muid_dec ... rec2mdst ... B2BIIFixMdst ..."
              << std::endl << "   This program is terminated.");
      exit(-1);
    }
    Mdst_ecl_trk_Manager& eclTrkMgr = Mdst_ecl_trk_Manager::get_manager();
    Mdst_ecl_trk_Index eclTrkIdx = eclTrkMgr.index("trk");
    eclTrkIdx.update();

// For each Mdst_klm_mu_ex, refigure the likelihoods by using the
// most up-to-date probability density functions of muons, pions, and
// kaons for the KLM range and reduced chi-squared.  Optionally, fold
// in the probability density function of muons, pions, and kaons for
// the ECL energy associated with the corresponding charged track.
// The chain of pointers among panther tables is a little byzantine.

    for (std::vector<Mdst_klm_mu_ex>::iterator iMuex = muexMgr.begin();
         iMuex < muexMgr.end(); ++iMuex) {
      Mdst_charged& chg      = iMuex->pMDST_Charged();
      Mdst_muid&    muid     = chg.muid();
      int ECMaxLyr = (muid.quality() & 0x400000) ? 1 : 0;
      if (m_eklm_max_layer == 11) {
        ECMaxLyr = 1;
      } else {
        if ((m_eklm_max_layer == 13) && (ECMaxLyr == 0)) {
          ECMaxLyr = 0;
        }
      }
      double       px        = chg.px();
      double       py        = chg.py();
      double       pz        = chg.pz();
      double       p         = sqrt(px * px + py * py + pz * pz);
      double       eEcl      = -1.0;
      double       muon      = iMuex->Muon_likelihood();
      double       pion      = iMuex->Pion_likelihood();
      double       kaon      = iMuex->Kaon_likelihood();
      double       miss      = iMuex->Miss_likelihood();
      double       junk      = iMuex->Junk_likelihood();
      int          outcome   = iMuex->Outcome();
      int          lyrPasMax = iMuex->Layer_trk_brl() +
                               iMuex->Layer_trk_end() + 1;
      int          lyrPicMax = iMuex->Layer_hit_brl();
      if (iMuex->Layer_hit_end() != -1) {
        lyrPicMax            = iMuex->Layer_trk_brl() +
                               iMuex->Layer_hit_end() + 1;
      }
      int          lyrDiff   = lyrPasMax - lyrPicMax;
      int          lyrExt    = iMuex->Layer_trk_brl();
      if ((outcome == 2) || (outcome == 4)) {
        lyrExt               = iMuex->Layer_trk_end();
        if ((ECMaxLyr == 1) && (lyrExt > 11)) {
          outcome            = 4;         // unconditionally an escape from endcap
          lyrExt             = 11;
          int lyrEnd         = std::min(iMuex->Layer_hit_end(), 11);
          lyrPasMax          = iMuex->Layer_trk_brl() + lyrExt + 1;
          lyrPicMax          = iMuex->Layer_trk_brl() + lyrEnd + 1;
          lyrDiff            = lyrPasMax - lyrPicMax;
        }
      }
      int          nPoints   = iMuex->N_hits() * 2;
      double       rChi2     = 1.0E10;
      if (nPoints > 0) {
        rChi2                = iMuex->Chi_2() / nPoints;
      }

      if (outcome > 0) {
        muon  = m_muonprob->prob(ECMaxLyr, outcome, lyrExt, lyrDiff, rChi2);
        pion  = m_pionprob->prob(ECMaxLyr, outcome, lyrExt, lyrDiff, rChi2);
        kaon  = m_kaonprob->prob(ECMaxLyr, outcome, lyrExt, lyrDiff, rChi2);
        double denom = muon + pion + kaon;
        if (denom < 1.0E-10) {
          denom = 1.0;
          muon  = pion = kaon = 0.0;
          junk  = 1.0;                    // should happen very rarely
        }
        muon /= denom;
        pion /= denom;
        kaon /= denom;
      }
      if (m_use_ecl) {
        Mdst_trk& trk = chg.trk();
        std::vector<Mdst_ecl_trk> eclTrk = point_from(trk.get_ID(), eclTrkIdx);
        std::vector<Mdst_ecl_trk>::iterator iEclTrk = eclTrk.begin();
        if (iEclTrk != eclTrk.end()) {
          Mdst_ecl& ecl = iEclTrk->ecl();
          if (ecl) {
            if (ecl.match() >= 1) {
              eEcl  = ecl.energy();
              if ((eEcl > 0.0) && (p > 0.25)) {
                double muonE = m_muonprob->probECL(eEcl, p);
                double pionE = m_pionprob->probECL(eEcl, p);
                double kaonE = m_kaonprob->probECL(eEcl, p);
                double denom = muonE + pionE + kaonE;
                if (denom > 1.0E-10) {
                  if (miss + junk > 0.0) {
                    muon    = muonE;
                    pion    = pionE;
                    kaon    = kaonE;
                    miss    = 0.0;
                    junk    = 0.0;
                    outcome = 5;
                  } else {
                    if (nPoints == 0) {
                      muon   *= muonE;
                      pion   *= pionE;
                      kaon   *= kaonE;
                    }
                  }
                }
                denom = muon + pion + kaon;
                if (denom < 1.0E-10) {
                  denom = 1.0;
                  muon  = pion = kaon = 0.0;
                  junk  = 1.0;                 // should happen very rarely
                }
                muon /= denom;
                pion /= denom;
                kaon /= denom;
              }   // if ( (eEcl > 0.0) && (p > 0.25) )
            }     // if ( ecl.match() >= 1 )
          }       // if ( ecl )
        } // if ( iEclTrk != eclTrk.end() )
      }           // if ( m_use_ecl )

      iMuex->Muon_likelihood(muon);
      iMuex->Pion_likelihood(pion);
      iMuex->Kaon_likelihood(kaon);
      iMuex->Miss_likelihood(miss);
      iMuex->Junk_likelihood(junk);
      iMuex->Outcome(outcome);

    }     // for iMuex loop

    return;

  }

// End of muid routines in B2BIIFixMdst ____________________________________

// Start of class MuidProb _____________________________________________

// Default constructor: open database file and read in the tables

  MuidProb::MuidProb(const char* dbtemplate, int& expNo)
  {

    readDB(dbtemplate, expNo);

  }

  void MuidProb::readDB(const char* dbtemplate, int& expNo)
  {

    double dx;    // bin size
    double probSum; // normalization denominator

    if (expNo == 0) return;   // do nothing if pdf file was not found previously

    if ((expNo < 5) || ((expNo & 1) == 0)) {
      expNo = 5;    // Runs 1-4 and all even-numbered runs map to Exp #5
    }

    char dbname[] = "muid_xxxx_e000000.dat";
    std::string pathname = "";
    bool tmp = set_belfnm_verbose(false);
    while ((pathname == "") && (expNo >= 5)) {
      std::sprintf(dbname, "%s%s%s%06d%s", "muid_", dbtemplate,
                   "_e", expNo, ".dat");
      pathname = belfnm(dbname, 0, "share/belle_legacy/data-files/muid");
      expNo -= 2;
    }
    (void)set_belfnm_verbose(tmp);
    expNo += 2;
    if (pathname == "") {
      B2ERROR("%MuidProb: Failed to open database file."
              << "Module B2BIIFixMdst muid will do nothing.");
      expNo = 0;
      return;
    }

    std::ifstream probDB;
    char ident[90];
    probDB.open(pathname.c_str());
    probDB.get(ident, 80, '\n');
    B2INFO("%MuidProb: Reading file " << pathname
           << std::endl << "%MuidProb: " << ident
           << std::endl << "%MuidProb: ");

    for (int l = 0; l < 2; l++) {
      B2INFO("range ");
      for (int k = 0; k < 4; k++) {
        for (int j = 0; j < 15; j++) {
          for (int i = 0; i < kRange; i++) {
            probDB >> fRange[l][k][j][i];
          }
        }
      }
      B2INFO("chi**2 ");
      dx = kRchisqMax / kRchisq;
      for (int k = 0; k < 4; k++) {
        for (int i = 0; i < kRchisq; i++) {
          probDB >> fRchisq[l][k][i];
        }
        spline(kRchisq, dx, &fRchisq[l][k][0], &fRchisqD1[l][k][0],
               &fRchisqD2[l][k][0], &fRchisqD3[l][k][0]);
      }
      for (int k = 0; k < 4; k++) {
        for (int j = 0; j < 15; j++) {
          probDB >> fRchisqN[l][k][j];
        }
      }
    }
    B2INFO("eEcl ");
    dx = kEEclMax / kEEcl;
    for (int k = 0; k < kPTrk; k++) {
      probSum = 0.0;
      for (int i = 0; i < kEEcl; i++) {
        probDB >> fEEcl[k][i];
        probSum += fEEcl[k][i];
      }
      fEEcl[k][kEEcl] = 1.0 - probSum;            // overflow bin
      if (fEEcl[k][kEEcl] < 0.0) { fEEcl[k][kEEcl] = 0.0; }
      if (fEEcl[k][kEEcl] > 1.0) { fEEcl[k][kEEcl] = 1.0; }
      spline(kEEcl, dx, &fEEcl[k][0], &fEEclD1[k][0],
             &fEEclD2[k][0], &fEEclD3[k][0]);
    }

    probDB.close();

  }

//____________________________________________________________________________

  void MuidProb::spline(int n, double dx, double Y[], double B[],
                        double C[], double D[])
  {

    // Generate the spline interpolation coefficients B, C, D to smooth out a
    // binned histogram. Restrictions:  equal-size bins. more than 3 bins.

    D[0] = dx;          // let's do it!
    C[1] = (Y[1] - Y[0]) / dx;
    for (int i = 1; i < n - 1; i++) {
      D[i]   = dx;
      B[i]   = dx * 4.0;
      C[i + 1] = (Y[i + 1] - Y[i]) / dx;
      C[i]   = C[i + 1] - C[i];
    }
    B[0]   = -dx;
    B[n - 1] = -dx;
    C[0]   = (C[2]   - C[1]) / 6.0;
    C[n - 1] = -(C[n - 2] - C[n - 3]) / 6.0;
    for (int i = 1; i < n; i++) {
      double temp = dx / B[i - 1];
      B[i] -= temp * dx;
      C[i] -= temp * C[i - 1];
    }
    C[n - 1] /= B[n - 1];
    for (int i = n - 2; i >= 0; i--) {
      C[i] = (C[i] - D[i] * C[i + 1]) / B[i];
    }
    B[n - 1] = (Y[n - 1] - Y[n - 2]) / dx + (C[n - 2] + C[n - 1] * 2.0) * dx;
    for (int i = 0; i < n - 1; i++) {
      B[i] = (Y[i + 1] - Y[i]) / dx - (C[i + 1] + C[i] * 2.0) * dx;
      D[i] = (C[i + 1] - C[i]) / dx;
      C[i] = C[i] * 3.0;
    }
    C[n - 1] = C[n - 1] * 3.0;
    D[n - 1] = D[n - 2];
  }

//____________________________________________________________________________

  double MuidProb::prob(int ECMaxLyr, int outcome,
                        int lyr_ext, int lyr_dif, double chi2_red) const
  {

    // ECMaxLyr: 0 if Endcap_MX_layer == 13; 1 if Endcap_MX_layer == 11
    // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
    // lyr_ext:  last layer that Ext track touched
    // lyr_dif:  difference between last Ext layer and last hit layer
    // chi2_red: reduced chi**2 of the transverse deviations of all associated
    //           hits from the corresponding Ext track crossings

    return probRange(ECMaxLyr, outcome, lyr_ext, lyr_dif) *
           probRchisq(ECMaxLyr, outcome, lyr_ext, chi2_red);

  }

//____________________________________________________________________________

  double MuidProb::probRange(int ECMaxLyr, int outcome, int lyr_ext,
                             int lyr_dif) const
  {


    // ECMaxLyr: 0 if max endcap layer is 13; 1 if it's 11.
    // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
    // lyr_ext:  last layer that Ext track touched
    // lyr_dif:  difference between last Ext layer and last hit layer

    if (outcome <=  0) { return 0.0; }
    if (outcome >   4) { return 0.0; }
    if (lyr_ext <   0) { return 0.0; }
    if (lyr_ext >  14) { return 0.0; }
    if (lyr_dif <   0) { return 0.0; }
    if (lyr_dif >= kRange) { lyr_dif = kRange - 1; }

    return fRange[ECMaxLyr][outcome - 1][lyr_ext][lyr_dif];

  }

//____________________________________________________________________________

  double MuidProb::probRchisq(int ECMaxLyr, int outcome, int lyr_ext,
                              double chi2_red) const
  {

    // ECMaxLyr: 0 if max endcap layer is 13; 1 if it's 11.
    // outcome:  0=??, 1=Barrel Stop, 2=Endcap Stop, 3=Barrel Exit, 4=Endcap Exit
    // lyr_ext:  last layer that Ext track touched
    // chi2_red: reduced chi**2 of the transverse deviations of all associated
    //           hits from the corresponding Ext track crossings

    // Extract the probability density for a particular value of reduced
    // chi-squared by spline interpolation of the binned histogram values.
    // This avoids binning artifacts that were seen when the histogram
    // was sampled directly.

    if (outcome  <= 0) { return 0.0; }
    if (outcome  >  4) { return 0.0; }
    if (lyr_ext  <  0) { return 0.0; }
    if (lyr_ext  > 14) { return 0.0; }
    if (chi2_red <  0.0) { return 0.0; }

    double prob;
    double area = fRchisqN[ECMaxLyr][outcome - 1][lyr_ext];
    if (chi2_red >= kRchisqMax) {
      prob = area * fRchisq[ECMaxLyr][outcome - 1][kRchisq - 1] + (1.0 - area);
    } else {
      int    i  = (int)(chi2_red / (kRchisqMax / kRchisq));
      double dx = chi2_red - i * (kRchisqMax / kRchisq);
      prob = fRchisq[ECMaxLyr][outcome - 1][i] +
             dx * (fRchisqD1[ECMaxLyr][outcome - 1][i] +
                   dx * (fRchisqD2[ECMaxLyr][outcome - 1][i] +
                         dx * fRchisqD3[ECMaxLyr][outcome - 1][i]));
      prob = (prob < 0.0) ? 0.0 : area * prob;
    }
    return prob;
  }

//____________________________________________________________________________

  double MuidProb::probECL(double eEcl, double p) const
  {

    // eEcl: ECL energy (GeV)
    // p:    CDC momentum (GeV/c)

    double prob;
    int jTrk = (p < kPTrkMax) ? (int)(p / (kPTrkMax / kPTrk)) : kPTrk - 1;
    if (eEcl >= kEEclMax) {
      prob = fEEcl[jTrk][kEEcl];       // overflow bin
    } else {
      int    i  = (int)(eEcl / (kEEclMax / kEEcl));
      double dx = eEcl - i * (kEEclMax / kEEcl);
      prob = fEEcl[jTrk][i] +
             dx * (fEEclD1[jTrk][i] +
                   dx * (fEEclD2[jTrk][i] +
                         dx * fEEclD3[jTrk][i]));
      if (prob < 0.0) { prob = 0.0; }     // don't let spline fit go negative
    }
    return prob;

  }

// End of class MuidProb _______________________________________________

} // namespace Belle
