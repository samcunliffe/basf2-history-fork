#include <TFile.h>
#include <TTree.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <ecl/digitization/algorithms.h>
#include <iostream>
#include <TH1.h>
#include <TF1.h>
#include <TH2.h>
#include <TMath.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include "stdlib.h"
#include <TChain.h>
#include <string>
#include <fstream>
#include <vector>
#include <cassert>
#include <framework/utilities/FileSystem.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

int num1(int a)
{
  int out;
  out = -1;
  if (!(a > 135 || a < 0)) {
    if (a ==  0)  { out =   0;}
    else if (a <   3)  { out =   1;}
    else if (a <   6)  { out =   2;}
    else if (a <  10)  { out =   3;}
    else if (a <  15)  { out =   4;}
    else if (a <  21)  { out =   5;}
    else if (a <  28)  { out =   6;}
    else if (a <  36)  { out =   7;}
    else if (a <  45)  { out =   8;}
    else if (a <  55)  { out =   9;}
    else if (a <  66)  { out =  10;}
    else if (a <  78)  { out =  11;}
    else if (a <  91)  { out =  12;}
    else if (a < 105)  { out =  13;}
    else if (a < 120)  { out =  14;}
    else if (a < 136)  { out =  15;}
  }
  return out;
}

int num2(int a)
{
  int out;
  out = -1;
  if (!(a > 135 || a < 0)) {
    if (a ==  0)  { out =       0;}
    else if (a <   3)  { out = a -   1;}
    else if (a <   6)  { out = a -   3;}
    else if (a <  10)  { out = a -   6;}
    else if (a <  15)  { out = a -  10;}
    else if (a <  21)  { out = a -  15;}
    else if (a <  28)  { out = a -  21;}
    else if (a <  36)  { out = a -  28;}
    else if (a <  45)  { out = a -  36;}
    else if (a <  55)  { out = a -  45;}
    else if (a <  66)  { out = a -  55;}
    else if (a <  78)  { out = a -  66;}
    else if (a <  91)  { out = a -  78;}
    else if (a < 105)  { out = a -  91;}
    else if (a < 120)  { out = a - 105;}
    else if (a < 136)  { out = a - 120;}
  }
  return out;
}

int mum1(int a)
{
  int out;
  out = -1;
  if (!(a > 495 || a < 0)) {
    if (a ==  0)  { out =   0;}
    else if (a <   3)  { out =   1;}
    else if (a <   6)  { out =   2;}
    else if (a <  10)  { out =   3;}
    else if (a <  15)  { out =   4;}
    else if (a <  21)  { out =   5;}
    else if (a <  28)  { out =   6;}
    else if (a <  36)  { out =   7;}
    else if (a <  45)  { out =   8;}
    else if (a <  55)  { out =   9;}
    else if (a <  66)  { out =  10;}
    else if (a <  78)  { out =  11;}
    else if (a <  91)  { out =  12;}
    else if (a < 105)  { out =  13;}
    else if (a < 120)  { out =  14;}
    else if (a < 136)  { out =  15;}
    else if (a < 153)  { out =  16;}
    else if (a < 171)  { out =  17;}
    else if (a < 190)  { out =  18;}
    else if (a < 210)  { out =  19;}
    else if (a < 231)  { out =  20;}
    else if (a < 253)  { out =  21;}
    else if (a < 276)  { out =  22;}
    else if (a < 300)  { out =  23;}
    else if (a < 325)  { out =  24;}
    else if (a < 351)  { out =  25;}
    else if (a < 378)  { out =  26;}
    else if (a < 406)  { out =  27;}
    else if (a < 435)  { out =  28;}
    else if (a < 465)  { out =  29;}
    else if (a < 496)  { out =  30;}

  }
  return out;

}

