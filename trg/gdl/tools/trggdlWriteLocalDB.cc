/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty   hearty@physics.ubc.ca               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/database/DBStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/logging/LogSystem.h>
#include <mdst/dbobjects/TRGGDLDBPrescales.h>
#include <mdst/dbobjects/TRGGDLDBFTDLBits.h>
#include <mdst/dbobjects/TRGGDLDBInputBits.h>
#include <mdst/dbobjects/TRGGDLDBBadrun.h>
#include <trg/gdl/dbobjects/TRGGDLDBUnpacker.h>
#include <trg/gdl/dbobjects/TRGGDLDBDelay.h>
#include <trg/gdl/dbobjects/TRGGDLDBAlgs.h>
#include <iostream>
#include <fstream>
//#include <TFile.h>
//#include <TH1F.h>

using namespace Belle2;

#define ONLINE 0
//#define ONLINE 1
//#define ONLINE 2


/* common for setalg, setinputbits, setftdlbits */
const int N_FTD_VERS = 15;
const int ftd_run[N_FTD_VERS][4] = { //itnitial exp, initial run, end exp, end run
  0,  0,    -1,   -1, //  0
  7,  0,     7, 2102, //  1 11
  7,  2103,  7, 2367, //  2 12 +{tsf0b2b,sl0b2b}
  7,  2368,  8, 4023, //  3 13 +{eklm_hit,eklm(4),eklmb2b(3),mu_epair,mu_eb2b,eklmhit}
  7,  4024,  8, 1185, //  4 14 +{fffc,fffc2,ffoc2}
  8,  1186,  8, 1285, //  5 15 +{fffo,fffov}
  8,  1286,  8, 1505, //  6 14 gdl0070p2
  8,  1506,  8, 2043, //  7 15 gdl0070r
  8,  2044,  8, 2782, //  8 16 +{tsf[12]b2b,sl[12]b2b,sl12b2b}
  8,  2783,  8,   -1, //  9 17 +{t_a(4),ecl_bst,a,aa,aao,aab,aaao,aaa,eclbst}
  10, 135,  10, 2539, // 10 22 +{ecl_lml_13,ecl_lml_12,injv,hade,vetout}, -ecl_lml_11
  10, 2540, 10, 4600, // 11 23 {t_a,ecl_bst,ecl_3dbha,injv,tsf[12]b2b,ecl_lml_13} moved,
  //      +{s2[sf][35o],grl{gg,bb}} -nimin[23], #klm  reduced
  10, 4601, 12, 1659, // 12 24 o and b for short trk chged to s2[sf][o5]
  12, 1660, 12, 1858, // 13 25 +{bff,bhie}
  12, 1859, -1,   -1  // 14 26 trk req removed from mu_pair
};
const int ftd_version[] = {
  24, 11, 12, 13, 14, 15, 14, 15, 16, 17, 22, 23, 24, 25, 26
};


