/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDigitizer/ECLDigitizerModule.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLTrig.h>
#include <ecl/dataobjects/ECLWaveformData.h>
#include <ecl/geometry/ECLGeometryPar.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <framework/utilities/FileSystem.h>

//C++ STL
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <utility> //contains pair

#define PI 3.14159265358979323846


// ROOT
#include <TVector3.h>
#include <TRandom.h>
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDigitizer)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDigitizerModule::ECLDigitizerModule() : Module()
{
  //Set module properties
  setDescription("Creates ECLDigiHits from ECLHits.");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("Background", m_background, "Flag to use the Digitizer configuration with backgrounds; Default is no background", false);
//  addParam("RandomSeed", m_randSeed, "User-supplied random seed; Default 0 for ctime", (unsigned int)(0));

}


ECLDigitizerModule::~ECLDigitizerModule()
{

}

void ECLDigitizerModule::initialize()
{
  string dataFileName;
  if (m_background)
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF-BG.root");
  else
    dataFileName = FileSystem::findFile("/data/ecl/ECL-WF.root");

  assert(! dataFileName.empty());
  if (! dataFileName.empty()) {

    StoreArray<ECLWaveformData> eclWaveformData("ECLWaveformData", DataStore::c_Persistent);
    eclWaveformData.registerInDataStore();
    StoreObjPtr< ECLLookupTable > eclWaveformDataTable("ECLWaveformDataTable", DataStore::c_Persistent);;
    eclWaveformDataTable.registerInDataStore();
    eclWaveformDataTable.create();
    StoreArray<ECLWFAlgoParams> eclWFAlgoParams("ECLWFAlgoParams", DataStore::c_Persistent);
    eclWFAlgoParams.registerInDataStore();
    StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);
    eclWFAlgoParamsTable.registerInDataStore();
    eclWFAlgoParamsTable.create();
    StoreArray<ECLNoiseData> eclNoiseData("ECLNoiseData", DataStore::c_Persistent);
    eclNoiseData.registerInDataStore();
    StoreObjPtr< ECLLookupTable > eclNoiseDataTable("ECLNoiseDataTable", DataStore::c_Persistent);;
    eclNoiseDataTable.registerInDataStore();
    eclNoiseDataTable.create();

    TFile f(dataFileName.c_str());
    TTree* t = (TTree*) f.Get("EclWF");
    TTree* t2 = (TTree*) f.Get("EclAlgo");
    TTree* t3 = (TTree*) f.Get("EclNoise");

    if (t == 0 || t2 == 0 || t3 == 0)
      B2FATAL("Data not found");

    Int_t ncellId;
    Int_t cellId[8736];
    Int_t ncellId2;
    Int_t cellId2[8736];
    Int_t ncellId3;
    Int_t cellId3[8736];


    t->GetBranch("CovarianceM")->SetAutoDelete(kFALSE);
    t->SetBranchAddress("ncellId", &ncellId);
    t->SetBranchAddress("cellId", cellId);

    t2->GetBranch("Algopars")->SetAutoDelete(kFALSE);
    t2->SetBranchAddress("ncellId", &ncellId2);
    t2->SetBranchAddress("cellId", cellId2);

    t3->GetBranch("NoiseM")->SetAutoDelete(kFALSE);
    t3->SetBranchAddress("ncellId", &ncellId3);
    t3->SetBranchAddress("cellId", cellId3);

    ECLWaveformData* info = new ECLWaveformData;
    t->SetBranchAddress("CovarianceM", &info);
    Long64_t nentries = t->GetEntries();
    for (Long64_t ev = 0; ev < nentries; ev++) {
      t->GetEntry(ev);
      eclWaveformData.appendNew(*info);
      for (Int_t i = 0; i < ncellId; ++i)
        (*eclWaveformDataTable) [cellId[i]] = ev;
    }

    ECLWFAlgoParams* algo = new ECLWFAlgoParams;
    t2->SetBranchAddress("Algopars", &algo);
    nentries = t2->GetEntries();
    for (Long64_t ev = 0; ev < nentries; ev++) {
      t2->GetEntry(ev);
      eclWFAlgoParams.appendNew(*algo);
      for (Int_t i = 0; i < ncellId2; ++i)
        (*eclWFAlgoParamsTable) [cellId2[i]] = ev;
    }

    ECLNoiseData* noise = new ECLNoiseData;
    t3->SetBranchAddress("NoiseM", &noise);
    nentries = t3->GetEntries();
    for (Long64_t ev = 0; ev < nentries; ev++) {
      t3->GetEntry(ev);
      eclNoiseData.appendNew(*noise);
      if (ncellId3 == 0) {
        for (int i = 1; i <= 8736; i++)(*eclNoiseDataTable)[i] = 0;
        break;
      } else {
        for (Int_t i = 0; i < ncellId3; ++i)
          (*eclNoiseDataTable)[cellId3[i]] = ev;
      }
    }

    f.Close();
  }


  m_nEvent  = 0 ;

  readDSPDB();

  StoreArray<ECLDsp>::registerPersistent();
  StoreArray<ECLDigit>::registerPersistent();
  StoreArray<ECLTrig>::registerPersistent();




}

void ECLDigitizerModule::beginRun()
{
}


void ECLDigitizerModule::event()
{
  //Get the waveform, covariance matrix and fit algoritm parameters from the Event Store

  StoreArray<ECLWaveformData> eclWaveformData("ECLWaveformData", DataStore::c_Persistent);;
  StoreObjPtr< ECLLookupTable > eclWaveformDataTable("ECLWaveformDataTable", DataStore::c_Persistent);
  StoreArray<ECLWFAlgoParams> eclWFAlgoParams("ECLWFAlgoParams", DataStore::c_Persistent);;
  StoreObjPtr< ECLLookupTable> eclWFAlgoParamsTable("ECLWFAlgoParamsTable", DataStore::c_Persistent);
  StoreArray<ECLNoiseData> eclNoiseData("ECLNoiseData", DataStore::c_Persistent);
  StoreObjPtr< ECLLookupTable > eclNoiseDataTable("ECLNoiseDataTable", DataStore::c_Persistent);

  // the following is just a test to show how to get
  // the correct ECLWaveformData object associated
  // to a crystal cellId
  // we loop over all the cellId and verify that a
  // all the required objects exist in memory
  // Moreover it shows how to get configuration data needed
  // by the Digitizer for a given crystal.

  static bool firstEvent = true;
  if (firstEvent) {
    firstEvent = false;
    for (int aCellId = 1; aCellId <= 8736; ++aCellId) {
      B2INFO("Checking ECL crystal id == " << aCellId);
      const ECLWaveformData* eclWFData = eclWaveformData[(*eclWaveformDataTable)[ aCellId ] ];
      B2INFO("ECLWaveformData address: " << eclWFData);
      const ECLWFAlgoParams* eclWFAlgo = eclWFAlgoParams[(*eclWFAlgoParamsTable)[ aCellId ] ];
      B2INFO("ECLAlgoParams address: " << eclWFAlgo);
      const ECLNoiseData* eclNoise = eclNoiseData[(*eclNoiseDataTable)[ aCellId ] ];
      B2INFO("ECLNoise address: " << eclNoise);
    }
  }
  // end of the test





  //Input Array
  StoreArray<ECLHit>  eclArray;
  if (!eclArray) {
    B2DEBUG(100, "ECLHit in empty in event " << m_nEvent);
  }

  //cout<<"Total Hits in Digi "<<eclArray->GetEntriesFast()<<endl;
  int energyFit[8736] = {0}; //fit output : Amplitude
  int tFit[8736] = {0};    //fit output : T_ave
  int qualityFit[8736] = {0};    //fit output : T_ave
  double HitEnergy[8736][31] = {{0}};
  double E_tmp[8736] = {0};
  double test_A[31] = {0};
  float AdcNoise[31];
  float genNoise[31];



  double dt = .02; //delta t for interpolation
  int n = 1250;//provide a shape array for interpolation
  // double DeltaT =  gRandom->Uniform(0, 24);
  double DeltaT =  0.0; // test by KM 20140620

  for (int ii = 0; ii <  eclArray.getEntries(); ii++) {

    ECLHit* aECLHit = eclArray[ii];
    int hitCellId       =  aECLHit->getCellId() - 1; //0~8735
    double hitE         =  aECLHit->getEnergyDep() / Unit::GeV;
    double hitTimeAve       =  aECLHit->getTimeAve()   / Unit::us;
    double sampleTime ;


    if (hitTimeAve > 8.5) { continue;}
    E_tmp[hitCellId] = hitE + E_tmp[hitCellId];//for summation deposit energy; do fit if this summation > 0.1 MeV

    for (int T_clock = 0; T_clock < 31; T_clock++) {
      double timeInt =  DeltaT * 12. / 508.; //us
      sampleTime = (24. * 12. / 508.)  * (T_clock - 15) - hitTimeAve - timeInt + 0.32
                   ;//There is some time shift~0.32 us that is found Alex 2013.06.19.
      //test_A[T_clock] = ShaperDSP(sampleTime);
      DspSamplingArray(&n, &sampleTime, &dt, m_ft, &test_A[T_clock]);//interpolation from shape array n=1250; dt =20ns
      HitEnergy[hitCellId][T_clock] = test_A[T_clock]  * hitE  +  HitEnergy[hitCellId][T_clock];
    }//for T_clock 31 clock

  } //end loop over eclHitArray ii



  for (int iECLCell = 0; iECLCell < 8736; iECLCell++) {
    if (E_tmp[iECLCell] > 0.0001) {
      //Noise generation
      for (int iCal = 0; iCal < 31; iCal++) {
        genNoise[iCal] =  gRandom->Gaus(0, 1);
      }

      for (int T_clock = 0; T_clock < 31; T_clock++) {
        AdcNoise[T_clock] = 0;
        for (int iCal = 0; iCal < T_clock; iCal++) {
          AdcNoise[T_clock] = m_vmat[T_clock][iCal] * genNoise[iCal] + AdcNoise[T_clock];
        }
      }

      for (int  T_clock = 0; T_clock < 31; T_clock++) {
        FitA[T_clock] = (int)(HitEnergy[iECLCell][T_clock] * 20000 + 3000 + AdcNoise[T_clock] * 20) ;

      }

      m_n16 = 16;
      m_ch = 0;
      m_lar = 0;
      m_ltr = 0;
      m_lq = 0;

      m_ttrig = int(DeltaT) ;
      if (m_ttrig < 0)m_ttrig = 0;
      if (m_ttrig > 23)m_ttrig = 23;

      shapeFitter(&(m_id[0][0]), &(m_f[0][0]), &(m_f1[0][0]), &(m_fg41[0][0]), &(m_fg43[0][0]), &(m_fg31[0][0]), &(m_fg32[0][0]),
                  &(m_fg33[0][0]), &(FitA[0]), &m_ttrig,  &m_n16, &m_ch, &m_lar, &m_ltr, &m_lq);

      energyFit[iECLCell] = m_lar; //fit output : Amplitude 18-bits
      tFit[iECLCell] = m_ltr;    //fit output : T_ave 12 bits
      qualityFit[iECLCell] = m_lq;    //fit output : quality 2 bits


      if (energyFit[iECLCell] > 0) {

        StoreArray<ECLDsp> eclDspArray;
        if (!eclDspArray) eclDspArray.create();
        eclDspArray.appendNew();
        m_hitNum = eclDspArray.getEntries() - 1;
        eclDspArray[m_hitNum]->setCellId(iECLCell + 1);
        eclDspArray[m_hitNum]->setDspA(FitA);

        StoreArray<ECLDigit> eclDigiArray;
        if (!eclDigiArray) eclDigiArray.create();
        eclDigiArray.appendNew();
        m_hitNum1 = eclDigiArray.getEntries() - 1;
        eclDigiArray[m_hitNum1]->setCellId(iECLCell + 1);//iECLCell + 1= 1~8736
        eclDigiArray[m_hitNum1]->setAmp(energyFit[iECLCell]);//E (GeV) = energyFit/20000;
        eclDigiArray[m_hitNum1]->setTimeFit(tFit[iECLCell]);//t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;
        eclDigiArray[m_hitNum1]->setQuality(qualityFit[iECLCell]);
      }

    }//if Energy > 0.1 MeV
  } //store  each crystal hit

  StoreArray<ECLTrig> eclTrigArray;
  if (!eclTrigArray) eclTrigArray.create();
  eclTrigArray.appendNew();
  m_hitNum2 = eclTrigArray.getEntries() - 1;
  eclTrigArray[m_hitNum2]->setTimeTrig(DeltaT * 12. / 508.); //t0 (us)= (1520 - m_ltr)*24.*12/508/(3072/2) ;

  //cout<<"Event "<< m_nEvent<<" Total output entries of Digi Array "<<++m_hitNum1<<endl;
  m_nEvent++;
}

void ECLDigitizerModule::endRun()
{
}

void ECLDigitizerModule::terminate()
{
}


void ECLDigitizerModule::DspSamplingArray(int* n, double* t, double* dt, double* ft, double* ff)
{
  int i, i1;
  double s1, s2;
  i = (*t) / (*dt);

  //cout<<"i= "<<i<<endl;
  //cout<<" ft[i+1]  "<< ft[i+1]<<" ft[i]  "<<ft[i]<<" t "<<*t<<" dt "<<*dt<<endl;
  if (i < 0 || (i + 1) >= (*n)) {
    *ff = 0;
    return;
  }
  s1 = ft[i];

  i1 = i + 1;
  if (i < *n) {
    s2 = ft[i1];
  } else {
    s2 = 0;
  }
  *ff = s1 + (s2 - s1) * (*t / (*dt) - i);

  return;
}



double ECLDigitizerModule::ShaperDSP(double Ti)
{
  double svp = 0;
  double tr1 = Ti * 0.881944444;
  double s[12] = {0, 27.7221, 0.5, 0.6483, 0.4017, 0.3741, 0.8494, 0.00144547, 4.7071, 0.8156, 0.5556, 0.2752};


  double tr = tr1 - s[2];
  double tr2 = tr + .2;
  double tr3 = tr - .2;
  if (tr2 > 0.) {

    svp = (Sv123(tr , s[4], s[5], s[9], s[10], s[3], s[6]) * (1 - s[11])
           + s[11] * .5 * (Sv123(tr2, s[4], s[5], s[9], s[10], s[3], s[6])
                           + Sv123(tr3, s[4], s[5], s[9], s[10], s[3], s[6])));
    double x = tr / s[4];


    svp = s[1] * (svp - s[7] * (exp(-tr / s[8]) *
                                (1 - exp(-x) * (1 + x + x * x / 2 + x * x * x / 6 + x * x * x * x / 24 + x * x * x * x * x / 120))));
  } else svp = 0 ;
  return svp;

}


