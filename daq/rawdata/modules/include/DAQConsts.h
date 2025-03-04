/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DAQCONSTS_H
#define DAQCONSTS_H

#define REDUCED_RAWCOPPER

//#define NONSTOP
//#define NONSTOP_DEBUG
//#define NONSTOP_SLC

//#define RAWDATASIZE 1
/* #define WORD_SENDHEADER 1 */
/* #define WORD_SENDTRAILER 1 */
/* #define WORD_RAWCPRHEADER 16 */


#define BASE_PORT_ROPC_COPPER 30000
#define NUM_EVT_PER_BASF2LOOP_COPPER 50
#define NUM_EVT_PER_BASF2LOOP_PC 1
#define NUM_PREALLOC_BUF 500

// global variables for non-stop DAQ



#ifdef NONSTOP
extern int g_run_pause;
extern int g_run_error;
extern int g_run_resuming;
#endif

const int sp8test_run_starttime[93] = {
  -1, 1381029053
  , 1381029116
  , 1381029212
  , 1381029294
  , 1381029461
  , 1381029515
  , 1381029585
  , 1381029634
  , 1381029779
  , 1381029884
  , 1381029967
  , 1381030359
  , 1381031134
  , 1381031221
  , 1381031325
  , 1381031418
  , 1381031554
  , 1381031621
  , 1381031836
  , 1381034808
  , 1381034941
  , 1381035044
  , 1381035354
  , 1381036072
  , 1381037030
  , 1381037146
  , 1381037345
  , 1381037638
  , 1381038699
  , 1381039391
  , 1381039816
  , 1381040036
  , 1381041389
  , 1381041442
  , 1381041497
  , 1381041762
  , 1381042464
  , 1381043331
  , 1381046109
  , 1381046342
  , 1381046390
  , 1381046571
  , 1381046589
  , 1381049032
  , 1381049196
  , 1381049867
  , 1381049925
  , 1381050883
  , 1381054508
  , 1381054875
  , 1381055316
  , 1381055550
  , 1381055772
  , 1381056812
  , 1381057213
  , 1381057307
  , 1381058937
  , 1381059752
  , 1381060015
  , 1381060050
  , 1381060524
  , 1381061531
  , 1381061572
  , 1381061648
  , 1381061749
  , 1381062569
  , 1381062687
  , 1381063320
  , 1381063720
  , 1381064134
  , 1381067028
  , 1381067698
  , 1381069387
  , 1381070333
  , 1381071744
  , 1381071838
  , 1381072694
  , 1381073760
  , 1381073790
  , 1381074369
  , 1381074621
  , 1381074946
  , 1381075404
  , 1381075516
  , 1381077354
  , 1381077382
  , 1381078479
  , 1381078693
  , 1381079579
  , 1381079710
  , 1381079865
  , 1381079906
};

//#define WITH_HEADER_TRAILER

//#define DUMMY

#endif