//prescale setting
void setprescale()
{

  const int N_BITS_RESERVED = 320;
  const int N_PSNM_ARRAY = 23 + 1; //start from -1

  const int run[N_PSNM_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,    -1, -1,
    0, 0,     3, 107,
    3, 108,   3, 295,
    3, 296,   3, 1511,
    3, 1512,  3, 1810,
    3, 1811,  3, 1994,
    3, 1995,  3, 2000,
    3, 2001,  3, 2313,
    3, 2314,  3, 3503,
    3, 3504,  3, 5340,
    3, 5341,  3, -1,
    7,  920,  7, 1371,
    7, 1372,  7, 1478,
    7, 1479,  7, 3215,
    7, 3216,  8, 1201,
    8, 1202,  8, -1,    // fffo:0->1, ftd unchanged.
    10, 3129, 10, 3130,
    10, 3131, 10, 3436,
    10, 3437, 10, 3442,
    10, 3443, 10, 3548,
    10, 3549, 10, 4608,
    10, 4609, 10, 5387,
    10, 5388, 10, 5728,
    10, 5729, 10, -1
  };

  const int nbit[N_PSNM_ARRAY] = {  0, 18, 44, 63, 63,
                                    63, 72, 75, 75, 75,
                                    75, 133, 133, 133, 133,
                                    142, 160, 160, 160, 160,
                                    160, 160, 160, 160
                                 };

  const unsigned
  psnmValues[N_PSNM_ARRAY][N_BITS_RESERVED] = {
    // -1
    {
      0
    },

    // 0
    // psn_0055 r59-
    {
      0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 1
    },

    // 1
    // psn_0056 r108-
    {
      0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
      0, 1, 0, 1
    },

    // 2
    // psn_0057 r296-
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 3
    // psn_0058 r1512-r1615 ecltiming=1
    // Not used for good data
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 1, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 4
    // psn_0057 r1811- ecltiming=0
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20
    },

    // 5
    // psn_0059 r1995- 5 Dark bits
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1
    },

    // 6
    // psn_0059 r2001- 3 Dark bits
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 30, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 7
    // psn_0060 r2314- eclnimo03 30->300
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 8
    // psn_0063 e3504- bg(26)->1
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    // 9
    // psn_0064=65. e5341- eed,fed,fp,bha3d
    // Can be earlier than 5341 but
    // runsum files lost.
    {
      0, 0, 1, 1, 0, 0, 1, 1, 1, 0,
      1, 150, 0, 1, 0, 1, 1, 1, 1, 20,
      1500, 150, 0, 0, 1, 1, 1, 0, 0, 0,
      0, 0, 0, 300, 0, 0, 20000, 0, 0, 0,
      0, 0, 400000, 1, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 1, 1,
      1, 0, 20, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1
    },

    //10
    //e7r920
    {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      20, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
      150, 50, 1, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 20, 1500, 150, 1, 1, 1, 0,
      0, 0, 60, 40, 40, 160000, 0, 0, 1, 400,
      0, 2000, 1, 1, 1, 0, 0, 2000, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 700,
      40, 1, 1, 1, 1, 1, 0, 200, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    },

    //11
    //e7r1372
    {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      20, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
      150, 50, 1, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 20, 1500, 150, 1, 1, 1, 0,
      0, 0, 60, 40, 40, 160000, 0, 0, 1, 400,
      0, 2000, 1, 1, 1, 0, 0, 2000, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    },

    //12
    //e7r1479
    {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      20, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
      150, 50, 1, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 20, 1500, 150, 1, 1, 1, 0,
      0, 0, 60, 40, 40, 160000, 0, 0, 1, 400,
      0, 2000, 1, 1, 0, 0, 0, 2000, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    },

    //13
    //e7r3216
    {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      20, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
      150, 50, 1, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 20, 1500, 150, 1, 1, 1, 0,
      0, 0, 0, 40, 40, 0, 0, 0, 1, 0,
      0, 0, 1, 1, 0, 0, 0, 2000, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, 0, 0, 3000,
      150, 1, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0
    },

    //14
    //e8r1202
    {
      1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      20, 0, 0, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
      150, 50, 1, 0, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 20, 1500, 150, 1, 1, 1, 0,
      0, 0, 0, 40, 40, 0, 0, 0, 1, 0,
      0, 0, 1, 1, 0, 0, 0, 2000, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 1, 1, 0, 0, 3000,
      150, 1, 1, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 1
    },

    // 15
    // e10r3129
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 16
    // e10r3131
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 17
    // e10r3437
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 18
    // e10r3443
    {1, 100, 0, 100, 0, 0, 0, 0, 0, 0, 20, 0, 400, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 19
    // e10r3549
    {1, 100, 0, 100, 0, 0, 0, 0, 0, 0, 20, 0, 400, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 40, 0, 0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 20
    // e10r4609
    {1, 100, 0, 100, 0, 0, 0, 0, 0, 0, 20, 0, 400, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 40, 0, 0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 21
    // e10r5388
    {1, 100, 0, 100, 0, 0, 0, 1, 0, 0, 20, 0, 400, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 40, 0, 0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    // 22
    // e10r5729
    {1, 100, 0, 100, 0, 0, 0, 1, 0, 0, 20, 0, 400, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 150, 50, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 20, 1500, 150, 1, 1, 1, 0, 0, 0, 0, 40, 40, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 40, 0, 0, 0, 2000, 4000, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3000, 150, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

  };


  DBImportObjPtr<TRGGDLDBPrescales> prescales;
  prescales.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_PSNM_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        prescales->setprescales(j, 0);
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        prescales->setprescales(j, psnmValues[i][j]);
      }
      prescales->setnoutbit(nbit[i]);
      prescales.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_PSNM_ARRAY - 1; i < N_PSNM_ARRAY; i++) {
      //for (int i = 0; i < 1; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        prescales->setprescales(j, 0);
      }
      //set
      for (int j = 0; j < nbit[i]; j++) {
        prescales->setprescales(j, psnmValues[i][j]);
      }
      prescales->setnoutbit(nbit[i]);
      prescales.import(iov);
    }
  }


}


//input and output bits
void setftdlbits()
{

  const int N_BITS_RESERVED = 320;
//const int nbit[N_OUTPUT_ARRAY] = {
//  160, 134/*11*/, 135/*12*/, 138/*13*/, 142/*14*/,
//  144/*15*/, 142/*14*/, 144/*15*/, 147/*16*/, 154/*17*/,
//  157/*22*/, 160/*23*/, 160/*24*/, 160/*25*/, 160/*26*/
//};


  DBImportObjPtr<TRGGDLDBFTDLBits> ftdlbits;
  ftdlbits.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_FTD_VERS; i++) {
      IntervalOfValidity iov(ftd_run[i][0], ftd_run[i][1], ftd_run[i][2], ftd_run[i][3]);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        ftdlbits->setoutbitname(j, "");
      }
      //set
      char logname[2000];
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.oup", ftd_version[i]);
      std::ifstream isinp(logname, std::ios::in);
      std::string str;
      int j = 0;
      while (std::getline(isinp, str)) {
        int bitnum;
        char bitname[30];
        sscanf(str.data(), "%d  %s", &bitnum, bitname);
        ftdlbits->setoutbitname(j, bitname);
        j++;
      }
      isinp.close();
      ftdlbits->setnoutbit(j);
      ftdlbits.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_FTD_VERS - 1; i < N_FTD_VERS; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        ftdlbits->setoutbitname(j, "");
      }
      //set
      char logname[2000];
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.oup", ftd_version[i]);
      std::ifstream isinp(logname, std::ios::in);
      std::string str;
      int j = 0;
      while (std::getline(isinp, str)) {
        int bitnum;
        char bitname[30];
        sscanf(str.data(), "%d  %s", &bitnum, bitname);
        ftdlbits->setoutbitname(j, bitname);
        j++;
      }
      isinp.close();
      ftdlbits->setnoutbit(j);
      ftdlbits.import(iov);
    }
  }

}

