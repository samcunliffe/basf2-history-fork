#define TRG_SHORT_NAMES

#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdio>
#include <vector>
#include <math.h>
#include <iomanip>
#include "trg/trg/Utilities.h"

using namespace std;
using namespace Belle2;

#define NAME          "HoughMapping"
#define VERSION       "version 0.01"

struct XY {double x; double y;};
struct Plane {unsigned x; unsigned y;};

void printHeader(ofstream& out, const string function);
void superLayer(const unsigned id);

//Hough Plane Parameter
//Phi Range (X Axial)
const double PI2 = 2 * M_PI;
//log10(r/cm) Range (Y Axial)
const double minY = 1.823908740944321;
const double maxY = 3.204119982655926;
//cell number
const unsigned nX = 160;
const unsigned nY = 16;

vector<vector<vector<int>>> HPcellM(nY, vector<vector<int>>(nX, vector<int>()));
ofstream outputM("minus_total.dat");

vector<vector<vector<int>>> HPcellP(nY, vector<vector<int>>(nX, vector<int>()));
ofstream outputP("plus_total.dat");


//...C++ for TSIM...
const string fncM = "HoughMappingMinus.cc";
const string fncP = "HoughMappingPlus.cc";
ofstream outcM(fncM);
ofstream outcP(fncP);

int
main(int, char**)
{
  cout << NAME << " ... " << VERSION << endl;

  printHeader(outcM, "HoughMappingMinus");
  printHeader(outcP, "HoughMappingPlus");

  for (unsigned isl = 0; isl < 5 ; isl++)
    superLayer(isl);

  for (unsigned iy = 0; iy < nY; iy++) {
    for (unsigned ix = 0; ix < nX; ix++) {
      outputM << ix << " " << iy + 1 << " ";
      outputP << ix << " " << iy + 1 << " ";
      for (unsigned its = 0; its < HPcellM[iy][ix].size(); its++) {
        outputM << HPcellM[iy][ix][its] << " ";
      }
      for (unsigned its = 0; its < HPcellP[iy][ix].size(); its++) {
        outputP << HPcellP[iy][ix][its] << " ";
      }
      outputM << " " << endl;
      outputP << " " << endl;
    }
  }

  outcM << endl << "}" << endl;
  outcP << endl << "}" << endl;

  outcM.close();
  outcP.close();

  //...Termination...
  cout << "Files generated" << endl;
  cout << "    c++ for tsim firmware : " << fncM << endl;
  cout << "    c++ for tsim firmware : " << fncP << endl;
}

void
printHeader(ofstream& out, const string function)
{
  //...Date...
  string ts = TRGUtil::dateString();

  out << "// This file is generated by " << NAME << "(" << VERSION << ")" << endl;
  out << "// " << ts << endl << endl;
  out << "#define TRGCDC_SHORT_NAMES" << endl;
  out << "#include \"trg/trg/State.h\"" << endl;
  out << "#include \"trg/cdc/Tracker2D.h\"" << endl;
  out << "using namespace std;" << endl;
  out << "using namespace Belle2;" << endl;
  out << "void" << endl;
  out << "TCTracker2D::" << function << "(void) {" << endl;

  out << "    //...TS hit map..." << endl;
  out << "    TRGState SL0_TS = _ts.subset(0, 160);" << endl;
  out << "    TRGState SL2_TS = _ts.subset(160, 192);" << endl;
  out << "    TRGState SL4_TS = _ts.subset(160 + 192, 256);" << endl;
  out << "    TRGState SL6_TS = _ts.subset(160 + 192 + 256, 320);" << endl;
  out << "    TRGState SL8_TS = _ts.subset(160 + 192 + 256 + 320, 384);" << endl;

  out << "    //...Hough cells..." << endl;

  for (unsigned isl = 0; isl < 5; isl++) {
    for (unsigned iy = 1; iy < nY + 1; iy++) {
      out << "    TRGState SL" << to_string(isl * 2) << "_row"
          << to_string(iy) << "(" << to_string(nX) << ");" << endl;
    }
  }
}

