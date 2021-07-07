/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
/** \file
 *    Generated by: ../../../genfft/gen_r2r.native -compact -variables 4 -pipeline-latency 4 -redft10 -n 31 -name e10_31 -include rdft/scalar/r2r.h
 *    Generated by: ../../../genfft/gen_r2r.native -compact -variables 4 -pipeline-latency 4 -redft01 -n 31 -name e01_31 -include rdft/scalar/r2r.h
 *
 *
 *    This file was autogenerated by the FFTW package explicitly for 31-point DCT transformation.
 *    It is optimized version of the following functions:
 *
 *    DCT-II or "the" DCT transformation of 31-point signal
 *    void e10_31(const double *A, double *C){
 *      for(int i=0;i<31;i++){
 *        double c = 0; for(int j=0;j<31;j++) c += A[j]*cos(M_PI/31*(j+0.5)*i);
 *        C[i] = 2*c;
 *      }
 *    }
 *
 *    and DCT-III or "the inverse" DCT transformation of 31-point signal
 *    void e01_31(const double *C, double *A){
 *      for(int i=0;i<31;i++){
 *        double a = 0.5*C[0]; for(int j=1;j<31;j++) a += C[j]*cos(M_PI/31*j*(i+0.5));
 *        A[i] = 2*a;
 *      }
 *    }
 *
 *    Those functions and the functions defined below give exactly the same results (up to floating point rounding errors).
 */

typedef double R;
typedef R E;  /* internal precision of codelets. */
#define K(x) ((E) x)
#define DK(name, value) const E name = K(value)
#define WS(x,y) (y)
#define FMA(a, b, c) (((a) * (b)) + (c))
#define FMS(a, b, c) (((a) * (b)) - (c))
#define FNMA(a, b, c) (- (((a) * (b)) + (c)))
#define FNMS(a, b, c) ((c) - ((a) * (b)))

extern "C" {
  void e10_31(const R* I, R* O);
  void e01_31(const R* I, R* O);
}

namespace {
  DK(KP000412259, +0.000412259418562871938380998445334699821046170);
  DK(KP015708004, +0.015708004810545652602792720509151343637341044);
  DK(KP019941366, +0.019941366459822654495429853833981662789297058);
  DK(KP025400502, +0.025400502734294785428452465754780624947550210);
  DK(KP028866483, +0.028866483847295741954970658534562655092360970);
  DK(KP029606561, +0.029606561198652297994480542192674295188153583);
  DK(KP045346848, +0.045346848173899962231923625268893718434894172);
  DK(KP066666666, +0.066666666666666666666666666666666666666666667);
  DK(KP092681288, +0.092681288904379450142256318609598804525206067);
  DK(KP102097497, +0.102097497864916063688242067516611448492966715);
  DK(KP112172063, +0.112172063906358903891072106654229302317378058);
  DK(KP122761339, +0.122761339421712417807572754092970003891850465);
  DK(KP127938670, +0.127938670558678996799573548799714027347967121);
  DK(KP147857608, +0.147857608946689579852313890437569859447378619);
  DK(KP155909426, +0.155909426230360388401557646847789940246255225);
  DK(KP160793728, +0.160793728520323189459149287981372086275142541);
  DK(KP183215435, +0.183215435972067868363105533577134775661644325);
  DK(KP183333495, +0.183333495452244782819904055070309212901710558);
  DK(KP183845747, +0.183845747585549357937166766576821269206738322);
  DK(KP184517712, +0.184517712830393344154095734604975602001386285);
  DK(KP184926209, +0.184926209687313710109434775837815115985225567);
  DK(KP185591687, +0.185591687547196603013206497513733070197662098);
  DK(KP202100941, +0.202100941504002851338890151760897670549611107);
  DK(KP213702830, +0.213702830714905671421951896566570134065492948);
  DK(KP245522678, +0.245522678843424835615145508185940007783700930);
  DK(KP250000000, +0.250000000000000000000000000000000000000000000);
  DK(KP251026872, +0.251026872929094175322677333303375485053014277);
  DK(KP255877341, +0.255877341117357993599147097599428054695934242);
  DK(KP258006924, +0.258006924095276452089799714364401388739221940);
  DK(KP293892626, +0.293892626146236564584352977319536384298826219);
  DK(KP296373721, +0.296373721102994137554600958572269203487908448);
  DK(KP303494444, +0.303494444631551941253967923387361806243372364);
  DK(KP311340628, +0.311340628927503445870467381445371310537082980);
  DK(KP341720569, +0.341720569276894099786524583841162921922802620);
  DK(KP348438623, +0.348438623509873804361149807347644092563702804);
  DK(KP350296205, +0.350296205119560058350720150718018638663792697);
  DK(KP360104421, +0.360104421960192515778041781881012837232188655);
  DK(KP371184290, +0.371184290855334794807964753261236634698426225);
  DK(KP387067417, +0.387067417450794062018448209110056640357696792);
  DK(KP404201883, +0.404201883008005702677780303521795341099222215);
  DK(KP427405661, +0.427405661429811342843903793133140268130985897);
  DK(KP433012701, +0.433012701892219323381861585376468091735701313);
  DK(KP462201919, +0.462201919825108579466283849397624285725155370);
  DK(KP475528258, +0.475528258147576786058219666689691071702849317);
  DK(KP500000000, +0.500000000000000000000000000000000000000000000);
  DK(KP559016994, +0.559016994374947424102293417182819058860154590);
  DK(KP587785252, +0.587785252292473129168705954639072768597652438);
  DK(KP606988889, +0.606988889263103882507935846774723612486744729);
  DK(KP618111346, +0.618111346055468967867841496245414225971410594);
  DK(KP622681257, +0.622681257855006891740934762890742621074165960);
  DK(KP638094290, +0.638094290379888237341125542413432125410711069);
  DK(KP696877247, +0.696877247019747608722299614695288185127405608);
  DK(KP700592410, +0.700592410239120116701440301436037277327585395);
  DK(KP951056516, +0.951056516295153572116439333379382143405698634);//cos(pi()/10)
  DK(KP968245836, +0.968245836551854221294816349945599902708230426);//sqrt(5)*sqrt(3)/2
  DK(KP1_018073920, +1.018073920910254366901961726787815297021466329);//sin(pi()/5)*sqrt(3)
  DK(KP1_118033988, +1.118033988749894848204586834365638117720309180);//sqrt(5)/2
  DK(KP1_175570504, +1.175570504584946258337411909278145537195304875);//2*sin(pi()/5)
  DK(KP1_647278207, +1.647278207092663851754840078556380006059321028);//cos(pi()*1/15)+cos(pi()*4/15)=2*cos(pi()/6)*cos(pi()/10)
  DK(KP1_732050807, +1.732050807568877293527446341505872366942805254);// sqrt(3)
  DK(KP1_902113032, +1.902113032590307144232878666758764286811397268); // 2*cos(pi/10)
  DK(KP2_000000000, +2.000000000000000000000000000000000000000000000);
  DK(KP3_464101615, +3.464101615137754587054892683011744733885610508); // 2*sqrt(3)
  DK(KP4_000000000, +4.000000000000000000000000000000000000000000000);
}