void setinputbits()
{

  const int N_BITS_RESERVED = 320;

  DBImportObjPtr<TRGGDLDBInputBits> inputbits;
  inputbits.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_FTD_VERS; i++) {
      IntervalOfValidity iov(ftd_run[i][0], ftd_run[i][1], ftd_run[i][2], ftd_run[i][3]);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        inputbits->setinbitname(j, "");
      }
      //set
      char logname[2000];
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.inp", ftd_version[i]);
      std::ifstream isinp(logname, std::ios::in);
      std::string str;
      int j = 0;
      while (std::getline(isinp, str)) {
        int bitnum;
        char bitname[30];
        sscanf(str.data(), "%d  %s", &bitnum, bitname);
        inputbits->setinbitname(j, bitname);
        j++;
      }
      isinp.close();
      inputbits->setninbit(j);
      inputbits.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_FTD_VERS - 1; i < N_FTD_VERS; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      //initialize
      for (int j = 0; j < N_BITS_RESERVED; j++) {
        inputbits->setinbitname(j, "");
      }
      //set
      char logname[2000];
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.inp", ftd_version[i]);
      std::ifstream isinp(logname, std::ios::in);
      std::string str;
      int j = 0;
      while (std::getline(isinp, str)) {
        int bitnum;
        char bitname[30];
        sscanf(str.data(), "%d  %s", &bitnum, bitname);
        inputbits->setinbitname(j, bitname);
        j++;
      }
      isinp.close();
      inputbits->setninbit(j);
      inputbits.import(iov);
    }
  }

}