double  ECLDigitizerModule::Sv123(double t, double t01, double tb1, double t02, double tb2, double td1, double ts1)
{

  double sv123 = 0.;
  double  dks0, dks1, dksm,
          dw0, dw1, dwp, dwm, das1, dac1, das0, dac0, dzna, dksm2, ds, dd,
          dcs0, dsn0, dzn0, td, ts, dr,
          dcs0s, dsn0s, dcs0d, dsn0d, dcs1s, dsn1s, dcs1d, dsn1d;


  if (t < 0.) return 0.;

  dr = (ts1 - td1) / td1;
  if (abs(dr) >= 0.0000001) {
    td = td1;
    ts = ts1;
  } else {
    td = td1;
    if (ts1 > td1) {
      ts = td1 * 1.00001;
    } else {
      ts = td1 * 0.99999;
    }
  }

  dr = ((t01 - t02) * (t01 - t02) + (tb1 - tb2) * (tb1 - tb2)) / ((t01) * (t01) + (tb1) * (tb1));
  dks0 = 1.0 / t01;
  dks1 = 1.0 / t02;

  if (dr < 0.0000000001) {

    if (dks0 > dks1) {
      dks0 = dks1 * 1.00001;
    } else {
      dks0 = dks1 * 0.99999;
    }
  }

  if (t < 0.) return 0;



  dksm = dks1 - dks0;

  ds = 1. / ts;
  dd = 1. / td;

  dw0 = 1. / tb1;
  dw1 = 1. / tb2;
  dwp = dw0 + dw1;
  dwm = dw1 - dw0;

  dksm2 = dksm * dksm;

  dzna = (dksm2 + dwm * dwm) * (dksm2 + dwp * dwp);


  das0 = dw1 * (dksm2 + dwp * dwm);
  dac0 = -2 * dksm * dw0 * dw1;
  das1 = dw0 * (dksm2 - dwp * dwm);
  dac1 = -dac0;





  dsn0 = (ds - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0s = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0s = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (ds - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1s = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1s = (dcs0 * das1 + dsn0 * dac1) / dzn0;


  dsn0 = (dd - dks0);
  dcs0 = -dw0;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn0d = (dsn0 * das0 - dcs0 * dac0) / dzn0;
  dcs0d = (dcs0 * das0 + dsn0 * dac0) / dzn0;

  dsn0 = (dd - dks1);
  dcs0 = -dw1;
  dzn0 = dcs0 * dcs0 + dsn0 * dsn0;

  dsn1d = (dsn0 * das1 - dcs0 * dac1) / dzn0;
  dcs1d = (dcs0 * das1 + dsn0 * dac1) / dzn0;

  //cppcheck dr = (ts - td) / td;




  sv123 = ((((dsn0s - dsn0d) * sin(dw0 * t)
             + (dcs0s - dcs0d) * cos(dw0 * t)) * exp(-t * dks0)
            - (dcs0s + dcs1s) * exp(-t * ds) + (dcs0d + dcs1d) * exp(-t * dd)
            + ((dsn1s - dsn1d) * sin(dw1 * t)
               + (dcs1s - dcs1d) * cos(dw1 * t)) * exp(-t * dks1)) / dzna / (ts - td));

  return sv123;


}


void ECLDigitizerModule::shapeFitter(short int* id, int* f, int* f1, int* fg41, int* fg43, int* fg31, int* fg32, int* fg33, int* y,
                                     int* ttrig, int* n16, int* ch, int* lar, int* ltr, int* lq)
{



  /*
   cout<<"Fit Array: \n";
   for(int testi=0;testi<1;testi++) {
    for(int testj=0;testj<31;testj++){
    cout<<*(y+testi *16+testj)<<" ";
   }cout<<endl;}cout<<endl<<endl;;

   cout<<"fg41: " <<endl;
   for(int testi=0;testi<24;testi++) {
    for(int testj=0;testj<16;testj++){
    cout<<*(fg41+testi *16+testj)<<" ";
   }cout<<endl;}cout<<endl<<endl;;

   cout<<"fg43: " <<endl;
   for(int testi=0;testi<24;testi++) {
    for(int testj=0;testj<16;testj++){
    cout<<*(fg43+testi *16+testj)<<" ";
   }cout<<endl;}cout<<endl<<endl;;

   cout<<"id: " <<endl;
   for(int testi=0;testi<16;testi++) {
    for(int testj=0;testj<16;testj++){
    cout<<*(id+testi *16+testj)<<" ";
   }cout<<endl;}cout<<endl<<endl;
    */
  static long long int k_np[16] = {
    65536,
    32768,
    21845,
    16384,
    13107,
    10923,
    9362,
    8192,
    7282,
    6554,
    5958,
    5461,
    5041,
    4681,
    4369,
    4096
  };
//  long long int b32=4294967295;
  int A0  = (int) * (id + 128 + *ch) - 128;
  int Askip  = (int) * (id + 192 + *ch) - 128;
  //int Askip  = -127-128;
  int Ahard  = (int) * (id + 64 + *ch);
  int k_a = (int) * ((unsigned char*)id + 26);
  int k_b = (int) * ((unsigned char*)id + 27);
  int k_c = (int) * ((unsigned char*)id + 28);
  int k_16 = (int) * ((unsigned char*)id + 29);

  int k1_chi = (int) * ((unsigned char*)id + 32);
  int k2_chi = (int) * ((unsigned char*)id + 33);
  int chi_thres = (int) * (id + 15);


  int s1, s2;


  long long int z0;
  int it, it0;
//  long long d_it;
  int it_h, it_l;
  long long A1, B1, A2, C1, ch1, ch2, B2, B3, B5 ;
  int low_ampl, i, T, iter;


  if (k_16 + *n16 != 16) {
    printf("disagreement in number of the points %d %d \n", k_16, *n16);
  }


  int validity_code = 0;
  for (i = 0, z0 = 0; i < 16; i++)
    z0 += y[i];

  //initial time index

//  it0 = 48 + ((23 - *ttrig) << 2);//Alex modify
//  it_h = 191;
//  it_l = 0;
//  it = it0;

  if (*ttrig > 23) {cout << "*Ttrig  Warning" << *ttrig << endl; *ttrig = 23;}
  if (*ttrig < 0) {cout << "*Ttrig  Warning" << *ttrig << endl; *ttrig = 0;}


//  it0 = 96 + ((12 - *ttrig) << 3);Alex kuzmin modify 2013.06.18
  it0 = 144 - ((*ttrig) << 2);


  it_h = 191;
  it_l = 0;
  if (it0 < it_l)it0 = it_l;
  if (it0 > it_h)it0 = it_h;
  it = it0;

  //first approximation without time correction

  //  int it00=23-it0;

  A2 = (*(fg41 + *ttrig * 16));

  A2 = (A2 * z0);


  for (i = 1; i < 16; i++) {
    s1 = (*(fg41 + *ttrig * 16 + i));
    B3 = y[15 + i];
    B3 = s1 * B3;
    A2 += B3;

  }
  A2 += (1 << (k_a - 1));
  A2 >>= k_a;
  T = it0 << 4;


  //too large amplitude
  if (A2 > 262143) {
    A1 = A2 >> 3;
    validity_code = 1;

    goto ou;
  }


  low_ampl = 0;

  if (A2 >= A0) {

    for (iter = 0, it = it0; iter < 3;) {
      iter++;
      s1 = (*(fg31 + it * 16));
      s2 = (*(fg32 + it * 16));
      A1 = (s1 * z0);
      B1 = (s2 * z0);



      for (i = 1; i < 16; i++) {
        s1 = (*(fg31 + i + it * 16));
        s2 = (*(fg32 + i + it * 16));
        B5 = y[15 + i];
        B5 = s1 * B5;
        A1 += B5;

        B3 = y[15 + i];
        B3 = s2 * B3;
        B1 += B3;
      }
      A1 += (1 << (k_a - 1));
      A1 = A1 >> k_a;


      if (A1 > 262143)
        goto ou;
      if (A1 < A0) {

        low_ampl = 1;
        it = it0;

        goto lam;
      }

      if (iter != 3) {

        B2 = B1 >> (k_b - 9);
        B1 = B2 >> 9;

        B2 += (A1 << 9);


        B3 = (B2 / A1);

        it += ((B3 + 1) >> 1) - 256;
        it = it > it_h ? it_h : it;
        it = it < it_l ? it_l : it;

//        cout<<" A1:Amp "<<A1<<" B3:time "<< B3<<" it "<<it<<" it0 "<<it0<<endl;
      } else {
        B2 = B1 >> (k_b - 13);
        B5 = B1 >> (k_b - 9);

        //cppcheck reassigned a value before the old one has been used//  B1 = B2 >> 13;
        B2 += (A1 << 13);
        B3 = (B2 / A1);

        T = ((it) << 4) + ((B3 + 1) >> 1) - 4096;

        B1 = B5 >> 9;
        B5 += (A1 << 9);
        B3 = (B5 / A1);
        it += ((B3 + 1) >> 1) - 256;
        it = it > it_h ? it_h : it;
        it = it < it_l ? it_l : it;

        T = T > 3071 ?  3071 : T;

        T = T < 0 ? 0 : T;

        C1 = (*(fg33 + it * 16) * z0);
        for (i = 1; i < 16; i++)
          C1 += *(fg33 + i + it * 16) * y[15 + i];
        C1 += (1 << (k_c - 1));
        C1 >>= k_c;

//        cout<<" A1:Amp "<<A1<<" B3:time "<< T<<" it "<<it<<" it0 "<<it0<<" C1:pred "<<C1 <<endl;
      }

    }
  } else
    low_ampl = 1;

  if (low_ampl == 1) {

lam:
    A1 = A2;
    validity_code = 0;
    B1 = 0;
    C1 = (*(fg43 + *ttrig * 16) * z0);
    for (i = 1; i < 16; i++) {
      B5 = y[15 + i];
      C1 += *(fg43 + i + *ttrig * 16) * B5;
    }
    C1 += (1 << (k_c - 1));
    C1 >>= k_c;
  }
  ch2 = (A1** (f + it * 16) + B1** (f1 + it * 16)) >> k1_chi;
  ch2 += C1;
  ch2 = z0 - *n16 * ch2;
  ch1 = ((ch2) * (ch2));
  ch1 = ch1 * k_np[*n16 - 1];
  ch1 = ch1 >> 16;
  for (i = 1; i < 16; i++) {
    ch2 = A1 * (*(f + i + it * 16)) + B1 * (*(f1 + i + it * 16));
    ch2 >>= k1_chi;
    ch2 = (y[i + 15] - ch2 - C1);

    ch1 = ch1 + ch2 * ch2;

  }
  B2 = (A1 >> 1) * (A1 >> 1);
  B2 >>= (k2_chi - 2);
  B2 += chi_thres;
  if (ch1 > B2)validity_code = 3;
//        cout<<" A1:Amp "<<A1<<" A2 "<<A2<<" A0 "<<A0 <<endl;
ou:

  *lar = A1;
  *ltr = T;
//        cout<<" A1:Amp "<<*lar <<" B3:time "<< *ltr<<" it "<<it<<" it0 "<<it0<<endl;

  if (A1 < Askip)validity_code = validity_code + 8;

  int ss = (y[20] + y[21]);

  if (ss <= Ahard)validity_code = validity_code + 4;


  *lq = validity_code;


  return ;
}


void ECLDigitizerModule::readDSPDB()
{

  /*
  double t=0;
  double dt=.02;
  int n=1250;
  cout<<"  double par_shape[] = {";
  for(int i=0;i<1250;i++)
  {
  if(i!=0&&i%10==0 )cout<<endl;
  cout<<ShaperDSP(t)<<",";
  t=t+dt;
  }
  cout<<"};"<<endl;
  */

  double par_shape[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, -7.51697e-07, -3.78028e-07,
                        -8.67619e-08, 3.41302e-07, 1.23072e-06, 3.04439e-06, 6.40817e-06, 1.21281e-05, 2.12008e-05, 3.48187e-05, 5.43696e-05, 8.14324e-05,
                        0.000117797, 0.000165613, 0.000227605, 0.000307253, 0.000408928, 0.000537989, 0.000700847, 0.00090499, 0.00115899, 0.00147249,
                        0.00185614, 0.00232157, 0.00288129, 0.00354872, 0.00433806, 0.00526424, 0.00634286, 0.00759005, 0.00902239, 0.0106568,
                        0.0125104, 0.0146005, 0.0169442, 0.0195586, 0.0224606, 0.0256664, 0.0291921, 0.0330529, 0.0372636, 0.0418379,
                        0.0467888, 0.0521282, 0.0578671, 0.0640153, 0.0705814, 0.077573, 0.084996, 0.0928553, 0.101154, 0.109896,
                        0.119079, 0.128705, 0.138772, 0.149274, 0.160209, 0.17157, 0.183349, 0.195538, 0.208126, 0.221103,
                        0.234457, 0.248173, 0.262237, 0.276634, 0.291346, 0.306357, 0.321649, 0.337201, 0.352995, 0.369009,
                        0.385223, 0.401615, 0.418163, 0.434844, 0.451637, 0.468517, 0.485461, 0.502447, 0.519451, 0.536449,
                        0.553419, 0.570336, 0.587178, 0.603922, 0.620546, 0.637027, 0.653343, 0.669473, 0.685397, 0.701093,
                        0.716542, 0.731725, 0.746622, 0.761216, 0.775489, 0.789426, 0.803009, 0.816223, 0.829055, 0.84149,
                        0.853516, 0.86512, 0.876292, 0.88702, 0.897296, 0.907109, 0.916454, 0.925321, 0.933705, 0.941601,
                        0.949003, 0.955907, 0.962311, 0.968212, 0.973608, 0.978499, 0.982884, 0.986764, 0.99014, 0.993014,
                        0.995389, 0.997267, 0.998653, 0.999552, 0.999967, 0.999905, 0.999371, 0.998373, 0.996918, 0.995012,
                        0.992664, 0.989882, 0.986675, 0.983052, 0.979022, 0.974596, 0.969782, 0.964592, 0.959037, 0.953126,
                        0.946871, 0.940282, 0.933372, 0.926152, 0.918632, 0.910826, 0.902744, 0.894397, 0.885799, 0.876961,
                        0.867894, 0.85861, 0.849121, 0.839439, 0.829576, 0.819541, 0.809349, 0.799008, 0.788532, 0.77793,
                        0.767214, 0.756394, 0.745481, 0.734486, 0.723418, 0.712288, 0.701106, 0.689881, 0.678622, 0.66734,
                        0.656041, 0.644737, 0.633434, 0.622142, 0.610868, 0.59962, 0.588405, 0.577232, 0.566106, 0.555035,
                        0.544025, 0.533082, 0.522213, 0.511423, 0.500718, 0.490102, 0.479582, 0.469161, 0.458845, 0.448637,
                        0.438542, 0.428563, 0.418704, 0.408969, 0.399359, 0.389879, 0.380531, 0.371318, 0.362241, 0.353302,
                        0.344505, 0.335849, 0.327337, 0.31897, 0.310749, 0.302674, 0.294748, 0.286969, 0.279339, 0.271858,
                        0.264525, 0.257342, 0.250307, 0.24342, 0.236682, 0.23009, 0.223645, 0.217346, 0.211192, 0.205182,
                        0.199314, 0.193587, 0.188001, 0.182553, 0.177243, 0.172068, 0.167028, 0.162119, 0.157341, 0.152692,
                        0.14817, 0.143773, 0.139499, 0.135345, 0.131311, 0.127393, 0.123591, 0.1199, 0.116321, 0.112849,
                        0.109484, 0.106222, 0.103062, 0.100002, 0.0970386, 0.0941702, 0.0913946, 0.0887094, 0.0861124, 0.0836015,
                        0.0811744, 0.0788289, 0.0765629, 0.0743742, 0.0722606, 0.0702201, 0.0682505, 0.0663496, 0.0645156, 0.0627463,
                        0.0610398, 0.059394, 0.057807, 0.0562769, 0.0548018, 0.0533799, 0.0520093, 0.0506882, 0.049415, 0.0481878,
                        0.047005, 0.0458649, 0.044766, 0.0437067, 0.0426853, 0.0417005, 0.0407508, 0.0398346, 0.0389507, 0.0380977,
                        0.0372743, 0.0364791, 0.035711, 0.0349687, 0.0342511, 0.0335571, 0.0328854, 0.0322352, 0.0316053, 0.0309948,
                        0.0304027, 0.0298281, 0.0292701, 0.0287278, 0.0282004, 0.0276872, 0.0271873, 0.0267001, 0.0262248, 0.0257607,
                        0.0253073, 0.0248639, 0.02443, 0.0240049, 0.0235881, 0.0231792, 0.0227776, 0.022383, 0.0219948, 0.0216127,
                        0.0212363, 0.0208651, 0.020499, 0.0201375, 0.0197804, 0.0194273, 0.0190781, 0.0187324, 0.01839, 0.0180508,
                        0.0177145, 0.0173809, 0.01705, 0.0167215, 0.0163953, 0.0160712, 0.0157492, 0.0154292, 0.0151111, 0.0147948,
                        0.0144802, 0.0141673, 0.013856, 0.0135463, 0.0132382, 0.0129316, 0.0126265, 0.012323, 0.012021, 0.0117205,
                        0.0114215, 0.011124, 0.0108282, 0.0105339, 0.0102412, 0.00995022, 0.00966089, 0.00937329, 0.00908744, 0.0088034,
                        0.00852122, 0.00824093, 0.00796259, 0.00768625, 0.00741196, 0.00713978, 0.00686975, 0.00660192, 0.00633636, 0.00607311,
                        0.00581223, 0.00555377, 0.00529778, 0.00504432, 0.00479343, 0.00454516, 0.00429957, 0.00405669, 0.00381658, 0.00357928,
                        0.00334484, 0.00311329, 0.00288467, 0.00265902, 0.00243638, 0.00221679, 0.00200027, 0.00178685, 0.00157657, 0.00136944,
                        0.0011655, 0.000964766, 0.000767257, 0.000572992, 0.000381986, 0.000194254, 9.80634e-06, -0.000171347, -0.000349198, -0.000523742,
                        -0.000694976, -0.000862898, -0.00102751, -0.00118881, -0.00134681, -0.00150151, -0.00165293, -0.00180106, -0.00194593, -0.00208755,
                        -0.00222593, -0.00236109, -0.00249306, -0.00262184, -0.00274746, -0.00286995, -0.00298932, -0.00310562, -0.00321886, -0.00332907,
                        -0.00343628, -0.00354052, -0.00364184, -0.00374025, -0.00383579, -0.00392851, -0.00401842, -0.00410558, -0.00419002, -0.00427178,
                        -0.00435089, -0.0044274, -0.00450135, -0.00457277, -0.00464171, -0.00470822, -0.00477232, -0.00483407, -0.00489351, -0.00495068,
                        -0.00500563, -0.00505839, -0.00510901, -0.00515753, -0.005204, -0.00524846, -0.00529096, -0.00533153, -0.00537023, -0.00540708,
                        -0.00544215, -0.00547546, -0.00550707, -0.005537, -0.00556532, -0.00559205, -0.00561723, -0.00564092, -0.00566314, -0.00568395,
                        -0.00570337, -0.00572145, -0.00573822, -0.00575373, -0.00576801, -0.0057811, -0.00579303, -0.00580384, -0.00581356, -0.00582224,
                        -0.0058299, -0.00583658, -0.00584231, -0.00584712, -0.00585104, -0.00585411, -0.00585635, -0.0058578, -0.00585849, -0.00585843,
                        -0.00585767, -0.00585622, -0.00585412, -0.00585139, -0.00584805, -0.00584413, -0.00583965, -0.00583464, -0.00582912, -0.00582311,
                        -0.00581663, -0.00580971, -0.00580236, -0.0057946, -0.00578645, -0.00577794, -0.00576907, -0.00575986, -0.00575034, -0.00574052,
                        -0.00573041, -0.00572002, -0.00570939, -0.0056985, -0.00568739, -0.00567606, -0.00566453, -0.0056528, -0.00564089, -0.00562881,
                        -0.00561657, -0.00560419, -0.00559166, -0.005579, -0.00556622, -0.00555333, -0.00554033, -0.00552723, -0.00551405, -0.00550078,
                        -0.00548743, -0.00547401, -0.00546053, -0.00544699, -0.0054334, -0.00541976, -0.00540607, -0.00539234, -0.00537858, -0.00536479,
                        -0.00535097, -0.00533713, -0.00532326, -0.00530938, -0.00529549, -0.00528158, -0.00526766, -0.00525373, -0.0052398, -0.00522586,
                        -0.00521192, -0.00519798, -0.00518404, -0.00517011, -0.00515617, -0.00514225, -0.00512832, -0.00511441, -0.0051005, -0.00508659,
                        -0.0050727, -0.00505881, -0.00504493, -0.00503106, -0.0050172, -0.00500335, -0.00498951, -0.00497568, -0.00496186, -0.00494805,
                        -0.00493424, -0.00492045, -0.00490666, -0.00489289, -0.00487912, -0.00486536, -0.00485161, -0.00483786, -0.00482413, -0.0048104,
                        -0.00479668, -0.00478296, -0.00476925, -0.00475555, -0.00474185, -0.00472815, -0.00471447, -0.00470078, -0.0046871, -0.00467343,
                        -0.00465976, -0.00464609, -0.00463242, -0.00461876, -0.0046051, -0.00459145, -0.00457779, -0.00456414, -0.00455049, -0.00453685,
                        -0.0045232, -0.00450956, -0.00449591, -0.00448227, -0.00446864, -0.004455, -0.00444136, -0.00442773, -0.0044141, -0.00440047,
                        -0.00438684, -0.00437322, -0.0043596, -0.00434598, -0.00433236, -0.00431875, -0.00430513, -0.00429153, -0.00427792, -0.00426432,
                        -0.00425073, -0.00423713, -0.00422355, -0.00420997, -0.00419639, -0.00418282, -0.00416926, -0.0041557, -0.00414215, -0.00412861,
                        -0.00411508, -0.00410155, -0.00408803, -0.00407453, -0.00406103, -0.00404754, -0.00403407, -0.0040206, -0.00400715, -0.00399371,
                        -0.00398028, -0.00396687, -0.00395347, -0.00394008, -0.00392671, -0.00391336, -0.00390002, -0.00388669, -0.00387339, -0.0038601,
                        -0.00384683, -0.00383358, -0.00382035, -0.00380713, -0.00379394, -0.00378077, -0.00376762, -0.00375449, -0.00374139, -0.0037283,
                        -0.00371525, -0.00370221, -0.0036892, -0.00367621, -0.00366325, -0.00365031, -0.0036374, -0.00362452, -0.00361166, -0.00359884,
                        -0.00358603, -0.00357326, -0.00356052, -0.0035478, -0.00353512, -0.00352246, -0.00350984, -0.00349725, -0.00348468, -0.00347215,
                        -0.00345965, -0.00344718, -0.00343475, -0.00342234, -0.00340997, -0.00339764, -0.00338534, -0.00337307, -0.00336083, -0.00334863,
                        -0.00333646, -0.00332433, -0.00331224, -0.00330018, -0.00328815, -0.00327616, -0.00326421, -0.00325229, -0.00324041, -0.00322856,
                        -0.00321675, -0.00320498, -0.00319325, -0.00318155, -0.00316989, -0.00315826, -0.00314667, -0.00313513, -0.00312361, -0.00311214,
                        -0.0031007, -0.0030893, -0.00307794, -0.00306662, -0.00305533, -0.00304408, -0.00303287, -0.0030217, -0.00301057, -0.00299947,
                        -0.00298841, -0.00297739, -0.00296641, -0.00295547, -0.00294456, -0.00293369, -0.00292286, -0.00291207, -0.00290131, -0.0028906,
                        -0.00287992, -0.00286927, -0.00285867, -0.0028481, -0.00283757, -0.00282708, -0.00281663, -0.00280621, -0.00279583, -0.00278549,
                        -0.00277518, -0.00276491, -0.00275468, -0.00274449, -0.00273433, -0.00272421, -0.00271412, -0.00270407, -0.00269406, -0.00268408,
                        -0.00267414, -0.00266424, -0.00265437, -0.00264454, -0.00263474, -0.00262498, -0.00261525, -0.00260556, -0.00259591, -0.00258629,
                        -0.0025767, -0.00256715, -0.00255764, -0.00254816, -0.00253871, -0.0025293, -0.00251992, -0.00251058, -0.00250127, -0.00249199,
                        -0.00248275, -0.00247354, -0.00246437, -0.00245522, -0.00244612, -0.00243704, -0.002428, -0.00241899, -0.00241001, -0.00240107,
                        -0.00239216, -0.00238328, -0.00237443, -0.00236562, -0.00235684, -0.00234809, -0.00233937, -0.00233068, -0.00232203, -0.00231341,
                        -0.00230482, -0.00229625, -0.00228773, -0.00227923, -0.00227076, -0.00226232, -0.00225392, -0.00224554, -0.0022372, -0.00222888,
                        -0.0022206, -0.00221234, -0.00220412, -0.00219593, -0.00218776, -0.00217963, -0.00217152, -0.00216345, -0.0021554, -0.00214738,
                        -0.00213939, -0.00213144, -0.00212351, -0.00211561, -0.00210773, -0.00209989, -0.00209207, -0.00208429, -0.00207653, -0.0020688,
                        -0.0020611, -0.00205343, -0.00204578, -0.00203816, -0.00203057, -0.00202301, -0.00201548, -0.00200797, -0.00200049, -0.00199304,
                        -0.00198561, -0.00197821, -0.00197084, -0.0019635, -0.00195618, -0.00194889, -0.00194163, -0.00193439, -0.00192718, -0.00191999,
                        -0.00191283, -0.0019057, -0.0018986, -0.00189152, -0.00188446, -0.00187744, -0.00187043, -0.00186346, -0.00185651, -0.00184958,
                        -0.00184268, -0.00183581, -0.00182896, -0.00182213, -0.00181533, -0.00180856, -0.00180181, -0.00179508, -0.00178839, -0.00178171,
                        -0.00177506, -0.00176843, -0.00176183, -0.00175526, -0.0017487, -0.00174217, -0.00173567, -0.00172919, -0.00172273, -0.0017163,
                        -0.00170989, -0.0017035, -0.00169714, -0.0016908, -0.00168449, -0.0016782, -0.00167193, -0.00166568, -0.00165946, -0.00165326,
                        -0.00164709, -0.00164093, -0.0016348, -0.0016287, -0.00162261, -0.00161655, -0.00161051, -0.00160449, -0.0015985, -0.00159252,
                        -0.00158657, -0.00158064, -0.00157474, -0.00156885, -0.00156299, -0.00155715, -0.00155133, -0.00154553, -0.00153976, -0.001534,
                        -0.00152827, -0.00152256, -0.00151687, -0.0015112, -0.00150555, -0.00149992, -0.00149432, -0.00148873, -0.00148316, -0.00147762,
                        -0.0014721, -0.00146659, -0.00146111, -0.00145565, -0.00145021, -0.00144479, -0.00143939, -0.00143401, -0.00142865, -0.00142331,
                        -0.00141798, -0.00141268, -0.0014074, -0.00140214, -0.0013969, -0.00139168, -0.00138647, -0.00138129, -0.00137613, -0.00137098,
                        -0.00136586, -0.00136075, -0.00135566, -0.00135059, -0.00134554, -0.00134051, -0.0013355, -0.00133051, -0.00132553, -0.00132058,
                        -0.00131564, -0.00131072, -0.00130582, -0.00130094, -0.00129607, -0.00129123, -0.0012864, -0.00128159, -0.0012768, -0.00127203,
                        -0.00126727, -0.00126253, -0.00125781, -0.00125311, -0.00124842, -0.00124375, -0.0012391, -0.00123447, -0.00122985, -0.00122526,
                        -0.00122067, -0.00121611, -0.00121156, -0.00120703, -0.00120252, -0.00119802, -0.00119354, -0.00118908, -0.00118463, -0.00118021,
                        -0.00117579, -0.0011714, -0.00116702, -0.00116265, -0.0011583, -0.00115397, -0.00114966, -0.00114536, -0.00114108, -0.00113681,
                        -0.00113256, -0.00112832, -0.0011241, -0.0011199, -0.00111571, -0.00111154, -0.00110738, -0.00110324, -0.00109912, -0.00109501,
                        -0.00109091, -0.00108683, -0.00108277, -0.00107872, -0.00107469, -0.00107067, -0.00106666, -0.00106268, -0.0010587, -0.00105474,
                        -0.0010508, -0.00104687, -0.00104295, -0.00103905, -0.00103517, -0.0010313, -0.00102744, -0.0010236, -0.00101977, -0.00101596,
                        -0.00101216, -0.00100837, -0.0010046, -0.00100085, -0.000997103, -0.000993374, -0.000989659, -0.000985958, -0.000982271, -0.000978597,
                        -0.000974938, -0.000971292, -0.000967659, -0.000964041, -0.000960435, -0.000956843, -0.000953265, -0.0009497, -0.000946148, -0.00094261,
                        -0.000939085, -0.000935573, -0.000932074, -0.000928588, -0.000925115, -0.000921656, -0.000918209, -0.000914775, -0.000911354, -0.000907945,
                        -0.00090455, -0.000901167, -0.000897796, -0.000894439, -0.000891094, -0.000887761, -0.000884441, -0.000881133, -0.000877838, -0.000874555,
                        -0.000871284, -0.000868025, -0.000864779, -0.000861545, -0.000858322, -0.000855112, -0.000851914, -0.000848728, -0.000845554, -0.000842391,
                        -0.000839241, -0.000836102, -0.000832975, -0.000829859, -0.000826756, -0.000823664, -0.000820583, -0.000817514, -0.000814456, -0.00081141,
                        -0.000808376, -0.000805352, -0.00080234, -0.000799339, -0.00079635, -0.000793371, -0.000790404, -0.000787448, -0.000784503, -0.000781568,
                        -0.000778645, -0.000775733, -0.000772832, -0.000769941, -0.000767062, -0.000764193, -0.000761334, -0.000758487, -0.00075565, -0.000752824,
                        -0.000750008, -0.000747203, -0.000744408, -0.000741624, -0.00073885, -0.000736087, -0.000733334, -0.000730591, -0.000727858, -0.000725136,
                        -0.000722424, -0.000719722, -0.00071703, -0.000714348, -0.000711676, -0.000709015, -0.000706363, -0.000703721, -0.000701089, -0.000698467,
                        -0.000695854, -0.000693252, -0.000690659, -0.000688075, -0.000685502, -0.000682938, -0.000680384, -0.000677839, -0.000675304, -0.000672778,
                        -0.000670261, -0.000667755, -0.000665257, -0.000662769, -0.00066029, -0.00065782, -0.00065536, -0.000652909, -0.000650467, -0.000648034,
                        -0.00064561, -0.000643195, -0.00064079, -0.000638393, -0.000636005, -0.000633626, -0.000631256, -0.000628895, -0.000626543, -0.0006242,
                        -0.000621865, -0.000619539, -0.000617222, -0.000614913, -0.000612613, -0.000610322, -0.000608039, -0.000605765, -0.0006035, -0.000601242,
                        -0.000598993, -0.000596753, -0.000594521, -0.000592297, -0.000590082, -0.000587875, -0.000585676, -0.000583486, -0.000581303, -0.000579129,
                        -0.000576963, -0.000574805, -0.000572655, -0.000570513, -0.000568379, -0.000566254, -0.000564136, -0.000562026, -0.000559924, -0.000557829,
                        -0.000555743, -0.000553664, -0.000551593, -0.00054953, -0.000547475, -0.000545427, -0.000543387, -0.000541355, -0.00053933, -0.000537313,
                        -0.000535303, -0.000533301, -0.000531306, -0.000529319, -0.000527339, -0.000525367, -0.000523402, -0.000521444, -0.000519494, -0.000517551,
                        -0.000515615, -0.000513687, -0.000511765, -0.000509851, -0.000507944, -0.000506044, -0.000504152, -0.000502266, -0.000500387, -0.000498516,
                        -0.000496651, -0.000494793, -0.000492943, -0.000491099, -0.000489262, -0.000487432, -0.000485609, -0.000483793, -0.000481983, -0.000480181,
                        -0.000478385, -0.000476595, -0.000474813, -0.000473037, -0.000471268, -0.000469505, -0.000467749, -0.000465999, -0.000464256, -0.00046252,
                        -0.00046079, -0.000459066, -0.000457349, -0.000455639, -0.000453935, -0.000452237, -0.000450545, -0.00044886, -0.000447181, -0.000445509,
                        -0.000443842, -0.000442182, -0.000440528, -0.000438881, -0.000437239, -0.000435604, -0.000433975, -0.000432351, -0.000430734, -0.000429123,
                        -0.000427518, -0.000425919, -0.000424326, -0.000422739, -0.000421158, -0.000419583, -0.000418013, -0.00041645, -0.000414892, -0.00041334
                       };


  int par_f[192][16] = {
    { 0, 0, 1, 541, 5103, 12632, 16362, 14163, 9144, 4693, 2061, 904, 479, 288, 145, 29},
    { 0, 0, 1, 561, 5177, 12700, 16368, 14118, 9091, 4656, 2043, 897, 476, 286, 144, 28},
    { 0, 0, 1, 581, 5251, 12768, 16372, 14073, 9037, 4619, 2025, 890, 473, 285, 142, 27},
    { 0, 0, 2, 602, 5326, 12836, 16376, 14027, 8984, 4583, 2007, 883, 471, 283, 141, 26},
    { 0, 0, 2, 623, 5401, 12903, 16379, 13982, 8931, 4546, 1989, 876, 468, 281, 140, 25},
    { 0, 0, 2, 645, 5476, 12969, 16381, 13935, 8878, 4510, 1971, 869, 466, 280, 138, 24},
    { 0, 0, 2, 667, 5552, 13035, 16383, 13889, 8825, 4474, 1953, 863, 463, 278, 137, 23},
    { 0, 0, 2, 690, 5628, 13101, 16384, 13842, 8772, 4438, 1936, 856, 461, 277, 136, 22},
    { 0, 0, 3, 713, 5705, 13166, 16384, 13796, 8719, 4402, 1919, 849, 458, 275, 134, 21},
    { 0, 0, 3, 737, 5781, 13230, 16384, 13748, 8667, 4367, 1902, 843, 456, 273, 133, 20},
    { 0, 0, 3, 762, 5859, 13294, 16383, 13701, 8614, 4332, 1885, 837, 453, 272, 132, 19},
    { 0, 0, 4, 787, 5936, 13358, 16381, 13653, 8562, 4297, 1868, 830, 451, 270, 130, 18},
    { 0, 0, 4, 812, 6014, 13421, 16378, 13605, 8509, 4262, 1851, 824, 448, 269, 129, 17},
    { 0, 0, 4, 838, 6092, 13483, 16375, 13557, 8457, 4227, 1835, 818, 446, 267, 128, 16},
    { 0, 0, 5, 865, 6170, 13545, 16372, 13509, 8405, 4193, 1819, 812, 444, 266, 126, 15},
    { 0, 0, 5, 893, 6248, 13606, 16367, 13460, 8353, 4159, 1802, 806, 441, 264, 125, 14},
    { 0, 0, 6, 920, 6327, 13667, 16362, 13411, 8301, 4125, 1786, 800, 439, 262, 124, 13},
    { 0, 0, 6, 949, 6406, 13727, 16356, 13362, 8249, 4091, 1770, 794, 436, 261, 122, 12},
    { 0, 0, 7, 978, 6486, 13786, 16350, 13313, 8198, 4058, 1755, 788, 434, 259, 121, 11},
    { 0, 0, 7, 1008, 6565, 13845, 16343, 13263, 8146, 4024, 1739, 782, 432, 258, 120, 10},
    { 0, 0, 8, 1038, 6645, 13903, 16335, 13213, 8095, 3991, 1724, 777, 430, 256, 118, 9},
    { 0, 0, 8, 1069, 6725, 13961, 16327, 13163, 8043, 3958, 1708, 771, 427, 255, 117, 8},
    { 0, 0, 9, 1100, 6805, 14018, 16318, 13113, 7992, 3926, 1693, 765, 425, 253, 116, 7},
    { 0, 0, 10, 1132, 6886, 14075, 16308, 13063, 7941, 3893, 1678, 760, 423, 252, 114, 6},
    { 0, 0, 11, 1165, 6966, 14131, 16298, 13012, 7891, 3861, 1663, 754, 421, 250, 113, 6},
    { 0, 0, 12, 1198, 7047, 14186, 16287, 12961, 7840, 3829, 1649, 749, 418, 248, 112, 5},
    { 0, 0, 12, 1232, 7128, 14241, 16276, 12910, 7789, 3797, 1634, 744, 416, 247, 110, 4},
    { 0, 0, 13, 1266, 7209, 14295, 16264, 12859, 7739, 3766, 1620, 738, 414, 245, 109, 3},
    { 0, 0, 14, 1301, 7290, 14348, 16251, 12808, 7689, 3734, 1605, 733, 412, 244, 108, 2},
    { 0, 0, 16, 1337, 7371, 14401, 16238, 12757, 7639, 3703, 1591, 728, 410, 242, 107, 1},
    { 0, 0, 17, 1373, 7453, 14453, 16224, 12705, 7589, 3672, 1577, 723, 408, 241, 105, 0},
    { 0, 0, 18, 1410, 7534, 14504, 16209, 12653, 7539, 3641, 1563, 718, 406, 239, 104, -1},
    { 0, 0, 19, 1448, 7616, 14555, 16194, 12601, 7489, 3611, 1549, 713, 403, 238, 103, -2},
    { 0, 0, 21, 1486, 7698, 14605, 16178, 12549, 7440, 3580, 1536, 708, 401, 236, 101, -2},
    { 0, 0, 22, 1525, 7780, 14655, 16162, 12497, 7390, 3550, 1522, 703, 399, 235, 100, -3},
    { 0, 0, 24, 1564, 7862, 14704, 16145, 12445, 7341, 3520, 1509, 698, 397, 233, 99, -4},
    { 0, 0, 26, 1605, 7944, 14752, 16127, 12392, 7292, 3490, 1495, 693, 395, 232, 98, -5},
    { 0, 0, 28, 1645, 8026, 14800, 16109, 12339, 7243, 3461, 1482, 688, 393, 230, 96, -6},
    { 0, 0, 29, 1687, 8108, 14847, 16091, 12287, 7195, 3432, 1469, 684, 391, 229, 95, -7},
    { 0, 0, 32, 1729, 8190, 14893, 16071, 12234, 7146, 3403, 1456, 679, 389, 227, 94, -8},
    { 0, 0, 34, 1771, 8273, 14938, 16051, 12181, 7098, 3374, 1444, 674, 387, 226, 93, -8},
    { 0, 0, 36, 1814, 8355, 14983, 16031, 12128, 7050, 3345, 1431, 670, 385, 224, 91, -9},
    { 0, 0, 38, 1858, 8437, 15027, 16010, 12074, 7002, 3316, 1419, 665, 383, 223, 90, -10},
    { 0, 0, 41, 1903, 8519, 15071, 15988, 12021, 6954, 3288, 1406, 661, 381, 221, 89, -11},
    { 0, 0, 44, 1948, 8602, 15114, 15966, 11968, 6906, 3260, 1394, 656, 379, 220, 88, -12},
    { 0, 0, 47, 1994, 8684, 15156, 15944, 11914, 6859, 3232, 1382, 652, 377, 218, 86, -13},
    { 0, 0, 49, 2040, 8766, 15197, 15920, 11860, 6811, 3204, 1370, 648, 375, 217, 85, -13},
    { 0, 0, 53, 2087, 8848, 15238, 15897, 11807, 6764, 3177, 1358, 643, 373, 215, 84, -14},
    { 0, 0, 56, 2135, 8930, 15278, 15872, 11753, 6717, 3150, 1346, 639, 371, 214, 83, -15},
    { 0, 0, 59, 2183, 9013, 15318, 15848, 11699, 6671, 3122, 1335, 635, 369, 212, 82, -16},
    { 0, 0, 63, 2232, 9095, 15356, 15822, 11645, 6624, 3096, 1323, 631, 368, 211, 80, -17},
    { 0, 0, 67, 2282, 9176, 15394, 15796, 11591, 6578, 3069, 1312, 627, 366, 209, 79, -17},
    { 0, 0, 71, 2332, 9258, 15432, 15770, 11537, 6531, 3042, 1300, 623, 364, 208, 78, -18},
    { 0, 0, 75, 2383, 9340, 15468, 15743, 11483, 6485, 3016, 1289, 619, 362, 206, 77, -19},
    { 0, 0, 80, 2434, 9422, 15504, 15715, 11428, 6440, 2990, 1278, 615, 360, 205, 76, -20},
    { 0, 0, 84, 2486, 9503, 15539, 15688, 11374, 6394, 2964, 1267, 611, 358, 203, 74, -20},
    { 0, 0, 89, 2539, 9585, 15574, 15659, 11320, 6348, 2938, 1256, 607, 356, 202, 73, -21},
    { 0, 0, 94, 2592, 9666, 15607, 15630, 11265, 6303, 2913, 1246, 603, 354, 200, 72, -22},
    { 0, 0, 100, 2646, 9747, 15640, 15601, 11211, 6258, 2887, 1235, 599, 353, 199, 71, -23},
    { 0, 0, 105, 2700, 9828, 15673, 15571, 11156, 6213, 2862, 1224, 595, 351, 198, 70, -24},
    { 0, 0, 111, 2755, 9909, 15704, 15540, 11102, 6168, 2837, 1214, 592, 349, 196, 68, -24},
    { 0, 0, 117, 2811, 9989, 15735, 15509, 11047, 6124, 2813, 1204, 588, 347, 195, 67, -25},
    { 0, 0, 123, 2867, 10070, 15765, 15478, 10993, 6080, 2788, 1193, 584, 345, 193, 66, -26},
    { 0, 0, 130, 2924, 10150, 15795, 15446, 10938, 6035, 2764, 1183, 581, 344, 192, 65, -26},
    { 0, 0, 137, 2982, 10230, 15823, 15414, 10883, 5991, 2739, 1173, 577, 342, 190, 64, -27},
    { 0, 0, 144, 3040, 10310, 15851, 15381, 10829, 5948, 2715, 1164, 573, 340, 189, 63, -28},
    { 0, 0, 151, 3098, 10389, 15879, 15348, 10774, 5904, 2692, 1154, 570, 338, 187, 61, -29},
    { 0, 0, 159, 3158, 10469, 15905, 15314, 10719, 5861, 2668, 1144, 566, 337, 186, 60, -29},
    { 0, 0, 167, 3217, 10548, 15931, 15280, 10665, 5818, 2645, 1134, 563, 335, 185, 59, -30},
    { 0, 0, 175, 3278, 10627, 15956, 15245, 10610, 5775, 2621, 1125, 560, 333, 183, 58, -31},
    { 0, 0, 184, 3339, 10705, 15981, 15210, 10555, 5732, 2598, 1116, 556, 331, 182, 57, -31},
    { 0, 0, 193, 3400, 10784, 16004, 15175, 10501, 5690, 2575, 1106, 553, 330, 180, 56, -32},
    { 0, 0, 202, 3462, 10862, 16027, 15139, 10446, 5647, 2553, 1097, 550, 328, 179, 55, -33},
    { 0, 0, 211, 3525, 10940, 16049, 15103, 10391, 5605, 2530, 1088, 546, 326, 177, 53, -34},
    { 0, 0, 221, 3588, 11017, 16071, 15066, 10337, 5563, 2508, 1079, 543, 324, 176, 52, -34},
    { 0, 0, 232, 3651, 11094, 16092, 15029, 10282, 5521, 2486, 1070, 540, 323, 175, 51, -35},
    { 0, 0, 242, 3716, 11171, 16112, 14992, 10227, 5480, 2464, 1061, 537, 321, 173, 50, -36},
    { 0, 0, 253, 3780, 11248, 16131, 14954, 10173, 5439, 2442, 1053, 533, 319, 172, 49, -36},
    { 0, 0, 265, 3846, 11324, 16150, 14915, 10118, 5397, 2420, 1044, 530, 318, 170, 48, -37},
    { 0, 0, 276, 3911, 11400, 16168, 14877, 10064, 5357, 2399, 1035, 527, 316, 169, 47, -38},
    { 0, 0, 288, 3978, 11476, 16185, 14837, 10009, 5316, 2378, 1027, 524, 314, 167, 46, -38},
    { 0, 0, 301, 4045, 11551, 16201, 14798, 9955, 5275, 2357, 1019, 521, 312, 166, 45, -39},
    { 0, 0, 314, 4112, 11626, 16217, 14758, 9900, 5235, 2336, 1010, 518, 311, 165, 44, -39},
    { 0, 0, 327, 4180, 11700, 16232, 14718, 9846, 5195, 2315, 1002, 515, 309, 163, 42, -40},
    { 0, 0, 341, 4248, 11774, 16246, 14677, 9792, 5155, 2295, 994, 512, 307, 162, 41, -41},
    { 0, 0, 355, 4317, 11848, 16260, 14636, 9737, 5116, 2274, 986, 509, 306, 160, 40, -41},
    { 0, 0, 370, 4386, 11921, 16273, 14595, 9683, 5076, 2254, 978, 506, 304, 159, 39, -42},
    { 0, 0, 385, 4456, 11994, 16285, 14553, 9629, 5037, 2234, 970, 503, 302, 158, 38, -43},
    { 0, 0, 400, 4526, 12067, 16296, 14511, 9575, 4998, 2214, 963, 501, 301, 156, 37, -43},
    { 0, 0, 416, 4596, 12139, 16307, 14469, 9521, 4959, 2194, 955, 498, 299, 155, 36, -44},
    { 0, 1, 433, 4667, 12211, 16317, 14426, 9467, 4920, 2175, 947, 495, 298, 153, 35, -44},
    { 0, 1, 449, 4739, 12282, 16326, 14383, 9413, 4882, 2156, 940, 492, 296, 152, 34, -45},
    { 0, 1, 467, 4811, 12353, 16335, 14340, 9359, 4844, 2136, 932, 489, 294, 151, 33, -46},
    { 0, 1, 485, 4883, 12423, 16343, 14296, 9305, 4806, 2117, 925, 487, 293, 149, 32, -46},
    { 0, 1, 503, 4956, 12493, 16350, 14252, 9251, 4768, 2098, 918, 484, 291, 148, 31, -47},
    { 0, 1, 522, 5029, 12563, 16357, 14208, 9198, 4731, 2080, 911, 481, 289, 147, 30, -47},
    { 0, 1, 541, 5103, 12632, 16362, 14163, 9144, 4693, 2061, 904, 479, 288, 145, 29, -48},
    { 0, 1, 561, 5177, 12700, 16368, 14118, 9091, 4656, 2043, 897, 476, 286, 144, 28, -49},
    { 0, 1, 581, 5251, 12768, 16372, 14073, 9037, 4619, 2025, 890, 473, 285, 142, 27, -49},
    { 0, 2, 602, 5326, 12836, 16376, 14027, 8984, 4583, 2007, 883, 471, 283, 141, 26, -50},
    { 0, 2, 623, 5401, 12903, 16379, 13982, 8931, 4546, 1989, 876, 468, 281, 140, 25, -50},
    { 0, 2, 645, 5476, 12969, 16381, 13935, 8878, 4510, 1971, 869, 466, 280, 138, 24, -51},
    { 0, 2, 667, 5552, 13035, 16383, 13889, 8825, 4474, 1953, 863, 463, 278, 137, 23, -51},
    { 0, 2, 690, 5628, 13101, 16384, 13842, 8772, 4438, 1936, 856, 461, 277, 136, 22, -52},
    { 0, 3, 713, 5705, 13166, 16384, 13796, 8719, 4402, 1919, 849, 458, 275, 134, 21, -52},
    { 0, 3, 737, 5781, 13230, 16384, 13748, 8667, 4367, 1902, 843, 456, 273, 133, 20, -53},
    { 0, 3, 762, 5859, 13294, 16383, 13701, 8614, 4332, 1885, 837, 453, 272, 132, 19, -54},
    { 0, 4, 787, 5936, 13358, 16381, 13653, 8562, 4297, 1868, 830, 451, 270, 130, 18, -54},
    { 0, 4, 812, 6014, 13421, 16378, 13605, 8509, 4262, 1851, 824, 448, 269, 129, 17, -55},
    { 0, 4, 838, 6092, 13483, 16375, 13557, 8457, 4227, 1835, 818, 446, 267, 128, 16, -55},
    { 0, 5, 865, 6170, 13545, 16372, 13509, 8405, 4193, 1819, 812, 444, 266, 126, 15, -56},
    { 0, 5, 893, 6248, 13606, 16367, 13460, 8353, 4159, 1802, 806, 441, 264, 125, 14, -56},
    { 0, 6, 920, 6327, 13667, 16362, 13411, 8301, 4125, 1786, 800, 439, 262, 124, 13, -57},
    { 0, 6, 949, 6406, 13727, 16356, 13362, 8249, 4091, 1770, 794, 436, 261, 122, 12, -57},
    { 0, 7, 978, 6486, 13786, 16350, 13313, 8198, 4058, 1755, 788, 434, 259, 121, 11, -58},
    { 0, 7, 1008, 6565, 13845, 16343, 13263, 8146, 4024, 1739, 782, 432, 258, 120, 10, -58},
    { 0, 8, 1038, 6645, 13903, 16335, 13213, 8095, 3991, 1724, 777, 430, 256, 118, 9, -59},
    { 0, 8, 1069, 6725, 13961, 16327, 13163, 8043, 3958, 1708, 771, 427, 255, 117, 8, -59},
    { 0, 9, 1100, 6805, 14018, 16318, 13113, 7992, 3926, 1693, 765, 425, 253, 116, 7, -60},
    { 0, 10, 1132, 6886, 14075, 16308, 13063, 7941, 3893, 1678, 760, 423, 252, 114, 6, -60},
    { 0, 11, 1165, 6966, 14131, 16298, 13012, 7891, 3861, 1663, 754, 421, 250, 113, 6, -61},
    { 0, 12, 1198, 7047, 14186, 16287, 12961, 7840, 3829, 1649, 749, 418, 248, 112, 5, -61},
    { 0, 12, 1232, 7128, 14241, 16276, 12910, 7789, 3797, 1634, 744, 416, 247, 110, 4, -62},
    { 0, 13, 1266, 7209, 14295, 16264, 12859, 7739, 3766, 1620, 738, 414, 245, 109, 3, -62},
    { 0, 14, 1301, 7290, 14348, 16251, 12808, 7689, 3734, 1605, 733, 412, 244, 108, 2, -62},
    { 0, 16, 1337, 7371, 14401, 16238, 12757, 7639, 3703, 1591, 728, 410, 242, 107, 1, -63},
    { 0, 17, 1373, 7453, 14453, 16224, 12705, 7589, 3672, 1577, 723, 408, 241, 105, 0, -63},
    { 0, 18, 1410, 7534, 14504, 16209, 12653, 7539, 3641, 1563, 718, 406, 239, 104, -1, -64},
    { 0, 19, 1448, 7616, 14555, 16194, 12601, 7489, 3611, 1549, 713, 403, 238, 103, -2, -64},
    { 0, 21, 1486, 7698, 14605, 16178, 12549, 7440, 3580, 1536, 708, 401, 236, 101, -2, -65},
    { 0, 22, 1525, 7780, 14655, 16162, 12497, 7390, 3550, 1522, 703, 399, 235, 100, -3, -65},
    { 0, 24, 1564, 7862, 14704, 16145, 12445, 7341, 3520, 1509, 698, 397, 233, 99, -4, -66},
    { 0, 26, 1605, 7944, 14752, 16127, 12392, 7292, 3490, 1495, 693, 395, 232, 98, -5, -66},
    { 0, 28, 1645, 8026, 14800, 16109, 12339, 7243, 3461, 1482, 688, 393, 230, 96, -6, -66},
    { 0, 29, 1687, 8108, 14847, 16091, 12287, 7195, 3432, 1469, 684, 391, 229, 95, -7, -67},
    { 0, 32, 1729, 8190, 14893, 16071, 12234, 7146, 3403, 1456, 679, 389, 227, 94, -8, -67},
    { 0, 34, 1771, 8273, 14938, 16051, 12181, 7098, 3374, 1444, 674, 387, 226, 93, -8, -68},
    { 0, 36, 1814, 8355, 14983, 16031, 12128, 7050, 3345, 1431, 670, 385, 224, 91, -9, -68},
    { 0, 38, 1858, 8437, 15027, 16010, 12074, 7002, 3316, 1419, 665, 383, 223, 90, -10, -69},
    { 0, 41, 1903, 8519, 15071, 15988, 12021, 6954, 3288, 1406, 661, 381, 221, 89, -11, -69},
    { 0, 44, 1948, 8602, 15114, 15966, 11968, 6906, 3260, 1394, 656, 379, 220, 88, -12, -69},
    { 0, 47, 1994, 8684, 15156, 15944, 11914, 6859, 3232, 1382, 652, 377, 218, 86, -13, -70},
    { 0, 49, 2040, 8766, 15197, 15920, 11860, 6811, 3204, 1370, 648, 375, 217, 85, -13, -70},
    { 0, 53, 2087, 8848, 15238, 15897, 11807, 6764, 3177, 1358, 643, 373, 215, 84, -14, -70},
    { 0, 56, 2135, 8930, 15278, 15872, 11753, 6717, 3150, 1346, 639, 371, 214, 83, -15, -71},
    { 0, 59, 2183, 9013, 15318, 15848, 11699, 6671, 3122, 1335, 635, 369, 212, 82, -16, -71},
    { 0, 63, 2232, 9095, 15356, 15822, 11645, 6624, 3096, 1323, 631, 368, 211, 80, -17, -72},
    { 0, 67, 2282, 9176, 15394, 15796, 11591, 6578, 3069, 1312, 627, 366, 209, 79, -17, -72},
    { 0, 71, 2332, 9258, 15432, 15770, 11537, 6531, 3042, 1300, 623, 364, 208, 78, -18, -72},
    { 0, 75, 2383, 9340, 15468, 15743, 11483, 6485, 3016, 1289, 619, 362, 206, 77, -19, -73},
    { 0, 80, 2434, 9422, 15504, 15715, 11428, 6440, 2990, 1278, 615, 360, 205, 76, -20, -73},
    { 0, 84, 2486, 9503, 15539, 15688, 11374, 6394, 2964, 1267, 611, 358, 203, 74, -20, -73},
    { 0, 89, 2539, 9585, 15574, 15659, 11320, 6348, 2938, 1256, 607, 356, 202, 73, -21, -74},
    { 0, 94, 2592, 9666, 15607, 15630, 11265, 6303, 2913, 1246, 603, 354, 200, 72, -22, -74},
    { 0, 100, 2646, 9747, 15640, 15601, 11211, 6258, 2887, 1235, 599, 353, 199, 71, -23, -74},
    { 0, 105, 2700, 9828, 15673, 15571, 11156, 6213, 2862, 1224, 595, 351, 198, 70, -24, -75},
    { 0, 111, 2755, 9909, 15704, 15540, 11102, 6168, 2837, 1214, 592, 349, 196, 68, -24, -75},
    { 0, 117, 2811, 9989, 15735, 15509, 11047, 6124, 2813, 1204, 588, 347, 195, 67, -25, -75},
    { 0, 123, 2867, 10070, 15765, 15478, 10993, 6080, 2788, 1193, 584, 345, 193, 66, -26, -76},
    { 0, 130, 2924, 10150, 15795, 15446, 10938, 6035, 2764, 1183, 581, 344, 192, 65, -26, -76},
    { 0, 137, 2982, 10230, 15823, 15414, 10883, 5991, 2739, 1173, 577, 342, 190, 64, -27, -76},
    { 0, 144, 3040, 10310, 15851, 15381, 10829, 5948, 2715, 1164, 573, 340, 189, 63, -28, -77},
    { 0, 151, 3098, 10389, 15879, 15348, 10774, 5904, 2692, 1154, 570, 338, 187, 61, -29, -77},
    { 0, 159, 3158, 10469, 15905, 15314, 10719, 5861, 2668, 1144, 566, 337, 186, 60, -29, -77},
    { 0, 167, 3217, 10548, 15931, 15280, 10665, 5818, 2645, 1134, 563, 335, 185, 59, -30, -78},
    { 0, 175, 3278, 10627, 15956, 15245, 10610, 5775, 2621, 1125, 560, 333, 183, 58, -31, -78},
    { 0, 184, 3339, 10705, 15981, 15210, 10555, 5732, 2598, 1116, 556, 331, 182, 57, -31, -78},
    { 0, 193, 3400, 10784, 16004, 15175, 10501, 5690, 2575, 1106, 553, 330, 180, 56, -32, -79},
    { 0, 202, 3462, 10862, 16027, 15139, 10446, 5647, 2553, 1097, 550, 328, 179, 55, -33, -79},
    { 0, 211, 3525, 10940, 16049, 15103, 10391, 5605, 2530, 1088, 546, 326, 177, 53, -34, -79},
    { 0, 221, 3588, 11017, 16071, 15066, 10337, 5563, 2508, 1079, 543, 324, 176, 52, -34, -80},
    { 0, 232, 3651, 11094, 16092, 15029, 10282, 5521, 2486, 1070, 540, 323, 175, 51, -35, -80},
    { 0, 242, 3716, 11171, 16112, 14992, 10227, 5480, 2464, 1061, 537, 321, 173, 50, -36, -80},
    { 0, 253, 3780, 11248, 16131, 14954, 10173, 5439, 2442, 1053, 533, 319, 172, 49, -36, -80},
    { 0, 265, 3846, 11324, 16150, 14915, 10118, 5397, 2420, 1044, 530, 318, 170, 48, -37, -81},
    { 0, 276, 3911, 11400, 16168, 14877, 10064, 5357, 2399, 1035, 527, 316, 169, 47, -38, -81},
    { 0, 288, 3978, 11476, 16185, 14837, 10009, 5316, 2378, 1027, 524, 314, 167, 46, -38, -81},
    { 0, 301, 4045, 11551, 16201, 14798, 9955, 5275, 2357, 1019, 521, 312, 166, 45, -39, -81},
    { 0, 314, 4112, 11626, 16217, 14758, 9900, 5235, 2336, 1010, 518, 311, 165, 44, -39, -82},
    { 0, 327, 4180, 11700, 16232, 14718, 9846, 5195, 2315, 1002, 515, 309, 163, 42, -40, -82},
    { 0, 341, 4248, 11774, 16246, 14677, 9792, 5155, 2295, 994, 512, 307, 162, 41, -41, -82},
    { 0, 355, 4317, 11848, 16260, 14636, 9737, 5116, 2274, 986, 509, 306, 160, 40, -41, -83},
    { 0, 370, 4386, 11921, 16273, 14595, 9683, 5076, 2254, 978, 506, 304, 159, 39, -42, -83},
    { 0, 385, 4456, 11994, 16285, 14553, 9629, 5037, 2234, 970, 503, 302, 158, 38, -43, -83},
    { 0, 400, 4526, 12067, 16296, 14511, 9575, 4998, 2214, 963, 501, 301, 156, 37, -43, -83},
    { 0, 416, 4596, 12139, 16307, 14469, 9521, 4959, 2194, 955, 498, 299, 155, 36, -44, -84},
    { 0, 433, 4667, 12211, 16317, 14426, 9467, 4920, 2175, 947, 495, 298, 153, 35, -44, -84},
    { 0, 449, 4739, 12282, 16326, 14383, 9413, 4882, 2156, 940, 492, 296, 152, 34, -45, -84},
    { 0, 467, 4811, 12353, 16335, 14340, 9359, 4844, 2136, 932, 489, 294, 151, 33, -46, -84},
    { 0, 485, 4883, 12423, 16343, 14296, 9305, 4806, 2117, 925, 487, 293, 149, 32, -46, -84},
    { 0, 503, 4956, 12493, 16350, 14252, 9251, 4768, 2098, 918, 484, 291, 148, 31, -47, -85},
    { 0, 522, 5029, 12563, 16357, 14208, 9198, 4731, 2080, 911, 481, 289, 147, 30, -47, -85}
  };
  int par_f1[192][16] = {
    { 0, 0, 26, 3743, 14162, 13203, 1048, -8599, -10272, -7147, -3541, -1357, -509, -310, -263, -196},
    { 0, 0, 29, 3839, 14237, 13111, 914, -8657, -10255, -7106, -3510, -1342, -505, -309, -263, -195},
    { 0, 0, 31, 3937, 14309, 13018, 781, -8713, -10238, -7065, -3479, -1328, -501, -308, -262, -195},
    { 0, 0, 34, 4035, 14381, 12924, 648, -8769, -10221, -7025, -3448, -1314, -497, -308, -262, -194},
    { 0, 0, 37, 4135, 14450, 12828, 516, -8824, -10202, -6984, -3418, -1299, -493, -307, -261, -193},
    { 0, 0, 40, 4236, 14518, 12731, 384, -8878, -10184, -6943, -3387, -1285, -489, -306, -261, -192},
    { 0, 0, 43, 4338, 14585, 12633, 252, -8931, -10165, -6902, -3357, -1272, -485, -306, -260, -191},
    { 0, 0, 47, 4441, 14649, 12534, 121, -8983, -10145, -6861, -3327, -1258, -481, -305, -260, -190},
    { 0, 0, 51, 4545, 14713, 12433, -9, -9035, -10125, -6821, -3297, -1244, -477, -304, -259, -189},
    { 0, 0, 55, 4650, 14774, 12331, -139, -9085, -10105, -6780, -3267, -1231, -473, -304, -259, -189},
    { 0, 0, 59, 4756, 14834, 12228, -269, -9134, -10084, -6739, -3238, -1217, -470, -303, -258, -188},
    { 0, 0, 63, 4863, 14892, 12124, -398, -9182, -10062, -6698, -3208, -1204, -466, -303, -258, -187},
    { 0, 0, 68, 4971, 14948, 12019, -526, -9230, -10040, -6657, -3179, -1191, -462, -302, -257, -186},
    { 0, 0, 73, 5079, 15003, 11912, -654, -9276, -10018, -6616, -3150, -1178, -459, -301, -256, -185},
    { 0, 0, 79, 5189, 15056, 11805, -782, -9322, -9995, -6576, -3121, -1166, -456, -301, -256, -184},
    { 0, 0, 85, 5299, 15107, 11696, -909, -9366, -9972, -6535, -3093, -1153, -452, -300, -255, -183},
    { 0, 0, 91, 5410, 15156, 11586, -1035, -9410, -9948, -6494, -3064, -1140, -449, -300, -255, -182},
    { 0, 0, 98, 5522, 15204, 11476, -1161, -9453, -9924, -6454, -3036, -1128, -446, -299, -254, -181},
    { 0, 0, 105, 5635, 15250, 11364, -1286, -9495, -9900, -6413, -3008, -1116, -442, -299, -254, -181},
    { 0, 0, 113, 5748, 15294, 11251, -1410, -9536, -9875, -6372, -2980, -1104, -439, -298, -253, -180},
    { 0, 0, 121, 5862, 15336, 11137, -1534, -9576, -9849, -6332, -2952, -1092, -436, -297, -252, -179},
    { 0, 0, 130, 5977, 15376, 11022, -1658, -9615, -9824, -6291, -2924, -1080, -433, -297, -252, -178},
    { 0, 0, 139, 6093, 15415, 10907, -1780, -9653, -9798, -6251, -2897, -1068, -430, -296, -251, -177},
    { 0, 0, 149, 6209, 15452, 10790, -1902, -9690, -9771, -6210, -2870, -1057, -427, -296, -251, -176},
    { 0, 0, 159, 6325, 15487, 10673, -2024, -9727, -9744, -6170, -2843, -1045, -424, -295, -250, -175},
    { 0, 0, 170, 6442, 15520, 10554, -2144, -9762, -9717, -6129, -2816, -1034, -421, -295, -249, -174},
    { 0, 0, 182, 6560, 15551, 10435, -2264, -9797, -9689, -6089, -2789, -1023, -418, -294, -249, -173},
    { 0, 0, 194, 6678, 15580, 10315, -2384, -9831, -9661, -6049, -2763, -1012, -416, -294, -248, -172},
    { 0, 0, 207, 6796, 15608, 10194, -2502, -9864, -9633, -6009, -2736, -1001, -413, -293, -248, -172},
    { 0, 0, 221, 6915, 15634, 10072, -2620, -9896, -9604, -5969, -2710, -990, -410, -293, -247, -171},
    { 0, 0, 236, 7035, 15657, 9949, -2738, -9927, -9575, -5929, -2684, -980, -408, -293, -246, -170},
    { 0, 0, 251, 7154, 15679, 9826, -2854, -9957, -9545, -5889, -2658, -969, -405, -292, -246, -169},
    { 0, 0, 267, 7274, 15699, 9701, -2970, -9986, -9516, -5849, -2633, -959, -403, -292, -245, -168},
    { 0, 0, 284, 7395, 15717, 9577, -3085, -10015, -9485, -5809, -2607, -948, -400, -291, -244, -167},
    { 0, 0, 302, 7515, 15734, 9451, -3199, -10042, -9455, -5769, -2582, -938, -398, -291, -244, -166},
    { 0, 0, 321, 7636, 15748, 9325, -3313, -10069, -9424, -5730, -2557, -928, -395, -290, -243, -165},
    { 0, 0, 340, 7757, 15761, 9198, -3426, -10095, -9393, -5690, -2532, -918, -393, -290, -242, -164},
    { 0, 0, 361, 7878, 15771, 9070, -3538, -10120, -9362, -5651, -2508, -909, -391, -289, -242, -163},
    { 0, 0, 382, 7999, 15780, 8942, -3649, -10144, -9330, -5611, -2483, -899, -388, -289, -241, -163},
    { 0, 0, 405, 8121, 15787, 8813, -3759, -10168, -9298, -5572, -2459, -889, -386, -288, -240, -162},
    { 0, 0, 428, 8242, 15792, 8683, -3869, -10190, -9266, -5533, -2435, -880, -384, -288, -240, -161},
    { 0, 0, 452, 8364, 15795, 8553, -3978, -10212, -9233, -5494, -2411, -871, -382, -288, -239, -160},
    { 0, 0, 478, 8485, 15796, 8422, -4086, -10233, -9200, -5454, -2387, -861, -380, -287, -238, -159},
    { 0, 0, 504, 8607, 15795, 8291, -4193, -10253, -9167, -5416, -2363, -852, -378, -287, -238, -158},
    { 0, 0, 532, 8728, 15792, 8159, -4300, -10272, -9134, -5377, -2340, -843, -376, -286, -237, -157},
    { 0, 0, 561, 8849, 15788, 8027, -4405, -10291, -9100, -5338, -2316, -834, -374, -286, -236, -156},
    { 0, 0, 591, 8970, 15781, 7894, -4510, -10308, -9066, -5299, -2293, -826, -372, -285, -236, -155},
    { 0, 0, 622, 9091, 15773, 7761, -4614, -10325, -9032, -5261, -2270, -817, -370, -285, -235, -154},
    { 0, 0, 654, 9212, 15763, 7628, -4717, -10341, -8997, -5223, -2248, -808, -368, -285, -234, -153},
    { 0, 0, 687, 9332, 15751, 7494, -4820, -10356, -8963, -5184, -2225, -800, -366, -284, -234, -153},
    { 0, 0, 721, 9453, 15737, 7359, -4921, -10371, -8928, -5146, -2203, -792, -364, -284, -233, -152},
    { 0, 0, 757, 9573, 15721, 7224, -5022, -10384, -8892, -5108, -2181, -783, -363, -283, -232, -151},
    { 0, 0, 794, 9692, 15703, 7089, -5121, -10397, -8857, -5070, -2159, -775, -361, -283, -231, -150},
    { 0, 0, 832, 9811, 15683, 6953, -5220, -10409, -8821, -5032, -2137, -767, -359, -283, -231, -149},
    { 0, 0, 871, 9930, 15662, 6817, -5318, -10420, -8786, -4995, -2115, -759, -357, -282, -230, -148},
    { 0, 0, 912, 10049, 15639, 6681, -5415, -10431, -8749, -4957, -2094, -752, -356, -282, -229, -147},
    { 0, 0, 954, 10166, 15613, 6545, -5511, -10441, -8713, -4920, -2072, -744, -354, -281, -229, -146},
    { 0, 0, 997, 10284, 15587, 6408, -5607, -10450, -8677, -4882, -2051, -736, -353, -281, -228, -145},
    { 0, 0, 1042, 10401, 15558, 6271, -5701, -10458, -8640, -4845, -2030, -729, -351, -280, -227, -144},
    { 0, 0, 1088, 10517, 15527, 6134, -5794, -10465, -8603, -4808, -2009, -721, -350, -280, -226, -143},
    { 0, 0, 1135, 10633, 15495, 5996, -5887, -10472, -8566, -4771, -1989, -714, -348, -280, -226, -143},
    { 0, 0, 1183, 10748, 15460, 5858, -5979, -10478, -8529, -4734, -1968, -707, -347, -279, -225, -142},
    { 0, 0, 1233, 10862, 15424, 5721, -6070, -10484, -8492, -4698, -1948, -700, -345, -279, -224, -141},
    { 0, 0, 1284, 10976, 15386, 5582, -6159, -10488, -8454, -4661, -1928, -693, -344, -278, -223, -140},
    { 0, 0, 1337, 11089, 15347, 5444, -6248, -10492, -8416, -4625, -1908, -686, -342, -278, -223, -139},
    { 0, 0, 1391, 11202, 15305, 5306, -6336, -10495, -8378, -4589, -1888, -679, -341, -277, -222, -138},
    { 0, 0, 1446, 11313, 15262, 5168, -6424, -10498, -8340, -4552, -1869, -672, -340, -277, -221, -137},
    { 0, 0, 1503, 11424, 15217, 5029, -6510, -10499, -8302, -4516, -1849, -666, -338, -277, -220, -136},
    { 0, 0, 1561, 11534, 15170, 4890, -6595, -10501, -8264, -4481, -1830, -659, -337, -276, -219, -135},
    { 0, 0, 1621, 11643, 15122, 4752, -6679, -10501, -8225, -4445, -1811, -653, -336, -276, -219, -135},
    { 0, 0, 1681, 11751, 15072, 4613, -6763, -10501, -8186, -4409, -1792, -646, -335, -275, -218, -134},
    { 0, 0, 1744, 11858, 15020, 4474, -6845, -10500, -8148, -4374, -1773, -640, -333, -275, -217, -133},
    { 0, 0, 1807, 11964, 14966, 4336, -6927, -10498, -8109, -4339, -1755, -634, -332, -275, -216, -132},
    { 0, 0, 1872, 12070, 14911, 4197, -7007, -10496, -8070, -4304, -1736, -628, -331, -274, -215, -131},
    { 0, 0, 1939, 12174, 14854, 4058, -7087, -10493, -8030, -4269, -1718, -622, -330, -274, -215, -130},
    { 0, 0, 2007, 12277, 14795, 3920, -7166, -10489, -7991, -4234, -1700, -616, -329, -273, -214, -129},
    { 0, 0, 2076, 12379, 14735, 3781, -7243, -10485, -7952, -4199, -1682, -610, -328, -273, -213, -128},
    { 0, -1, 2147, 12481, 14673, 3643, -7320, -10480, -7912, -4165, -1664, -604, -327, -272, -212, -127},
    { 0, -1, 2219, 12581, 14610, 3504, -7396, -10474, -7873, -4131, -1647, -598, -326, -272, -211, -127},
    { 0, -1, 2292, 12679, 14544, 3366, -7471, -10468, -7833, -4097, -1629, -593, -325, -271, -211, -126},
    { 0, -1, 2367, 12777, 14478, 3228, -7545, -10461, -7793, -4063, -1612, -587, -324, -271, -210, -125},
    { 0, -2, 2443, 12874, 14409, 3090, -7618, -10454, -7753, -4029, -1595, -582, -323, -271, -209, -124},
    { 0, -2, 2521, 12969, 14339, 2952, -7690, -10446, -7713, -3995, -1578, -576, -322, -270, -208, -123},
    { 0, -3, 2600, 13063, 14268, 2814, -7761, -10437, -7673, -3962, -1561, -571, -321, -270, -207, -122},
    { 0, -4, 2680, 13156, 14195, 2677, -7831, -10428, -7633, -3928, -1544, -566, -320, -269, -207, -121},
    { 0, -5, 2761, 13247, 14120, 2540, -7901, -10418, -7593, -3895, -1528, -561, -319, -269, -206, -120},
    { 0, -7, 2844, 13337, 14044, 2403, -7969, -10407, -7553, -3862, -1512, -556, -318, -268, -205, -120},
    { 0, -9, 2928, 13426, 13966, 2266, -8036, -10396, -7512, -3829, -1496, -551, -317, -268, -204, -119},
    { 0, -13, 3014, 13514, 13887, 2129, -8102, -10385, -7472, -3797, -1480, -546, -316, -267, -203, -118},
    { 0, -17, 3101, 13600, 13807, 1993, -8168, -10373, -7431, -3764, -1464, -541, -315, -267, -202, -117},
    { 0, -24, 3189, 13685, 13725, 1857, -8232, -10360, -7391, -3732, -1448, -536, -315, -266, -201, -116},
    { 0, -36, 3278, 13768, 13641, 1721, -8296, -10347, -7350, -3700, -1432, -532, -314, -266, -201, -115},
    { 0, -57, 3368, 13850, 13556, 1586, -8358, -10333, -7310, -3668, -1417, -527, -313, -265, -200, -114},
    { 0, -108, 3460, 13930, 13470, 1451, -8420, -10318, -7269, -3636, -1402, -523, -312, -265, -199, -113},
    { 0, -357, 3553, 14009, 13382, 1316, -8481, -10303, -7228, -3604, -1387, -518, -311, -264, -198, -113},
    { 0, 436, 3647, 14086, 13293, 1182, -8540, -10288, -7188, -3573, -1372, -514, -311, -264, -197, -112},
    { 0, 26, 3743, 14162, 13203, 1048, -8599, -10272, -7147, -3541, -1357, -509, -310, -263, -196, -111},
    { 0, 29, 3839, 14237, 13111, 914, -8657, -10255, -7106, -3510, -1342, -505, -309, -263, -195, -110},
    { 0, 31, 3937, 14309, 13018, 781, -8713, -10238, -7065, -3479, -1328, -501, -308, -262, -195, -109},
    { 0, 34, 4035, 14381, 12924, 648, -8769, -10221, -7025, -3448, -1314, -497, -308, -262, -194, -108},
    { 0, 37, 4135, 14450, 12828, 516, -8824, -10202, -6984, -3418, -1299, -493, -307, -261, -193, -108},
    { 0, 40, 4236, 14518, 12731, 384, -8878, -10184, -6943, -3387, -1285, -489, -306, -261, -192, -107},
    { 0, 43, 4338, 14585, 12633, 252, -8931, -10165, -6902, -3357, -1272, -485, -306, -260, -191, -106},
    { 0, 47, 4441, 14649, 12534, 121, -8983, -10145, -6861, -3327, -1258, -481, -305, -260, -190, -105},
    { 0, 51, 4545, 14713, 12433, -9, -9035, -10125, -6821, -3297, -1244, -477, -304, -259, -189, -104},
    { 0, 55, 4650, 14774, 12331, -139, -9085, -10105, -6780, -3267, -1231, -473, -304, -259, -189, -103},
    { 0, 59, 4756, 14834, 12228, -269, -9134, -10084, -6739, -3238, -1217, -470, -303, -258, -188, -103},
    { 0, 63, 4863, 14892, 12124, -398, -9182, -10062, -6698, -3208, -1204, -466, -303, -258, -187, -102},
    { 0, 68, 4971, 14948, 12019, -526, -9230, -10040, -6657, -3179, -1191, -462, -302, -257, -186, -101},
    { 0, 73, 5079, 15003, 11912, -654, -9276, -10018, -6616, -3150, -1178, -459, -301, -256, -185, -100},
    { 0, 79, 5189, 15056, 11805, -782, -9322, -9995, -6576, -3121, -1166, -456, -301, -256, -184, -99},
    { 0, 85, 5299, 15107, 11696, -909, -9366, -9972, -6535, -3093, -1153, -452, -300, -255, -183, -98},
    { 0, 91, 5410, 15156, 11586, -1035, -9410, -9948, -6494, -3064, -1140, -449, -300, -255, -182, -98},
    { 0, 98, 5522, 15204, 11476, -1161, -9453, -9924, -6454, -3036, -1128, -446, -299, -254, -181, -97},
    { 0, 105, 5635, 15250, 11364, -1286, -9495, -9900, -6413, -3008, -1116, -442, -299, -254, -181, -96},
    { 0, 113, 5748, 15294, 11251, -1410, -9536, -9875, -6372, -2980, -1104, -439, -298, -253, -180, -95},
    { 0, 121, 5862, 15336, 11137, -1534, -9576, -9849, -6332, -2952, -1092, -436, -297, -252, -179, -94},
    { 0, 130, 5977, 15376, 11022, -1658, -9615, -9824, -6291, -2924, -1080, -433, -297, -252, -178, -94},
    { 0, 139, 6093, 15415, 10907, -1780, -9653, -9798, -6251, -2897, -1068, -430, -296, -251, -177, -93},
    { 0, 149, 6209, 15452, 10790, -1902, -9690, -9771, -6210, -2870, -1057, -427, -296, -251, -176, -92},
    { 0, 159, 6325, 15487, 10673, -2024, -9727, -9744, -6170, -2843, -1045, -424, -295, -250, -175, -91},
    { 0, 170, 6442, 15520, 10554, -2144, -9762, -9717, -6129, -2816, -1034, -421, -295, -249, -174, -90},
    { 0, 182, 6560, 15551, 10435, -2264, -9797, -9689, -6089, -2789, -1023, -418, -294, -249, -173, -90},
    { 0, 194, 6678, 15580, 10315, -2384, -9831, -9661, -6049, -2763, -1012, -416, -294, -248, -172, -89},
    { 0, 207, 6796, 15608, 10194, -2502, -9864, -9633, -6009, -2736, -1001, -413, -293, -248, -172, -88},
    { 0, 221, 6915, 15634, 10072, -2620, -9896, -9604, -5969, -2710, -990, -410, -293, -247, -171, -87},
    { 0, 236, 7035, 15657, 9949, -2738, -9927, -9575, -5929, -2684, -980, -408, -293, -246, -170, -87},
    { 0, 251, 7154, 15679, 9826, -2854, -9957, -9545, -5889, -2658, -969, -405, -292, -246, -169, -86},
    { 0, 267, 7274, 15699, 9701, -2970, -9986, -9516, -5849, -2633, -959, -403, -292, -245, -168, -85},
    { 0, 284, 7395, 15717, 9577, -3085, -10015, -9485, -5809, -2607, -948, -400, -291, -244, -167, -84},
    { 0, 302, 7515, 15734, 9451, -3199, -10042, -9455, -5769, -2582, -938, -398, -291, -244, -166, -83},
    { 0, 321, 7636, 15748, 9325, -3313, -10069, -9424, -5730, -2557, -928, -395, -290, -243, -165, -83},
    { 0, 340, 7757, 15761, 9198, -3426, -10095, -9393, -5690, -2532, -918, -393, -290, -242, -164, -82},
    { 0, 361, 7878, 15771, 9070, -3538, -10120, -9362, -5651, -2508, -909, -391, -289, -242, -163, -81},
    { 0, 382, 7999, 15780, 8942, -3649, -10144, -9330, -5611, -2483, -899, -388, -289, -241, -163, -80},
    { 0, 405, 8121, 15787, 8813, -3759, -10168, -9298, -5572, -2459, -889, -386, -288, -240, -162, -80},
    { 0, 428, 8242, 15792, 8683, -3869, -10190, -9266, -5533, -2435, -880, -384, -288, -240, -161, -79},
    { 0, 452, 8364, 15795, 8553, -3978, -10212, -9233, -5494, -2411, -871, -382, -288, -239, -160, -78},
    { 0, 478, 8485, 15796, 8422, -4086, -10233, -9200, -5454, -2387, -861, -380, -287, -238, -159, -77},
    { 0, 504, 8607, 15795, 8291, -4193, -10253, -9167, -5416, -2363, -852, -378, -287, -238, -158, -77},
    { 0, 532, 8728, 15792, 8159, -4300, -10272, -9134, -5377, -2340, -843, -376, -286, -237, -157, -76},
    { 0, 561, 8849, 15788, 8027, -4405, -10291, -9100, -5338, -2316, -834, -374, -286, -236, -156, -75},
    { 0, 591, 8970, 15781, 7894, -4510, -10308, -9066, -5299, -2293, -826, -372, -285, -236, -155, -75},
    { 0, 622, 9091, 15773, 7761, -4614, -10325, -9032, -5261, -2270, -817, -370, -285, -235, -154, -74},
    { 0, 654, 9212, 15763, 7628, -4717, -10341, -8997, -5223, -2248, -808, -368, -285, -234, -153, -73},
    { 0, 687, 9332, 15751, 7494, -4820, -10356, -8963, -5184, -2225, -800, -366, -284, -234, -153, -72},
    { 0, 721, 9453, 15737, 7359, -4921, -10371, -8928, -5146, -2203, -792, -364, -284, -233, -152, -72},
    { 0, 757, 9573, 15721, 7224, -5022, -10384, -8892, -5108, -2181, -783, -363, -283, -232, -151, -71},
    { 0, 794, 9692, 15703, 7089, -5121, -10397, -8857, -5070, -2159, -775, -361, -283, -231, -150, -70},
    { 0, 832, 9811, 15683, 6953, -5220, -10409, -8821, -5032, -2137, -767, -359, -283, -231, -149, -70},
    { 0, 871, 9930, 15662, 6817, -5318, -10420, -8786, -4995, -2115, -759, -357, -282, -230, -148, -69},
    { 0, 912, 10049, 15639, 6681, -5415, -10431, -8749, -4957, -2094, -752, -356, -282, -229, -147, -68},
    { 0, 954, 10166, 15613, 6545, -5511, -10441, -8713, -4920, -2072, -744, -354, -281, -229, -146, -67},
    { 0, 997, 10284, 15587, 6408, -5607, -10450, -8677, -4882, -2051, -736, -353, -281, -228, -145, -67},
    { 0, 1042, 10401, 15558, 6271, -5701, -10458, -8640, -4845, -2030, -729, -351, -280, -227, -144, -66},
    { 0, 1088, 10517, 15527, 6134, -5794, -10465, -8603, -4808, -2009, -721, -350, -280, -226, -143, -65},
    { 0, 1135, 10633, 15495, 5996, -5887, -10472, -8566, -4771, -1989, -714, -348, -280, -226, -143, -65},
    { 0, 1183, 10748, 15460, 5858, -5979, -10478, -8529, -4734, -1968, -707, -347, -279, -225, -142, -64},
    { 0, 1233, 10862, 15424, 5721, -6070, -10484, -8492, -4698, -1948, -700, -345, -279, -224, -141, -63},
    { 0, 1284, 10976, 15386, 5582, -6159, -10488, -8454, -4661, -1928, -693, -344, -278, -223, -140, -63},
    { 0, 1337, 11089, 15347, 5444, -6248, -10492, -8416, -4625, -1908, -686, -342, -278, -223, -139, -62},
    { 0, 1391, 11202, 15305, 5306, -6336, -10495, -8378, -4589, -1888, -679, -341, -277, -222, -138, -61},
    { 0, 1446, 11313, 15262, 5168, -6424, -10498, -8340, -4552, -1869, -672, -340, -277, -221, -137, -61},
    { 0, 1503, 11424, 15217, 5029, -6510, -10499, -8302, -4516, -1849, -666, -338, -277, -220, -136, -60},
    { 0, 1561, 11534, 15170, 4890, -6595, -10501, -8264, -4481, -1830, -659, -337, -276, -219, -135, -59},
    { 0, 1621, 11643, 15122, 4752, -6679, -10501, -8225, -4445, -1811, -653, -336, -276, -219, -135, -59},
    { 0, 1681, 11751, 15072, 4613, -6763, -10501, -8186, -4409, -1792, -646, -335, -275, -218, -134, -58},
    { 0, 1744, 11858, 15020, 4474, -6845, -10500, -8148, -4374, -1773, -640, -333, -275, -217, -133, -57},
    { 0, 1807, 11964, 14966, 4336, -6927, -10498, -8109, -4339, -1755, -634, -332, -275, -216, -132, -57},
    { 0, 1872, 12070, 14911, 4197, -7007, -10496, -8070, -4304, -1736, -628, -331, -274, -215, -131, -56},
    { 0, 1939, 12174, 14854, 4058, -7087, -10493, -8030, -4269, -1718, -622, -330, -274, -215, -130, -56},
    { 0, 2007, 12277, 14795, 3920, -7166, -10489, -7991, -4234, -1700, -616, -329, -273, -214, -129, -55},
    { 0, 2076, 12379, 14735, 3781, -7243, -10485, -7952, -4199, -1682, -610, -328, -273, -213, -128, -54},
    { 0, 2147, 12481, 14673, 3643, -7320, -10480, -7912, -4165, -1664, -604, -327, -272, -212, -127, -54},
    { 0, 2219, 12581, 14610, 3504, -7396, -10474, -7873, -4131, -1647, -598, -326, -272, -211, -127, -53},
    { 0, 2292, 12679, 14544, 3366, -7471, -10468, -7833, -4097, -1629, -593, -325, -271, -211, -126, -52},
    { 0, 2367, 12777, 14478, 3228, -7545, -10461, -7793, -4063, -1612, -587, -324, -271, -210, -125, -52},
    { 0, 2443, 12874, 14409, 3090, -7618, -10454, -7753, -4029, -1595, -582, -323, -271, -209, -124, -51},
    { 0, 2521, 12969, 14339, 2952, -7690, -10446, -7713, -3995, -1578, -576, -322, -270, -208, -123, -51},
    { 0, 2600, 13063, 14268, 2814, -7761, -10437, -7673, -3962, -1561, -571, -321, -270, -207, -122, -50},
    { 0, 2680, 13156, 14195, 2677, -7831, -10428, -7633, -3928, -1544, -566, -320, -269, -207, -121, -49},
    { 0, 2761, 13247, 14120, 2540, -7901, -10418, -7593, -3895, -1528, -561, -319, -269, -206, -120, -49},
    { 0, 2844, 13337, 14044, 2403, -7969, -10407, -7553, -3862, -1512, -556, -318, -268, -205, -120, -48},
    { -1, 2928, 13426, 13966, 2266, -8036, -10396, -7512, -3829, -1496, -551, -317, -268, -204, -119, -48},
    { -1, 3014, 13514, 13887, 2129, -8102, -10385, -7472, -3797, -1480, -546, -316, -267, -203, -118, -47},
    { -1, 3101, 13600, 13807, 1993, -8168, -10373, -7431, -3764, -1464, -541, -315, -267, -202, -117, -46},
    { -2, 3189, 13685, 13725, 1857, -8232, -10360, -7391, -3732, -1448, -536, -315, -266, -201, -116, -46},
    { -2, 3278, 13768, 13641, 1721, -8296, -10347, -7350, -3700, -1432, -532, -314, -266, -201, -115, -45},
    { -4, 3368, 13850, 13556, 1586, -8358, -10333, -7310, -3668, -1417, -527, -313, -265, -200, -114, -45},
    { -7, 3460, 13930, 13470, 1451, -8420, -10318, -7269, -3636, -1402, -523, -312, -265, -199, -113, -44},
    { -22, 3553, 14009, 13382, 1316, -8481, -10303, -7228, -3604, -1387, -518, -311, -264, -198, -113, -44},
    { 27, 3647, 14086, 13293, 1182, -8540, -10288, -7188, -3573, -1372, -514, -311, -264, -197, -112, -43}
  };
  int par_fg31[192][16] = {
    { -2114, 2497, -149, 888, 631, 5970, 5339, 3463, 3904, 2106, 2106, 1891, 1668, 1009, 1000, 1501},
    { -2114, 2501, -128, 841, 693, 5983, 5326, 3454, 3897, 2098, 2101, 1888, 1667, 1005, 1003, 1492},
    { -2114, 2505, -107, 795, 755, 5994, 5314, 3445, 3891, 2090, 2096, 1886, 1665, 1001, 1007, 1482},
    { -2113, 2510, -88, 750, 818, 6005, 5301, 3437, 3884, 2082, 2091, 1883, 1663, 997, 1010, 1473},
    { -2113, 2515, -69, 705, 881, 6015, 5288, 3428, 3877, 2075, 2087, 1881, 1662, 993, 1013, 1463},
    { -2113, 2521, -51, 662, 945, 6024, 5276, 3419, 3870, 2067, 2082, 1878, 1660, 989, 1016, 1453},
    { -2113, 2527, -34, 618, 1009, 6033, 5263, 3410, 3863, 2059, 2077, 1876, 1658, 985, 1019, 1443},
    { -2113, 2534, -18, 576, 1073, 6041, 5251, 3401, 3856, 2051, 2072, 1873, 1657, 981, 1022, 1434},
    { -2113, 2541, -3, 534, 1138, 6049, 5238, 3393, 3849, 2044, 2067, 1871, 1655, 977, 1025, 1424},
    { -2112, 2549, 11, 494, 1203, 6056, 5226, 3384, 3842, 2036, 2062, 1869, 1653, 974, 1028, 1414},
    { -2112, 2557, 24, 454, 1268, 6062, 5213, 3375, 3835, 2028, 2058, 1866, 1651, 970, 1031, 1404},
    { -2112, 2566, 36, 414, 1334, 6067, 5201, 3366, 3828, 2021, 2053, 1864, 1650, 966, 1034, 1394},
    { -2112, 2575, 48, 376, 1400, 6072, 5188, 3357, 3820, 2013, 2048, 1862, 1648, 962, 1037, 1384},
    { -2112, 2585, 58, 339, 1466, 6077, 5176, 3348, 3813, 2005, 2043, 1860, 1646, 958, 1040, 1373},
    { -2112, 2595, 67, 303, 1532, 6081, 5164, 3339, 3806, 1998, 2038, 1857, 1644, 954, 1043, 1363},
    { -2111, 2606, 75, 267, 1599, 6084, 5152, 3330, 3799, 1990, 2033, 1855, 1642, 950, 1046, 1353},
    { -2111, 2617, 82, 233, 1665, 6086, 5140, 3321, 3792, 1983, 2029, 1853, 1640, 946, 1049, 1343},
    { -2111, 2629, 88, 200, 1732, 6088, 5128, 3311, 3785, 1975, 2024, 1851, 1639, 942, 1052, 1332},
    { -2111, 2642, 93, 168, 1799, 6090, 5116, 3302, 3778, 1968, 2019, 1849, 1637, 938, 1055, 1322},
    { -2111, 2655, 96, 137, 1866, 6090, 5104, 3293, 3770, 1960, 2014, 1846, 1635, 934, 1058, 1311},
    { -2110, 2668, 99, 107, 1933, 6091, 5092, 3284, 3763, 1953, 2009, 1844, 1633, 930, 1061, 1301},
    { -2110, 2682, 100, 78, 2001, 6090, 5080, 3274, 3756, 1945, 2004, 1842, 1631, 926, 1064, 1290},
    { -2110, 2697, 100, 51, 2068, 6089, 5069, 3265, 3749, 1938, 1999, 1840, 1629, 922, 1067, 1280},
    { -2110, 2712, 99, 24, 2135, 6088, 5057, 3256, 3742, 1930, 1995, 1838, 1627, 918, 1070, 1269},
    { -2110, 2728, 97, -1, 2202, 6086, 5045, 3246, 3734, 1923, 1990, 1836, 1625, 914, 1072, 1258},
    { -2110, 2745, 93, -24, 2269, 6083, 5034, 3237, 3727, 1916, 1985, 1834, 1623, 910, 1075, 1247},
    { -2109, 2761, 89, -46, 2336, 6080, 5023, 3227, 3720, 1908, 1980, 1832, 1620, 906, 1078, 1237},
    { -2109, 2779, 83, -67, 2402, 6077, 5011, 3218, 3713, 1901, 1975, 1830, 1618, 902, 1081, 1226},
    { -2109, 2797, 75, -87, 2469, 6072, 5000, 3208, 3705, 1894, 1971, 1828, 1616, 898, 1084, 1215},
    { -2109, 2816, 67, -105, 2535, 6068, 4989, 3199, 3698, 1887, 1966, 1826, 1614, 894, 1087, 1204},
    { -2109, 2835, 57, -121, 2601, 6063, 4978, 3189, 3691, 1879, 1961, 1824, 1612, 890, 1090, 1193},
    { -2109, 2855, 46, -137, 2667, 6057, 4968, 3179, 3684, 1872, 1956, 1823, 1610, 886, 1092, 1182},
    { -2109, 2875, 34, -150, 2733, 6051, 4957, 3170, 3676, 1865, 1952, 1821, 1607, 882, 1095, 1171},
    { -2108, 2896, 20, -162, 2798, 6044, 4946, 3160, 3669, 1858, 1947, 1819, 1605, 878, 1098, 1159},
    { -2108, 2917, 5, -173, 2863, 6037, 4936, 3150, 3662, 1851, 1942, 1817, 1603, 874, 1101, 1148},
    { -2108, 2939, -11, -182, 2927, 6029, 4925, 3140, 3655, 1844, 1937, 1815, 1601, 870, 1103, 1137},
    { -2108, 2961, -28, -190, 2991, 6021, 4915, 3130, 3648, 1837, 1933, 1814, 1598, 866, 1106, 1126},
    { -2108, 2984, -47, -196, 3055, 6013, 4905, 3121, 3640, 1830, 1928, 1812, 1596, 862, 1109, 1114},
    { -2108, 3008, -67, -200, 3118, 6004, 4895, 3111, 3633, 1823, 1923, 1810, 1594, 858, 1112, 1103},
    { -2108, 3032, -88, -203, 3181, 5995, 4885, 3101, 3626, 1816, 1919, 1809, 1591, 854, 1114, 1092},
    { -2108, 3056, -110, -204, 3243, 5985, 4875, 3091, 3619, 1809, 1914, 1807, 1589, 850, 1117, 1080},
    { -2107, 3081, -134, -203, 3304, 5975, 4865, 3081, 3612, 1802, 1909, 1806, 1587, 846, 1119, 1069},
    { -2107, 3106, -158, -201, 3365, 5964, 4856, 3071, 3605, 1795, 1905, 1804, 1584, 842, 1122, 1057},
    { -2107, 3132, -184, -197, 3425, 5953, 4846, 3061, 3598, 1789, 1900, 1802, 1582, 838, 1125, 1046},
    { -2107, 3158, -212, -192, 3485, 5942, 4837, 3051, 3591, 1782, 1896, 1801, 1579, 834, 1127, 1034},
    { -2107, 3185, -240, -185, 3543, 5931, 4828, 3041, 3584, 1775, 1891, 1799, 1577, 830, 1130, 1023},
    { -2107, 3212, -270, -176, 3602, 5919, 4819, 3030, 3576, 1769, 1886, 1798, 1574, 827, 1132, 1011},
    { -2107, 3239, -300, -166, 3659, 5906, 4810, 3020, 3569, 1762, 1882, 1797, 1572, 823, 1135, 1000},
    { -2107, 3267, -332, -154, 3716, 5894, 4801, 3010, 3563, 1756, 1877, 1795, 1569, 819, 1137, 988},
    { -2107, 3296, -365, -140, 3772, 5881, 4792, 3000, 3556, 1749, 1873, 1794, 1567, 815, 1140, 977},
    { -2106, 3324, -399, -125, 3827, 5868, 4784, 2990, 3549, 1743, 1868, 1793, 1564, 811, 1142, 965},
    { -2106, 3353, -434, -108, 3882, 5854, 4775, 2979, 3542, 1736, 1864, 1791, 1562, 807, 1145, 953},
    { -2106, 3382, -470, -90, 3935, 5841, 4767, 2969, 3535, 1730, 1859, 1790, 1559, 803, 1147, 942},
    { -2106, 3412, -507, -69, 3988, 5827, 4759, 2959, 3528, 1724, 1855, 1789, 1557, 799, 1150, 930},
    { -2106, 3442, -545, -48, 4040, 5813, 4750, 2948, 3521, 1717, 1851, 1787, 1554, 795, 1152, 918},
    { -2106, 3472, -584, -25, 4091, 5798, 4742, 2938, 3514, 1711, 1846, 1786, 1552, 791, 1155, 907},
    { -2106, 3502, -624, 0, 4141, 5783, 4734, 2928, 3507, 1705, 1842, 1785, 1549, 787, 1157, 895},
    { -2106, 3532, -664, 26, 4191, 5769, 4727, 2917, 3501, 1699, 1837, 1784, 1546, 783, 1159, 883},
    { -2106, 3563, -706, 54, 4239, 5754, 4719, 2907, 3494, 1693, 1833, 1783, 1544, 779, 1162, 872},
    { -2105, 3594, -748, 83, 4287, 5738, 4711, 2896, 3487, 1687, 1829, 1782, 1541, 775, 1164, 860},
    { -2105, 3625, -792, 114, 4334, 5723, 4704, 2886, 3481, 1681, 1824, 1781, 1538, 771, 1166, 848},
    { -2105, 3657, -836, 146, 4380, 5707, 4697, 2875, 3474, 1675, 1820, 1779, 1536, 767, 1168, 837},
    { -2105, 3688, -880, 180, 4425, 5691, 4689, 2865, 3467, 1669, 1815, 1778, 1533, 763, 1171, 825},
    { -2105, 3720, -926, 215, 4469, 5675, 4682, 2854, 3461, 1663, 1811, 1777, 1530, 759, 1173, 813},
    { -2105, 3751, -972, 252, 4512, 5659, 4675, 2844, 3454, 1657, 1807, 1776, 1527, 755, 1175, 801},
    { -2104, 3783, -1019, 289, 4555, 5643, 4668, 2833, 3447, 1651, 1802, 1775, 1525, 751, 1177, 790},
    { -2104, 3815, -1066, 329, 4596, 5627, 4661, 2823, 3441, 1645, 1798, 1774, 1522, 747, 1179, 778},
    { -2104, 3847, -1114, 369, 4637, 5610, 4654, 2812, 3434, 1639, 1794, 1773, 1519, 743, 1181, 766},
    { -2104, 3879, -1163, 411, 4677, 5593, 4647, 2801, 3428, 1634, 1789, 1773, 1516, 739, 1184, 754},
    { -2104, 3911, -1212, 454, 4716, 5577, 4641, 2791, 3421, 1628, 1785, 1772, 1513, 735, 1186, 742},
    { -2103, 3944, -1262, 499, 4754, 5560, 4634, 2780, 3414, 1622, 1781, 1771, 1511, 731, 1188, 730},
    { -2103, 3976, -1312, 545, 4791, 5543, 4627, 2769, 3408, 1617, 1776, 1770, 1508, 727, 1190, 719},
    { -2103, 4008, -1363, 592, 4827, 5526, 4621, 2758, 3401, 1611, 1772, 1769, 1505, 723, 1192, 707},
    { -2103, 4040, -1414, 640, 4862, 5508, 4614, 2747, 3395, 1605, 1768, 1768, 1502, 719, 1194, 695},
    { -2103, 4072, -1466, 689, 4897, 5491, 4608, 2737, 3388, 1600, 1763, 1767, 1499, 715, 1196, 683},
    { -2102, 4104, -1518, 740, 4931, 5474, 4602, 2726, 3382, 1594, 1759, 1767, 1496, 711, 1198, 671},
    { -2102, 4136, -1570, 792, 4964, 5456, 4595, 2715, 3375, 1589, 1754, 1766, 1493, 707, 1200, 659},
    { -2102, 4168, -1623, 845, 4996, 5439, 4589, 2704, 3369, 1583, 1750, 1765, 1490, 703, 1202, 647},
    { -2101, 4200, -1676, 899, 5027, 5421, 4583, 2693, 3362, 1578, 1746, 1764, 1487, 699, 1204, 635},
    { -2101, 4232, -1729, 955, 5057, 5403, 4577, 2682, 3356, 1572, 1741, 1764, 1484, 695, 1206, 623},
    { -2101, 4263, -1783, 1011, 5087, 5385, 4571, 2671, 3349, 1567, 1737, 1763, 1481, 691, 1208, 611},
    { -2100, 4295, -1837, 1069, 5115, 5367, 4564, 2659, 3343, 1561, 1732, 1762, 1478, 687, 1210, 599},
    { -2100, 4326, -1891, 1127, 5143, 5349, 4558, 2648, 3336, 1556, 1728, 1761, 1475, 683, 1211, 587},
    { -2100, 4357, -1945, 1187, 5170, 5331, 4552, 2637, 3330, 1551, 1723, 1761, 1472, 679, 1213, 575},
    { -2099, 4389, -2000, 1248, 5197, 5313, 4546, 2625, 3323, 1545, 1718, 1760, 1468, 675, 1215, 563},
    { -2099, 4419, -2054, 1310, 5223, 5295, 4540, 2614, 3316, 1540, 1714, 1759, 1465, 670, 1217, 550},
    { -2098, 4450, -2109, 1373, 5247, 5277, 4534, 2603, 3310, 1534, 1709, 1759, 1462, 666, 1219, 538},
    { -2098, 4481, -2164, 1438, 5272, 5258, 4528, 2591, 3303, 1529, 1704, 1758, 1459, 662, 1221, 526},
    { -2097, 4511, -2219, 1503, 5295, 5239, 4522, 2579, 3296, 1524, 1700, 1757, 1455, 658, 1222, 513},
    { -2097, 4541, -2275, 1570, 5318, 5221, 4516, 2568, 3290, 1518, 1695, 1757, 1452, 654, 1224, 501},
    { -2096, 4571, -2330, 1638, 5340, 5202, 4509, 2556, 3283, 1513, 1690, 1756, 1448, 649, 1226, 488},
    { -2096, 4601, -2387, 1707, 5363, 5182, 4503, 2543, 3276, 1507, 1685, 1756, 1445, 645, 1228, 475},
    { -2095, 4631, -2444, 1780, 5386, 5162, 4496, 2530, 3268, 1501, 1679, 1755, 1441, 640, 1230, 462},
    { -2094, 4660, -2504, 1858, 5411, 5140, 4488, 2516, 3260, 1495, 1673, 1754, 1437, 635, 1232, 447},
    { -2092, 4689, -2580, 1967, 5463, 5102, 4474, 2492, 3246, 1484, 1661, 1753, 1430, 628, 1235, 424},
    { -2096, 4703, -2552, 1907, 5379, 5143, 4494, 2519, 3263, 1497, 1677, 1755, 1437, 634, 1232, 445},
    { -2093, 4743, -2654, 2052, 5447, 5094, 4475, 2487, 3244, 1482, 1661, 1753, 1428, 624, 1236, 415},
    { -2092, 4771, -2709, 2125, 5463, 5075, 4469, 2475, 3237, 1477, 1656, 1752, 1424, 620, 1238, 402},
    { -2091, 4798, -2763, 2198, 5478, 5056, 4463, 2463, 3230, 1471, 1651, 1752, 1421, 615, 1239, 389},
    { -2091, 4825, -2816, 2273, 5492, 5038, 4457, 2451, 3223, 1466, 1646, 1751, 1417, 611, 1241, 377},
    { -2090, 4852, -2870, 2349, 5506, 5019, 4450, 2438, 3216, 1461, 1641, 1751, 1414, 607, 1243, 364},
    { -2089, 4878, -2923, 2425, 5519, 5000, 4444, 2426, 3209, 1456, 1636, 1750, 1410, 602, 1244, 351},
    { -2088, 4903, -2976, 2502, 5531, 4982, 4438, 2414, 3203, 1450, 1630, 1750, 1406, 598, 1246, 338},
    { -2088, 4929, -3028, 2580, 5543, 4963, 4432, 2401, 3196, 1445, 1625, 1750, 1402, 593, 1248, 325},
    { -2087, 4953, -3080, 2659, 5553, 4944, 4426, 2389, 3189, 1440, 1620, 1749, 1398, 588, 1249, 312},
    { -2086, 4978, -3132, 2738, 5563, 4925, 4419, 2376, 3181, 1435, 1614, 1749, 1395, 584, 1251, 299},
    { -2085, 5001, -3183, 2818, 5572, 4907, 4413, 2364, 3174, 1429, 1609, 1748, 1391, 579, 1252, 286},
    { -2084, 5024, -3234, 2899, 5581, 4888, 4407, 2351, 3167, 1424, 1603, 1748, 1387, 575, 1254, 272},
    { -2083, 5047, -3284, 2980, 5589, 4869, 4400, 2338, 3160, 1419, 1597, 1747, 1383, 570, 1255, 259},
    { -2082, 5069, -3333, 3062, 5596, 4850, 4394, 2326, 3153, 1414, 1592, 1747, 1379, 565, 1257, 246},
    { -2081, 5090, -3382, 3145, 5602, 4832, 4387, 2313, 3146, 1408, 1586, 1747, 1374, 561, 1258, 232},
    { -2080, 5111, -3430, 3228, 5607, 4813, 4381, 2300, 3139, 1403, 1580, 1746, 1370, 556, 1260, 219},
    { -2079, 5131, -3477, 3312, 5612, 4794, 4374, 2287, 3131, 1398, 1574, 1746, 1366, 551, 1261, 205},
    { -2078, 5151, -3524, 3396, 5616, 4775, 4367, 2274, 3124, 1393, 1568, 1746, 1362, 546, 1263, 192},
    { -2077, 5170, -3570, 3481, 5619, 4757, 4361, 2261, 3117, 1387, 1562, 1745, 1358, 541, 1264, 178},
    { -2076, 5188, -3615, 3566, 5622, 4738, 4354, 2248, 3109, 1382, 1556, 1745, 1353, 536, 1266, 164},
    { -2075, 5205, -3659, 3651, 5624, 4720, 4347, 2235, 3102, 1377, 1550, 1745, 1349, 532, 1267, 150},
    { -2073, 5222, -3702, 3737, 5625, 4701, 4340, 2221, 3094, 1372, 1544, 1745, 1345, 527, 1269, 137},
    { -2072, 5237, -3744, 3823, 5625, 4683, 4333, 2208, 3087, 1366, 1537, 1744, 1340, 522, 1270, 123},
    { -2071, 5253, -3785, 3910, 5625, 4664, 4326, 2195, 3079, 1361, 1531, 1744, 1336, 517, 1272, 109},
    { -2070, 5267, -3824, 3996, 5624, 4646, 4319, 2181, 3072, 1356, 1524, 1744, 1331, 512, 1273, 95},
    { -2068, 5280, -3863, 4083, 5623, 4628, 4312, 2168, 3064, 1351, 1518, 1744, 1327, 507, 1275, 81},
    { -2067, 5293, -3901, 4170, 5620, 4609, 4305, 2155, 3057, 1346, 1511, 1743, 1322, 501, 1276, 67},
    { -2066, 5304, -3937, 4257, 5617, 4591, 4297, 2141, 3049, 1340, 1505, 1743, 1318, 496, 1278, 53},
    { -2064, 5315, -3972, 4344, 5613, 4573, 4290, 2128, 3041, 1335, 1498, 1743, 1313, 491, 1279, 38},
    { -2063, 5325, -4005, 4431, 5609, 4556, 4282, 2114, 3034, 1330, 1491, 1743, 1308, 486, 1280, 24},
    { -2062, 5334, -4038, 4518, 5604, 4538, 4275, 2101, 3026, 1325, 1484, 1743, 1304, 481, 1282, 10},
    { -2060, 5342, -4068, 4605, 5598, 4520, 4267, 2087, 3018, 1320, 1478, 1743, 1299, 475, 1283, -4},
    { -2059, 5349, -4098, 4692, 5592, 4503, 4260, 2074, 3010, 1315, 1471, 1743, 1294, 470, 1285, -19},
    { -2057, 5356, -4125, 4778, 5585, 4485, 4252, 2060, 3003, 1310, 1463, 1743, 1289, 465, 1286, -33},
    { -2056, 5361, -4152, 4865, 5577, 4468, 4244, 2047, 2995, 1304, 1456, 1743, 1285, 460, 1287, -47},
    { -2054, 5365, -4176, 4951, 5569, 4451, 4236, 2033, 2987, 1299, 1449, 1743, 1280, 454, 1289, -62},
    { -2053, 5368, -4199, 5036, 5561, 4434, 4229, 2019, 2979, 1294, 1442, 1743, 1275, 449, 1290, -76},
    { -2051, 5370, -4221, 5121, 5551, 4417, 4221, 2006, 2971, 1289, 1435, 1743, 1270, 443, 1291, -91},
    { -2050, 5372, -4240, 5206, 5541, 4401, 4213, 1992, 2963, 1284, 1427, 1743, 1265, 438, 1293, -105},
    { -2048, 5372, -4258, 5290, 5531, 4384, 4204, 1979, 2956, 1279, 1420, 1743, 1260, 433, 1294, -120},
    { -2046, 5371, -4274, 5374, 5520, 4368, 4196, 1966, 2948, 1274, 1413, 1743, 1255, 427, 1295, -134},
    { -2045, 5369, -4289, 5457, 5508, 4352, 4188, 1952, 2940, 1269, 1405, 1743, 1250, 422, 1297, -149},
    { -2043, 5367, -4301, 5539, 5496, 4336, 4180, 1939, 2932, 1264, 1398, 1743, 1245, 416, 1298, -163},
    { -2041, 5363, -4312, 5620, 5484, 4321, 4171, 1926, 2924, 1260, 1390, 1743, 1240, 411, 1299, -178},
    { -2040, 5358, -4321, 5701, 5471, 4305, 4163, 1912, 2916, 1255, 1383, 1743, 1235, 405, 1301, -192},
    { -2038, 5352, -4327, 5781, 5458, 4290, 4154, 1899, 2908, 1250, 1375, 1743, 1230, 399, 1302, -207},
    { -2036, 5346, -4332, 5860, 5444, 4275, 4146, 1886, 2900, 1245, 1367, 1743, 1224, 394, 1303, -221},
    { -2034, 5338, -4335, 5938, 5430, 4260, 4137, 1873, 2893, 1240, 1359, 1743, 1219, 388, 1305, -236},
    { -2033, 5329, -4337, 6015, 5415, 4245, 4129, 1860, 2885, 1236, 1352, 1743, 1214, 383, 1306, -250},
    { -2031, 5319, -4336, 6091, 5400, 4231, 4120, 1847, 2877, 1231, 1344, 1744, 1209, 377, 1307, -265},
    { -2029, 5309, -4333, 6166, 5385, 4217, 4111, 1834, 2869, 1226, 1336, 1744, 1204, 371, 1308, -279},
    { -2027, 5297, -4328, 6239, 5369, 4203, 4102, 1821, 2861, 1222, 1328, 1744, 1199, 366, 1310, -294},
    { -2026, 5285, -4321, 6312, 5353, 4189, 4093, 1809, 2853, 1217, 1320, 1744, 1193, 360, 1311, -308},
    { -2024, 5271, -4313, 6383, 5337, 4176, 4084, 1796, 2846, 1213, 1313, 1744, 1188, 354, 1312, -322},
    { -2022, 5257, -4302, 6453, 5320, 4162, 4075, 1784, 2838, 1208, 1305, 1745, 1183, 349, 1313, -337},
    { -2020, 5242, -4290, 6522, 5303, 4149, 4066, 1771, 2830, 1204, 1297, 1745, 1178, 343, 1315, -351},
    { -2018, 5226, -4275, 6590, 5286, 4137, 4057, 1759, 2822, 1199, 1289, 1745, 1173, 337, 1316, -365},
    { -2017, 5209, -4259, 6656, 5269, 4124, 4048, 1747, 2815, 1195, 1281, 1745, 1167, 332, 1317, -380},
    { -2015, 5191, -4241, 6721, 5252, 4112, 4039, 1735, 2807, 1191, 1273, 1746, 1162, 326, 1318, -394},
    { -2013, 5173, -4221, 6784, 5234, 4100, 4030, 1723, 2799, 1186, 1265, 1746, 1157, 320, 1320, -408},
    { -2011, 5154, -4199, 6846, 5216, 4088, 4021, 1711, 2792, 1182, 1257, 1746, 1152, 314, 1321, -422},
    { -2009, 5134, -4175, 6907, 5199, 4076, 4011, 1700, 2784, 1178, 1249, 1746, 1147, 309, 1322, -436},
    { -2008, 5113, -4149, 6966, 5181, 4065, 4002, 1688, 2777, 1174, 1242, 1747, 1141, 303, 1323, -450},
    { -2006, 5092, -4122, 7023, 5162, 4054, 3993, 1677, 2769, 1170, 1234, 1747, 1136, 297, 1324, -464},
    { -2004, 5070, -4093, 7079, 5144, 4043, 3983, 1666, 2762, 1166, 1226, 1747, 1131, 292, 1326, -478},
    { -2002, 5048, -4062, 7134, 5126, 4032, 3974, 1655, 2754, 1162, 1218, 1747, 1126, 286, 1327, -492},
    { -2000, 5025, -4030, 7187, 5108, 4022, 3965, 1644, 2747, 1158, 1210, 1748, 1121, 280, 1328, -506},
    { -1999, 5002, -3996, 7238, 5090, 4012, 3955, 1633, 2739, 1154, 1202, 1748, 1115, 274, 1329, -519},
    { -1997, 4978, -3960, 7288, 5071, 4002, 3946, 1622, 2732, 1150, 1195, 1748, 1110, 269, 1330, -533},
    { -1995, 4953, -3923, 7337, 5053, 3992, 3936, 1612, 2725, 1146, 1187, 1749, 1105, 263, 1331, -546},
    { -1993, 4928, -3884, 7384, 5035, 3983, 3927, 1601, 2717, 1142, 1179, 1749, 1100, 257, 1333, -560},
    { -1991, 4903, -3844, 7429, 5016, 3973, 3917, 1591, 2710, 1139, 1172, 1749, 1095, 252, 1334, -573},
    { -1990, 4878, -3802, 7473, 4998, 3964, 3908, 1581, 2703, 1135, 1164, 1749, 1090, 246, 1335, -587},
    { -1988, 4852, -3760, 7515, 4980, 3955, 3898, 1571, 2696, 1131, 1156, 1750, 1085, 240, 1336, -600},
    { -1986, 4826, -3715, 7556, 4962, 3947, 3889, 1562, 2689, 1128, 1149, 1750, 1080, 235, 1337, -613},
    { -1984, 4799, -3670, 7595, 4944, 3938, 3879, 1552, 2682, 1124, 1141, 1750, 1075, 229, 1338, -626},
    { -1983, 4773, -3623, 7633, 4926, 3930, 3870, 1543, 2675, 1121, 1134, 1750, 1070, 223, 1339, -639},
    { -1981, 4746, -3576, 7669, 4909, 3922, 3861, 1533, 2668, 1117, 1127, 1750, 1065, 218, 1340, -652},
    { -1979, 4719, -3527, 7704, 4891, 3914, 3851, 1524, 2661, 1114, 1119, 1751, 1060, 212, 1341, -665},
    { -1978, 4692, -3477, 7737, 4874, 3906, 3842, 1515, 2654, 1110, 1112, 1751, 1055, 206, 1342, -678},
    { -1976, 4665, -3426, 7769, 4856, 3899, 3832, 1506, 2647, 1107, 1105, 1751, 1050, 201, 1344, -691},
    { -1974, 4638, -3375, 7799, 4839, 3892, 3823, 1498, 2641, 1104, 1098, 1751, 1045, 195, 1345, -703},
    { -1973, 4611, -3322, 7828, 4822, 3885, 3814, 1490, 2634, 1101, 1091, 1751, 1040, 190, 1346, -716},
    { -1971, 4584, -3269, 7855, 4806, 3878, 3804, 1481, 2628, 1098, 1084, 1752, 1036, 184, 1347, -728},
    { -1970, 4557, -3215, 7881, 4789, 3871, 3795, 1473, 2621, 1095, 1077, 1752, 1031, 179, 1348, -740},
    { -1968, 4530, -3161, 7905, 4773, 3864, 3786, 1466, 2615, 1092, 1070, 1752, 1026, 174, 1349, -752},
    { -1967, 4504, -3106, 7927, 4758, 3858, 3777, 1458, 2609, 1089, 1064, 1752, 1022, 168, 1350, -764},
    { -1965, 4479, -3052, 7948, 4742, 3852, 3768, 1451, 2603, 1086, 1058, 1752, 1017, 163, 1351, -775},
    { -1964, 4454, -2997, 7966, 4728, 3847, 3760, 1444, 2597, 1083, 1052, 1753, 1013, 158, 1352, -786},
    { -1963, 4430, -2943, 7982, 4714, 3842, 3752, 1438, 2591, 1081, 1046, 1753, 1009, 154, 1352, -796},
    { -1962, 4407, -2891, 7995, 4701, 3837, 3744, 1433, 2587, 1079, 1041, 1754, 1006, 150, 1353, -805},
    { -1962, 4388, -2843, 8003, 4690, 3834, 3738, 1429, 2583, 1078, 1037, 1754, 1003, 147, 1354, -812},
    { -1962, 4376, -2804, 8001, 4684, 3834, 3735, 1428, 2582, 1078, 1036, 1756, 1002, 146, 1354, -814},
    { -1965, 4382, -2793, 7971, 4690, 3842, 3741, 1435, 2588, 1083, 1042, 1759, 1006, 152, 1353, -804},
    { -1986, 4514, -2982, 7761, 4786, 3906, 3805, 1498, 2639, 1113, 1091, 1772, 1042, 194, 1348, -718},
    { -1902, 3978, -2159, 8465, 4445, 3691, 3562, 1260, 2437, 985, 885, 1706, 891, -2, 1380, -1099}
  };
  int par_fg32[192][16] = {
    { 155, -103, 6959, -12479, 13646, 4760, -3497, -2417, -1903, -2181, -1320, -809, -438, -1065, 773, -2399},
    { 155, -13, 6807, -12408, 13819, 4570, -3496, -2413, -1906, -2172, -1320, -802, -442, -1063, 769, -2407},
    { 155, 80, 6650, -12328, 13987, 4381, -3495, -2410, -1910, -2164, -1320, -795, -446, -1060, 765, -2415},
    { 155, 175, 6487, -12239, 14149, 4193, -3492, -2407, -1913, -2155, -1320, -788, -451, -1058, 761, -2423},
    { 155, 271, 6319, -12143, 14305, 4007, -3489, -2405, -1916, -2146, -1320, -781, -455, -1056, 758, -2431},
    { 155, 370, 6145, -12038, 14454, 3822, -3484, -2403, -1919, -2138, -1320, -774, -459, -1054, 754, -2439},
    { 155, 471, 5966, -11924, 14598, 3639, -3479, -2402, -1922, -2129, -1320, -767, -464, -1052, 751, -2448},
    { 155, 574, 5780, -11802, 14735, 3457, -3473, -2401, -1925, -2121, -1320, -760, -468, -1050, 747, -2456},
    { 155, 678, 5590, -11672, 14866, 3276, -3466, -2400, -1928, -2113, -1319, -753, -473, -1048, 743, -2465},
    { 155, 785, 5394, -11533, 14991, 3097, -3458, -2400, -1930, -2104, -1319, -747, -477, -1046, 740, -2473},
    { 155, 893, 5192, -11386, 15110, 2919, -3449, -2400, -1933, -2096, -1318, -740, -481, -1045, 736, -2482},
    { 155, 1003, 4986, -11230, 15221, 2743, -3440, -2400, -1935, -2088, -1318, -733, -486, -1043, 733, -2491},
    { 155, 1115, 4774, -11066, 15327, 2569, -3430, -2401, -1938, -2080, -1317, -726, -490, -1041, 729, -2500},
    { 155, 1229, 4557, -10893, 15425, 2396, -3418, -2402, -1940, -2071, -1316, -720, -495, -1040, 725, -2509},
    { 154, 1344, 4335, -10713, 15517, 2224, -3406, -2403, -1942, -2063, -1315, -713, -499, -1038, 722, -2518},
    { 154, 1460, 4108, -10524, 15602, 2055, -3394, -2405, -1944, -2055, -1314, -707, -504, -1037, 718, -2527},
    { 154, 1578, 3877, -10327, 15681, 1887, -3380, -2407, -1946, -2047, -1313, -700, -508, -1036, 715, -2536},
    { 154, 1697, 3641, -10121, 15752, 1720, -3366, -2409, -1947, -2038, -1311, -693, -513, -1035, 711, -2545},
    { 153, 1817, 3400, -9908, 15816, 1556, -3350, -2411, -1949, -2030, -1310, -687, -517, -1033, 707, -2554},
    { 153, 1939, 3155, -9687, 15873, 1393, -3334, -2414, -1950, -2022, -1308, -680, -522, -1032, 704, -2562},
    { 153, 2061, 2906, -9458, 15924, 1233, -3318, -2416, -1951, -2013, -1306, -674, -526, -1031, 700, -2571},
    { 152, 2185, 2653, -9221, 15966, 1074, -3300, -2419, -1951, -2004, -1305, -667, -531, -1030, 696, -2580},
    { 152, 2309, 2396, -8977, 16002, 917, -3282, -2422, -1952, -1996, -1303, -660, -535, -1029, 692, -2589},
    { 151, 2433, 2136, -8725, 16030, 762, -3263, -2426, -1952, -1987, -1300, -654, -540, -1027, 689, -2597},
    { 151, 2559, 1872, -8466, 16052, 609, -3243, -2429, -1952, -1978, -1298, -647, -544, -1026, 685, -2606},
    { 150, 2684, 1606, -8200, 16065, 458, -3223, -2432, -1952, -1969, -1296, -641, -548, -1025, 681, -2614},
    { 150, 2810, 1336, -7928, 16072, 309, -3202, -2436, -1952, -1960, -1293, -634, -553, -1024, 677, -2622},
    { 149, 2936, 1063, -7648, 16071, 163, -3180, -2440, -1951, -1951, -1290, -627, -557, -1023, 673, -2630},
    { 149, 3062, 789, -7362, 16063, 18, -3158, -2443, -1950, -1942, -1287, -621, -561, -1022, 669, -2637},
    { 148, 3188, 512, -7070, 16047, -124, -3135, -2447, -1949, -1932, -1284, -614, -565, -1020, 665, -2645},
    { 148, 3314, 233, -6772, 16024, -263, -3111, -2451, -1947, -1923, -1281, -607, -569, -1019, 661, -2652},
    { 147, 3439, -48, -6467, 15994, -401, -3087, -2455, -1946, -1913, -1278, -601, -574, -1018, 656, -2659},
    { 147, 3564, -330, -6158, 15956, -536, -3062, -2459, -1944, -1904, -1274, -594, -578, -1017, 652, -2666},
    { 146, 3688, -613, -5843, 15912, -669, -3037, -2462, -1941, -1894, -1271, -587, -582, -1015, 648, -2673},
    { 146, 3812, -897, -5523, 15860, -799, -3011, -2466, -1939, -1884, -1267, -580, -586, -1014, 644, -2679},
    { 145, 3934, -1182, -5199, 15801, -926, -2985, -2470, -1936, -1874, -1263, -574, -589, -1013, 639, -2685},
    { 145, 4055, -1466, -4870, 15735, -1052, -2958, -2474, -1933, -1863, -1259, -567, -593, -1011, 635, -2691},
    { 144, 4175, -1751, -4537, 15662, -1174, -2931, -2478, -1930, -1853, -1255, -560, -597, -1010, 630, -2697},
    { 144, 4294, -2036, -4200, 15582, -1294, -2903, -2481, -1926, -1843, -1251, -553, -601, -1009, 626, -2702},
    { 143, 4411, -2320, -3859, 15496, -1411, -2875, -2485, -1923, -1832, -1247, -547, -605, -1007, 621, -2707},
    { 143, 4526, -2603, -3516, 15403, -1526, -2847, -2489, -1918, -1821, -1242, -540, -608, -1006, 616, -2712},
    { 142, 4640, -2885, -3169, 15304, -1638, -2819, -2492, -1914, -1810, -1238, -533, -612, -1004, 612, -2716},
    { 142, 4752, -3166, -2820, 15198, -1747, -2790, -2495, -1910, -1800, -1233, -526, -615, -1003, 607, -2720},
    { 141, 4862, -3445, -2469, 15086, -1854, -2761, -2499, -1905, -1789, -1229, -520, -619, -1001, 602, -2724},
    { 141, 4970, -3722, -2115, 14968, -1957, -2732, -2502, -1900, -1777, -1224, -513, -622, -999, 597, -2727},
    { 141, 5075, -3997, -1760, 14845, -2058, -2702, -2505, -1895, -1766, -1219, -506, -626, -998, 592, -2731},
    { 141, 5178, -4269, -1404, 14716, -2156, -2673, -2508, -1890, -1755, -1215, -500, -629, -996, 587, -2734},
    { 140, 5279, -4539, -1047, 14581, -2252, -2643, -2512, -1884, -1744, -1210, -493, -632, -994, 582, -2736},
    { 140, 5377, -4805, -689, 14441, -2345, -2613, -2514, -1879, -1732, -1205, -486, -636, -993, 577, -2738},
    { 140, 5472, -5069, -331, 14296, -2435, -2584, -2517, -1873, -1721, -1200, -480, -639, -991, 572, -2741},
    { 140, 5565, -5329, 27, 14146, -2522, -2554, -2520, -1867, -1710, -1196, -473, -642, -989, 567, -2742},
    { 140, 5655, -5585, 385, 13991, -2606, -2524, -2523, -1861, -1698, -1191, -467, -645, -987, 561, -2744},
    { 140, 5742, -5837, 742, 13832, -2688, -2495, -2525, -1855, -1687, -1186, -460, -648, -986, 556, -2745},
    { 140, 5826, -6086, 1099, 13669, -2767, -2465, -2528, -1848, -1675, -1181, -454, -651, -984, 551, -2746},
    { 140, 5907, -6330, 1454, 13502, -2843, -2436, -2530, -1842, -1663, -1176, -448, -654, -982, 546, -2747},
    { 141, 5985, -6569, 1808, 13330, -2917, -2407, -2533, -1836, -1652, -1172, -442, -657, -980, 540, -2747},
    { 141, 6059, -6804, 2160, 13156, -2988, -2378, -2535, -1829, -1640, -1167, -435, -660, -979, 535, -2747},
    { 141, 6131, -7034, 2510, 12978, -3057, -2349, -2537, -1823, -1629, -1163, -429, -663, -977, 530, -2747},
    { 142, 6199, -7258, 2859, 12796, -3123, -2321, -2540, -1816, -1617, -1158, -423, -666, -975, 524, -2747},
    { 142, 6264, -7478, 3204, 12612, -3186, -2293, -2542, -1809, -1606, -1154, -417, -669, -973, 519, -2747},
    { 143, 6325, -7692, 3547, 12426, -3247, -2265, -2544, -1803, -1595, -1150, -411, -672, -971, 513, -2746},
    { 144, 6383, -7901, 3887, 12236, -3305, -2238, -2546, -1796, -1583, -1145, -405, -675, -970, 508, -2745},
    { 144, 6437, -8104, 4224, 12045, -3362, -2211, -2548, -1790, -1572, -1141, -400, -678, -968, 502, -2745},
    { 145, 6489, -8302, 4558, 11851, -3415, -2184, -2550, -1783, -1561, -1138, -394, -680, -966, 497, -2743},
    { 146, 6536, -8494, 4889, 11655, -3467, -2158, -2552, -1777, -1550, -1134, -388, -683, -965, 491, -2742},
    { 147, 6580, -8680, 5216, 11458, -3516, -2132, -2554, -1770, -1539, -1130, -383, -686, -963, 486, -2741},
    { 148, 6621, -8860, 5539, 11259, -3563, -2107, -2556, -1764, -1528, -1127, -377, -689, -962, 481, -2740},
    { 149, 6658, -9034, 5858, 11059, -3608, -2082, -2558, -1757, -1517, -1123, -372, -692, -960, 475, -2738},
    { 151, 6692, -9202, 6173, 10858, -3651, -2058, -2560, -1751, -1507, -1120, -367, -695, -959, 470, -2736},
    { 152, 6722, -9364, 6485, 10656, -3691, -2034, -2562, -1745, -1496, -1117, -361, -698, -957, 464, -2735},
    { 154, 6749, -9520, 6791, 10453, -3730, -2011, -2564, -1739, -1486, -1115, -356, -701, -956, 459, -2733},
    { 155, 6772, -9670, 7094, 10249, -3767, -1989, -2566, -1733, -1476, -1112, -351, -704, -954, 454, -2731},
    { 157, 6792, -9813, 7392, 10045, -3802, -1967, -2568, -1727, -1466, -1110, -346, -707, -953, 448, -2729},
    { 159, 6808, -9950, 7686, 9841, -3835, -1945, -2570, -1722, -1456, -1108, -341, -710, -952, 443, -2727},
    { 160, 6820, -10081, 7975, 9636, -3866, -1924, -2572, -1716, -1446, -1106, -337, -713, -951, 438, -2726},
    { 162, 6830, -10205, 8259, 9432, -3896, -1904, -2575, -1711, -1436, -1104, -332, -716, -950, 432, -2724},
    { 164, 6835, -10324, 8539, 9228, -3924, -1885, -2577, -1706, -1427, -1102, -327, -719, -949, 427, -2722},
    { 167, 6837, -10435, 8814, 9023, -3950, -1866, -2579, -1701, -1417, -1101, -323, -722, -948, 422, -2720},
    { 169, 6836, -10541, 9084, 8820, -3975, -1847, -2582, -1696, -1408, -1100, -318, -725, -947, 417, -2718},
    { 171, 6831, -10640, 9349, 8616, -3998, -1830, -2584, -1691, -1399, -1099, -314, -728, -946, 411, -2716},
    { 174, 6823, -10732, 9609, 8414, -4020, -1813, -2587, -1687, -1391, -1099, -310, -732, -946, 406, -2715},
    { 176, 6811, -10818, 9865, 8212, -4040, -1796, -2589, -1683, -1382, -1099, -306, -735, -945, 401, -2713},
    { 179, 6796, -10898, 10115, 8011, -4060, -1781, -2592, -1679, -1373, -1099, -302, -738, -944, 396, -2711},
    { 181, 6777, -10971, 10360, 7811, -4077, -1766, -2595, -1675, -1365, -1099, -298, -742, -944, 391, -2710},
    { 184, 6754, -11038, 10600, 7611, -4094, -1751, -2598, -1671, -1357, -1099, -294, -745, -944, 386, -2708},
    { 187, 6727, -11097, 10835, 7414, -4109, -1738, -2601, -1668, -1349, -1100, -290, -749, -943, 381, -2707},
    { 190, 6697, -11150, 11065, 7217, -4124, -1725, -2605, -1665, -1342, -1101, -286, -752, -943, 376, -2706},
    { 193, 6662, -11196, 11289, 7022, -4137, -1713, -2608, -1662, -1334, -1103, -282, -756, -943, 371, -2704},
    { 197, 6622, -11234, 11507, 6829, -4149, -1701, -2612, -1660, -1327, -1105, -279, -760, -943, 367, -2703},
    { 200, 6577, -11263, 11719, 6638, -4160, -1691, -2616, -1657, -1320, -1107, -275, -764, -943, 362, -2702},
    { 204, 6525, -11283, 11925, 6450, -4171, -1681, -2620, -1655, -1313, -1110, -271, -768, -943, 357, -2701},
    { 208, 6462, -11289, 12121, 6267, -4181, -1673, -2625, -1654, -1307, -1114, -268, -772, -943, 352, -2700},
    { 212, 6381, -11275, 12305, 6091, -4192, -1666, -2631, -1654, -1301, -1119, -264, -776, -942, 348, -2699},
    { 218, 6249, -11210, 12460, 5938, -4205, -1664, -2640, -1655, -1297, -1129, -260, -780, -941, 343, -2697},
    { 233, 5787, -10828, 12437, 5924, -4241, -1687, -2670, -1670, -1301, -1165, -251, -786, -932, 337, -2687},
    { 200, 6975, -11994, 13248, 5184, -4140, -1579, -2585, -1613, -1259, -1063, -264, -783, -959, 336, -2707},
    { 223, 6288, -11406, 13126, 5285, -4202, -1625, -2637, -1642, -1273, -1121, -253, -791, -947, 330, -2699},
    { 227, 6228, -11395, 13304, 5101, -4205, -1619, -2641, -1641, -1267, -1125, -250, -795, -948, 326, -2699},
    { 231, 6165, -11379, 13476, 4918, -4208, -1613, -2645, -1640, -1261, -1128, -247, -800, -949, 321, -2699},
    { 234, 6098, -11356, 13644, 4737, -4209, -1608, -2649, -1640, -1255, -1132, -244, -804, -950, 317, -2700},
    { 238, 6029, -11328, 13807, 4557, -4210, -1603, -2653, -1639, -1250, -1137, -241, -809, -952, 313, -2700},
    { 242, 5957, -11293, 13964, 4379, -4210, -1600, -2657, -1639, -1244, -1141, -239, -813, -953, 309, -2700},
    { 247, 5881, -11252, 14117, 4202, -4209, -1596, -2661, -1639, -1239, -1146, -236, -818, -954, 305, -2701},
    { 251, 5803, -11205, 14265, 4027, -4207, -1593, -2665, -1639, -1234, -1151, -233, -822, -956, 300, -2702},
    { 255, 5721, -11152, 14407, 3854, -4205, -1591, -2669, -1640, -1229, -1156, -230, -827, -957, 296, -2702},
    { 259, 5637, -11093, 14545, 3682, -4202, -1589, -2674, -1640, -1224, -1162, -228, -831, -959, 292, -2703},
    { 264, 5550, -11027, 14677, 3512, -4198, -1588, -2678, -1641, -1220, -1167, -225, -836, -960, 289, -2704},
    { 268, 5460, -10956, 14804, 3343, -4194, -1588, -2682, -1642, -1215, -1173, -223, -841, -962, 285, -2706},
    { 273, 5366, -10878, 14926, 3176, -4189, -1588, -2686, -1643, -1211, -1179, -220, -846, -964, 281, -2707},
    { 277, 5270, -10794, 15043, 3011, -4183, -1588, -2690, -1644, -1206, -1186, -218, -851, -966, 277, -2708},
    { 282, 5172, -10705, 15155, 2847, -4177, -1589, -2694, -1645, -1202, -1192, -215, -855, -968, 273, -2709},
    { 286, 5070, -10609, 15262, 2685, -4170, -1590, -2699, -1646, -1198, -1199, -213, -860, -970, 270, -2711},
    { 291, 4966, -10507, 15363, 2525, -4162, -1592, -2703, -1648, -1194, -1206, -210, -865, -972, 266, -2712},
    { 295, 4858, -10399, 15459, 2366, -4154, -1595, -2706, -1650, -1190, -1213, -208, -870, -974, 262, -2714},
    { 300, 4749, -10285, 15549, 2209, -4145, -1598, -2710, -1651, -1186, -1220, -206, -875, -976, 259, -2716},
    { 305, 4636, -10165, 15635, 2054, -4136, -1601, -2714, -1653, -1182, -1228, -203, -880, -979, 255, -2717},
    { 310, 4521, -10039, 15714, 1900, -4126, -1605, -2718, -1655, -1178, -1235, -201, -886, -981, 252, -2719},
    { 314, 4403, -9906, 15789, 1748, -4115, -1609, -2721, -1658, -1175, -1243, -199, -891, -984, 248, -2721},
    { 319, 4283, -9768, 15857, 1598, -4103, -1613, -2725, -1660, -1171, -1251, -197, -896, -986, 245, -2723},
    { 324, 4160, -9624, 15920, 1450, -4091, -1618, -2728, -1662, -1167, -1259, -194, -901, -989, 241, -2724},
    { 329, 4035, -9474, 15978, 1303, -4079, -1624, -2731, -1664, -1164, -1267, -192, -906, -991, 238, -2726},
    { 334, 3907, -9318, 16030, 1158, -4066, -1630, -2734, -1667, -1160, -1275, -190, -911, -994, 235, -2728},
    { 339, 3778, -9157, 16076, 1015, -4052, -1636, -2737, -1669, -1157, -1283, -188, -916, -996, 232, -2730},
    { 344, 3645, -8989, 16117, 874, -4037, -1642, -2739, -1672, -1153, -1292, -186, -922, -999, 228, -2732},
    { 349, 3511, -8816, 16152, 734, -4022, -1649, -2742, -1674, -1150, -1300, -184, -927, -1002, 225, -2734},
    { 353, 3375, -8637, 16181, 597, -4007, -1656, -2744, -1677, -1146, -1309, -182, -932, -1005, 222, -2735},
    { 358, 3236, -8453, 16204, 461, -3991, -1664, -2746, -1680, -1143, -1317, -180, -937, -1007, 219, -2737},
    { 363, 3096, -8263, 16221, 327, -3974, -1672, -2747, -1682, -1139, -1326, -178, -942, -1010, 216, -2739},
    { 368, 2954, -8067, 16233, 195, -3956, -1680, -2749, -1685, -1136, -1335, -176, -947, -1013, 213, -2741},
    { 373, 2809, -7867, 16238, 65, -3938, -1688, -2750, -1688, -1133, -1343, -174, -952, -1016, 210, -2742},
    { 378, 2664, -7661, 16238, -64, -3920, -1697, -2751, -1691, -1129, -1352, -172, -958, -1019, 207, -2744},
    { 383, 2516, -7449, 16232, -190, -3901, -1706, -2751, -1693, -1126, -1361, -170, -963, -1022, 204, -2745},
    { 388, 2367, -7233, 16220, -314, -3881, -1715, -2751, -1696, -1122, -1370, -168, -968, -1025, 202, -2747},
    { 392, 2217, -7012, 16202, -436, -3861, -1725, -2751, -1699, -1119, -1379, -166, -973, -1028, 199, -2748},
    { 397, 2065, -6785, 16177, -556, -3840, -1734, -2751, -1702, -1115, -1387, -164, -977, -1031, 196, -2749},
    { 402, 1912, -6554, 16147, -674, -3819, -1744, -2750, -1704, -1112, -1396, -163, -982, -1034, 193, -2750},
    { 407, 1758, -6319, 16111, -790, -3798, -1755, -2749, -1707, -1108, -1405, -161, -987, -1037, 190, -2752},
    { 411, 1603, -6079, 16069, -904, -3775, -1765, -2748, -1710, -1105, -1413, -159, -992, -1040, 188, -2752},
    { 416, 1447, -5834, 16022, -1016, -3753, -1776, -2746, -1712, -1101, -1422, -158, -997, -1043, 185, -2753},
    { 421, 1290, -5586, 15968, -1126, -3730, -1786, -2744, -1715, -1098, -1431, -156, -1002, -1046, 183, -2754},
    { 425, 1133, -5333, 15908, -1233, -3706, -1797, -2741, -1717, -1094, -1439, -154, -1006, -1049, 180, -2755},
    { 430, 975, -5076, 15843, -1338, -3682, -1808, -2738, -1720, -1090, -1447, -153, -1011, -1052, 177, -2755},
    { 434, 816, -4816, 15772, -1442, -3657, -1820, -2735, -1722, -1087, -1456, -151, -1015, -1055, 175, -2756},
    { 439, 658, -4552, 15695, -1542, -3633, -1831, -2731, -1725, -1083, -1464, -150, -1020, -1058, 172, -2756},
    { 443, 499, -4285, 15612, -1641, -3607, -1842, -2727, -1727, -1079, -1472, -149, -1024, -1061, 170, -2756},
    { 448, 340, -4015, 15524, -1737, -3582, -1854, -2723, -1729, -1075, -1480, -147, -1029, -1064, 167, -2756},
    { 452, 181, -3741, 15430, -1832, -3556, -1866, -2718, -1732, -1072, -1488, -146, -1033, -1067, 165, -2756},
    { 456, 22, -3465, 15331, -1923, -3529, -1878, -2713, -1734, -1068, -1496, -145, -1037, -1070, 163, -2756},
    { 460, -136, -3186, 15227, -2013, -3503, -1889, -2707, -1736, -1064, -1503, -144, -1041, -1073, 160, -2756},
    { 464, -294, -2905, 15117, -2100, -3476, -1901, -2701, -1738, -1060, -1511, -143, -1045, -1076, 158, -2755},
    { 468, -451, -2622, 15002, -2185, -3449, -1913, -2695, -1740, -1056, -1518, -142, -1049, -1079, 156, -2754},
    { 472, -608, -2336, 14883, -2268, -3421, -1926, -2688, -1742, -1052, -1525, -141, -1053, -1082, 153, -2754},
    { 476, -764, -2049, 14758, -2348, -3393, -1938, -2681, -1744, -1048, -1532, -140, -1057, -1085, 151, -2753},
    { 480, -918, -1760, 14628, -2426, -3366, -1950, -2673, -1746, -1044, -1539, -139, -1061, -1088, 149, -2752},
    { 484, -1072, -1469, 14494, -2502, -3337, -1962, -2666, -1747, -1040, -1545, -139, -1065, -1091, 147, -2751},
    { 488, -1224, -1178, 14355, -2576, -3309, -1975, -2657, -1749, -1036, -1552, -138, -1068, -1094, 145, -2750},
    { 492, -1376, -885, 14211, -2647, -3281, -1987, -2649, -1751, -1032, -1558, -138, -1072, -1097, 142, -2748},
    { 495, -1525, -592, 14064, -2716, -3253, -1999, -2640, -1752, -1027, -1564, -137, -1076, -1100, 140, -2747},
    { 499, -1673, -298, 13912, -2783, -3224, -2012, -2630, -1754, -1023, -1570, -137, -1079, -1103, 138, -2745},
    { 502, -1820, -3, 13756, -2847, -3196, -2024, -2621, -1755, -1019, -1576, -137, -1082, -1106, 136, -2744},
    { 506, -1964, 291, 13596, -2909, -3167, -2036, -2611, -1756, -1015, -1581, -136, -1086, -1109, 134, -2742},
    { 509, -2107, 586, 13433, -2969, -3138, -2049, -2600, -1758, -1011, -1586, -136, -1089, -1112, 132, -2740},
    { 512, -2248, 880, 13266, -3027, -3110, -2061, -2590, -1759, -1006, -1591, -136, -1092, -1115, 130, -2738},
    { 516, -2387, 1174, 13095, -3083, -3081, -2074, -2579, -1760, -1002, -1596, -136, -1095, -1117, 128, -2736},
    { 519, -2523, 1468, 12921, -3136, -3053, -2086, -2567, -1761, -998, -1601, -137, -1098, -1120, 126, -2734},
    { 522, -2657, 1760, 12744, -3188, -3024, -2098, -2556, -1763, -994, -1605, -137, -1101, -1123, 124, -2732},
    { 525, -2789, 2052, 12564, -3237, -2996, -2110, -2544, -1764, -989, -1609, -137, -1104, -1126, 122, -2730},
    { 528, -2918, 2343, 12382, -3284, -2968, -2123, -2532, -1765, -985, -1613, -138, -1107, -1129, 120, -2728},
    { 531, -3045, 2632, 12196, -3330, -2940, -2135, -2519, -1766, -981, -1617, -139, -1110, -1132, 118, -2725},
    { 534, -3169, 2920, 12008, -3373, -2912, -2147, -2507, -1767, -977, -1621, -139, -1113, -1135, 116, -2723},
    { 536, -3291, 3207, 11818, -3414, -2885, -2159, -2494, -1768, -973, -1624, -140, -1115, -1138, 114, -2721},
    { 539, -3410, 3492, 11625, -3453, -2858, -2171, -2481, -1769, -968, -1627, -141, -1118, -1141, 113, -2718},
    { 542, -3526, 3775, 11431, -3491, -2830, -2184, -2468, -1770, -964, -1630, -142, -1121, -1144, 111, -2716},
    { 544, -3639, 4056, 11234, -3526, -2804, -2196, -2454, -1771, -960, -1633, -143, -1123, -1147, 109, -2713},
    { 547, -3749, 4334, 11036, -3560, -2777, -2208, -2440, -1772, -956, -1635, -144, -1126, -1150, 107, -2710},
    { 549, -3856, 4611, 10836, -3592, -2751, -2220, -2426, -1773, -952, -1637, -145, -1128, -1153, 105, -2708},
    { 552, -3960, 4885, 10635, -3622, -2725, -2232, -2412, -1774, -947, -1639, -147, -1130, -1156, 103, -2705},
    { 554, -4061, 5157, 10433, -3651, -2700, -2243, -2398, -1774, -943, -1641, -148, -1133, -1158, 101, -2702},
    { 556, -4158, 5426, 10229, -3677, -2674, -2255, -2383, -1775, -939, -1643, -150, -1135, -1161, 99, -2699},
    { 558, -4253, 5693, 10024, -3703, -2650, -2267, -2369, -1776, -935, -1644, -151, -1137, -1164, 98, -2696},
    { 560, -4344, 5956, 9819, -3727, -2625, -2279, -2354, -1777, -931, -1645, -153, -1139, -1166, 96, -2692},
    { 562, -4432, 6218, 9612, -3749, -2602, -2290, -2339, -1778, -926, -1645, -154, -1141, -1169, 94, -2689},
    { 564, -4517, 6476, 9406, -3770, -2578, -2302, -2324, -1778, -922, -1646, -155, -1143, -1171, 91, -2685},
    { 565, -4598, 6731, 9199, -3790, -2555, -2313, -2308, -1779, -917, -1645, -156, -1144, -1172, 89, -2680},
    { 566, -4676, 6984, 8991, -3809, -2533, -2325, -2292, -1779, -912, -1644, -157, -1145, -1173, 87, -2674},
    { 567, -4750, 7234, 8784, -3827, -2512, -2336, -2276, -1779, -906, -1643, -157, -1146, -1173, 84, -2666},
    { 567, -4820, 7483, 8578, -3845, -2491, -2347, -2259, -1778, -900, -1640, -156, -1146, -1171, 81, -2656},
    { 565, -4885, 7730, 8373, -3864, -2472, -2358, -2241, -1776, -891, -1635, -153, -1143, -1165, 76, -2641},
    { 561, -4945, 7978, 8171, -3885, -2455, -2368, -2220, -1772, -879, -1625, -145, -1138, -1152, 69, -2613},
    { 548, -4992, 8237, 7979, -3919, -2443, -2376, -2192, -1759, -853, -1601, -121, -1120, -1115, 55, -2550},
    { 475, -4970, 8579, 7844, -4036, -2466, -2368, -2109, -1687, -739, -1479, 8, -1023, -912, -9, -2235},
    { 702, -5230, 8396, 7349, -3684, -2295, -2421, -2283, -1899, -1070, -1836, -401, -1321, -1540, 181, -3174}
  };
  int par_fg33[192][16] = {
    { 6732, 2587, 3627, 5377, 2724, -1898, -1335, 885, 220, 1805, 1608, 1502, 1102, 2956, -1134, 3326},
    { 6733, 2574, 3616, 5422, 2642, -1898, -1323, 893, 226, 1812, 1613, 1504, 1104, 2960, -1137, 3336},
    { 6734, 2561, 3607, 5465, 2560, -1898, -1312, 901, 232, 1819, 1617, 1505, 1105, 2963, -1141, 3347},
    { 6734, 2547, 3598, 5508, 2478, -1897, -1300, 909, 239, 1826, 1622, 1507, 1107, 2967, -1144, 3357},
    { 6735, 2532, 3591, 5548, 2396, -1896, -1289, 917, 245, 1833, 1626, 1508, 1108, 2971, -1148, 3368},
    { 6736, 2517, 3586, 5588, 2314, -1894, -1278, 925, 251, 1840, 1630, 1509, 1109, 2974, -1151, 3379},
    { 6736, 2501, 3582, 5625, 2232, -1892, -1266, 933, 257, 1847, 1635, 1511, 1111, 2978, -1155, 3389},
    { 6737, 2485, 3579, 5662, 2150, -1888, -1255, 941, 264, 1854, 1639, 1512, 1112, 2982, -1158, 3400},
    { 6738, 2467, 3578, 5697, 2069, -1885, -1244, 949, 270, 1861, 1644, 1513, 1114, 2986, -1162, 3411},
    { 6739, 2449, 3578, 5730, 1987, -1880, -1234, 958, 276, 1868, 1648, 1514, 1116, 2989, -1165, 3422},
    { 6739, 2430, 3580, 5762, 1905, -1875, -1223, 966, 283, 1874, 1652, 1516, 1117, 2993, -1169, 3433},
    { 6740, 2411, 3583, 5792, 1824, -1870, -1212, 975, 289, 1881, 1657, 1517, 1119, 2997, -1172, 3444},
    { 6741, 2391, 3587, 5821, 1743, -1864, -1202, 983, 295, 1888, 1661, 1518, 1120, 3001, -1176, 3456},
    { 6741, 2370, 3594, 5847, 1662, -1857, -1191, 992, 302, 1895, 1666, 1519, 1122, 3004, -1179, 3467},
    { 6742, 2348, 3601, 5872, 1581, -1850, -1181, 1000, 308, 1902, 1670, 1520, 1124, 3008, -1182, 3478},
    { 6743, 2326, 3611, 5896, 1501, -1843, -1171, 1009, 314, 1909, 1674, 1521, 1125, 3012, -1186, 3489},
    { 6743, 2303, 3622, 5917, 1421, -1835, -1161, 1018, 321, 1915, 1679, 1523, 1127, 3016, -1189, 3501},
    { 6744, 2279, 3634, 5937, 1342, -1826, -1151, 1026, 327, 1922, 1683, 1524, 1129, 3020, -1193, 3512},
    { 6745, 2255, 3648, 5955, 1262, -1817, -1141, 1035, 333, 1929, 1687, 1525, 1131, 3023, -1196, 3524},
    { 6746, 2230, 3664, 5971, 1184, -1807, -1132, 1044, 340, 1936, 1692, 1526, 1133, 3027, -1199, 3536},
    { 6746, 2204, 3681, 5986, 1105, -1797, -1122, 1053, 346, 1942, 1696, 1527, 1134, 3031, -1203, 3547},
    { 6747, 2178, 3700, 5998, 1028, -1787, -1113, 1062, 353, 1949, 1700, 1528, 1136, 3035, -1206, 3559},
    { 6748, 2151, 3720, 6009, 950, -1775, -1104, 1071, 359, 1956, 1705, 1529, 1138, 3039, -1209, 3571},
    { 6748, 2123, 3742, 6017, 874, -1764, -1095, 1081, 365, 1962, 1709, 1530, 1140, 3043, -1213, 3583},
    { 6749, 2094, 3766, 6023, 798, -1752, -1086, 1090, 372, 1969, 1713, 1531, 1142, 3047, -1216, 3595},
    { 6750, 2065, 3791, 6028, 722, -1740, -1077, 1099, 378, 1975, 1717, 1531, 1144, 3050, -1219, 3607},
    { 6751, 2035, 3818, 6031, 647, -1727, -1068, 1108, 384, 1982, 1722, 1532, 1146, 3054, -1223, 3619},
    { 6751, 2005, 3846, 6031, 573, -1714, -1060, 1118, 391, 1988, 1726, 1533, 1148, 3058, -1226, 3631},
    { 6752, 1974, 3876, 6029, 500, -1700, -1052, 1127, 397, 1995, 1730, 1534, 1150, 3062, -1229, 3643},
    { 6753, 1942, 3908, 6026, 428, -1686, -1044, 1137, 403, 2001, 1734, 1535, 1152, 3066, -1232, 3655},
    { 6754, 1910, 3941, 6020, 356, -1672, -1035, 1146, 410, 2007, 1738, 1535, 1154, 3070, -1235, 3667},
    { 6755, 1877, 3975, 6013, 285, -1657, -1028, 1156, 416, 2014, 1742, 1536, 1156, 3074, -1239, 3679},
    { 6755, 1844, 4011, 6003, 215, -1642, -1020, 1165, 422, 2020, 1747, 1537, 1158, 3078, -1242, 3691},
    { 6756, 1810, 4049, 5991, 146, -1627, -1012, 1175, 428, 2026, 1751, 1538, 1160, 3082, -1245, 3704},
    { 6757, 1775, 4088, 5977, 78, -1611, -1005, 1185, 434, 2032, 1755, 1538, 1162, 3086, -1248, 3716},
    { 6758, 1740, 4128, 5961, 11, -1595, -998, 1194, 441, 2039, 1759, 1539, 1164, 3089, -1251, 3728},
    { 6758, 1705, 4170, 5943, -55, -1578, -991, 1204, 447, 2045, 1763, 1539, 1167, 3093, -1254, 3741},
    { 6759, 1669, 4213, 5923, -120, -1562, -984, 1214, 453, 2051, 1767, 1540, 1169, 3097, -1257, 3753},
    { 6760, 1632, 4258, 5901, -184, -1545, -977, 1224, 459, 2057, 1771, 1540, 1171, 3101, -1260, 3765},
    { 6761, 1596, 4304, 5877, -247, -1528, -970, 1234, 465, 2063, 1775, 1541, 1173, 3105, -1263, 3778},
    { 6762, 1558, 4351, 5850, -309, -1511, -964, 1244, 471, 2069, 1779, 1541, 1175, 3109, -1266, 3790},
    { 6762, 1521, 4400, 5822, -369, -1493, -958, 1254, 477, 2074, 1783, 1542, 1178, 3113, -1269, 3803},
    { 6763, 1483, 4449, 5792, -429, -1475, -951, 1264, 483, 2080, 1787, 1542, 1180, 3117, -1272, 3815},
    { 6764, 1444, 4500, 5760, -488, -1457, -945, 1274, 489, 2086, 1790, 1543, 1182, 3121, -1275, 3828},
    { 6765, 1406, 4552, 5726, -545, -1439, -940, 1284, 495, 2092, 1794, 1543, 1184, 3125, -1278, 3840},
    { 6765, 1367, 4605, 5690, -601, -1421, -934, 1294, 500, 2097, 1798, 1543, 1187, 3129, -1281, 3852},
    { 6766, 1328, 4659, 5652, -656, -1403, -928, 1304, 506, 2103, 1802, 1544, 1189, 3132, -1284, 3865},
    { 6767, 1288, 4715, 5612, -710, -1384, -923, 1314, 512, 2109, 1806, 1544, 1191, 3136, -1287, 3877},
    { 6768, 1248, 4771, 5571, -762, -1365, -917, 1324, 518, 2114, 1810, 1544, 1194, 3140, -1290, 3890},
    { 6768, 1209, 4828, 5527, -814, -1347, -912, 1334, 523, 2120, 1813, 1544, 1196, 3144, -1292, 3902},
    { 6769, 1168, 4886, 5482, -864, -1328, -907, 1344, 529, 2125, 1817, 1545, 1198, 3148, -1295, 3915},
    { 6770, 1128, 4945, 5435, -913, -1309, -902, 1355, 535, 2130, 1821, 1545, 1201, 3152, -1298, 3927},
    { 6771, 1088, 5005, 5387, -960, -1290, -897, 1365, 540, 2136, 1825, 1545, 1203, 3156, -1301, 3940},
    { 6771, 1048, 5065, 5336, -1007, -1270, -893, 1375, 546, 2141, 1828, 1545, 1206, 3160, -1303, 3952},
    { 6772, 1007, 5126, 5285, -1052, -1251, -888, 1385, 551, 2146, 1832, 1545, 1208, 3164, -1306, 3965},
    { 6773, 967, 5188, 5231, -1096, -1232, -884, 1396, 557, 2151, 1836, 1545, 1211, 3167, -1309, 3977},
    { 6774, 926, 5251, 5176, -1139, -1213, -879, 1406, 562, 2157, 1839, 1545, 1213, 3171, -1311, 3989},
    { 6774, 886, 5314, 5120, -1181, -1193, -875, 1416, 568, 2162, 1843, 1545, 1215, 3175, -1314, 4002},
    { 6775, 845, 5377, 5062, -1221, -1174, -871, 1427, 573, 2167, 1847, 1546, 1218, 3179, -1317, 4014},
    { 6776, 804, 5441, 5002, -1261, -1155, -867, 1437, 578, 2172, 1851, 1546, 1220, 3183, -1319, 4027},
    { 6776, 764, 5506, 4941, -1299, -1135, -863, 1448, 584, 2177, 1854, 1546, 1223, 3187, -1322, 4039},
    { 6777, 724, 5571, 4879, -1336, -1116, -859, 1458, 589, 2182, 1858, 1545, 1226, 3191, -1324, 4052},
    { 6778, 684, 5636, 4816, -1372, -1097, -855, 1469, 595, 2187, 1862, 1545, 1228, 3195, -1327, 4064},
    { 6778, 643, 5702, 4751, -1407, -1077, -851, 1479, 600, 2192, 1865, 1545, 1231, 3198, -1329, 4076},
    { 6779, 603, 5768, 4685, -1441, -1058, -847, 1490, 605, 2196, 1869, 1545, 1233, 3202, -1332, 4089},
    { 6780, 564, 5834, 4618, -1473, -1038, -843, 1500, 610, 2201, 1873, 1545, 1236, 3206, -1334, 4101},
    { 6780, 524, 5900, 4549, -1505, -1019, -840, 1511, 616, 2206, 1876, 1545, 1239, 3210, -1336, 4114},
    { 6781, 485, 5967, 4480, -1535, -1000, -836, 1521, 621, 2211, 1880, 1545, 1241, 3214, -1339, 4126},
    { 6781, 445, 6034, 4409, -1565, -981, -832, 1532, 626, 2216, 1884, 1545, 1244, 3218, -1341, 4139},
    { 6782, 406, 6101, 4337, -1593, -961, -829, 1543, 632, 2220, 1888, 1545, 1247, 3222, -1344, 4151},
    { 6782, 368, 6168, 4265, -1621, -942, -825, 1553, 637, 2225, 1892, 1545, 1249, 3226, -1346, 4163},
    { 6783, 329, 6235, 4191, -1647, -923, -822, 1564, 642, 2230, 1895, 1544, 1252, 3230, -1348, 4176},
    { 6783, 291, 6302, 4116, -1672, -904, -818, 1575, 647, 2234, 1899, 1544, 1255, 3234, -1350, 4188},
    { 6784, 253, 6368, 4040, -1697, -885, -815, 1586, 653, 2239, 1903, 1544, 1258, 3238, -1353, 4201},
    { 6784, 216, 6435, 3964, -1721, -866, -811, 1597, 658, 2244, 1907, 1544, 1260, 3242, -1355, 4213},
    { 6785, 179, 6502, 3886, -1743, -847, -808, 1608, 663, 2248, 1911, 1544, 1263, 3246, -1357, 4226},
    { 6785, 142, 6569, 3808, -1765, -828, -805, 1619, 669, 2253, 1915, 1543, 1266, 3250, -1359, 4238},
    { 6785, 105, 6635, 3728, -1786, -809, -801, 1630, 674, 2258, 1919, 1543, 1269, 3254, -1362, 4251},
    { 6786, 69, 6702, 3648, -1806, -790, -798, 1641, 679, 2262, 1923, 1543, 1272, 3258, -1364, 4264},
    { 6786, 33, 6768, 3567, -1825, -771, -794, 1652, 685, 2267, 1927, 1543, 1275, 3262, -1366, 4276},
    { 6786, -2, 6834, 3486, -1843, -753, -791, 1663, 690, 2271, 1931, 1542, 1278, 3266, -1368, 4289},
    { 6786, -37, 6899, 3403, -1861, -734, -787, 1674, 695, 2276, 1935, 1542, 1281, 3270, -1370, 4302},
    { 6787, -71, 6964, 3320, -1877, -715, -784, 1685, 701, 2280, 1939, 1542, 1284, 3274, -1372, 4314},
    { 6787, -105, 7029, 3236, -1893, -696, -780, 1697, 706, 2285, 1944, 1542, 1287, 3278, -1375, 4327},
    { 6787, -139, 7094, 3152, -1908, -678, -776, 1708, 712, 2290, 1948, 1541, 1290, 3282, -1377, 4340},
    { 6787, -171, 7158, 3067, -1922, -659, -773, 1719, 717, 2294, 1952, 1541, 1293, 3286, -1379, 4353},
    { 6787, -204, 7222, 2981, -1936, -640, -769, 1731, 723, 2299, 1957, 1541, 1296, 3290, -1381, 4365},
    { 6787, -235, 7285, 2895, -1949, -622, -765, 1742, 728, 2303, 1961, 1540, 1300, 3295, -1383, 4378},
    { 6788, -266, 7347, 2808, -1961, -603, -762, 1754, 734, 2308, 1966, 1540, 1303, 3299, -1385, 4391},
    { 6788, -297, 7409, 2720, -1973, -585, -758, 1766, 739, 2312, 1970, 1540, 1306, 3303, -1387, 4404},
    { 6788, -326, 7469, 2633, -1984, -566, -754, 1778, 745, 2317, 1975, 1539, 1309, 3307, -1389, 4417},
    { 6787, -353, 7529, 2545, -1995, -548, -750, 1790, 751, 2322, 1980, 1539, 1313, 3312, -1391, 4430},
    { 6787, -378, 7585, 2458, -2005, -529, -745, 1802, 757, 2326, 1985, 1539, 1316, 3316, -1393, 4443},
    { 6787, -396, 7634, 2375, -2018, -510, -741, 1814, 763, 2331, 1991, 1538, 1320, 3320, -1395, 4456},
    { 6785, -367, 7639, 2317, -2047, -489, -733, 1829, 770, 2337, 1999, 1537, 1323, 3323, -1397, 4467},
    { 6790, -570, 7867, 2128, -1986, -481, -741, 1832, 771, 2338, 1991, 1539, 1327, 3332, -1399, 4486},
    { 6787, -509, 7841, 2084, -2027, -457, -730, 1849, 780, 2344, 2003, 1538, 1330, 3334, -1401, 4497},
    { 6787, -536, 7900, 1992, -2033, -439, -726, 1861, 785, 2349, 2008, 1537, 1334, 3338, -1403, 4510},
    { 6787, -562, 7957, 1899, -2038, -421, -722, 1873, 791, 2354, 2013, 1537, 1337, 3343, -1405, 4524},
    { 6787, -588, 8014, 1806, -2042, -403, -717, 1886, 797, 2358, 2018, 1536, 1341, 3348, -1407, 4537},
    { 6786, -613, 8070, 1712, -2045, -384, -713, 1898, 804, 2363, 2024, 1536, 1345, 3352, -1409, 4551},
    { 6786, -637, 8125, 1618, -2048, -366, -708, 1910, 810, 2368, 2029, 1536, 1349, 3357, -1410, 4564},
    { 6786, -661, 8180, 1523, -2050, -348, -704, 1923, 816, 2372, 2034, 1535, 1352, 3361, -1412, 4578},
    { 6785, -684, 8233, 1428, -2051, -330, -699, 1935, 822, 2377, 2040, 1535, 1356, 3366, -1414, 4592},
    { 6785, -706, 8286, 1333, -2052, -313, -694, 1948, 828, 2382, 2045, 1535, 1360, 3371, -1416, 4606},
    { 6784, -727, 8338, 1238, -2052, -295, -689, 1961, 835, 2386, 2051, 1534, 1364, 3376, -1418, 4620},
    { 6784, -748, 8389, 1142, -2051, -277, -684, 1974, 841, 2391, 2057, 1534, 1368, 3380, -1420, 4633},
    { 6784, -768, 8438, 1046, -2050, -259, -679, 1986, 847, 2396, 2062, 1533, 1372, 3385, -1422, 4647},
    { 6783, -787, 8487, 949, -2048, -241, -674, 1999, 854, 2400, 2068, 1533, 1376, 3390, -1424, 4662},
    { 6782, -805, 8535, 853, -2046, -224, -669, 2012, 860, 2405, 2074, 1532, 1380, 3395, -1425, 4676},
    { 6782, -823, 8581, 756, -2043, -206, -663, 2025, 867, 2410, 2080, 1532, 1384, 3400, -1427, 4690},
    { 6781, -839, 8626, 659, -2039, -189, -658, 2038, 873, 2414, 2086, 1532, 1389, 3405, -1429, 4704},
    { 6781, -855, 8670, 562, -2035, -171, -653, 2052, 880, 2419, 2092, 1531, 1393, 3410, -1431, 4719},
    { 6780, -870, 8713, 465, -2030, -154, -647, 2065, 887, 2424, 2098, 1531, 1397, 3415, -1433, 4733},
    { 6779, -884, 8755, 368, -2024, -137, -641, 2078, 893, 2429, 2105, 1530, 1402, 3420, -1434, 4747},
    { 6778, -897, 8795, 271, -2018, -119, -635, 2091, 900, 2433, 2111, 1530, 1406, 3425, -1436, 4762},
    { 6778, -909, 8834, 174, -2011, -102, -630, 2105, 907, 2438, 2118, 1529, 1410, 3430, -1438, 4777},
    { 6777, -920, 8871, 77, -2004, -85, -624, 2118, 914, 2443, 2124, 1529, 1415, 3435, -1440, 4791},
    { 6776, -930, 8907, -20, -1996, -68, -617, 2131, 921, 2448, 2131, 1528, 1419, 3441, -1442, 4806},
    { 6775, -940, 8942, -117, -1987, -51, -611, 2145, 928, 2452, 2138, 1527, 1424, 3446, -1443, 4821},
    { 6774, -948, 8975, -214, -1978, -35, -605, 2158, 935, 2457, 2144, 1527, 1429, 3451, -1445, 4836},
    { 6773, -955, 9006, -310, -1968, -18, -599, 2172, 942, 2462, 2151, 1526, 1433, 3457, -1447, 4850},
    { 6772, -961, 9036, -406, -1958, -1, -592, 2185, 949, 2467, 2158, 1526, 1438, 3462, -1449, 4865},
    { 6771, -966, 9064, -502, -1947, 15, -586, 2199, 956, 2471, 2165, 1525, 1443, 3467, -1450, 4880},
    { 6770, -970, 9090, -598, -1936, 31, -579, 2213, 963, 2476, 2172, 1525, 1447, 3473, -1452, 4895},
    { 6769, -974, 9115, -693, -1924, 47, -572, 2226, 970, 2481, 2180, 1524, 1452, 3478, -1454, 4910},
    { 6768, -976, 9138, -788, -1911, 63, -565, 2240, 978, 2486, 2187, 1523, 1457, 3484, -1455, 4925},
    { 6767, -976, 9159, -882, -1898, 79, -558, 2253, 985, 2490, 2194, 1523, 1462, 3489, -1457, 4941},
    { 6766, -976, 9178, -975, -1885, 95, -551, 2267, 992, 2495, 2201, 1522, 1467, 3495, -1459, 4956},
    { 6764, -975, 9195, -1068, -1871, 110, -544, 2281, 1000, 2500, 2209, 1521, 1472, 3501, -1461, 4971},
    { 6763, -973, 9211, -1161, -1856, 126, -537, 2294, 1007, 2504, 2216, 1521, 1477, 3506, -1462, 4986},
    { 6762, -970, 9224, -1252, -1841, 141, -529, 2308, 1014, 2509, 2224, 1520, 1482, 3512, -1464, 5002},
    { 6761, -965, 9236, -1343, -1826, 156, -522, 2321, 1022, 2514, 2232, 1519, 1487, 3518, -1466, 5017},
    { 6760, -960, 9245, -1433, -1810, 171, -514, 2335, 1029, 2518, 2239, 1519, 1492, 3523, -1467, 5032},
    { 6758, -953, 9253, -1522, -1794, 185, -506, 2348, 1037, 2523, 2247, 1518, 1497, 3529, -1469, 5047},
    { 6757, -945, 9258, -1611, -1777, 200, -499, 2362, 1044, 2528, 2255, 1517, 1502, 3535, -1471, 5063},
    { 6756, -936, 9262, -1698, -1760, 214, -491, 2375, 1051, 2532, 2263, 1517, 1507, 3541, -1472, 5078},
    { 6754, -926, 9263, -1785, -1743, 228, -483, 2388, 1059, 2537, 2271, 1516, 1512, 3546, -1474, 5094},
    { 6753, -915, 9262, -1870, -1725, 242, -475, 2402, 1066, 2541, 2279, 1515, 1518, 3552, -1476, 5109},
    { 6751, -903, 9259, -1954, -1707, 256, -467, 2415, 1074, 2546, 2287, 1514, 1523, 3558, -1477, 5124},
    { 6750, -890, 9254, -2037, -1688, 269, -459, 2428, 1081, 2550, 2295, 1514, 1528, 3564, -1479, 5140},
    { 6749, -876, 9247, -2119, -1669, 282, -450, 2441, 1089, 2555, 2303, 1513, 1533, 3570, -1481, 5155},
    { 6747, -861, 9238, -2200, -1650, 295, -442, 2454, 1096, 2559, 2311, 1512, 1539, 3576, -1482, 5171},
    { 6746, -845, 9226, -2279, -1631, 308, -433, 2467, 1104, 2564, 2320, 1511, 1544, 3582, -1484, 5186},
    { 6744, -828, 9212, -2357, -1611, 321, -425, 2480, 1112, 2568, 2328, 1510, 1549, 3588, -1485, 5201},
    { 6743, -810, 9197, -2434, -1591, 333, -416, 2493, 1119, 2572, 2336, 1510, 1555, 3594, -1487, 5217},
    { 6741, -791, 9179, -2509, -1571, 345, -408, 2506, 1127, 2577, 2344, 1509, 1560, 3600, -1489, 5232},
    { 6740, -771, 9159, -2583, -1551, 357, -399, 2518, 1134, 2581, 2353, 1508, 1565, 3606, -1490, 5247},
    { 6738, -750, 9136, -2655, -1530, 368, -390, 2531, 1142, 2585, 2361, 1507, 1571, 3612, -1492, 5263},
    { 6737, -728, 9112, -2726, -1510, 380, -381, 2543, 1149, 2589, 2369, 1506, 1576, 3618, -1493, 5278},
    { 6735, -705, 9086, -2795, -1489, 391, -372, 2555, 1156, 2593, 2378, 1506, 1581, 3624, -1495, 5293},
    { 6734, -682, 9057, -2863, -1468, 402, -363, 2567, 1164, 2597, 2386, 1505, 1587, 3630, -1497, 5308},
    { 6732, -657, 9027, -2929, -1447, 412, -354, 2579, 1171, 2601, 2394, 1504, 1592, 3636, -1498, 5323},
    { 6731, -632, 8994, -2995, -1426, 423, -345, 2591, 1179, 2606, 2403, 1503, 1598, 3642, -1500, 5339},
    { 6729, -606, 8960, -3057, -1404, 433, -336, 2603, 1186, 2610, 2411, 1502, 1603, 3648, -1501, 5354},
    { 6728, -580, 8924, -3119, -1383, 443, -326, 2615, 1194, 2614, 2419, 1501, 1608, 3655, -1503, 5369},
    { 6726, -553, 8885, -3178, -1362, 453, -317, 2626, 1201, 2617, 2428, 1501, 1614, 3661, -1504, 5384},
    { 6725, -525, 8845, -3236, -1341, 462, -308, 2637, 1208, 2621, 2436, 1500, 1619, 3667, -1506, 5399},
    { 6723, -497, 8803, -3292, -1319, 472, -298, 2649, 1216, 2625, 2444, 1499, 1624, 3673, -1507, 5413},
    { 6721, -468, 8759, -3347, -1298, 481, -289, 2660, 1223, 2629, 2453, 1498, 1630, 3679, -1509, 5428},
    { 6720, -438, 8713, -3400, -1277, 490, -280, 2671, 1230, 2633, 2461, 1497, 1635, 3685, -1510, 5443},
    { 6718, -408, 8666, -3451, -1256, 498, -270, 2681, 1237, 2636, 2469, 1497, 1641, 3691, -1512, 5458},
    { 6717, -378, 8617, -3500, -1235, 507, -260, 2692, 1244, 2640, 2477, 1496, 1646, 3698, -1513, 5472},
    { 6715, -347, 8566, -3548, -1214, 515, -251, 2702, 1252, 2644, 2486, 1495, 1651, 3704, -1515, 5487},
    { 6714, -315, 8514, -3594, -1193, 523, -241, 2713, 1259, 2647, 2494, 1494, 1657, 3710, -1516, 5502},
    { 6712, -284, 8460, -3638, -1172, 530, -232, 2723, 1266, 2651, 2502, 1494, 1662, 3716, -1518, 5516},
    { 6711, -252, 8405, -3681, -1152, 538, -222, 2733, 1273, 2654, 2510, 1493, 1667, 3722, -1519, 5530},
    { 6709, -220, 8349, -3721, -1131, 545, -212, 2742, 1280, 2658, 2518, 1492, 1672, 3728, -1521, 5545},
    { 6708, -187, 8291, -3761, -1111, 553, -203, 2752, 1287, 2661, 2526, 1491, 1678, 3734, -1522, 5559},
    { 6706, -155, 8232, -3798, -1091, 560, -193, 2762, 1294, 2664, 2534, 1491, 1683, 3741, -1524, 5573},
    { 6705, -122, 8171, -3834, -1071, 566, -184, 2771, 1300, 2668, 2542, 1490, 1688, 3747, -1525, 5587},
    { 6703, -90, 8110, -3868, -1051, 573, -174, 2780, 1307, 2671, 2550, 1489, 1693, 3753, -1527, 5601},
    { 6702, -57, 8047, -3900, -1032, 579, -164, 2789, 1314, 2674, 2557, 1489, 1698, 3759, -1528, 5615},
    { 6701, -24, 7984, -3930, -1013, 585, -155, 2798, 1321, 2677, 2565, 1488, 1704, 3765, -1529, 5629},
    { 6699, 8, 7919, -3959, -994, 591, -145, 2806, 1327, 2680, 2572, 1488, 1709, 3771, -1531, 5642},
    { 6698, 41, 7854, -3986, -975, 597, -136, 2814, 1334, 2683, 2580, 1487, 1714, 3777, -1532, 5656},
    { 6696, 73, 7788, -4012, -957, 603, -126, 2822, 1340, 2686, 2587, 1486, 1719, 3783, -1534, 5669},
    { 6695, 105, 7722, -4035, -939, 608, -117, 2830, 1347, 2689, 2594, 1486, 1724, 3789, -1535, 5682},
    { 6694, 137, 7655, -4057, -921, 613, -108, 2838, 1353, 2692, 2601, 1485, 1728, 3794, -1536, 5695},
    { 6693, 168, 7588, -4076, -904, 618, -99, 2845, 1359, 2695, 2608, 1485, 1733, 3800, -1538, 5708},
    { 6692, 198, 7520, -4094, -888, 622, -90, 2852, 1365, 2697, 2615, 1484, 1738, 3806, -1539, 5720},
    { 6691, 228, 7454, -4109, -872, 626, -81, 2858, 1370, 2699, 2621, 1483, 1742, 3811, -1540, 5732},
    { 6690, 257, 7388, -4121, -857, 629, -73, 2864, 1375, 2702, 2627, 1483, 1746, 3816, -1542, 5743},
    { 6689, 284, 7323, -4131, -844, 632, -66, 2869, 1380, 2704, 2633, 1482, 1750, 3821, -1543, 5753},
    { 6689, 308, 7261, -4136, -832, 634, -59, 2873, 1384, 2705, 2637, 1481, 1753, 3825, -1544, 5763},
    { 6689, 330, 7203, -4135, -822, 634, -54, 2876, 1387, 2706, 2641, 1480, 1756, 3828, -1545, 5770},
    { 6689, 346, 7153, -4126, -816, 632, -51, 2877, 1388, 2706, 2643, 1479, 1757, 3830, -1546, 5774},
    { 6691, 352, 7118, -4102, -818, 626, -53, 2873, 1385, 2703, 2641, 1477, 1756, 3829, -1546, 5773},
    { 6696, 338, 7115, -4047, -834, 610, -64, 2860, 1375, 2697, 2632, 1474, 1750, 3823, -1546, 5760},
    { 6708, 262, 7212, -3901, -896, 567, -104, 2820, 1344, 2680, 2603, 1467, 1729, 3801, -1545, 5712},
    { 6763, -221, 7997, -3140, -1274, 329, -335, 2607, 1172, 2595, 2447, 1452, 1619, 3690, -1540, 5463},
    { 6533, 1477, 5278, -5524, -88, 1068, 448, 3350, 1789, 2947, 3048, 1587, 2052, 4200, -1603, 6510}
  };
  int par_fg41[24][16] = {
    { -2076, 5260, -3860, 4328, 5577, 4625, 4318, 2149, 3064, 1349, 1507, 1756, 1322, 496, 1285, 45},
    { -2080, 5226, -3753, 4045, 5585, 4681, 4341, 2192, 3088, 1366, 1528, 1756, 1336, 513, 1280, 93},
    { -2083, 5182, -3630, 3760, 5585, 4740, 4364, 2236, 3113, 1383, 1550, 1756, 1351, 529, 1274, 140},
    { -2086, 5128, -3492, 3473, 5577, 4800, 4386, 2280, 3137, 1400, 1571, 1756, 1365, 546, 1269, 187},
    { -2089, 5064, -3340, 3187, 5561, 4862, 4408, 2324, 3161, 1418, 1591, 1756, 1380, 563, 1263, 234},
    { -2092, 4991, -3176, 2903, 5535, 4925, 4430, 2368, 3186, 1436, 1611, 1757, 1393, 579, 1257, 281},
    { -2095, 4909, -3001, 2622, 5499, 4990, 4452, 2413, 3210, 1454, 1631, 1758, 1407, 596, 1251, 329},
    { -2097, 4819, -2817, 2346, 5453, 5056, 4474, 2457, 3235, 1473, 1650, 1759, 1421, 612, 1245, 376},
    { -2100, 4722, -2625, 2077, 5396, 5123, 4496, 2501, 3260, 1492, 1669, 1761, 1434, 628, 1238, 423},
    { -2102, 4619, -2428, 1816, 5328, 5190, 4519, 2545, 3284, 1512, 1687, 1763, 1447, 644, 1232, 470},
    { -2104, 4509, -2227, 1565, 5247, 5258, 4541, 2589, 3309, 1532, 1705, 1765, 1459, 661, 1225, 517},
    { -2105, 4396, -2024, 1326, 5155, 5326, 4565, 2632, 3334, 1552, 1723, 1767, 1472, 676, 1218, 564},
    { -2107, 4278, -1821, 1100, 5049, 5394, 4588, 2676, 3360, 1573, 1741, 1769, 1484, 692, 1210, 611},
    { -2108, 4158, -1621, 890, 4931, 5461, 4612, 2718, 3385, 1594, 1758, 1772, 1496, 708, 1203, 657},
    { -2109, 4037, -1424, 696, 4800, 5526, 4637, 2761, 3410, 1616, 1776, 1775, 1507, 724, 1195, 704},
    { -2110, 3914, -1234, 520, 4656, 5591, 4663, 2803, 3435, 1638, 1793, 1779, 1518, 739, 1187, 750},
    { -2110, 3793, -1051, 363, 4500, 5653, 4690, 2844, 3461, 1660, 1810, 1782, 1529, 755, 1178, 796},
    { -2111, 3672, -878, 226, 4331, 5713, 4718, 2885, 3487, 1683, 1827, 1786, 1540, 770, 1170, 842},
    { -2111, 3554, -716, 111, 4149, 5771, 4747, 2926, 3513, 1707, 1844, 1790, 1550, 786, 1161, 887},
    { -2112, 3440, -567, 18, 3956, 5825, 4778, 2966, 3539, 1731, 1861, 1795, 1560, 801, 1152, 932},
    { -2112, 3329, -431, -52, 3752, 5875, 4810, 3005, 3565, 1755, 1878, 1800, 1570, 816, 1142, 977},
    { -2112, 3224, -311, -99, 3537, 5921, 4844, 3044, 3592, 1780, 1895, 1805, 1579, 832, 1132, 1021},
    { -2113, 3124, -207, -123, 3312, 5962, 4879, 3083, 3618, 1806, 1913, 1811, 1589, 847, 1122, 1065},
    { -2113, 3030, -120, -123, 3079, 5997, 4916, 3121, 3645, 1832, 1930, 1817, 1598, 862, 1112, 1109}
  };
  int par_fg43[24][16] = {
    { 6815, -717, 8439, 393, -1876, -297, -733, 2009, 819, 2383, 2075, 1490, 1374, 3404, -1451, 4719},
    { 6817, -672, 8313, 721, -1879, -356, -754, 1962, 795, 2366, 2052, 1492, 1359, 3385, -1444, 4665},
    { 6818, -615, 8168, 1052, -1872, -417, -775, 1915, 771, 2348, 2029, 1495, 1344, 3366, -1436, 4611},
    { 6819, -547, 8006, 1384, -1857, -480, -795, 1868, 747, 2330, 2008, 1497, 1329, 3348, -1428, 4558},
    { 6820, -468, 7828, 1714, -1831, -545, -815, 1820, 724, 2312, 1986, 1498, 1315, 3329, -1420, 4504},
    { 6820, -378, 7637, 2042, -1794, -610, -834, 1773, 700, 2293, 1966, 1500, 1301, 3311, -1411, 4451},
    { 6821, -278, 7434, 2365, -1746, -677, -853, 1726, 676, 2274, 1945, 1501, 1287, 3293, -1403, 4397},
    { 6821, -170, 7221, 2682, -1686, -746, -872, 1679, 653, 2254, 1926, 1502, 1274, 3276, -1394, 4344},
    { 6820, -53, 6999, 2991, -1614, -814, -891, 1632, 629, 2235, 1907, 1503, 1261, 3258, -1385, 4291},
    { 6820, 71, 6772, 3289, -1529, -883, -910, 1586, 606, 2215, 1888, 1503, 1248, 3241, -1376, 4239},
    { 6819, 201, 6541, 3575, -1430, -953, -929, 1540, 582, 2194, 1870, 1504, 1236, 3224, -1366, 4186},
    { 6817, 336, 6309, 3848, -1317, -1022, -948, 1495, 559, 2173, 1853, 1504, 1224, 3207, -1357, 4134},
    { 6816, 475, 6077, 4104, -1191, -1090, -968, 1450, 536, 2152, 1835, 1503, 1213, 3190, -1346, 4082},
    { 6814, 617, 5848, 4343, -1050, -1157, -989, 1406, 512, 2131, 1818, 1503, 1202, 3173, -1336, 4030},
    { 6812, 760, 5625, 4563, -895, -1223, -1010, 1362, 489, 2109, 1802, 1502, 1191, 3157, -1325, 3979},
    { 6809, 904, 5409, 4761, -726, -1286, -1031, 1319, 466, 2087, 1785, 1501, 1181, 3140, -1314, 3928},
    { 6807, 1047, 5202, 4938, -543, -1348, -1054, 1277, 443, 2064, 1769, 1500, 1171, 3124, -1303, 3878},
    { 6804, 1188, 5006, 5092, -346, -1406, -1078, 1236, 419, 2041, 1753, 1498, 1161, 3108, -1291, 3828},
    { 6801, 1326, 4823, 5221, -137, -1461, -1103, 1195, 396, 2017, 1737, 1497, 1152, 3092, -1279, 3779},
    { 6798, 1459, 4655, 5325, 86, -1512, -1130, 1155, 373, 1993, 1721, 1494, 1144, 3076, -1267, 3730},
    { 6795, 1588, 4503, 5404, 320, -1558, -1158, 1116, 349, 1969, 1704, 1492, 1135, 3060, -1255, 3681},
    { 6792, 1711, 4369, 5456, 566, -1600, -1187, 1078, 325, 1944, 1688, 1489, 1127, 3044, -1242, 3633},
    { 6789, 1828, 4253, 5482, 821, -1636, -1219, 1040, 301, 1918, 1672, 1486, 1120, 3029, -1229, 3586},
    { 6786, 1936, 4156, 5482, 1086, -1665, -1252, 1002, 277, 1892, 1655, 1482, 1112, 3013, -1216, 3539}
  };


  short int par_id[16][16] = {
    {17221 , 17484 , 20563 , 17952 , 19529 , 69 , 0 , 0 , 257 , -1 , 0 , 0 , 0 , 3598 , 17 , 3000 },
    {2574 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 , 228 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 , 108 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 },
    {0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 }
  };


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


//double t=0;
//double dt=.02;
//int n=1250;
  for (int i = 0; i < 1250 ; i++) {
    m_ft[i] = par_shape[i];
    //  if(abs (par_shape[i] -ShaperDSP(t) )>1e-8 )
    //  cout<<i<<" "<<t<<" " << m_ft[i]<<" "<<ShaperDSP(t) <<endl;
    //  t=t+dt;

  }



  for (int i = 0; i < 192 ; i++) {
    for (int j = 0; j < 16 ; j++) {
      m_f[i][j] = par_f[i][j];
      m_f1[i][j] = par_f1[i][j];
      m_fg31[i][j] = par_fg31[i][j];
      m_fg32[i][j] = par_fg32[i][j];
      m_fg33[i][j] = par_fg33[i][j];
    }
  }

  for (int i = 0; i < 24 ; i++) {
    for (int j = 0; j < 16 ; j++) {
      m_fg41[i][j] = par_fg41[i][j];
      m_fg43[i][j] = par_fg43[i][j];
    }
  }


  for (int i = 0; i < 16 ; i++) {
    for (int j = 0; j < 16 ; j++) {
      m_id[i][j] = par_id[i][j];
    }
  }


  for (int i = 0; i < 31 ; i++) {
    for (int j = 0; j < 31 ; j++) {
      m_vmat[i][j] = (float) par_vmat[i][j];
    }
  }


}