void
superLayer(const unsigned id)
{
  //Radius of SL_center cell
  double r_SL = 0;
  //Number of TS each SL
  int N_TS_SL = 0;
  int SL = 2 * id;

  if (id == 0) {
    r_SL = 19.8;
    N_TS_SL = 160;
  } else if (id == 1) {
    r_SL = 40.16;
    N_TS_SL = 192;
  } else if (id == 2) {
    r_SL = 62.0;
    N_TS_SL = 256;
  } else if (id == 3) {
    r_SL = 83.84;
    N_TS_SL = 320;
  } else if (id == 4) {
    r_SL = 105.68;
    N_TS_SL = 384;
  } else {
    cout << NAME << " !!! bad super layer ID" << endl;
    exit(-1);
  }

  //Hough Plane database
  vector<XY> xymatrix;
  XY xy = {0, 0};

/////////////////////////////////////////////////base of ts

  for (int i = 0; i < N_TS_SL; ++i) {
    xy.x = r_SL * cos((PI2 / N_TS_SL) * i);
    xy.y = r_SL * sin((PI2 / N_TS_SL) * i);
    xymatrix.push_back(xy);
  }
///////////////////////////////////////////////////HP to TS

  const double r0 = minY;
  const double phi0 = 0;
  const double dr = (maxY - minY) / nY;
  const double dphi = PI2 / nX;
  double r1;
  double r2;
  double phi1;
  double phi2;
  double minus1;
  double minus2;
  double plus1;
  double plus2;

  const string vhM = "UT3_0_SL" + to_string(id * 2) + ".vhd";
  const string vhP = "UT3_0_SL" + to_string(id * 2) + "_p.vhd";
  ofstream outputfM(vhM);
  ofstream outputfP(vhP);

  //generate firware code(Minus)
  outputfM << "library IEEE;" << endl;
  outputfM << "use IEEE.STD_LOGIC_1164.ALL;" << endl;
  outputfM << " " << endl;
  outputfM << " " << endl;
  outputfM << "entity UT3_0_SL" << SL << " is" << endl;
  outputfM << " " << endl;
  outputfM << "Port (" << endl;
  for (unsigned irow = 1; irow < nY + 1; ++irow) {
    outputfM << "           SL" << SL << "_row" << left << setw(2) << to_string(irow)
             << " : out  STD_LOGIC_VECTOR (79 downto 40);" << endl;
  }
  outputfM << "           SL" << SL << "_TS    : in   STD_LOGIC_VECTOR ("
           << N_TS_SL / 2 << " downto 0));" << endl;
  outputfM << "end UT3_0_SL" << SL << ";" << endl;
  outputfM << " " << endl;
  outputfM << " " << endl;
  outputfM << "architecture Behavioral of UT3_0_SL" << SL << " is" << endl;
  outputfM << " " << endl;
  outputfM << "begin" << endl;
  outputfM << " " << endl;

  //generate firmware code(Plus)
  outputfP << "library IEEE;" << endl;
  outputfP << "use IEEE.STD_LOGIC_1164.ALL;" << endl;
  outputfP << " " << endl;
  outputfP << " " << endl;
  outputfP << "entity UT3_0_SL" << SL << "_P is" << endl;
  outputfP << " " << endl;
  outputfP << "Port (" << endl;
  for (unsigned irow = 1; irow < nY + 1; ++irow) {
    outputfP << "           SL" << SL << "_row" << left << setw(2) << to_string(irow)
             << " : out  STD_LOGIC_VECTOR (39 downto 0);" << endl;
  }
  outputfP << "           SL" << SL << "_TS    : in   STD_LOGIC_VECTOR ("
           << N_TS_SL / 2 << " downto 0));" << endl;
  outputfP << "end UT3_0_SL" << SL << "_P;" << endl;
  outputfP << " " << endl;
  outputfP << " " << endl;
  outputfP << "architecture Behavioral of UT3_0_SL" << SL << "_P is" << endl;
  outputfP << " " << endl;
  outputfP << "begin" << endl;
  outputfP << " " << endl;

  //vertical
  for (unsigned k = 0 ; k < nY ; ++k) {
    //horizontal
    for (unsigned j = 0 ; j < nX ; ++j) {
      double  ffM = 0;
      double  ffP = 0;

      if (j > 39 && j < 80)
        outputfM << "SL" << SL << "_row" << k + 1 << "(" << j << ")<=";
      if (j < 40)
        outputfP << "SL" << SL << "_row" << k + 1 << "(" << j << ")<=";
      outcM << "    SL" << SL << "_row" << k + 1 << ".set(" << j << ", ";
      outcP << "    SL" << SL << "_row" << k + 1 << ".set(" << j << ", ";
      bool firstM = true;
      bool firstP = true;

      phi1 = phi0 + j * dphi;
      phi2 = phi0 + (j + 1) * dphi;

      //TS
      for (int i = 0 ; i < N_TS_SL ; i++) {
        // calculate r(phi) at Hough cell borders phi1 and phi2
        r1 = ((xymatrix[i].x * xymatrix[i].x) + (xymatrix[i].y * xymatrix[i].y)) /
             ((2 * xymatrix[i].x * cos(phi1)) + (2 * xymatrix[i].y * sin(phi1)));
        r2 = ((xymatrix[i].x * xymatrix[i].x) + (xymatrix[i].y * xymatrix[i].y)) /
             ((2 * xymatrix[i].x * cos(phi2)) + (2 * xymatrix[i].y * sin(phi2)));

        /* Check whether f(phi) = log(r(phi)) crosses the Hough cell
         * defined by (phi1, phi2, log(r1), log(r2))
         * The slope determines the charge of the track.
         *
         * Since f is not defined for all phi, 3 cases can occur:
         * 1. f(phi1) and f(phi2) both defined:
         *    compare f(phi1) and f(phi2) to log(r1) and log(r2),
         *    get slope from f(phi2) - f(phi1)
         * 2. f(phi1) defined, f(phi2) not defined (or vice-versa):
         *    compare f(phi1) to log(r1) and log(r2),
         *    slope is known
         * 3. f(phi1) and f(phi2) both not defined: no entry
         */

        //...minus...
        if (r1 >= 0 && r2 >= 0 && r1 < r2) {
          /* positive slope:
           * crossing if f(phi1) < log(r2) and f(phi2) > log(r1)
           */
          minus1 = r0 + (k + 1) * dr - log10(r1);
          minus2 = r0 + k * dr - log10(r2);
          if (minus1 * minus2 <= 0.0) {
            HPcellM[k][j].push_back(SL);
            HPcellM[k][j].push_back(i);

            if (! firstM)
              outcM << " or ";

            if (ffM != 0) {
              if (j > 39 && j < 80)
                outputfM << "or ";
            }

            ffM++;
            if (j > 39 && j < 80)
              outputfM << "SL" << SL << "_TS(" << i << ") ";

            outcM << "SL" << SL << "_TS[" << i << "]";
            firstM = false;
          }
        } else if (r2 < 0 && r1 >= 0) {
          /* positive slope, f(phi2) = inf:
           * crossing if f(phi1) < log(r2)
           */
          minus1 = r0 + (k + 1) * dr - log10(r1);
          if (minus1 > 0) {
            HPcellM[k][j].push_back(SL);
            HPcellM[k][j].push_back(i);

            if (! firstM)
              outcM << " or ";

            if (ffM != 0) {
              if (j > 39 && j < 80)
                outputfM << "or ";
            }
            ffM++;
            if (j > 39 && j < 80)
              outputfM << "SL" << SL << "_TS(" << i << ") ";

            outcM << "SL" << SL << "_TS[" << i << "]";
            firstM = false;
          }
        }

        //plus
        if (r1 >= 0 && r2 >= 0 && r2 < r1) {
          /* negative slope:
           * crossing if f(phi2) < log(r2) and f(phi1) > log(r1)
           */
          plus1 = r0 + (k + 1) * dr - log10(r2);
          plus2 = r0 + k * dr - log10(r1);
          if (plus1 * plus2 <= 0.0) {
            HPcellP[k][j].push_back(SL);
            HPcellP[k][j].push_back(i);
            if (! firstP)
              outcP << " or ";

            if (ffP != 0) {
              if (j < 40)
                outputfP << "or ";
            }
            ffP++;
            if (j < 40)
              outputfP << "SL" << SL << "_TS(" << i << ") ";

            outcP << "SL" << SL << "_TS[" << i << "]";
            firstP = false;
          }
        } else if (r1 < 0 && r2 >= 0) {
          /* negative slope, f(phi1) = inf:
           * crossing if f(phi2) < log(r2)
           */
          plus1 = r0 + (k + 1) * dr - log10(r2);
          if (plus1 > 0) {
            HPcellP[k][j].push_back(SL);
            HPcellP[k][j].push_back(i);
            if (! firstP)
              outcP << " or ";

            if (ffP != 0) {
              if (j < 40)
                outputfP << "or ";
            }
            ffP++;
            if (j < 40)
              outputfP << "SL" << SL << "_TS(" << i << ") ";

            outcP << "SL" << SL << "_TS[" << i << "]";
            firstP = false;
          }
        }
      }

      if (j > 39 && j < 80)
        outputfM << ";" << endl;
      if (j < 40)
        outputfP << ";" << endl;
      outcM << ");" << endl;
      outcP << ");" << endl;
    }
  }
  outputfM << " " << endl;
  outputfM << "end Behavioral;" << endl;
  outputfP << " " << endl;
  outputfP << "end Behavioral;" << endl;
  return;
}