void setunpacker()
{

  const int N_LEAF = 320;
  const int N_UNPACKER_ARRAY = 15;

  const int run[N_UNPACKER_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, -1,  -1,
    0,    0, 3,  528,
    3,  529, 3,  676,
    3,  677, 3, 1314,
    3, 1315, 3, 1865,
    3, 1866, 3, 4790,
    3, 4791, 3, 5313,
    3, 5314, 4, 6379,
    4, 6380, 4, 7433,
    4, 7434, 5,    0,
    5,    1, 6,   -1,
    7,    0, 7, 1560,
    7, 1561, 7, 2102,
    7, 2103, 10,  -1,
    12,   0, 12, -1
  };

  /** num of leafs in data_b2l **/
  const int nLeafs[N_UNPACKER_ARRAY] = {
    37, 37, 27, 26, 26,
    26, 31, 32, 31, 32,
    31, 31, 30, 27, 27
  };
  /** num of leafs for others **/
  const int nLeafsExtra[N_UNPACKER_ARRAY] = {
    8,   8,  9, 11, 11,
    11, 11, 11, 11, 11,
    11, 11, 13, 14, 16
  };
  /** num of clk time window **/
  const int nClks[N_UNPACKER_ARRAY] = {
    48, 48, 48, 48, 48,
    32, 32, 32, 32, 32,
    32, 32, 32, 32, 32
  };
  /** num of bits **/
  const int nBits[N_UNPACKER_ARRAY] = {
    640, 640, 640, 640, 640,
    640, 640, 640, 640, 640,
    640, 640, 640, 640, 640
  };

  /** num of inputleafmap raw **/
  const int nrows[N_UNPACKER_ARRAY] = {
    45, 45, 51, 52, 52,
    52, 57, 61, 61, 61,
    61, 61, 63, 66, 69
  };


  /** leaf names **/
  const char LeafNames[N_LEAF][100] = {
    "etffmv",   "l1rvc",    "timtype",  "etyp",    "final",
    "gdll1rvc", "etfout",   "etfvd",    "toprvc",  "topvd",
    "toptiming", "ecltiming", "cdctiming", "rvcout",  "rvcout0",
    "comrvc",   "etnrvc",   "nim0rvc",  "eclrvc",  "rvc",
    "drvc",     "ftd2",     "psn2",     "psn1",    "etfth2",
    "etfhdt",   "etfth",    "psn0",     "ftd1",    "cdcrvc",
    "ftd0",     "itd2",     "itd1",     "itd0",    "inp2",
    "inp1",     "inp0",     "evt",      "clk",     "firmid",
    "firmver",  "coml1rvc", "b2ldly",   "maxrvc",  "conf",
    "dtoprvc",  "declrvc",  "dcdcrvc",  "topslot1", "topslot0",
    "ntopslot", "finalrvc", "tttmdl",   "tdsrcp",  "tdtopp",
    "tdeclp",   "tdcdcp",   "psn3",     "ftd3",    "itd4",
    "itd3",     "cnttrg",   "cnttrg8",  "ftd4",    "psn4",
    "etmdata", "sepagdll1", "sepacoml1", "gdll1rev"
  };



  const int
  inputleafMap[N_UNPACKER_ARRAY][N_LEAF] = {
    {
      //-1
      0, 1, 2, 3, 4,
      5, 6, 7, 8, 9,
      10, 11, 12, 13, 14,
      15, 16, 17, 18, 19,
      20, 21, 22, 23, 24,
      25, 26, 27, 28, 29,
      30, 31, 32, 33, 34,
      35, 36, 37, 38, 39,
      40, 41, 42, 43, 44
    },

    {
      //0
      0, 1, 2, 3, 4,
      5, 6, 7, 8, 9,
      10, 11, 12, 13, 14,
      15, 16, 17, 18, 19,
      20, 21, 22, 23, 24,
      25, 26, 27, 28, 29,
      30, 31, 32, 33, 34,
      35, 36, 37, 38, 39,
      40, 41, 42, 43, 44
    },

    {
      //1
      -1, -1, 1, 2, -1,
      31, -1, -1, 5, -1,
      8, 9, 10, 3, -1,
      -1, -1, 11, 6, 0,
      4, 21, 15, 16, -1,
      -1, -1, 17, 22, 7,
      23, 24, 25, 26, -1,
      -1, -1, 27, 28, 29,
      30, 32, 33, 34, 35,
      12, 13, 14, 18, 19,
      20
    },

    {
      //2
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7, 8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //3
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7, 8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //4
      -1, -1, 1, 2, -1,
      32, -1, -1, 4, -1,
      7,  8, 9, 3, -1,
      -1, -1, 10, 5, 0,
      30, 20, 14, 15, -1,
      -1, -1, 16, 21, 6,
      22, 23, 24, 25, -1,
      -1, -1, 26, 27, 28,
      29, 33, 34, 35, 36,
      11, 12, 13, 17, 18,
      19, 31
    },

    {
      //5
      -1, -1, 1, 2, -1,
      37, -1, -1, 9, -1,
      12, 13, 14, 8, -1,
      -1, -1, 15, 10, 0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      16, 17, 18, 22, 23,
      24, 35, 3, 4, 5,
      6, 7
    },

    {
      //6
      -1, -1, 1, 2, -1,
      38, -1, -1, 9, -1,
      12, 13, 14, 8, -1,
      -1, -1, 15, 10, 0,
      37, 24, 17, 18, -1,
      -1, -1, 19, 25, 11,
      26, 29, 30, 31, -1,
      -1, -1, 32, 33, 34,
      35, 39, 40, 41, 42,
      -1, -1, -1, 20, 21,
      22, 36, 3, 4, 5,
      6, 7, 16, 23, 27,
      28
    },

    {
      //7
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    },

    {
      //8
      -1, -1, 1, 2, -1,
      38, -1, -1, 9, -1,
      12, 13, 14, 8, -1,
      -1, -1, 15, 10, 0,
      37, 24, 17, 18, -1,
      -1, -1, 19, 25, 11,
      26, 29, 30, 31, -1,
      -1, -1, 32, 33, 34,
      35, 39, 40, 41, 42,
      -1, -1, -1, 20, 21,
      22, 36, 3, 4, 5,
      6, 7, 16, 23, 27,
      28
    },

    {
      //9
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    },

    {
      //10
      -1, -1,  1,  2, -1,
      37, -1, -1,  9, -1,
      12, 13, 14,  8, -1,
      -1, -1, 15, 10,  0,
      36, 25, 19, 20, -1,
      -1, -1, 21, 26, 11,
      27, 28, 29, 30, -1,
      -1, -1, 31, 32, 33,
      34, 38, 39, 40, 41,
      -1, -1, -1, 22, 23,
      -1, 35,  3,  4,  5,
      6,   7, 18, 24, 16,
      17
    },

    {
      //11
      -1, -1, 41, -1, -1,
      36, -1, -1,  8, -1,
      11, 12, 13,  7, -1,
      -1, -1, 14,  9,  0,
      34, 24, 18, 19, -1,
      -1, -1, 20, 25, 10,
      26, 27, 28, 29, -1,
      -1, -1, 30, 31, 32,
      33, 37, 38, 39, 40,
      -1, -1, -1, 21, 22,
      -1, 35,  2,  3,  4,
      5,  6, 17, 23, 15,
      16, 42,  1
    },

    {
      //12
      -1, -1, 38, -1, -1,
      33, -1, -1, -1, -1,
      7,  8,  9, -1, -1,
      -1, -1, -1, -1,  0,
      31, 18, 24, 25, -1,
      -1, -1, 26, 19, -1,
      20, 12, 13, 14, -1,
      -1, -1, 27, 28, 29,
      30, 34, 35, 36, 37,
      -1, -1, -1, 15, 21,
      -1, 32,  2,  3,  4,
      5,  6, 23, 17, 10,
      11, 39,  1, 16, 22,
      40
    },

    {
      //13
      -1, -1, 36, -1, -1,
      33, -1, -1, -1, -1,
      7,  8,  9, 42, -1,
      -1, -1, -1, -1,  0,
      31, 18, 24, 25, -1,
      -1, -1, 26, 19, -1,
      20, 12, 13, 14, -1,
      -1, -1, 27, 28, 29,
      30, 34, -1, -1, 35,
      -1, -1, -1, 15, 21,
      -1, 32,  2,  3,  4,
      5,  6, 23, 17, 10,
      11, 37,  1, 16, 22,
      38, 39, 40, 41
    }

  };

  /** bus bit map. (a downto a-b) **/
  const int BitMap[N_UNPACKER_ARRAY][N_LEAF][2] = {

    {
      //-1
      639, 31, // etffmv
      575, 10, // l1rvc
      564, 2,  // timtype
      561, 2,  // etyp
      558, 0,  // final
      557, 0,  // gdll1rvc
      538, 12, // etfout
      525, 0,  // etfvd
      524, 14, // toprvc
      480, 0,  // topvd
      498, 17, // toptiming
      479, 13, // ecltiming (lsb1ns)
      465, 12, // cdctiming (lsb2ns)
      447, 14, // rvcout
      432,  0, // rvcout0
      431, 11, // comrvc
      419, 11, // etnrvc
      407, 11, // nim0rvc
      395, 11, // eclrvc
      383, 11, // rvc
      371, 11, // drvc
      355, 15, // ftd2
      339, 15, // psn2. Empty for 65c.
      323, 15, // psn1.
      307, 7,  // etfth2
      299, 7,  // etfhdt
      291, 3,  // etfth
      287, 31, // psn0
      255, 15, // ftd1
      234, 10, // cdcrvc
      223, 31, // ftd0
      191, 31, // itd2
      159, 31, // itd1
      127, 31, // itd0
      95,  31, // inp2
      63,  31, // inp1
      31,  31, // inp0
    },


    {
      //0
      639, 31, // etffmv
      575, 10, // l1rvc
      564, 2,  // timtype
      561, 2,  // etyp
      558, 0,  // final
      557, 0,  // gdll1rvc
      538, 12, // etfout
      525, 0,  // etfvd
      524, 14, // toprvc
      480, 0,  // topvd
      498, 17, // toptiming
      479, 13, // ecltiming (lsb1ns)
      465, 12, // cdctiming (lsb2ns)
      447, 14, // rvcout
      432,  0, // rvcout0
      431, 11, // comrvc
      419, 11, // etnrvc
      407, 11, // nim0rvc
      395, 11, // eclrvc
      383, 11, // rvc
      371, 11, // drvc
      355, 15, // ftd2
      339, 15, // psn2. Empty for 65c.
      323, 15, // psn1.
      307, 7,  // etfth2
      299, 7,  // etfhdt
      291, 3,  // etfth
      287, 31, // psn0
      255, 15, // ftd1
      234, 10, // cdcrvc
      223, 31, // ftd0
      191, 31, // itd2
      159, 31, // itd1
      127, 31, // itd0
      95,  31, // inp2
      63,  31, // inp1
      31,  31, // inp0
    },

    {
      //1
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      583, 14, // rvcout
      568, 10, // drvc
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //2
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //3
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //4
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //5
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 3,  // tdsrcp
      596, 2,  // tdtopp
      593, 2,  // tdeclp
      590, 2,  // tdcdcp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 11, // dtoprvc
      457, 11, // declrvc
      446, 11, // dcdcrvc
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //6
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 3,  // tdsrcp
      596, 2,  // tdtopp
      593, 2,  // tdeclp
      590, 2,  // tdcdcp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      218, 26, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      468, 15, // itd4
      452, 31, // itd3
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //7
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 0,  // tdsrcp
      599, 0,  // tdtopp
      598, 0,  // tdeclp
      597, 0,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //8
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 3,  // tdsrcp
      596, 2,  // tdtopp
      593, 2,  // tdeclp
      590, 2,  // tdcdcp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223,  4, // ntopslot
      218, 26, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      468, 15, // itd4
      452, 31, // itd3
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //9
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 0,  // tdsrcp
      599, 0,  // tdtopp
      598, 0,  // tdeclp
      597, 0,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //10
      623, 11, // rvc
      611, 2,  // timtype
      608, 2,  // etyp
      603, 2,  // tttmdl
      600, 0,  // tdsrcp
      599, 0,  // tdtopp
      598, 0,  // tdeclp
      597, 0,  // tdcdcp
      583, 14, // rvcout

      557, 10, // toprvc
      546, 10, // eclrvc
      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1
      319, 31, // psn0

      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //11
      623, 11, // rvc
      611, 7,  // cnttrg8
      603, 2,  // tttmdl
      600, 0,  // tdsrcp
      599, 0,  // tdtopp
      598, 0,  // tdeclp
      597, 0,  // tdcdcp
      583, 14, // rvcout
      557, 10, // toprvc
      546, 10, // eclrvc

      535, 10, // cdcrvc
      524, 13, // toptiming
      509, 13, // ecltiming
      494, 13, // cdctiming
      479, 10, // nim0rvc
      468, 15, // itd4
      452, 31, // itd3
      415, 31, // psn3
      383, 31, // psn2
      351, 31, // psn1

      319, 31, // psn0
      287, 31, // topslot1
      255, 31, // topslot0
      223, 31, // ftd3
      191, 31, // ftd2
      159, 31, // ftd1
      127, 31, // ftd0
      95, 31, // itd2
      63, 31, // itd1
      31, 31, // itd0
    },

    {
      //12
      623, 11, // rvc
      611, 15, // cnttrg8
      595, 2,  // tttmdl
      592, 0,  // tdsrcp
      591, 0,  // tdtopp
      590, 0,  // tdeclp
      589, 0,  // tdcdcp
      588, 13, // toptiming
      573, 13, // ecltiming
      558, 13, // cdctiming

      527, 15, // itd4
      511, 31, // itd3
      479, 31, // itd2
      447, 31, // itd1
      415, 31, // itd0
      383, 31, // topslot1
      351, 31, // ftd4
      319, 31, // ftd3
      287, 31, // ftd2
      255, 31, // ftd1

      223, 31, // ftd0
      191, 31, // topslot0
      159, 31, // psn4
      127, 31, // psn3
      95, 31, // psn2
      63, 31, // psn1
      31, 31, // psn0
    },

    {
      //13. same with 12.
      623, 11, // rvc
      611, 15, // cnttrg8
      595, 2,  // tttmdl
      592, 0,  // tdsrcp
      591, 0,  // tdtopp
      590, 0,  // tdeclp
      589, 0,  // tdcdcp
      588, 13, // toptiming
      573, 13, // ecltiming
      558, 13, // cdctiming

      543, 31, // itd4
      511, 31, // itd3
      479, 31, // itd2
      447, 31, // itd1
      415, 31, // itd0
      383, 31, // topslot1
      351, 31, // ftd4
      319, 31, // ftd3
      287, 31, // ftd2
      255, 31, // ftd1

      223, 31, // ftd0
      191, 31, // topslot0
      159, 31, // psn4
      127, 31, // psn3
      95, 31, // psn2
      63, 31, // psn1
      31, 31, // psn0
    }


  };

  int m_nword_header[N_UNPACKER_ARRAY] {
    3, 3, 4, 6, 6,
    6, 6, 6, 6, 6,
    6, 6, 6, 6, 6
  };
  int m_conf[N_UNPACKER_ARRAY] {
    0, 0, 1, 2, 3,
    4, 5, 6, 7, 6,
    7, 7, 8, 9, 10
  };
  int m_nword_input[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3,
    3, 3, 5, 5, 5,
    5, 5, 5, 5, 5
  };
  int m_nword_output[N_UNPACKER_ARRAY] {
    3, 3, 3, 3, 3,
    3, 3, 3, 3, 3,
    3, 4, 4, 5, 5
  };

  const int BitMap_extra[N_UNPACKER_ARRAY][N_LEAF][3] = {
    {
      //-1
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      2,  0, 12, //coml1rvc
      2, 12,  9, //b2ldly
      2, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },


    {
      //0
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      2,  0, 12, //coml1rvc
      2, 12,  9, //b2ldly
      2, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //1
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3,  0,  9, //gdll1rvc
      3,  1, 12, //coml1rvc
      3, 12,  9, //b2ldly
      3, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //2
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //3
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //4
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //5
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //6
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //7
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //8
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //9
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //10
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      5, 12,  9, //b2ldly
      5, 21, 11, //maxrvc
      -1, -1, -1  //conf
    },

    {
      //11
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      -1, -1, -1, //b2ldly
      -1, -1, -1, //maxrvc
      -1, -1, -1, //conf
      2, 29,  3, //timtype
      5, 12, 20  //cnttrg
    },

    {
      //12
      -1, -1, -1, //evt
      -1, -1, -1, //clk
      0, -1, -1, //firmid
      1, -1, -1, //firmver
      3, 11, 11, //drvc
      2,  0, 11, //finalrvc
      3,  0, 11, //gdll1rvc
      5,  0, 12, //coml1rvc
      -1, -1, -1, //b2ldly
      -1, -1, -1, //maxrvc
      -1, -1, -1, //conf
      2, 29,  3, //timtype
      -1, -1, -1, //cnttrg
      4, 11, 11  //etmdata
    },

    {
      //13
      -1, -1, -1, //evt,27
      -1, -1, -1, //clk,28
      0, -1, -1, //firmid,29
      1, -1, -1, //firmver,30
      3, 11, 11, //drvc,31
      2,  0, 11, //finalrvc,32
      3,  0, 11, //gdll1rvc,33
      5,  0, 12,//coml1rvc,34
      -1, -1, -1, //conf,35
      2, 29,  3, //timtype,36
      5, 12, 20, //cnttrg,37
      4,  0, 11, //etmdata,38
      3, 22, 10, //sepagdll1,39
      4, 11, 16, //sepacoml1,40
      4, 27, 5, //gdll1_rev,41
      2, 14, 15//rvcout,42
    }

  };


  DBImportObjPtr<TRGGDLDBUnpacker> unpacker;
  unpacker.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_UNPACKER_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);

      unpacker->setnLeafs(nLeafs[i]);
      unpacker->setnLeafsExtra(nLeafsExtra[i]);
      unpacker->setnClks(nClks[i]);
      unpacker->setnBits(nBits[i]);
      unpacker->set_nword_header(m_nword_header[i]);
      unpacker->set_nword_input(m_nword_input[i]);
      unpacker->set_nword_output(m_nword_output[i]);
      unpacker->setconf(m_conf[i]);

      //initialize
      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setLeafName(j, "");
        unpacker->setLeafMap(j, -1);
      }
      //set
      for (int j = 0; j < nrows[i]; j++) {
        if ((inputleafMap[i][j] >= 0) && (inputleafMap[i][j] < nLeafs[i] + nLeafsExtra[i])) {
          unpacker->setLeafName(j, LeafNames[j]);
          unpacker->setLeafMap(j, inputleafMap[i][j]);
        }
      }

      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setBitMap(j, 0, BitMap[i][j][0]);
        unpacker->setBitMap(j, 1, BitMap[i][j][1]);
      }
      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setBitMap_extra(j, 0, BitMap_extra[i][j][0]);
        unpacker->setBitMap_extra(j, 1, BitMap_extra[i][j][1]);
        unpacker->setBitMap_extra(j, 2, BitMap_extra[i][j][2]);
      }
      unpacker.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_UNPACKER_ARRAY - 1; i < N_UNPACKER_ARRAY; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);

      unpacker->setnLeafs(nLeafs[i]);
      unpacker->setnLeafsExtra(nLeafsExtra[i]);
      unpacker->setnClks(nClks[i]);
      unpacker->setnBits(nBits[i]);
      unpacker->set_nword_header(m_nword_header[i]);
      unpacker->set_nword_input(m_nword_input[i]);
      unpacker->set_nword_output(m_nword_output[i]);
      unpacker->setconf(m_conf[i]);

      //initialize
      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setLeafName(j, "");
        unpacker->setLeafMap(j, -1);
      }
      //set
      for (int j = 0; j < nrows[i]; j++) {
        if ((inputleafMap[i][j] >= 0) && (inputleafMap[i][j] < nLeafs[i] + nLeafsExtra[i])) {
          unpacker->setLeafName(j, LeafNames[j]);
          unpacker->setLeafMap(j, inputleafMap[i][j]);
        }
      }

      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setBitMap(j, 0, BitMap[i][j][0]);
        unpacker->setBitMap(j, 1, BitMap[i][j][1]);
      }
      for (int j = 0; j < N_LEAF; j++) {
        unpacker->setBitMap_extra(j, 0, BitMap_extra[i][j][0]);
        unpacker->setBitMap_extra(j, 1, BitMap_extra[i][j][1]);
        unpacker->setBitMap_extra(j, 2, BitMap_extra[i][j][2]);
      }
      unpacker.import(iov);
    }
  }

}