/** DCT-II or "the" DCT transformation of 31-point signal
 * This function contains 320 FP additions, 170 FP multiplications,
 * (or, 229 additions, 79 multiplications, 91 fused multiply/add),
 * 150 stack variables, 64 constants, and 62 memory accesses
 * @param I input signal array of 31 values
 * @param O output array with 31 DCT coefficients
 */
void e10_31(const R* I, R* O)
{
  E T2J, T1H, T1J, T1I, T1K, T1T, T2S, T3a, T31, T3b, T3M, T1M, T1P, T1N, T1Q;
  E T1i, T1l, T1A, T1D, T1b, T1k, T1v, T1C, T3n, T3I, T3y, T3D, T3S, T3V, T43;
  E T46, T3P, T3U, T40, T45;
  T2J = I[WS(is, 15)];
  {
    E TN, T2K, TW, T32, T16, T2T, T1d, T1e, T3e, T3d, T2R, T3p, TK, TO, Tu;
    E T1p, T3i, T3F, T12, T17, T30, T3s, Tf, T1o, T3l, T3G, TT, TX, T39, T3v;
    E TL, TM;
    TL = I[WS(is, 17)];
    TM = I[WS(is, 13)];
    TN = TL - TM;
    T2K = TL + TM;
    {
      E TU, TV, T14, T15;
      TU = I[WS(is, 25)];
      TV = I[WS(is, 5)];
      TW = TU - TV;
      T32 = TU + TV;
      T14 = I[WS(is, 3)];
      T15 = I[WS(is, 27)];
      T16 = T14 - T15;
      T2T = T14 + T15;
    }
    {
      E Ty, T2L, TI, T2P, TB, T2M, TF, T2O;
      {
        E Tw, Tx, TG, TH;
        Tw = I[WS(is, 14)];
        Tx = I[WS(is, 16)];
        Ty = Tw - Tx;
        T2L = Tw + Tx;
        TG = I[WS(is, 23)];
        TH = I[WS(is, 7)];
        TI = TG - TH;
        T2P = TG + TH;
      }
      {
        E Tz, TA, TD, TE;
        Tz = I[WS(is, 19)];
        TA = I[WS(is, 11)];
        TB = Tz - TA;
        T2M = Tz + TA;
        TD = I[WS(is, 30)];
        TE = I[0];
        TF = TD - TE;
        T2O = TD + TE;
      }
      T1d = Ty - TB;
      T1e = TF - TI;
      T3e = T2O - T2P;
      T3d = T2M - T2L;
      {
        E T2N, T2Q, TC, TJ;
        T2N = T2L + T2M;
        T2Q = T2O + T2P;
        T2R = T2N + T2Q;
        T3p = KP559016994 * (T2Q - T2N);
        TC = Ty + TB;
        TJ = TF + TI;
        TK = KP559016994 * (TC - TJ);
        TO = TC + TJ;
      }
    }
    {
      E Ti, T2U, Ts, T2Y, Tl, T2V, Tp, T2X;
      {
        E Tg, Th, Tq, Tr;
        Tg = I[WS(is, 9)];
        Th = I[WS(is, 21)];
        Ti = Tg - Th;
        T2U = Tg + Th;
        Tq = I[WS(is, 29)];
        Tr = I[WS(is, 1)];
        Ts = Tq - Tr;
        T2Y = Tq + Tr;
      }
      {
        E Tj, Tk, Tn, To;
        Tj = I[WS(is, 8)];
        Tk = I[WS(is, 22)];
        Tl = Tj - Tk;
        T2V = Tj + Tk;
        Tn = I[WS(is, 18)];
        To = I[WS(is, 12)];
        Tp = Tn - To;
        T2X = Tn + To;
      }
      {
        E Tm, Tt, T3g, T3h;
        Tm = Ti - Tl;
        Tt = Tp - Ts;
        Tu = FMA(KP475528258, Tm, KP293892626 * Tt);
        T1p = FNMS(KP475528258, Tt, KP293892626 * Tm);
        T3g = T2V - T2U;
        T3h = T2X - T2Y;
        T3i = FMA(KP293892626, T3g, KP475528258 * T3h);
        T3F = FNMS(KP293892626, T3h, KP475528258 * T3g);
      }
      {
        E T10, T11, T2W, T2Z;
        T10 = Ti + Tl;
        T11 = Tp + Ts;
        T12 = T10 - T11;
        T17 = T10 + T11;
        T2W = T2U + T2V;
        T2Z = T2X + T2Y;
        T30 = T2W + T2Z;
        T3s = KP559016994 * (T2Z - T2W);
      }
    }
    {
      E T3, T33, Td, T37, T6, T34, Ta, T36;
      {
        E T1, T2, Tb, Tc;
        T1 = I[WS(is, 10)];
        T2 = I[WS(is, 20)];
        T3 = T1 - T2;
        T33 = T1 + T2;
        Tb = I[WS(is, 6)];
        Tc = I[WS(is, 24)];
        Td = Tb - Tc;
        T37 = Tb + Tc;
      }
      {
        E T4, T5, T8, T9;
        T4 = I[WS(is, 26)];
        T5 = I[WS(is, 4)];
        T6 = T4 - T5;
        T34 = T4 + T5;
        T8 = I[WS(is, 28)];
        T9 = I[WS(is, 2)];
        Ta = T8 - T9;
        T36 = T8 + T9;
      }
      {
        E T7, Te, T3j, T3k;
        T7 = T3 - T6;
        Te = Ta - Td;
        Tf = FMA(KP475528258, T7, KP293892626 * Te);
        T1o = FNMS(KP475528258, Te, KP293892626 * T7);
        T3j = T34 - T33;
        T3k = T36 - T37;
        T3l = FMA(KP293892626, T3j, KP475528258 * T3k);
        T3G = FNMS(KP293892626, T3k, KP475528258 * T3j);
      }
      {
        E TR, TS, T35, T38;
        TR = T3 + T6;
        TS = Ta + Td;
        TT = KP559016994 * (TR - TS);
        TX = TR + TS;
        T35 = T33 + T34;
        T38 = T36 + T37;
        T39 = T35 + T38;
        T3v = T38 - T35;
      }
    }
    T1H = TN + TO;
    T1J = T16 + T17;
    T1I = TW + TX;
    T1K = FMA(KP258006924, T1H, KP102097497 * T1I) - (KP360104421 * T1J);
    T1T = KP371184290 * (T1H + T1J + T1I);
    T2S = T2K + T2R;
    T3a = T32 + T39;
    T31 = T2T + T30;
    T3b = T2S + T31 + T3a;
    T3M = FMA(KP045346848, T31, KP296373721 * T3a) - (KP341720569 * T2S);
    {
      E Tv, T1q, TQ, T1r, TZ, T1s, T1c, T1f, T1g, T19, T1t, T1w, T1x, T1y;
      {
        E TP, TY, T13, T18;
        Tv = KP3_464101615 * (Tf - Tu);
        T1q = KP3_464101615 * (T1o - T1p);
        TP = FNMS(KP250000000, TO, TN);
        TQ = TK + TP;
        T1r = TP - TK;
        TY = FNMS(KP250000000, TX, TW);
        TZ = TT + TY;
        T1s = TY - TT;
        T1c = Tu + Tf;
        T1f = FMA(KP475528258, T1d, KP293892626 * T1e);
        T1g = FNMS(KP4_000000000, T1f, KP2_000000000 * T1c);
        T13 = KP559016994 * T12;
        T18 = FNMS(KP250000000, T17, T16);
        T19 = T13 + T18;
        T1t = T18 - T13;
        T1w = T1p + T1o;
        T1x = FNMS(KP475528258, T1e, KP293892626 * T1d);
        T1y = FNMS(KP4_000000000, T1x, KP2_000000000 * T1w);
      }
      T1M = T1f + T1c;
      T1P = T1x + T1w;
      T1N = TQ + T19 + TZ;
      T1Q = T1r + T1t + T1s;
      {
        E T1h, T1z, T1a, T1u;
        T1h = KP1_732050807 * (TZ - T19);
        T1i = T1g + T1h;
        T1l = T1h - T1g;
        T1z = KP1_732050807 * (T1s - T1t);
        T1A = T1y + T1z;
        T1D = T1z - T1y;
        T1a = FMS(KP2_000000000, TQ, TZ) - T19;
        T1b = Tv + T1a;
        T1k = T1a - Tv;
        T1u = FMS(KP2_000000000, T1r, T1s) - T1t;
        T1v = T1q + T1u;
        T1C = T1u - T1q;
      }
    }
    {
      E T3O, T3Z, T3q, T3A, T3t, T3B, T3x, T3C, T3f, T3m, T3R, T3E, T3H, T42;
      {
        E T3o, T3r, T3w, T3u;
        T3O = KP3_464101615 * (T3l - T3i);
        T3Z = KP3_464101615 * (T3G - T3F);
        T3o = FMS(KP250000000, T2R, T2K);
        T3q = T3o - T3p;
        T3A = T3o + T3p;
        T3r = FMS(KP250000000, T30, T2T);
        T3t = T3r - T3s;
        T3B = T3r + T3s;
        T3w = KP559016994 * T3v;
        T3u = FMS(KP250000000, T39, T32);
        T3x = T3u - T3w;
        T3C = T3u + T3w;
        T3f = FMA(KP293892626, T3d, KP475528258 * T3e);
        T3m = T3i + T3l;
        T3R = FNMS(KP2_000000000, T3m, KP4_000000000 * T3f);
        T3E = FNMS(KP293892626, T3e, KP475528258 * T3d);
        T3H = T3F + T3G;
        T42 = FNMS(KP2_000000000, T3H, KP4_000000000 * T3E);
      }
      T3n = T3f + T3m;
      T3I = T3E + T3H;
      T3y = T3q + T3t + T3x;
      T3D = T3A + T3B + T3C;
      {
        E T3Q, T41, T3N, T3Y;
        T3Q = KP1_732050807 * (T3t - T3x);
        T3S = T3Q - T3R;
        T3V = T3R + T3Q;
        T41 = KP1_732050807 * (T3B - T3C);
        T43 = T41 - T42;
        T46 = T42 + T41;
        T3N = FMS(KP2_000000000, T3q, T3x) - T3t;
        T3P = T3N - T3O;
        T3U = T3O + T3N;
        T3Y = FMS(KP2_000000000, T3A, T3C) - T3B;
        T40 = T3Y - T3Z;
        T45 = T3Z + T3Y;
      }
    }
  }
  O[0] = KP2_000000000 * (T2J + T3b);
  {
    E T2l, T2B, T1U, T2o, T2C, T2a, T2w, T2c, T1G, T2d, T23, T25, T2y, T2h, T22;
    E T26, T2j, T2k, T24, T1L, T1V;
    T2j = FNMS(KP700592410, T1M, KP122761339 * T1N);
    T2k = FMA(KP404201883, T1P, KP311340628 * T1Q);
    T2l = FMA(KP1_902113032, T2j, KP1_175570504 * T2k);
    T2B = FNMS(KP1_902113032, T2k, KP1_175570504 * T2j);
    {
      E T1S, T2m, T1O, T1R, T2n;
      T1O = FMA(KP245522678, T1M, KP350296205 * T1N);
      T1R = FNMS(KP202100941, T1Q, KP622681257 * T1P);
      T1S = T1O + T1R;
      T2m = KP1_118033988 * (T1R - T1O);
      T1U = FMS(KP2_000000000, T1S, T1T);
      T2n = FMA(KP500000000, T1S, T1T);
      T2o = T2m + T2n;
      T2C = T2n - T2m;
    }
    {
      E T1n, T29, T1F, T28;
      {
        E T1j, T1m, T1B, T1E;
        T1j = FNMS(KP184517712, T1i, KP019941366 * T1b);
        T1m = FNMS(KP183845747, T1l, KP025400502 * T1k);
        T1n = T1j + T1m;
        T29 = T1m - T1j;
        T1B = FMA(KP184926209, T1v, KP015708004 * T1A);
        T1E = FMA(KP183215435, T1C, KP029606561 * T1D);
        T1F = T1B - T1E;
        T28 = T1B + T1E;
      }
      T2a = FNMS(KP1_647278207, T29, KP1_018073920 * T28);
      T2w = FMA(KP1_018073920, T29, KP1_647278207 * T28);
      T2c = KP559016994 * (T1F - T1n);
      T1G = T1n + T1F;
      T2d = FMA(KP250000000, T1G, T1K);
    }
    {
      E T21, T2g, T1Y, T2f;
      T23 = FMA(KP462201919, T1H, KP155909426 * T1J) - (KP618111346 * T1I);
      {
        E T1Z, T20, T1W, T1X;
        T1Z = FNMS(KP015708004, T1v, KP184926209 * T1A);
        T20 = FNMS(KP183215435, T1D, KP029606561 * T1C);
        T21 = T1Z + T20;
        T2g = T1Z - T20;
        T1W = FMA(KP184517712, T1b, KP019941366 * T1i);
        T1X = FMA(KP183845747, T1k, KP025400502 * T1l);
        T1Y = T1W + T1X;
        T2f = T1W - T1X;
      }
      T25 = KP968245836 * (T1Y - T21);
      T2y = FNMS(KP587785252, T2f, KP951056516 * T2g);
      T2h = FMA(KP951056516, T2f, KP587785252 * T2g);
      T22 = T1Y + T21;
      T26 = FNMS(KP433012701, T22, T23);
    }
    T24 = FMA(KP1_732050807, T22, T23);
    T1L = T1G - T1K;
    T1V = T1U - T1L;
    O[WS(os, 1)] = FMA(KP2_000000000, T1L, T1U);
    O[WS(os, 25)] = T24 + T1V;
    O[WS(os, 5)] = T1V - T24;
    {
      E T2D, T2G, T2x, T2H, T2A, T2F, T2v, T2z, T2E, T2I;
      T2D = T2B - T2C;
      T2G = T2B + T2C;
      T2v = T25 - T26;
      T2x = T2v - T2w;
      T2H = T2w + T2v;
      T2z = T2d - T2c;
      T2A = T2y + T2z;
      T2F = T2y - T2z;
      O[WS(os, 23)] = FNMS(KP2_000000000, T2A, T2D);
      O[WS(os, 27)] = FMS(KP2_000000000, T2F, T2G);
      T2E = T2A + T2D;
      O[WS(os, 17)] = T2x - T2E;
      O[WS(os, 9)] = T2x + T2E;
      T2I = T2F + T2G;
      O[WS(os, 11)] = T2H - T2I;
      O[WS(os, 7)] = T2H + T2I;
    }
    {
      E T2p, T2r, T2b, T2u, T2i, T2s, T27, T2e, T2q, T2t;
      T2p = T2l - T2o;
      T2r = T2l + T2o;
      T27 = T25 + T26;
      T2b = T27 - T2a;
      T2u = T2a + T27;
      T2e = T2c + T2d;
      T2i = T2e - T2h;
      T2s = T2h + T2e;
      O[WS(os, 15)] = FNMS(KP2_000000000, T2i, T2p);
      O[WS(os, 29)] = FMA(KP2_000000000, T2s, T2r);
      T2q = T2i + T2p;
      O[WS(os, 13)] = T2b - T2q;
      O[WS(os, 3)] = T2b + T2q;
      T2t = T2r - T2s;
      O[WS(os, 19)] = T2t - T2u;
      O[WS(os, 21)] = T2u + T2t;
    }
  }
  {
    E T4x, T4P, T3L, T4u, T4O, T4p, T4T, T4E, T49, T4o, T4c, T4n, T4M, T4A, T4j;
    E T4z, T4v, T4w, T4k, T4a, T4b;
    T4v = FNMS(KP127938670, T3y, KP696877247 * T3n);
    T4w = FMA(KP606988889, T3I, KP213702830 * T3D);
    T4x = FMA(KP1_175570504, T4v, KP1_902113032 * T4w);
    T4P = FNMS(KP1_902113032, T4v, KP1_175570504 * T4w);
    {
      E T3c, T3K, T4t, T3z, T3J, T4s;
      T3c = FNMS(KP2_000000000, T2J, KP066666666 * T3b);
      T3z = FMA(KP255877341, T3n, KP348438623 * T3y);
      T3J = FNMS(KP427405661, T3I, KP303494444 * T3D);
      T3K = T3z + T3J;
      T4t = KP1_118033988 * (T3J - T3z);
      T3L = FMA(KP2_000000000, T3K, T3c);
      T4s = FNMS(KP500000000, T3K, T3c);
      T4u = T4s + T4t;
      T4O = T4t - T4s;
    }
    {
      E T3X, T4C, T48, T4D;
      {
        E T3T, T3W, T44, T47;
        T3T = FMA(KP185591687, T3P, KP000412259 * T3S);
        T3W = FMA(KP112172063, T3U, KP147857608 * T3V);
        T3X = T3T - T3W;
        T4C = T3T + T3W;
        T44 = FMA(KP028866483, T40, KP183333495 * T43);
        T47 = FMA(KP092681288, T45, KP160793728 * T46);
        T48 = T44 - T47;
        T4D = T44 + T47;
      }
      T4p = KP559016994 * (T48 - T3X);
      T4T = FNMS(KP1_647278207, T4C, KP1_018073920 * T4D);
      T4E = FMA(KP1_018073920, T4C, KP1_647278207 * T4D);
      T49 = T3X + T48;
      T4o = FNMS(KP250000000, T49, T3M);
    }
    {
      E T4i, T4m, T4f, T4l;
      T4c = FMA(KP251026872, T2S, KP387067417 * T3a) - (KP638094290 * T31);
      {
        E T4g, T4h, T4d, T4e;
        T4g = FNMS(KP183333495, T40, KP028866483 * T43);
        T4h = FNMS(KP092681288, T46, KP160793728 * T45);
        T4i = T4g + T4h;
        T4m = T4h - T4g;
        T4d = FNMS(KP000412259, T3P, KP185591687 * T3S);
        T4e = FNMS(KP112172063, T3V, KP147857608 * T3U);
        T4f = T4d + T4e;
        T4l = T4e - T4d;
      }
      T4n = FMA(KP587785252, T4l, KP951056516 * T4m);
      T4M = FNMS(KP951056516, T4l, KP587785252 * T4m);
      T4A = KP968245836 * (T4i - T4f);
      T4j = T4f + T4i;
      T4z = FNMS(KP433012701, T4j, T4c);
    }
    T4k = FMA(KP1_732050807, T4j, T4c);
    T4a = T3M + T49;
    T4b = T3L - T4a;
    O[WS(os, 6)] = T4b - T4k;
    O[WS(os, 30)] = FMA(KP2_000000000, T4a, T3L);
    O[WS(os, 26)] = T4k + T4b;
    {
      E T4Q, T4W, T4N, T4V, T4U, T4Y, T4L, T4S, T4R, T4X;
      T4Q = T4O - T4P;
      T4W = T4O + T4P;
      T4L = T4p - T4o;
      T4N = T4L - T4M;
      T4V = T4M + T4L;
      T4S = T4z - T4A;
      T4U = T4S + T4T;
      T4Y = T4S - T4T;
      O[WS(os, 4)] = FMA(KP2_000000000, T4N, T4Q);
      O[WS(os, 8)] = FMA(KP2_000000000, T4V, T4W);
      T4R = T4Q - T4N;
      O[WS(os, 20)] = T4R - T4U;
      O[WS(os, 24)] = T4U + T4R;
      T4X = T4V - T4W;
      O[WS(os, 14)] = T4X - T4Y;
      O[WS(os, 22)] = T4Y + T4X;
    }
    {
      E T4y, T4J, T4r, T4I, T4F, T4H, T4q, T4B, T4G, T4K;
      T4y = T4u + T4x;
      T4J = T4x - T4u;
      T4q = T4o + T4p;
      T4r = T4n + T4q;
      T4I = T4n - T4q;
      T4B = T4z + T4A;
      T4F = T4B + T4E;
      T4H = T4B - T4E;
      O[WS(os, 2)] = FMA(KP2_000000000, T4r, T4y);
      O[WS(os, 16)] = FMA(KP2_000000000, T4I, T4J);
      T4G = T4y - T4r;
      O[WS(os, 12)] = T4F - T4G;
      O[WS(os, 10)] = T4F + T4G;
      T4K = T4I - T4J;
      O[WS(os, 28)] = T4H - T4K;
      O[WS(os, 18)] = T4H + T4K;
    }
  }
}


