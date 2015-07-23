#include <TFile.h>
#include <TChain.h>
#include <TH2.h>
#include <TH1.h>
#include <TF1.h>
#include <TMath.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "TCL1.h"


using namespace std;

// construction 3D array with large volume



// function for inversion simmetric matrix

void sim(double* ss, int* N, double* aa, double* sb)
{
  int i, k, j;
  int n, n1, n2;

  double s[80000];
  double b[80000];
  double r[80000];
  memset(s, 0, sizeof(s));
  memset(b, 0, sizeof(b));
  memset(r, 0, sizeof(r));
  double dbs;

  n = *N;
  n1 = (n * (n + 1)) / 2;
  n2 = n * n;


  for (i = 0; i < n2; i++) {
    *(aa + i) = *(ss + i);

  }

  TCL::trpck(aa, s, *N);
  TCL::trsinv(s, r, *N);

  TCL::trupck(r, aa, *N);
  *sb = 0.;
  for (j = 0; j < n; j++) {
    for (i = 0; i < n; i++) {
      dbs = 0.;
      for (k = 0; k < n; k++) {
        dbs = dbs + *(ss + i + k * n) * (*(aa + n * j + k));
      }
      if (!(i == j) && fabs(dbs) > *sb) {*sb = fabs(dbs);}
      if (i == j && fabs(dbs - 1.) > *sb) {*sb = fabs(1. - dbs);}


    }

  }   // for j

  for (j = 0; j < n2; j++) {


    *(ss + j) = *(aa + j);
  }



}




class DoubleArray3D {
public:
  DoubleArray3D(size_t N1, size_t N2, size_t N3, double init)
  {
    m_N1 = N1;
    m_N2 = N2;
    m_N3 = N3;

    m_arr = new double** [m_N1];
    for (size_t i1 = 0; i1 < m_N1; i1++) {
      m_arr[i1] = new double * [m_N2];
      for (size_t i2 = 0; i2 < m_N2; i2++) {
        m_arr[i1][i2] = new double [m_N3];
        for (size_t i3 = 0; i3 < m_N3; i3++) {
          m_arr[i1][i2][i3] = init;
        }
      }
    }
  }
  ~DoubleArray3D()
  {
    for (size_t i1 = 0; i1 < m_N1; i1++) {
      for (size_t i2 = 0; i2 < m_N2; i2++) {
        delete [] m_arr[i1][i2];
      }
      delete [] m_arr[i1];
    }
    delete [] m_arr;
  }
  double** operator[](size_t idx)
  {
    return m_arr[idx];
  }

private:
  double*** m_arr;
  size_t m_N1, m_N2, m_N3;
};