void setdelay()
{

  const int N_DELAY_ARRAY = 32;

  const int run[N_DELAY_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0,    0, -1,  -1,
    //0,    0, 3,   63,
    //0,  108, 3,  480,
    0,    0, 3,  480,
    3,  529, 3,  816,
    3,  932, 3, 1162,
    3, 1176, 3, 1562,
    3, 1596, 3, 2118,
    3, 2119, 3, 2119,
    3, 2120, 3, 2288,
    3, 2292, 3, 2392,
    3, 2401, 3, 2586,
    3, 2587, 3, 2587,
    3, 2588, 3, 3127,
    3, 3128, 3, 3128,
    3, 3129, 3, 3414,
    3, 3415, 3, 3421,
    3, 3422, 3, 3526,
    3, 3527, 3, 3530,
    3, 3531, 3, 3790,
    3, 3822, 3, 4068,
    3, 4069, 3, 4072,
    3, 4073, 3, 4790,
    3, 4791, 3, 4897,
    3, 4898, 3, 5187,
    3, 5188, 3, 5188,
    3, 5189, 3, 5226,
    3, 5227, 3, 5255,
    3, 5314, 3, 5326,
    3, 5341, 3, 5556,
    3, 5557, 3, 5592,
    3, 5594, 3, 5869,
    3, 5870, 3, 5959,
    3, 5975, 4,   0
  };

  const int data_num[N_DELAY_ARRAY] = {
    -1,
    //29,
    //30,
    30,
    31,
    32,
    33,
    34,
    -1,
    34,
    35,
    36,
    -1,
    36,
    -1,
    36,
    -1,
    36,
    -1,
    36,
    37,
    -1,
    37,
    -1,
    37,
    -1,
    37,
    38,
    -1,
    39,
    40,
    41,
    42,
    41
  };


  DBImportObjPtr<TRGGDLDBDelay> delay;
  delay.construct();
  char logname[2000];
  if (ONLINE == 0) {
    for (int i = 0; i < N_DELAY_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      for (int j = 0; j < 320; j++) {
        delay->setdelay(j, 0);
      }
      sprintf(logname, "trg/gdl/dbobjects/log/itd_%04d.dat", data_num[i]);
      std::ifstream ifs(logname);
      if (!ifs) {
        std::cout << "No logfile to read delay values" << " " << logname << std::endl;
      }
      int k = 0;
      int delay_temp = 0;
      while (ifs >> k >> delay_temp) {
        delay->setdelay(k, delay_temp);
      }
      delay.import(iov);
      ifs.close();
    }
  } else if (ONLINE == 1) {
    //for (int i = N_DELAY_ARRAY-1; i < N_DELAY_ARRAY; i++) {
    for (int i = 0; i < 1; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      for (int j = 0; j < 320; j++) {
        delay->setdelay(j, 0);
      }
      sprintf(logname, "trg/gdl/dbobjects/log/itd_%04d.dat", data_num[i]);
      std::ifstream ifs(logname);
      if (!ifs) {
        std::cout << "No logfile to read delay values" << " " << logname << std::endl;
      }
      int k = 0;
      int delay_temp = 0;
      while (ifs >> k >> delay_temp) {
        delay->setdelay(k, delay_temp);
      }
      delay.import(iov);
      ifs.close();
    }
  }

}