int mum2(int a)
{
  int out;
  out = -1;
  if (!(a > 495 || a < 0)) {
    if (a ==  0)  { out =       0;}
    else if (a <   3)  { out = a -   1;}
    else if (a <   6)  { out = a -   3;}
    else if (a <  10)  { out = a -   6;}
    else if (a <  15)  { out = a -  10;}
    else if (a <  21)  { out = a -  15;}
    else if (a <  28)  { out = a -  21;}
    else if (a <  36)  { out = a -  28;}
    else if (a <  45)  { out = a -  36;}
    else if (a <  55)  { out = a -  45;}
    else if (a <  66)  { out = a -  55;}
    else if (a <  78)  { out = a -  66;}
    else if (a <  91)  { out = a -  78;}
    else if (a < 105)  { out = a -  91;}
    else if (a < 120)  { out = a - 105;}
    else if (a < 136)  { out = a - 120;}
    else if (a < 153)  { out = a - 136;}
    else if (a < 171)  { out = a - 153;}
    else if (a < 190)  { out = a - 171;}
    else if (a < 210)  { out = a - 190;}
    else if (a < 231)  { out = a - 210;}
    else if (a < 253)  { out = a - 231;}
    else if (a < 276)  { out = a - 253;}
    else if (a < 300)  { out = a - 276;}
    else if (a < 325)  { out = a - 300;}
    else if (a < 351)  { out = a - 325;}
    else if (a < 378)  { out = a - 351;}
    else if (a < 406)  { out = a - 378;}
    else if (a < 435)  { out = a - 406;}
    else if (a < 465)  { out = a - 435;}
    else if (a < 496)  { out = a - 465;}
  }
  return out;
}


