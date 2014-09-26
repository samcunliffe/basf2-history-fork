//*****************************************************************************
//-----------------------------------------------------------------------------
// Fast simulation and reconstruction package for TOP counter (F77 core)
// M. Staric, April-2009, Sept-2011
//-----------------------------------------------------------------------------
//
// TOPreco.cc
// C++ interface to F77 functions: reconstruction
//-----------------------------------------------------------------------------
//*****************************************************************************

#include <iostream>
#include <iomanip>
#include <top/reconstruction/TOPreco.h>
#include <top/reconstruction/TOPf77fun.h>

#include <framework/gearbox/Const.h>

namespace Belle2 {
  namespace TOP {

    TOPreco::TOPreco(int Num, double Masses[], double BkgPerQbar, double ScaleN0):
      m_HYP(0), m_beta(0.0)
    {
      data_clear_();
      rtra_clear_();
      float masses[Num];
      for (int i = 0; i < Num; i++) {
        masses[i] = (float) Masses[i];
      }
      rtra_set_hypo_(&Num, masses);
      float b = (float) BkgPerQbar; float s = (float) ScaleN0;
      set_top_par_(&b, &s);
    }

    void TOPreco::setHypID(int NumHyp, int HypID[])
    {
      rtra_set_hypid_(&NumHyp, HypID);
    }

    void TOPreco::clearData()
    {
      data_clear_();
      rtra_clear_();
    }

    int TOPreco::addData(int QbarID, int chID, int TDC)
    {
      int status;
      QbarID--; // 0-based ID used in fortran
      chID--;   // 0-based ID used in fortran
      data_put_(&QbarID, &chID, &TDC, &status);
      return status;
    }

    int TOPreco::getDataSize() {return data_getnum_();}

    void TOPreco::reconstruct(double X, double Y, double Z, double Tlen,
                              double Px, double Py, double Pz, int Q, int HYP)
    {
      m_HYP = HYP;
      float x = (float) X; float y = (float) Y; float z = (float) Z;
      float t = float(Tlen / Const::speedOfLight);
      float px = (float) Px; float py = (float) Py; float pz = (float) Pz;
      int REF = 0; int MCREF = 0;
      rtra_clear_();
      rtra_put_(&x, &y, &z, &t, &px, &py, &pz, &Q, &m_HYP, &REF, &MCREF);
      top_reco_();
    }

    void TOPreco::reconstruct(TOPtrack& trk)
    {
      m_HYP = trk.getHypID();
      float x = (float) trk.getX();
      float y = (float) trk.getY();
      float z = (float) trk.getZ();
      float t = float(trk.getTrackLength() / Const::speedOfLight);
      float px = (float) trk.getPx();
      float py = (float) trk.getPy();
      float pz = (float) trk.getPz();
      int Q = trk.getCharge();
      int REF = 0;
      int MCREF = trk.getLabel(); // has no effect, can be set to 0
      rtra_clear_();
      rtra_put_(&x, &y, &z, &t, &px, &py, &pz, &Q, &m_HYP, &REF, &MCREF);
      top_reco_();

    }

    int TOPreco::getFlag()
    {
      int K = 1;
      return rtra_getflag_(&K);
    }

    void TOPreco::getLogL(int Size, double LogL[], double ExpNphot[], int& Nphot)
    {
      int K = 1;
      float logl[Size], sfot[Size];
      int Flag, MTRA, REF;
      rtra_get_(&K, logl, sfot, &Size, &Nphot, &Flag, &MTRA, &REF);
      for (int i = 0; i < Size; i++) {
        LogL[i] = logl[i];
        ExpNphot[i] = sfot[i];
      }
    }

    void TOPreco::getTrackHit(int LocGlob, double R[3], double Dir[3], double& Len,
                              double& Tlen, double& Mom, int& QbarID)
    {
      int K = 1;
      float r[3], dir[3], len, tof, p;
      rtra_gethit_(&K, &LocGlob, r, dir, &len, &tof, &p, &QbarID);
      for (int i = 0; i < 3; i++) {
        R[i] = r[i];
        Dir[i] = dir[i];
      }
      Len = len; Tlen = tof * Const::speedOfLight; Mom = p;
    }

    void TOPreco::dumpLogL(int Size)
    {
      double logl[Size], sfot[Size];
      int hypid[Size];
      int Nphot;
      getLogL(Size, logl, sfot, Nphot);
      rtra_get_hypid_(&Size, hypid);

      int i_max = 0;
      double logl_max = logl[0];
      for (int i = 1; i < Size; i++) {
        if (logl[i] > logl_max) {logl_max = logl[i]; i_max = i;}
      }

      using namespace std;
      cout << "TOPreco::dumpLogL: Flag=" << getFlag();
      cout << "  Detected Photons=" << Nphot << endl;
      cout << " i HypID   LogL   ExpPhot" << endl;
      cout << showpoint << fixed << right;
      for (int i = 0; i < Size; i++) {
        cout << setw(2) << i;
        cout << setw(4) << hypid[i];
        cout << setw(10) << setprecision(2) << logl[i];
        cout << setw(8) << setprecision(2) << sfot[i];
        if (i == i_max) cout << " <";
        if (hypid[i] == m_HYP) cout << " <-- truth";
        cout << endl;
      }
    }

    void TOPreco::dumpTrackHit(int LocGlob)
    {
      double r[3], dir[3], len, Tlen, p;
      int QbarID;
      getTrackHit(LocGlob, r, dir, len, Tlen, p, QbarID);

      using namespace std;
      cout << showpoint << fixed << right;
      cout << "TOPreco::dumpTrackHit: QbarID=" << QbarID;
      cout << "  Len=" << setprecision(2) << len;
      cout << "cm  Tlen=" << setprecision(1) << Tlen;
      cout << "cm  p=" << setprecision(2) << p << "GeV/c" << endl;
      cout << "position [cm]: ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(2) << r[i];
      }
      if (LocGlob == Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}

      cout << "direction:     ";
      for (int i = 0; i < 3; i++) {
        cout << setw(10) << setprecision(4) << dir[i];
      }
      if (LocGlob == Local) {cout << " (local)" << endl;}
      else {cout << " (global)" << endl;}
    }

    int TOPreco::getPullSize()
    {
      int n;
      getnum_pulls_(&n);
      return n;
    }

    void TOPreco::getPull(int K, double& T, double& T0, double& Wid, double& PhiCer,
                          double& Wt)
    {
      float t, t0, wid, fic, wt;
      int dum;
      K++;
      get_pulls_(&K, &t, &t0, &wid, &fic, &wt, &dum);
      T = t; T0 = t0; Wid = wid; PhiCer = fic; Wt = wt;
    }

    double TOPreco::getPDF(int chID, double T, double Mass)
    {
      float t = (float) T; float mass = (float) Mass;
      return get_pdf_(&chID, &t, &mass);
    }

  } // end top namespace
} // end Belle2 namespace