void setbadrun()
{

  const int N_BADRUN_ARRAY = 13;

  const int run[N_BADRUN_ARRAY][4] = { //itnitial exp, initial run, end exp, end run
    0, 0,   -1,   -1, // 0
    0, 0,    3, 3526, // 1
    3, 3527, 3, 3623, //-1
    3, 3624, 3, 3769, // 1
    3, 3770, 3, 3790, //-1
    3, 3791, 3, 4877, // 1
    3, 4878, 3, 5012, //-1
    3, 5013, 3, 5019, // 1
    3, 5020, 3, 5246, //-1
    3, 5247, 3, 5313, // 1
    3, 5314, 3, 5326, //-1
    3, 5327, 3, 6000, // 1
    3, 6001, 10,   0  // 0
  };

  int flag[N_BADRUN_ARRAY] = {
    0, 1, -1, 1, -1,
    1, -1, 1, -1,  1,
    -1, 1, 0
  };

  DBImportObjPtr<TRGGDLDBBadrun> badrun;
  badrun.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_BADRUN_ARRAY; i++) {
      IntervalOfValidity iov(run[i][0], run[i][1], run[i][2], run[i][3]);
      badrun->setflag(flag[i]);
      badrun.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_BADRUN_ARRAY - 1; i < N_BADRUN_ARRAY; i++) {
      IntervalOfValidity iov(0, 0, -1, -1);
      badrun->setflag(flag[i]);
      badrun.import(iov);
    }
  }
}