double par_vmat[31][31] = {
  { 3.7421890e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 2.3000690e-01, 2.9909840e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -1.3080280e-03, 2.9275420e-01, 2.3547880e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -1.1187620e-01, 7.9271060e-02, 2.5876040e-01, 2.3182400e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -8.9043590e-02, -7.3288660e-02, 7.3667100e-02, 2.6195730e-01, 2.3093170e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -3.1746960e-02, -8.5953160e-02, -7.4380120e-02, 8.0350670e-02, 2.6405940e-01, 2.2696520e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 2.5790630e-03, -4.2404220e-02, -9.1190030e-02, -7.3073500e-02, 8.2190800e-02, 2.5937040e-01, 2.2902850e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 1.1706650e-02, -6.0437690e-03, -4.6543940e-02, -9.2237390e-02, -6.7980100e-02, 7.9883190e-02, 2.5576430e-01, 2.3048010e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 5.2791750e-03, 7.3584680e-03, -9.1680320e-03, -4.0477280e-02, -8.5167550e-02, -6.8960090e-02, 7.9615790e-02, 2.6272700e-01, 2.2659670e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -2.7234810e-03, 7.6854350e-03, 7.3870300e-03, -1.5748630e-03, -3.7431840e-02, -8.6361930e-02, -7.1186920e-02, 8.4427120e-02, 2.5761590e-01, 2.2628580e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -4.8001330e-03, 3.7249190e-04, 5.7445500e-03, 6.2892080e-03, 1.2794980e-04, -4.4020030e-02, -9.4132710e-02, -6.5592440e-02, 7.9605540e-02, 2.5897260e-01, 2.2759290e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -1.9747340e-04, -1.0856600e-03, -2.9537980e-03, 1.2521320e-03, 9.2468480e-03, -7.9964520e-03, -5.0048620e-02, -8.6173260e-02, -6.9401890e-02, 8.5095670e-02, 2.5553720e-01, 2.2827970e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 2.2961840e-03, 5.1713580e-03, -9.9831220e-03, -4.8582790e-03, 5.6818920e-03, 2.5234470e-03, -1.1992620e-02, -4.3354460e-02, -8.8238830e-02, -6.3690040e-02, 7.4933660e-02, 2.6035110e-01, 2.2954150e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 1.9911500e-03, 5.6928930e-03, -6.5999590e-03, -5.1220450e-03, 1.3726110e-03, 2.6638390e-03, 6.0810940e-03, -5.5482670e-03, -4.4492380e-02, -8.4784940e-02, -7.4847700e-02, 8.0023210e-02, 2.5971760e-01, 2.2579760e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -2.8168460e-03, -1.5596470e-03, -3.5920740e-04, -1.5568680e-03, -3.0999980e-03, 4.0532330e-03, 5.9865620e-03, 7.5426870e-03, -5.0801370e-03, -4.4410850e-02, -8.9069310e-02, -7.1986760e-02, 8.3717220e-02, 2.5525320e-01, 2.3127400e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -6.2721110e-03, -7.3843720e-03, 4.4293180e-03, -3.1976570e-04, -9.5977030e-03, 4.7985730e-03, 1.3091160e-03, 6.2075590e-03, 1.0678170e-02, -3.9102230e-03, -3.8850100e-02, -9.0111270e-02, -7.2134940e-02, 8.1626010e-02, 2.6625160e-01, 2.3085280e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -8.6380870e-03, -8.7524700e-03, 2.2211860e-03, -2.0377460e-03, -1.1674680e-02, 9.5736500e-04, -4.6511550e-03, 1.1878210e-03, 8.2732240e-03, 1.1068450e-02, -2.4557110e-03, -3.9989100e-02, -9.6560530e-02, -6.7282040e-02, 8.6545660e-02, 2.6266760e-01, 2.2867910e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -5.0006910e-03, -5.3271550e-03, 2.8122650e-03, -3.3844180e-03, -1.2066290e-02, -5.0215660e-03, -5.9443560e-03, -7.7641110e-03, 6.8881110e-04, 1.1126780e-02, 3.5682230e-03, -1.5581290e-03, -4.9215720e-02, -8.7495330e-02, -6.9457910e-02, 8.3078790e-02, 2.5626930e-01, 2.2815010e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -2.8006430e-03, -1.8326510e-03, 4.2013250e-03, -7.3136040e-03, -7.2753960e-03, -1.2072630e-02, -4.9111860e-03, -1.2910630e-02, -7.4327140e-03, 1.7809370e-03, 3.5908200e-03, 9.9266630e-03, -6.1830250e-03, -4.3952940e-02, -9.4068760e-02, -6.9196380e-02, 7.4456340e-02, 2.6240600e-01, 2.2787320e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -5.3262280e-03, 1.0013970e-03, 2.7988670e-03, -7.0614570e-03, -1.2011950e-03, -1.0057020e-02, -2.0390070e-03, -9.7144740e-03, -1.1931430e-02, -4.3344790e-03, -1.0781390e-04, 4.9748730e-03, 7.6804310e-03, -7.0118680e-03, -4.9857660e-02, -9.0621760e-02, -7.3404560e-02, 8.7933660e-02, 2.6087080e-01, 2.2861880e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -8.0181630e-03, -4.9711830e-04, -4.1150970e-03, -7.1515530e-04, 3.4825090e-03, -5.7702340e-03, -2.7436930e-04, -3.0197620e-03, -7.0143680e-03, 1.9427750e-04, -4.9010800e-03, 8.0104930e-04, 5.2002340e-03, 7.9739980e-03, -5.1064490e-03, -4.4475750e-02, -8.8433030e-02, -6.6086290e-02, 7.9523210e-02, 2.6124720e-01, 2.2560000e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -8.3994180e-03, 1.7126790e-03, -7.4873680e-03, 1.2178530e-03, 1.7478160e-03, -4.8931630e-03, -2.6458510e-03, 2.1648390e-03, -1.1115890e-03, 2.2563300e-03, -4.1913370e-03, -1.8302860e-03, 1.9013060e-03, 1.0287630e-02, 9.4747190e-03, -3.1773300e-03, -4.4333930e-02, -8.9549180e-02, -7.0587700e-02, 8.2035590e-02, 2.5992680e-01, 2.2710300e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -5.3834890e-03, 2.2494540e-04, -6.6540030e-03, 9.9553950e-04, -3.4951750e-03, -6.3159040e-03, -2.7434850e-03, 3.1578660e-03, 1.1148350e-03, -3.6016090e-03, -3.0389420e-03, -2.4136690e-03, -4.2965950e-04, 4.0942390e-03, 9.4291110e-03, 1.3586890e-02, -6.0214740e-03, -4.9719230e-02, -9.1375190e-02, -7.3945560e-02, 8.5397150e-02, 2.6079940e-01, 2.2696250e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 2.5065560e-03, -3.8069260e-03, -6.7662800e-04, -2.3149350e-03, -4.6083060e-03, -7.5430530e-03, 1.6668370e-04, -6.3735420e-03, -3.1789190e-03, -7.7085180e-03, -5.2746390e-03, 1.7362840e-03, -9.6647280e-04, -2.9666610e-03, 1.4076840e-03, 1.2331600e-02, 5.9475350e-03, -1.1358520e-02, -4.4444720e-02, -9.5015340e-02, -6.8087640e-02, 8.5311790e-02, 2.5924030e-01, 2.2694380e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 3.6664960e-03, -5.9755790e-03, 1.1686970e-03, -3.2539950e-03, -5.1949060e-03, -7.4019820e-04, 2.9363740e-03, -1.0248240e-02, -2.6863790e-03, -5.4554580e-03, -6.1217710e-03, -2.0915870e-03, -4.4379890e-03, -4.6429420e-03, -6.0521850e-03, 6.9500020e-03, 2.0054410e-03, 1.0426860e-02, -3.7424950e-03, -4.6205980e-02, -9.1165860e-02, -6.6677650e-02, 8.0329110e-02, 2.5543810e-01, 2.3034990e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { 9.3078260e-05, -3.3525630e-03, 7.1849010e-04, -8.6913580e-03, -5.3926540e-03, -1.7520550e-03, 4.3132800e-03, -7.0162730e-03, -5.4201340e-03, 6.8684810e-05, -5.7997490e-03, -1.6304960e-03, -3.7536150e-03, -3.3821230e-03, -8.6243110e-03, -8.5879090e-04, -5.9199840e-03, 1.1289290e-02, 1.2017320e-02, -7.8448640e-03, -4.8806090e-02, -8.8978310e-02, -7.1498770e-02, 8.1850020e-02, 2.5965610e-01, 2.2584000e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -3.9975890e-03, -1.9684830e-03, -2.9398390e-03, -1.0742950e-02, -5.7260550e-04, -6.4837480e-03, -4.1097970e-04, -4.0319450e-03, -8.4026530e-03, -2.5424590e-04, -3.9595570e-04, -1.2657450e-03, -2.3319600e-03, -1.7530350e-03, -5.0691880e-03, -7.0347300e-03, -6.0129350e-03, 4.8913850e-03, 5.5869180e-03, 6.8177870e-03, -5.5931780e-03, -4.4098010e-02, -8.8670600e-02, -6.4754300e-02, 7.7477840e-02, 2.5689540e-01, 2.2915040e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -4.4068680e-03, -6.3513130e-03, -3.2901120e-03, -8.6245530e-03, 5.1832430e-04, -6.5335830e-03, -4.8460790e-03, -6.2148260e-03, -2.5604090e-03, -5.7988390e-03, 5.4880520e-04, -3.5538550e-03, -2.1809030e-03, -1.1549950e-03, -5.6306170e-03, -7.8568590e-03, -6.0848720e-04, -1.8847850e-03, -6.0237990e-03, 1.1852250e-02, 1.3302270e-02, -7.1505840e-03, -4.1669270e-02, -8.8059120e-02, -7.1328220e-02, 8.0681230e-02, 2.6154300e-01, 2.2745840e-01, 0.0000000e+00, 0.0000000e+00, 0.0000000e+00},
  { -1.5461320e-04, -1.1682510e-02, -7.9529830e-03, -1.3080170e-05, -1.2187170e-03, -7.0621350e-03, -6.8091830e-03, -3.7288430e-03, 4.7856920e-03, -8.9847700e-03, -4.5388100e-03, -4.1014860e-03, -3.6393970e-03, -5.8545800e-03, -5.6582780e-03, -5.2623870e-03, 5.0993990e-03, -6.7282240e-03, -1.0117930e-02, 6.4603410e-03, 3.5110150e-03, 3.1667650e-03, -5.7096600e-03, -4.7129750e-02, -9.0150530e-02, -6.9686500e-02, 8.3052490e-02, 2.5950580e-01, 2.2833470e-01, 0.0000000e+00, 0.0000000e+00},
  { 1.0202740e-03, -9.5381150e-03, -8.9831140e-03, 1.3487770e-03, -7.1046240e-03, -1.1002140e-02, 3.8150150e-04, -4.7184470e-03, 4.4583300e-03, -7.8033130e-03, -8.4168570e-03, -3.4055110e-03, -3.3224790e-03, -7.6967760e-03, -8.7475620e-03, -1.2469270e-03, 1.8047480e-03, -7.2658010e-03, -8.3518510e-03, 3.5113370e-04, -1.0644220e-02, 2.0328100e-03, 3.6619910e-03, -9.2137200e-03, -4.5310810e-02, -8.9684530e-02, -7.0173430e-02, 8.2865370e-02, 2.6143560e-01, 2.2800430e-01, 0.0000000e+00},
  { 5.3032550e-04, -3.7301180e-03, -1.1787730e-02, -1.3807920e-03, -5.6132690e-03, -1.1913140e-02, 9.5198650e-04, -1.1367470e-02, 2.2965180e-03, -5.7564360e-03, -9.0643900e-03, -5.9737380e-03, -3.9860480e-03, -5.9836610e-03, -1.1576390e-02, -2.3590400e-03, -4.7896110e-03, -1.5279510e-03, -7.4392120e-03, -1.8085080e-03, -1.3247650e-02, -5.4693190e-04, 5.4005680e-04, 4.9617630e-03, -7.2788800e-03, -4.5880410e-02, -9.1447090e-02, -6.8188230e-02, 8.2035530e-02, 2.5516690e-01, 2.2805310e-01}
};



float par[10] = {0.5, 0.6483, 0.4017, 0.3741, 0.8494, 0.00144547, 4.7071, 0.8156, 0.5556, 0.2752};



void writeWF(int typ, char* dataFileDir, char* paramsDir)
{

  double ss1[16][16];

  vector<Int_t> kA(252, 0);
  vector<Int_t> kB(252, 0);
  vector<Int_t> kC(252, 0);
  vector<Int_t> k1(252, 0);
  vector<Int_t> k2(252, 0);

  int id0;
  int id1;
  int id2;
  int id3;
  int id4;
  int id5;
  int id6;

  int ifile;
  ifile = 0;

  // ORIGINAL files were at
  //  sprintf(BMin,"/gpfs/home/belle/avbobrov/ecl/covmat/eclwaveform2/bitst%d.dat",typ);

  string inputFile(paramsDir);
  inputFile += "/bitst";
  inputFile += to_string(typ) + ".dat";

  ifstream inputData(inputFile);
  while (true) {
    inputData >> ifile >> id0 >> id1 >> id2 >> id3 >> id4 >> id5 >> id6;
    if (inputData.eof()) break;
    if (id6 < id4)id4 = id6;
    if (id5 < id2)id2 = id5;

    if (ifile < 252) {
      kA[ifile] = id2;
      kB[ifile] = id3;
      kC[ifile] = id4;
      k1[ifile] = 3;
      k2[ifile] = 4;
    }
  }

  const string crystalIDBlocks = FileSystem::findFile("/data/ecl/CIdToEclData.txt");
  if (crystalIDBlocks.empty()) exit(1);
  ifstream ctoecldatafile(crystalIDBlocks.c_str());

  int cid, group;
  int maxGroup = 0;

  //first find the largest group number (numbering goes from 0 to N-1)

  while (true) {
    ctoecldatafile >> cid >> group;
    if (ctoecldatafile.eof()) break;
    if (maxGroup < group) maxGroup = group;
  }

  int nGroups = maxGroup + 1;
  vector< vector<int> > grmap(nGroups); //should be 252 entries
  ctoecldatafile.close();
  ctoecldatafile.open(crystalIDBlocks.c_str());

  while (true) {
    ctoecldatafile >> cid >> group;
    if (ctoecldatafile.eof()) break;
    cout << cid << " " << group << endl;
    grmap[ group ].push_back(cid + 1);
  }

  for (int n = 0; n < nGroups; n++) {
    vector<int>& v = grmap[n];
    cout << v.size() << endl;
  }

  cout << "Done building map" << endl;



  //    TFile f("ECL-WF.root","recreate");
  TFile* f = new TFile("ECL-WF-BG-XX.root", "recreate");

  TTree* t =  new TTree("EclWF", "Waveform and covariance matrix");
  TTree* t2 = new TTree("EclAlgo", "Fitting algorithm parameters");
  TTree* t3 = new TTree("EclNoise", "Electronic noise matrix");
  //  TTree* t4 = new TTree("EclSampledSignalWF", "Signal Waveform in fine bins");

  ECLWaveformData* data = new ECLWaveformData;
  ECLWFAlgoParams* algo = new ECLWFAlgoParams;
  ECLNoiseData* noise = new ECLNoiseData;

  Int_t ncellId1;
  vector<Int_t> cellId1(8736);
  Int_t ncellId2;
  vector<Int_t> cellId2(8736);
  Int_t ncellId3;
  vector<Int_t> cellId3(8736);

  t->Branch("CovarianceM", &data, 256000);
  t->Branch("ncellId", &ncellId1, "ncellId/I");
  t->Branch("cellId", & cellId1[0], "cellId[ncellId]/I");

  cout << " t "  << endl;

  t2->Branch("Algopars", &algo, 256000);
  t2->Branch("ncellId", &ncellId2, "ncellId/I");
  t2->Branch("cellId", & cellId2[0], "cellId[ncellId]/I");
  cout << " t2 "  << endl;

  t3->Branch("NoiseM", &noise, 256000);
  t3->Branch("ncellId", &ncellId3, "ncellId/I");
  t3->Branch("cellId", & cellId3[0], "cellId[ncellId]/I");

  cout << " t3 "  << endl;

  //  Int_t nbins = par_shape.size();
  // auto shapeP = createDefSampledSignalWF();
  // t4->Branch("SignalShape", "std::vector<double>", &shapeP);
  // t4->Fill();

  for (int n = 0; n < nGroups; n++) {
    cout << " !n: " << n << endl;
    //    sprintf(BMin,"/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat",typ,n);
    string dataFileName(dataFileDir);
    dataFileName += "/corr" + to_string(typ) + "/Binmcor" + to_string(n) + "_L.dat";

    ifstream inputFile(dataFileName, ios::binary | ios::in);
    for (int index = 0; index < 256; index++) {
      inputFile.read(reinterpret_cast< char*>(&ss1[index]), sizeof(double));
    }
    inputFile.close();

    vector<int>& v = grmap[n];
    if (v.size() == 0) continue;
    ncellId1 = ncellId2 = v.size();
    for (size_t i = 0; i < v.size(); ++i) cellId2[i] = cellId1[i] = v[i];
    for (size_t idx = 0; idx < data -> c_nElements; ++idx) {
      int a = num1(idx);
      int b = num2(idx);
      data->setMatrixElement(idx, ss1[a][b]);
    }
    for (size_t idx = 0; idx < data -> c_nParams; ++idx) {

      if (idx < 10) {
        data->setWaveformPar(idx, par[idx]) ;
      }
    }
    cout << " write 136+10 "  << endl;

    t->Fill();



    algo->ka = kA[n];
    algo->kb = kB[n];
    algo->kc = kC[n];
    algo->y0Startr = 16;
    algo->chiThresh = 3000;
    algo->k1Chi = 14;
    algo->k2Chi = 10;
    algo->hitThresh = 100;
    algo->lowAmpThresh = 5;
    algo->skipThresh = -20;

    t2->Fill();

  }

  ncellId3 = 0;
  for (size_t idx = 0; idx < noise -> c_nElements; ++idx) {
    int a = mum1(idx);
    int b = mum2(idx);
    noise->m_matrixElement[ idx ] =  par_vmat[a][b];
    cout << "idx=" << idx << " a=" << a << " b=" << b << " " << par_vmat[a][b] << " " << par_vmat[b][a] << endl;
  }

  cout << " write 496 "  << endl;
  t3->Fill();

  t->Write(); t2->Write(); t3->Write();// t4->Write();
  f->Close();

}

// sprintf(BMin,"/hsm/belle/bdata2/users/avbobrov/belle2/corr%d/Binmcor%d_L.dat",typ,n);
int main(int argc, char** argv)

{
  assert(argc == 4 || argc == 1);
  if (argc == 1) {
    cout << "Usage " << endl;
    cout << argv[0] << " <type>  <covar_mat_path> <parameters_path>" << endl;
    cout << "type is an integer idenfifying the source of the calibration" << endl;
    cout << "cov_mat_path is the path to find where the directory corr<type> where the Binmcor*_L.dat files are located." << endl;
    cout << "These files contain the covariance matrices." << endl;
    cout << "parameters_path is the path to find bitst<type>.dat file that contains define parameters of the fit algorithms" << endl;
  } else
    writeWF(atoi(argv[1]), argv[2], argv[3]);
}