void matrix_cal(int cortyp)
{
  TChain fChain("m_tree");
  char treeroot[256];

  FILE* BMcoIN;
  Char_t BMin[256];

  FILE* cutIN;
  Char_t cutin[256];

  FILE* McoIN;
  Char_t Min[256];

  Int_t poq;

  double xx[16][16];

  sprintf(BMin, "/gpfs/home/belle/avbobrov/belle2/j15/ecl/examples/rootfile.txt");

  if ((BMcoIN = fopen(BMin, "r")) == NULL) {
    printf(" file %s is absent \n", BMin);
    exit(1);
  }

  while (!feof(BMcoIN)) {
    fscanf(BMcoIN, "%s", &treeroot);

  }
  fclose(BMcoIN);




//        sprintf(treeroot,"/gpfs/home/belle/avbobrov/spool/beaux2/m15/GammaAnalysis_usual005.root");

  std::cout << "file for calibration:" << treeroot << std::endl;

  fChain.Add(treeroot);


  Double_t        energy;
  Double_t        theta;
  Double_t        phi;
  Int_t           nhits;
  Int_t           cellID[8736];   //[nhits]
  Int_t           hitA[8736][31];   //[nhits]
  Double_t        hitT[8736];   //[nhits]
  Double_t        digiE[8736];   //[nhits]
  Int_t           digiT[8736];   //[nhits]
  Double_t        deltaT[8736];   //[nhits]
  Int_t           necl;
  Int_t           cID[493];   //[necl]
  Float_t         Edep[493];   //[necl]
  Float_t         TimeAve[493];   //[necl]

  // List of branches
  TBranch*        b_energy;   //!
  TBranch*        b_theta;   //!
  TBranch*        b_phi;   //!
  TBranch*        b_nhits;   //!
  TBranch*        b_cellID;   //!
  TBranch*        b_hitA;   //!
  TBranch*        b_hitT;   //!
  TBranch*        b_digiE;   //!
  TBranch*        b_digiT;   //!
  TBranch*        b_deltaT;   //!
  TBranch*        b_necl;   //!
  TBranch*        b_cID;   //!
  TBranch*        b_Edep;   //!
  TBranch*        b_TimeAve;   //!

  fChain.SetBranchAddress("energy", &energy, &b_energy);
  fChain.SetBranchAddress("theta", &theta, &b_theta);
  fChain.SetBranchAddress("phi", &phi, &b_phi);
  fChain.SetBranchAddress("nhits", &nhits, &b_nhits);
  fChain.SetBranchAddress("cellID", cellID, &b_cellID);
  fChain.SetBranchAddress("hitA", hitA, &b_hitA);
  fChain.SetBranchAddress("hitT", hitT, &b_hitT);



  double dec;

  int N1;
  N1 = 16;
  double delta;
  delta = 0.;

  double SI[16][16];
  double IS[16][16];

  double MI[16][16];
  double IM[16][16];
  double UU[16][16];
  double FF[16][16];



  int mapmax;
  mapmax = 252;

  /*
  typedef boost::multi_array<double, 3> array3d;
  array3d W(boost::extents[mapmax][16][1]);
  array3d WW(boost::extents[mapmax][16][16]);
  */
  DoubleArray3D W(mapmax, 16, 1, 0.0);

  DoubleArray3D WW(mapmax, 16, 16, 0.0);

  double Q[mapmax][16];
  double Mean[8736];
  double rms[8736];
  double sr[8736];
  double sl[8736];

  double dt[mapmax];


  double inmt[mapmax];
  int it;
  double A0;
  cortyp = 43;
  double Nsigcut;
  Nsigcut = 3.5;
  delta = 0.;
  int index;
  index = 0;
  int icn, id;


  int DS[8736];

  ifstream ctoecldatafile("CIdToEclData.txt");




  int cid, group;
  bool readerr = false;
  vector<int> grmap[252];
  while (! ctoecldatafile.eof()) {
    ctoecldatafile >> cid >> group;
//    cout << cid << " " << group << endl;

    if (ctoecldatafile.eof()) break;
    if (group <= -1 || group >= 252 || cid <= -1 || cid >= 8736) {
      cout << "Error cid="  << cid << " group=" << group << endl;
      readerr = true;
      break;
    }
    DS[cid] = group;
  }


  int ia, ib;

  int max = 8736;
  int j;
  int trun;



  double Y[16];




  // red mean and rms for channels
// sprintf(cutin,"/home/belle/avbobrov/ecl/covmat/chdatu005.txt");

  sprintf(cutin, "/home/belle/avbobrov/ecl/covmat/chdatxx.txt");

  if ((cutIN = fopen(cutin, "r")) == NULL) {
    std::cout << "file" << cutIN << " not found" << std::endl;
  }

  int jk;
  double mn;
  double rs;
  double el;
  double er;
  double ch2;

  while (!feof(cutIN)) {

    fscanf(cutIN, "%d %lf %lf  %lf %lf %lf ", &jk, &mn, &rs, &el, &er, &ch2);
    Mean[jk] = mn;
    rms[jk] = rs;
    sr[jk] = er;
    sl[jk] = el;

//printf("%d %f %f %f %f %f \n",jk,mn,rs,el,er,ch2);

//Mean[jk]=3000;
//rms[jk]=10;


  }






  for (icn = 0; icn < mapmax; icn++) {
    inmt[icn] = 0.;
    dt[icn] = 0.;
    for (it = 0; it < 16; it++) {
      W[icn][it][0] = 0.;
      Q[icn][it] = 0.;

      for (id = 0; id < 16; id++) {
        WW[icn][it][id] = 0.;

      }
    }
  }




  Int_t nevent = fChain.GetEntries();
  std::cout << "! nevent=" << nevent << std::endl;

//               for (Int_t i=0;i<nevent;i++) {
  for (Int_t i = 0; i < 250; i++) {
    fChain.GetEntry(i);

    if (i % 1000 == 0) {std::cout << " nevent=" << i << std::endl;}

    for (icn = 0; icn < max; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

      index = 1;
      for (id = 0; id < 16; id++) {
        Y[it] = 0.;
      }

      A0 = 0;
      for (j = 0; j < 31; j++) {
        if (j < 16) {
          A0 = A0 + (double)hitA[icn][j] / 16.;
          if ((double)hitA[icn][j] - Mean[icn] < -Nsigcut * sl[icn]) {index = 0;}

          if ((double)hitA[icn][j] - Mean[icn] > Nsigcut * sr[icn]) {index = 0;}


        } else {
          if ((double)hitA[icn][j] - Mean[icn] > Nsigcut * sr[icn] || (double)hitA[icn][j] - Mean[icn] < -Nsigcut * sl[icn]) {index = 0;}

        }

      }  // points cicle for selection events


      if (index == 1) {

        inmt[DS[icn]] = inmt[DS[icn]] + 1.;
        A0 = 0.;

        for (j = 0; j < 31; j++) {

          if (j < 16) {A0 = A0 + (double)hitA[icn][j] / 16.;}

          if (j == 15) {
            W[DS[icn]][0][0] = W[DS[icn]][0][0] + A0;
            Q[DS[icn]][0] = Q[DS[icn]][0] + A0;
            Y[0] = A0;

          }

          if (j > 15) {
            trun = j - 15;
            Y[trun] = (double)hitA[icn][j];
            W[DS[icn]][trun][0] = W[DS[icn]][trun][0] + (double)hitA[icn][j];
            Q[DS[icn]][trun] = Q[DS[icn]][trun] + (double)hitA[icn][j];

          }

        }  // points cicle for writing



        for (it = 0; it < 16; it++) {
          for (id = 0; id < 16; id++) {
            WW[DS[icn]][it][id] = WW[DS[icn]][it][id] + Y[it] * Y[id];
          }
        }
        //        printf(" \n");




      }  // index


    }  //channels cilce



  } // events cicle





  // MATRICES INVERTION

  //             for(icn=0;icn<max;icn++){  //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

//                  for(icn=0;icn<mapmax;icn++){  //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

  for (icn = 0; icn < 12; icn++) { //%%%%%%%%%%%%%%%%%%%%%%%%55555555555


    if (0 == 0) { // conventional comment
      if (icn % 10 == 0) {printf("icn=%d inmt=%lf \n ", icn, inmt[icn]);}

      if (inmt[icn] == 0) {printf("bad icn=%d\n", icn);}

      // WRITE DATA INTO ARRAY
      for (ia = 0; ia < 16; ia++) {
        for (ib = 0; ib < 16; ib++) {
          if (ib < 16 && ia < 16) {
            MI[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];

            UU[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];
            SI[ia][ib] = (WW[icn][ia][ib] - W[icn][ia][0] * W[icn][ib][0] / inmt[icn]) / inmt[icn];



          }

        }
        IS[ia][ib] = 0;

      }


      // INVERSION
      N1 = 16;
      sim(*MI, &N1, *IM, &dec);



      int ic;

      // CALCULATION RESIDUAL
      dt[icn] = 0.;
      for (ia = 0; ia < 16; ia++) {
        for (ib = 0; ib < 16; ib++) {
          FF[ia][ib] = 0.;
          for (ic = 0; ic < 16; ic++) {
            FF[ia][ib] = FF[ia][ib] + UU[ia][ic] * IM[ic][ib];
          }
          if (ia == ib && fabs(FF[ia][ib] - 1.) > delta) {delta = fabs(FF[ia][ib] - 1.);}
          if (ia != ib && fabs(FF[ia][ib]) > delta) {delta = fabs(FF[ia][ib]);}

          if (ia == ib && fabs(FF[ia][ib] - 1.) > dt[icn]) {dt[icn] = fabs(FF[ia][ib] - 1.);}
          if (ia != ib && fabs(FF[ia][ib]) > dt[icn]) {dt[icn] = fabs(FF[ia][ib]);}
        }
      }
      if (dt[icn] < 1.e-33 || dt[icn] > 1.e-9) {         printf("icn=%d inmt=%lf cut=%lf delta= %e \n  ", icn, inmt[icn], dt[icn]);      }

      if (0 == 1) { //convetrinal commnet
        printf("\n");

        for (ia = 0; ia < 16; ia++) {
          printf("\n");
          for (ib = 0; ib < 16; ib++) {
            printf("%.2e ", FF[ia][ib]);
          }
        }


      }  //conventinal comment

    }

    // WRITE INVERS MATRICES



    if (0 == 0) { // conventional comment
      sprintf(Min, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/mcor%d_L.dat", cortyp, icn);

      McoIN = fopen(Min, "w");
      for (poq = 0; poq < 16; poq++) {
        fprintf(McoIN,
                "%.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \t %.5e \n  ",
                UU[0][poq], UU[1][poq], UU[2][poq], UU[3][poq], UU[4][poq], UU[5][poq], UU[6][poq], UU[7][poq], UU[8][poq], UU[9][poq], UU[10][poq],
                UU[11][poq], UU[12][poq], UU[13][poq], UU[14][poq], UU[15][poq]);



      }
      fclose(McoIN);


      sprintf(Min, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/inmcor%d_L.dat", cortyp, icn);

      McoIN = fopen(Min, "w");
      for (poq = 0; poq < 16; poq++) {
        fprintf(McoIN,
                "%.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \t %.3e \n  ",
                IM[0][poq], IM[1][poq], IM[2][poq], IM[3][poq], IM[4][poq], IM[5][poq], IM[6][poq], IM[7][poq], IM[8][poq], IM[9][poq], IM[10][poq],
                IM[11][poq], IM[12][poq], IM[13][poq], IM[14][poq], IM[15][poq]);



      }
      fclose(McoIN);



      sprintf(BMin, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat", cortyp, icn);


      if ((BMcoIN = fopen(BMin, "wb")) == NULL) {
        fprintf(stderr, "Error opening file 1");
        exit(1);
      }

      if (fwrite(IM, sizeof(double), 256, BMcoIN) != 256)
        //      if (fwrite(UU, sizeof(double), 256, BMcoIN) != 256)
      {
        fprintf(stderr, "Error writing to file.");
        exit(1);
      }

      fclose(BMcoIN);



      sprintf(BMin, "/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat", cortyp, icn);



      if ((BMcoIN = fopen(BMin, "rb")) == NULL) {
        fprintf(stderr, "Error opening file 21");
        exit(1);
      }

      //
      if (fread(xx, sizeof(double), 256, BMcoIN) != 256) {
        fprintf(stderr, "Error reading file.");
        exit(1);
      }
      fclose(BMcoIN);




    }  // converional cooment
    //%%%%%%%%%%%%%%%%%%%%%%%%55555555555

  }  // icn


  printf("\n ");
  printf("delta= %e \n  ", delta);

}

int main(int argc, char** argv)
{
  matrix_cal(10);
}
