/** DCT-III or "the inverse" DCT transformation of 31-point signal
 * This function contains 320 FP additions, 169 FP multiplications,
 * (or, 228 additions, 77 multiplications, 92 fused multiply/add),
 * 149 stack variables, 64 constants, and 62 memory accesses
 * @param I input coefficients
 * @param O output signal amplitudes
 */
void e01_31(const R* I, R* O)
{
  E T22, T4l, T2R, T1S, T1W, T1X, T2O, T3t, T2L, T3s, T1M, T1B, T1N, T2D, T3q;
  E T2A, T3p, T1, Tt, Tu, T47, T25, T10, TV, T11, T2q, T3h, T2n, T3i, TQ;
  E TF, TR, T2f, T3e, T2c, T3f;
  {
    E T1c, T1a, T1O, T1L, T1b, T1d, T1f, T1y, T1o, T1w, T1m, T1n, T1x, T1z, T1P;
    E T1Q, T1R, T1E, T1H, T1I, T1Z, T21, T20;
    {
      E T19, T1K, T16, T1J;
      T1c = I[WS(is, 27)];
      {
        E T17, T18, T14, T15;
        T17 = I[WS(is, 1)];
        T18 = I[WS(is, 15)];
        T19 = T17 + T18;
        T1K = T18 - T17;
        T14 = I[WS(is, 29)];
        T15 = I[WS(is, 23)];
        T16 = T14 - T15;
        T1J = T14 + T15;
      }
      T1a = KP559016994 * (T16 + T19);
      T1O = FNMS(KP475528258, T1K, KP293892626 * T1J);
      T1L = FMA(KP475528258, T1J, KP293892626 * T1K);
      T1b = T16 - T19;
      T1d = FMA(KP250000000, T1b, T1c);
    }
    {
      E T1i, T1F, T1v, T1C, T1s, T1D, T1l, T1G;
      {
        E T1g, T1h, T1t, T1u;
        T1f = I[WS(is, 7)];
        T1y = I[WS(is, 11)];
        T1g = I[WS(is, 19)];
        T1h = I[WS(is, 17)];
        T1i = T1g + T1h;
        T1F = T1g - T1h;
        T1t = I[WS(is, 13)];
        T1u = I[WS(is, 5)];
        T1v = T1t - T1u;
        T1C = T1t + T1u;
        {
          E T1q, T1r, T1j, T1k;
          T1q = I[WS(is, 21)];
          T1r = I[WS(is, 9)];
          T1s = T1q - T1r;
          T1D = T1q + T1r;
          T1j = I[WS(is, 25)];
          T1k = I[WS(is, 3)];
          T1l = T1j + T1k;
          T1G = T1k - T1j;
        }
      }
      T1o = KP559016994 * (T1i + T1l);
      T1w = KP559016994 * (T1s - T1v);
      T1m = T1i - T1l;
      T1n = FNMS(KP250000000, T1m, T1f);
      T1x = T1s + T1v;
      T1z = FMA(KP250000000, T1x, T1y);
      T1P = FNMS(KP475528258, T1G, KP293892626 * T1F);
      T1Q = FMA(KP293892626, T1D, KP475528258 * T1C);
      T1R = T1P + T1Q;
      T1E = FNMS(KP475528258, T1D, KP293892626 * T1C);
      T1H = FMA(KP475528258, T1F, KP293892626 * T1G);
      T1I = T1E - T1H;
    }
    T1Z = T1b - T1c;
    T21 = T1x - T1y;
    T20 = T1f + T1m;
    T22 = KP371184290 * (T1Z + T20 + T21);
    T4l = FMA(KP462201919, T1Z, KP155909426 * T20) - (KP618111346 * T21);
    T2R = FMA(KP258006924, T1Z, KP102097497 * T21) - (KP360104421 * T20);
    {
      E T2I, T2F, T2G, T2J, T1T, T1V, T1U;
      T1S = T1O + T1R;
      T2I = KP3_464101615 * (T1Q - T1P);
      T2F = FNMS(KP4_000000000, T1O, KP2_000000000 * T1R);
      T1T = T1n - T1o;
      T1V = T1a + T1d;
      T1U = T1w + T1z;
      T2G = KP1_732050807 * (T1T + T1U);
      T2J = T1U + FNMA(KP2_000000000, T1V, T1T);
      T1W = T1T - T1U - T1V;
      T1X = FNMS(KP202100941, T1W, KP622681257 * T1S);
      {
        E T2M, T2N, T2H, T2K;
        T2M = T2F + T2G;
        T2N = T2J - T2I;
        T2O = FNMS(KP183215435, T2N, KP029606561 * T2M);
        T3t = FMA(KP183215435, T2M, KP029606561 * T2N);
        T2H = T2F - T2G;
        T2K = T2I + T2J;
        T2L = FMA(KP015708004, T2H, KP184926209 * T2K);
        T3s = FNMS(KP015708004, T2K, KP184926209 * T2H);
      }
    }
    {
      E T2y, T2v, T2x, T2u, T1e, T1A, T1p;
      T1M = T1I - T1L;
      T2y = FMA(KP4_000000000, T1L, KP2_000000000 * T1I);
      T2v = KP3_464101615 * (T1H + T1E);
      T1e = T1a - T1d;
      T1A = T1w - T1z;
      T1p = T1n + T1o;
      T2x = KP1_732050807 * (T1A - T1p);
      T2u = FMS(KP2_000000000, T1e, T1A) - T1p;
      T1B = T1e + T1p + T1A;
      T1N = FNMS(KP245522678, T1M, KP350296205 * T1B);
      {
        E T2B, T2C, T2w, T2z;
        T2B = T2v + T2u;
        T2C = T2y + T2x;
        T2D = FNMS(KP183845747, T2C, KP025400502 * T2B);
        T3q = FMA(KP183845747, T2B, KP025400502 * T2C);
        T2w = T2u - T2v;
        T2z = T2x - T2y;
        T2A = FNMS(KP184517712, T2z, KP019941366 * T2w);
        T3p = FMA(KP184517712, T2w, KP019941366 * T2z);
      }
    }
  }
  {
    E T2, Tw, TZ, TI, T9, Tv, Tb, Tk, Tz, TD, Ti, Ty, Tr, TB, TW;
    E TX, TY, TL, TO, TP, Ta, Ts, Tj;
    T1 = I[0];
    {
      E T8, TG, T5, TH;
      T2 = I[WS(is, 4)];
      {
        E T6, T7, T3, T4;
        T6 = I[WS(is, 16)];
        T7 = I[WS(is, 30)];
        T8 = T6 - T7;
        TG = T6 + T7;
        T3 = I[WS(is, 8)];
        T4 = I[WS(is, 2)];
        T5 = T3 - T4;
        TH = T3 + T4;
      }
      Tw = KP559016994 * (T5 - T8);
      TZ = FMA(KP475528258, TH, KP293892626 * TG);
      TI = FNMS(KP293892626, TH, KP475528258 * TG);
      T9 = T5 + T8;
      Tv = FNMS(KP250000000, T9, T2);
    }
    {
      E Te, TJ, Tq, TN, Tn, TM, Th, TK, TC;
      {
        E Tc, Td, To, Tp;
        Tb = I[WS(is, 24)];
        Tk = I[WS(is, 20)];
        Tc = I[WS(is, 12)];
        Td = I[WS(is, 14)];
        Te = Tc - Td;
        TJ = Tc + Td;
        To = I[WS(is, 26)];
        Tp = I[WS(is, 18)];
        Tq = To + Tp;
        TN = Tp - To;
        {
          E Tl, Tm, Tf, Tg;
          Tl = I[WS(is, 10)];
          Tm = I[WS(is, 22)];
          Tn = Tl + Tm;
          TM = Tm - Tl;
          Tf = I[WS(is, 28)];
          Tg = I[WS(is, 6)];
          Th = Tf - Tg;
          TK = Tf + Tg;
        }
      }
      Tz = KP559016994 * (Te - Th);
      TC = Tq - Tn;
      TD = KP559016994 * TC;
      Ti = Te + Th;
      Ty = FNMS(KP250000000, Ti, Tb);
      Tr = Tn + Tq;
      TB = FMA(KP250000000, Tr, Tk);
      TW = FNMS(KP293892626, TK, KP475528258 * TJ);
      TX = FMA(KP475528258, TM, KP293892626 * TN);
      TY = TW + TX;
      TL = FMA(KP293892626, TJ, KP475528258 * TK);
      TO = FNMS(KP475528258, TN, KP293892626 * TM);
      TP = TL + TO;
    }
    Ta = T2 + T9;
    Ts = Tk - Tr;
    Tj = Tb + Ti;
    Tt = Ta + Tj + Ts;
    Tu = FNMS(KP066666666, Tt, T1);
    T47 = FNMS(KP387067417, Ts, KP638094290 * Tj) - (KP251026872 * Ta);
    T25 = FNMS(KP296373721, Ts, KP341720569 * Ta) - (KP045346848 * Tj);
    {
      E T2l, T2i, T2h, T2k, TS, TU, TT;
      T10 = TY - TZ;
      T2l = KP3_464101615 * (TX - TW);
      T2i = FMA(KP4_000000000, TZ, KP2_000000000 * TY);
      TS = Tw + Tv;
      TU = TB + TD;
      TT = Tz + Ty;
      T2h = KP1_732050807 * (TT - TU);
      T2k = FMS(KP2_000000000, TS, TU) - TT;
      TV = TS + TT + TU;
      T11 = FNMS(KP427405661, T10, KP303494444 * TV);
      {
        E T2o, T2p, T2j, T2m;
        T2o = T2h - T2i;
        T2p = T2k + T2l;
        T2q = FMA(KP160793728, T2o, KP092681288 * T2p);
        T3h = FNMS(KP092681288, T2o, KP160793728 * T2p);
        T2j = T2h + T2i;
        T2m = T2k - T2l;
        T2n = FMA(KP183333495, T2j, KP028866483 * T2m);
        T3i = FNMS(KP183333495, T2m, KP028866483 * T2j);
      }
    }
    {
      E T2a, T27, T26, T29, Tx, TE, TA;
      TQ = TI + TP;
      T2a = KP3_464101615 * (TO - TL);
      T27 = FNMS(KP2_000000000, TP, KP4_000000000 * TI);
      Tx = Tv - Tw;
      TE = TB - TD;
      TA = Ty - Tz;
      T26 = KP1_732050807 * (TA - TE);
      T29 = FMS(KP2_000000000, Tx, TE) - TA;
      TF = Tx + TA + TE;
      TR = FMA(KP348438623, TF, KP255877341 * TQ);
      {
        E T2d, T2e, T28, T2b;
        T2d = T26 + T27;
        T2e = T29 + T2a;
        T2f = FMA(KP147857608, T2d, KP112172063 * T2e);
        T3e = FNMS(KP112172063, T2d, KP147857608 * T2e);
        T28 = T26 - T27;
        T2b = T29 - T2a;
        T2c = FMA(KP000412259, T28, KP185591687 * T2b);
        T3f = FNMS(KP000412259, T2b, KP185591687 * T28);
      }
    }
  }
  O[WS(os, 15)] = FMA(KP2_000000000, Tt, T1);
  {
    E T3k, T3Q, T4g, T4D, T4r, T4H, T3v, T3T, T2t, T3d, T3P, T4T, T4d, T4E, T4U;
    E T4o, T4G, T2S, T3o, T3S, T24, T2U, T32, T3A, T3N, T3Z, T3K, T3Y, T39, T3B;
    {
      E T3g, T3j, T4e, T4f;
      T3g = T3e - T3f;
      T3j = T3h - T3i;
      T3k = FMA(KP587785252, T3g, KP951056516 * T3j);
      T3Q = FNMS(KP587785252, T3j, KP951056516 * T3g);
      T4e = T2c + T2f;
      T4f = T2n + T2q;
      T4g = FMA(KP1_018073920, T4e, KP1_647278207 * T4f);
      T4D = FNMS(KP1_647278207, T4e, KP1_018073920 * T4f);
    }
    {
      E T4p, T4q, T3r, T3u;
      T4p = T2L - T2O;
      T4q = T2D - T2A;
      T4r = FNMS(KP1_647278207, T4q, KP1_018073920 * T4p);
      T4H = FMA(KP1_018073920, T4q, KP1_647278207 * T4p);
      T3r = T3p - T3q;
      T3u = T3s - T3t;
      T3v = FMA(KP951056516, T3r, KP587785252 * T3u);
      T3T = FNMS(KP587785252, T3r, KP951056516 * T3u);
    }
    {
      E T2s, T3c, T2g, T2r, T3b;
      T2g = T2c - T2f;
      T2r = T2n - T2q;
      T2s = T2g + T2r;
      T3c = KP559016994 * (T2r - T2g);
      T2t = T25 + T2s;
      T3b = FNMS(KP250000000, T2s, T25);
      T3d = T3b + T3c;
      T3P = T3b - T3c;
    }
    {
      E T4a, T4c, T48, T49, T4b;
      T48 = T3f + T3e;
      T49 = T3i + T3h;
      T4a = T48 + T49;
      T4c = KP968245836 * (T49 - T48);
      T4T = FMA(KP1_732050807, T4a, T47);
      T4b = FNMS(KP433012701, T4a, T47);
      T4d = T4b + T4c;
      T4E = T4b - T4c;
    }
    {
      E T4k, T4m, T4i, T4j, T4n;
      T4i = T3p + T3q;
      T4j = T3s + T3t;
      T4k = KP968245836 * (T4i - T4j);
      T4m = T4i + T4j;
      T4U = FMA(KP1_732050807, T4m, T4l);
      T4n = FNMS(KP433012701, T4m, T4l);
      T4o = T4k + T4n;
      T4G = T4n - T4k;
    }
    {
      E T2Q, T3n, T2E, T2P, T3m;
      T2E = T2A + T2D;
      T2P = T2L + T2O;
      T2Q = T2E + T2P;
      T3n = KP559016994 * (T2P - T2E);
      T2S = T2Q - T2R;
      T3m = FMA(KP250000000, T2Q, T2R);
      T3o = T3m + T3n;
      T3S = T3m - T3n;
    }
    {
      E T2X, T34, T13, T2W, T23, T33, T38, T3M, T31, T3J, T12, T1Y;
      T2X = KP1_118033988 * (T11 - TR);
      T34 = KP1_118033988 * (T1X - T1N);
      T12 = TR + T11;
      T13 = FMA(KP2_000000000, T12, Tu);
      T2W = FNMS(KP500000000, T12, Tu);
      T1Y = T1N + T1X;
      T23 = FMS(KP2_000000000, T1Y, T22);
      T33 = FMA(KP500000000, T1Y, T22);
      {
        E T36, T37, T2Z, T30;
        T36 = FMA(KP700592410, T1M, KP122761339 * T1B);
        T37 = FMA(KP404201883, T1S, KP311340628 * T1W);
        T38 = FMA(KP1_902113032, T36, KP1_175570504 * T37);
        T3M = FNMS(KP1_175570504, T36, KP1_902113032 * T37);
        T2Z = FNMS(KP127938670, TF, KP696877247 * TQ);
        T30 = FMA(KP213702830, TV, KP606988889 * T10);
        T31 = FMA(KP1_175570504, T2Z, KP1_902113032 * T30);
        T3J = FNMS(KP1_175570504, T30, KP1_902113032 * T2Z);
      }
      {
        E T2Y, T3L, T3I, T35;
        T24 = T13 - T23;
        T2U = T13 + T23;
        T2Y = T2W + T2X;
        T32 = T2Y - T31;
        T3A = T2Y + T31;
        T3L = T33 - T34;
        T3N = T3L - T3M;
        T3Z = T3L + T3M;
        T3I = T2W - T2X;
        T3K = T3I - T3J;
        T3Y = T3I + T3J;
        T35 = T33 + T34;
        T39 = T35 - T38;
        T3B = T35 + T38;
      }
    }
    {
      E T4X, T2T, T4W, T4V, T2V, T4S;
      T4X = T4T + T4U;
      T2T = T2t - T2S;
      T4W = T24 - T2T;
      O[WS(os, 30)] = FMA(KP2_000000000, T2T, T24);
      O[WS(os, 28)] = T4W + T4X;
      O[WS(os, 18)] = T4W - T4X;
      T4V = T4T - T4U;
      T2V = T2t + T2S;
      T4S = T2U - T2V;
      O[0] = FMA(KP2_000000000, T2V, T2U);
      O[WS(os, 2)] = T4S + T4V;
      O[WS(os, 12)] = T4S - T4V;
      {
        E T3a, T3y, T3x, T3z, T4t, T4v, T46, T4u;
        T3a = T32 - T39;
        T3y = T32 + T39;
        {
          E T3l, T3w, T4h, T4s;
          T3l = T3d - T3k;
          T3w = T3o - T3v;
          T3x = T3l - T3w;
          T3z = T3l + T3w;
          T4h = T4d - T4g;
          T4s = T4o - T4r;
          T4t = T4h - T4s;
          T4v = T4h + T4s;
        }
        O[WS(os, 7)] = FMA(KP2_000000000, T3x, T3a);
        O[WS(os, 23)] = FMA(KP2_000000000, T3z, T3y);
        T46 = T3a - T3x;
        O[WS(os, 1)] = T46 - T4t;
        O[WS(os, 24)] = T46 + T4t;
        T4u = T3y - T3z;
        O[WS(os, 29)] = T4u - T4v;
        O[WS(os, 6)] = T4u + T4v;
      }
    }
    {
      E T40, T44, T43, T45, T4P, T4R, T4M, T4Q;
      T40 = T3Y - T3Z;
      T44 = T3Y + T3Z;
      {
        E T41, T42, T4N, T4O;
        T41 = T3P + T3Q;
        T42 = T3S + T3T;
        T43 = T41 - T42;
        T45 = T41 + T42;
        T4N = T4E - T4D;
        T4O = T4H + T4G;
        T4P = T4N - T4O;
        T4R = T4N + T4O;
      }
      O[WS(os, 11)] = FMA(KP2_000000000, T43, T40);
      O[WS(os, 19)] = FMA(KP2_000000000, T45, T44);
      T4M = T40 - T43;
      O[WS(os, 22)] = T4M - T4P;
      O[WS(os, 4)] = T4M + T4P;
      T4Q = T44 - T45;
      O[WS(os, 8)] = T4Q - T4R;
      O[WS(os, 26)] = T4Q + T4R;
    }
    {
      E T3O, T3W, T3V, T3X, T4J, T4L, T4C, T4K;
      T3O = T3K - T3N;
      T3W = T3K + T3N;
      {
        E T3R, T3U, T4F, T4I;
        T3R = T3P - T3Q;
        T3U = T3S - T3T;
        T3V = T3R - T3U;
        T3X = T3R + T3U;
        T4F = T4D + T4E;
        T4I = T4G - T4H;
        T4J = T4F - T4I;
        T4L = T4F + T4I;
      }
      O[WS(os, 13)] = FMA(KP2_000000000, T3V, T3O);
      O[WS(os, 17)] = FMA(KP2_000000000, T3X, T3W);
      T4C = T3O - T3V;
      O[WS(os, 27)] = T4C - T4J;
      O[WS(os, 5)] = T4C + T4J;
      T4K = T3W - T3X;
      O[WS(os, 3)] = T4K - T4L;
      O[WS(os, 25)] = T4K + T4L;
    }
    {
      E T3C, T3G, T3F, T3H, T4z, T4B, T4w, T4A;
      T3C = T3A - T3B;
      T3G = T3A + T3B;
      {
        E T3D, T3E, T4x, T4y;
        T3D = T3d + T3k;
        T3E = T3o + T3v;
        T3F = T3D - T3E;
        T3H = T3D + T3E;
        T4x = T4g + T4d;
        T4y = T4r + T4o;
        T4z = T4x - T4y;
        T4B = T4x + T4y;
      }
      O[WS(os, 16)] = FMA(KP2_000000000, T3F, T3C);
      O[WS(os, 14)] = FMA(KP2_000000000, T3H, T3G);
      T4w = T3C - T3F;
      O[WS(os, 21)] = T4w - T4z;
      O[WS(os, 20)] = T4w + T4z;
      T4A = T3G - T3H;
      O[WS(os, 9)] = T4A - T4B;
      O[WS(os, 10)] = T4A + T4B;
    }
  }
}