void setalg()
{

  char logname[2000];
  DBImportObjPtr<TRGGDLDBAlgs> db_algs;
  db_algs.construct();
  if (ONLINE == 0) {
    for (int i = 0; i < N_FTD_VERS; i++) {
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.alg", ftd_version[i]);
      db_algs->clear();
      IntervalOfValidity iov(ftd_run[i][0], ftd_run[i][1], ftd_run[i][2], ftd_run[i][3]);
      std::string str;
      std::ifstream isload(logname, std::ios::in);
      while (std::getline(isload, str)) {
        db_algs->setalg(str);
        printf("i(%d), run[i][0](%d), run[i][1](%d), run[i][2](%d), run[i][3](%d), getnalgs(%d), str=%s\n",
               i, ftd_run[i][0], ftd_run[i][1], ftd_run[i][2], ftd_run[i][3],
               db_algs->getnalgs(), str.c_str());
      }
      isload.close();
      db_algs.import(iov);
    }
  } else if (ONLINE == 1) {
    for (int i = N_FTD_VERS - 1; i < N_FTD_VERS; i++) {
      sprintf(logname, "trg/gdl/dbobjects/log/ftd_%04d.alg", ftd_version[i]);
      db_algs->clear();
      IntervalOfValidity iov(0, 0, -1, -1);
      std::string str;
      std::ifstream isload(logname, std::ios::in);
      while (std::getline(isload, str)) {
        db_algs->setalg(str);
        printf("i(%d), run[i][0](%d), run[i][1](%d), run[i][2](%d), run[i][3](%d), getnalgs(%d), str=%s\n",
               i, ftd_run[i][0], ftd_run[i][1], ftd_run[i][2], ftd_run[i][3],
               db_algs->getnalgs(), str.c_str());
      }
      isload.close();
      db_algs.import(iov);
    }
  }
}


int main()
{

  setprescale();
  setftdlbits();
  setinputbits();
  setunpacker();
  //setdelay();
  //setbadrun();
  setalg();


}


