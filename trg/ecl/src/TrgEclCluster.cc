//---------------------------------------------------------------
// $Id$
//---------------------------------------------------------------
// Filename : TrgEclCluster.cc
// Section  : TRG ECL
// Owner    : InSu Lee/Yuuji Unno
// Email    : islee@hep.hanyang.ac.kr / yunno@post.kek.jp
//---------------------------------------------------------------
// Description : A class to represent TRG ECL
//---------------------------------------------------------------
// $Log$
//---------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGECLCLUSTER_SHORT_NAMES
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>


#include <trg/ecl/TrgEclCluster.h>
#include <framework/logging/Logger.h>
#include "trg/ecl/dataobjects/TRGECLCluster.h"


using namespace std;
using namespace Belle2;
//
//
//
TrgEclCluster::TrgEclCluster(): _BRICN(0), _FWDICN(0), _BWDICN(0), _BRNofCluster(0), _FWDNofCluster(0), _BWDNofCluster(0),
  _EventId(0), _Method(1), _LimitNCluster(10), _Position(1)
{
  _icnfwbrbw.clear();
  _BrCluster.clear();
  _FwCluster.clear();
  _BwCluster.clear();
  _Quadrant.clear();

  TCId.clear();
  Timing.clear();
  Energy.clear();

  ClusterTiming.clear();
  ClusterEnergy.clear();
  ClusterPositionX.clear();
  ClusterPositionY.clear();
  ClusterPositionZ.clear();
  NofTCinCluster.clear();
  MaxTCId.clear();
  TempCluster.clear();

  _icnfwbrbw.resize(3, 0);
  TempCluster.resize(9);
  _Quadrant.resize(3, std::vector<int>(4, 0.0));

  ClusterTiming.resize(3);
  ClusterEnergy.resize(3);
  ClusterPositionX.resize(3);
  ClusterPositionY.resize(3);
  ClusterPositionZ.resize(3);
  NofTCinCluster.resize(3);
  MaxTCId.resize(3);


  _TCMap = new TrgEclMapping();



}
//
//
//
TrgEclCluster::~TrgEclCluster()
{
  delete _TCMap;
}



//
int
TrgEclCluster::getICNFwBr(void)
{

  return _icnfwbrbw[0] + _icnfwbrbw[1];

}
//
//
//
int
TrgEclCluster::getICNSub(int FwBrBw)
{

  return _icnfwbrbw[FwBrBw];

}
//
//
//
int
TrgEclCluster::getBrICNCluster(int ICNId, int location)
{

  return _BrCluster[ICNId][location];

}
//
//
//
int
TrgEclCluster::getBwICNCluster(int ICNId, int location)
{

  return _BwCluster[ICNId][location];

}

//
//
//
int
TrgEclCluster::getFwICNCluster(int ICNId, int location)
{

  return _FwCluster[ICNId][location];

}
//
//
//
void
TrgEclCluster::setICN(std::vector<int> tcid)
{
  TCId = tcid ;
  // Energy = tcenergy;
  // Timing = tctiming;
  _Quadrant.clear();
  _Quadrant.resize(3, std::vector<int>(4, 0.0));
  _icnfwbrbw.clear();
  _icnfwbrbw.resize(3, 0);
  _BRICN = 0 ;
  _FWDICN = 0;
  _BWDICN = 0;

  _icnfwbrbw[1] = setBarrelICN();
  _icnfwbrbw[0] = setForwardICN();
  _icnfwbrbw[2] = setBackwardICN();


  return;
}
void
TrgEclCluster::setICN(std::vector<int> tcid, std::vector<double> tcenergy, std::vector<double> tctiming)
{
  TCId = tcid ;
  Energy = tcenergy;
  Timing = tctiming;

  setBarrelICN(_Method);
  setForwardICN(_Method);
  setBackwardICN(_Method);

  save(_EventId);

  return;
}
void
TrgEclCluster::save(int m_nEvent)
{
  int m_hitNum = 0;
  int clusterId = 0;
  // int NofCluster = ClusterEnergy[0].size()+ClusterEnergy[1].size()+ClusterEnergy[2].size();
  int counter = 0;
  for (int iposition = 0; iposition < 3 ; iposition ++) {
    const int Ncluster = ClusterEnergy[iposition].size();
    for (int icluster = 0; icluster < Ncluster; icluster++) {
      if (ClusterEnergy[iposition][icluster] <= 0) {continue;}

      if (counter >= _LimitNCluster) {
        continue;
      } else {
        counter ++;
      }

      StoreArray<TRGECLCluster> ClusterArray;
      ClusterArray.appendNew();
      m_hitNum = ClusterArray.getEntries() - 1;
      ClusterArray[m_hitNum]->setEventId(m_nEvent);
      ClusterArray[m_hitNum]->setClusterId(clusterId);
      ClusterArray[m_hitNum]->setMaxTCId(MaxTCId[iposition][icluster]);
      ClusterArray[m_hitNum]->setMaxThetaId(_TCMap->getTCThetaIdFromTCId(MaxTCId[iposition][icluster]));
      ClusterArray[m_hitNum]->setMaxPhiId(_TCMap->getTCPhiIdFromTCId(MaxTCId[iposition][icluster]));
      ClusterArray[m_hitNum]->setNofTCinCluster(NofTCinCluster[iposition][icluster]);
      ClusterArray[m_hitNum]->setEnergyDep(ClusterEnergy[iposition][icluster]);
      ClusterArray[m_hitNum]->setTimeAve(ClusterTiming[iposition][icluster]);

      ClusterArray[m_hitNum]->setPositionX(ClusterPositionX[iposition][icluster]);
      ClusterArray[m_hitNum]->setPositionY(ClusterPositionY[iposition][icluster]);
      ClusterArray[m_hitNum]->setPositionZ(ClusterPositionZ[iposition][icluster]);
      clusterId ++;
    }
  }


}
void TrgEclCluster::setBarrelICN(int Method)
{

  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;

  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(432, 0);
  TCFireEnergy.resize(432, 0.);
  TCFireTiming.resize(432, 0.);
  TCFirePosition.resize(432, std::vector<double>(3, 0.));




  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] >= 81 && TCId[ihit] <= 512) {
      TCFire[TCId[ihit] - 81] = TCId[ihit];
      TCFireEnergy[TCId[ihit] - 81] = Energy[ihit];
      TCFireTiming[TCId[ihit] - 81] = Timing[ihit];
      TCFirePosition[TCId[ihit] - 81][0] = (_TCMap->getTCPosition(TCId[ihit])).X();
      TCFirePosition[TCId[ihit] - 81][1] = (_TCMap->getTCPosition(TCId[ihit])).Y();
      TCFirePosition[TCId[ihit] - 81][2] = (_TCMap->getTCPosition(TCId[ihit])).Z();
    }
  }



  //  std::vector<std::vector<double>> TCFire;
  //  std::vector<std::vector<double>> TCFireEnergy;
  //  std::vector<std::vector<double>> TCFireTiming;
  //  std::vector<std::vector<double>> TCFirePositionX;
  //  std::vector<std::vector<double>> TCFirePositionY;
  //  std::vector<std::vector<double>> TCFirePositionZ;

  //  TCFire.clear();
  //  TCFireEnergy.clear();
  //  TCFireTiming.clear();
  //  TCFirePositionX.clear();
  //  TCFirePositionY.clear();
  //  TCFirePositionZ.clear();

  //  TCFire.resize(12, std::vector<double>(36, 0.));
  //  TCFireEnergy.resize(12, std::vector<double>(36, 0.));
  //  TCFireTiming.resize(12, std::vector<double>(36, 0.));
  //  TCFirePositionX.resize(12, std::vector<double>(36, 0.));
  //  TCFirePositionY.resize(12, std::vector<double>(36, 0.));
  //  TCFirePositionZ.resize(12, std::vector<double>(36, 0.));



  //  const int  hit_size  = TCId.size();
  //  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
  //    if (TCId[ihit] >= 81 && TCId[ihit] <= 512) {
  //      int thetaid = _TCMap->getTCThetaIdFromTCId(TCId[ihit])-4;
  //      int phiid = _TCMap->getTCPhiIdFromTCId(TCId[ihit])-1;

  //      TCFire[thetaid][phiid]= TCId[ihit];
  //      TCFireEnergy[thetaid][phiid]= Energy[ihit];
  //      TCFireTiming[thetaid][phiid]= Timing[ihit];

  //      TCFirePositionX[thetaid][phiid] = (_TCMap->getTCPosition(TCId[ihit])).X();
  //      TCFirePositionY[thetaid][phiid] = (_TCMap->getTCPosition(TCId[ihit])).Y();
  //      TCFirePositionZ[thetaid][phiid] = (_TCMap->getTCPosition(TCId[ihit])).Z();
  //    }
  //  }



  // // //
  // // //
  // // //
  int tc_upper = 0; // check upper TC
  int tc_upper_right = 0; // check right TC
  int tc_right = 0; // check right TC
  int tc_lower_right = 0;
  int tc_lower = 0; // check lower TC
  int tc_lower_left = 0; // check lower TC
  int tc_left = 0;
  int tc_upper_left = 0;

  //  for(int itheta=0; itheta<12;itheta++){
  //    for(int iphi =0; iphi<36; iphi++){
  //      if(TCFire[itheta][iphi]==0){continue;}
  //      if(iphi==0){
  //    tc_upper = 0; // check upper TC
  //    tc_upper_right = 0; // check right TC
  //    tc_right = 0; // check right TC
  //    tc_lower_right = 0;
  //    tc_lower = 0; // check lower TC
  //    tc_lower_left = 0; // check lower TC
  //    tc_left = 0;
  //    tc_upper_left = 0;
  //      }

  //      if(itheta==0){
  //    tc_upper_right = 0; // check right TC
  //    tc_right = 0; // check right TC
  //    tc_lower_right = 0;
  //       }
  //      else if(itheta==12){
  //    tc_lower_left = 0; // check lower TC
  //    tc_left = 0;
  //    tc_upper_left = 0;
  //      }
  //      else{

  //      }
  //    }
  //  }

  for (int iii = 0 ; iii < 432 ; iii++) {
    if (TCFire[iii] == 0) { continue; }

    if (iii < 12) {
      tc_upper      = TCFire[iii + 420] ;
      tc_upper_right = TCFire[iii + 419] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_lower_left = TCFire[iii + 13] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii + 421] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }
    if (iii > 11 && iii < 420) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_lower_left = TCFire[iii + 13] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii - 11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }

    }
    if (iii > 419) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii - 421] ;
      tc_lower      = TCFire[iii - 420] ;
      tc_lower_left = TCFire[iii  - 419];
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii  - 11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;
      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }


    TempCluster[0] = iii + 80 + 1; //middle of ICN
    TempCluster[1] = tc_upper; // upper
    TempCluster[2] = tc_upper_right; //right
    TempCluster[3] = tc_right; //right
    TempCluster[4] = tc_lower_right; //lower
    TempCluster[5] = tc_lower; //lower
    TempCluster[6] = tc_lower_left; //lower
    TempCluster[7] = tc_left; //lower
    TempCluster[8] = tc_upper_left; //lower right;

    //     cout << " "  <<TempCluster[0] << "  "<< TempCluster[1] << "  "<< TempCluster[2] << "  "<< TempCluster[3] << "  "<< TempCluster[4] << "  "<< TempCluster[5] << "  "<< TempCluster[6] << "  "<< TempCluster[7] << "  "<< TempCluster[8] <<endl;


    if (!(tc_upper != 0 || tc_left != 0)) {
      if (!(tc_lower != 0 && tc_lower_left != 0)) {

        if (Method == 1) { //for cluster method2(Consider TC energy in oderto find the center of cluster)

          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[TempCluster[iTC] - 81]) {
              maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 81];
              maxTCid = TempCluster[iTC];
            }
          }
          TempCluster[0] = maxTCid;

          if (maxTCid < 93) {

            TempCluster[1] = TCFire[maxTCid + 420 - 81 ] ;
            TempCluster[2] = TCFire[maxTCid + 419 - 81 ] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81 ] ;
            TempCluster[4] = TCFire[maxTCid + 11 - 81 ] ;
            TempCluster[5] = TCFire[maxTCid + 12 - 81 ] ;
            TempCluster[6] = TCFire[maxTCid + 13 - 81 ] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81 ] ;
            TempCluster[8] = TCFire[maxTCid + 421 - 81 ] ;


            if ((maxTCid - 81) % 12 == 0) {

              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }
          }
          if (maxTCid > 92 && maxTCid < 501) {

            TempCluster[1] = TCFire[maxTCid - 12 - 81] ;
            TempCluster[2] = TCFire[maxTCid - 13 - 81] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81] ;
            TempCluster[4] = TCFire[maxTCid + 11 - 81] ;
            TempCluster[5] = TCFire[maxTCid + 12 - 81] ;
            TempCluster[6] = TCFire[maxTCid + 13 - 81] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81] ;
            TempCluster[8] = TCFire[maxTCid - 11 - 81] ;

            if ((maxTCid - 81) % 12 == 0) {
              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }

          }
          if (maxTCid > 500) {
            TempCluster[1] = TCFire[maxTCid - 12 - 81] ;
            TempCluster[2] = TCFire[maxTCid - 13 - 81] ;
            TempCluster[3] = TCFire[maxTCid - 1 - 81] ;
            TempCluster[4] = TCFire[maxTCid - 421 - 81] ;
            TempCluster[5] = TCFire[maxTCid - 420 - 81] ;
            TempCluster[6] = TCFire[maxTCid - 419 - 81] ;
            TempCluster[7] = TCFire[maxTCid + 1 - 81] ;
            TempCluster[8] = TCFire[maxTCid - 11 - 81] ;



            if ((maxTCid - 81) % 12 == 0) {
              TempCluster[2] = 0 ;
              TempCluster[3] = 0 ;
              TempCluster[4] = 0 ;

            }
            if ((maxTCid - 81) % 12 == 11) {

              TempCluster[6] = 0 ;
              TempCluster[7] = 0 ;
              TempCluster[8] = 0 ;

            }

          }


        }

        for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
          for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
            if (TempCluster[iNearTC] == 0) {continue;}
            if (iNearTC == jNearTC) {continue;}
            if (TempCluster[iNearTC] == TempCluster[jNearTC]) {
              TempCluster[jNearTC] = 0;
            }
          }
        }

        //        double maxTC = 0;
        int maxTCId = 999;
        double clusterenergy = 0;
        double clustertiming = 0;
        double clusterpositionX = 0;
        double clusterpositionY = 0;
        double clusterpositionZ = 0;
        int noftcincluster = 0;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (TempCluster[iNearTC] == 0) {continue;}
          else {noftcincluster++;}
          // if (TempCluster[iNearTC] % 12 == 0) {
          //   if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}
          // }
          // if (iii % 12 == 11) {
          //   if (iNearTC == 6 || iNearTC == 7 || iNearTC == 8) {continue;}
          // }
          clusterenergy +=   TCFireEnergy[TempCluster[iNearTC] - 81];
          clustertiming +=   TCFireEnergy[TempCluster[iNearTC] - 81] * TCFireTiming[TempCluster[iNearTC] - 81];
          clusterpositionX += TCFireEnergy[TempCluster[iNearTC] - 81] * TCFirePosition[TempCluster[iNearTC] - 81][0];
          clusterpositionY += TCFireEnergy[TempCluster[iNearTC] - 81] * TCFirePosition[TempCluster[iNearTC] - 81][1];
          clusterpositionZ += TCFireEnergy[TempCluster[iNearTC] - 81] * TCFirePosition[TempCluster[iNearTC] - 81][2];


          // if (maxTC < TCFireEnergy[TempCluster[iNearTC] - 81]) {
          //   maxTC = TCFireEnergy[TempCluster[iNearTC]  - 81];
          //   maxTCId = TempCluster[iNearTC] ;
          // }

        }
        //
        maxTCId = TempCluster[0];
        //
        clustertiming /= clusterenergy;
        if (_Position == 1) {
          clusterpositionX /= clusterenergy;
          clusterpositionY /= clusterenergy;
          clusterpositionZ /= clusterenergy;
        } else if (_Position == 0) {
          clustertiming =  TCFireTiming[maxTCId - 81];
          clusterpositionX = TCFirePosition[maxTCId - 81][0];
          clusterpositionY = TCFirePosition[maxTCId - 81][1];
          clusterpositionZ = TCFirePosition[maxTCId - 81][2];
        }
        if (clustertiming == 0 && clusterenergy == 0) {continue;}
        _BrCluster.push_back(TempCluster);
        ClusterEnergy[0].push_back(clusterenergy);
        ClusterTiming[0].push_back(clustertiming);
        ClusterPositionX[0].push_back(clusterpositionX);
        ClusterPositionY[0].push_back(clusterpositionY);
        ClusterPositionZ[0].push_back(clusterpositionZ);
        NofTCinCluster[0].push_back(noftcincluster);
        MaxTCId[0].push_back(maxTCId);
      }
    }
  }// iii loop
  _BRNofCluster = MaxTCId[0].size();

}
void
TrgEclCluster::setForwardICN(int Method)
{

  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;

  std::vector<double> TempClusterEnergy;
  std::vector<double> TempClusterTiming;
  std::vector<double> TempClusterPositionX;
  std::vector<double> TempClusterPositionY;
  std::vector<double> TempClusterPositionZ;
  std::vector<int> TempNofTCinCluster;
  std::vector<int> TempMaxTCId;
  std::vector<int> sortTCId;
  int TempICNTCId = 0;;

  TempClusterEnergy.clear();
  TempClusterTiming.clear();
  TempClusterPositionX.clear();
  TempClusterPositionY.clear();
  TempClusterPositionZ.clear();
  TempNofTCinCluster.clear();
  TempMaxTCId.clear();
  sortTCId.clear();



  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(96, 0);
  TCFireEnergy.resize(96, 0.);
  TCFireTiming.resize(96, 0.);

  TCFirePosition.resize(96, std::vector<double>(3, 0.));


  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] > 80) {continue;}

    TCFireEnergy[TCId[ihit] - 1] = Energy[ihit];
    TCFireTiming[TCId[ihit] - 1] = Timing[ihit];
    TCFirePosition[TCId[ihit] - 1][0] = (_TCMap->getTCPosition(TCId[ihit])).X();
    TCFirePosition[TCId[ihit] - 1][1] = (_TCMap->getTCPosition(TCId[ihit])).Y();
    TCFirePosition[TCId[ihit] - 1][2] = (_TCMap->getTCPosition(TCId[ihit])).Z();



    //------------------------------------
    // To rearrange the hitted map
    //
    //   orignal       converted
    //  (<- Theta)    (<- Theta)
    //
    //   3  2  1       64 32  0
    //   4  5  -       65 33  1
    //   8  7  6  =>   66 34  2
    //   9 10  -       67 35  3
    //  ....           ...
    //  78 77 76       94 62 30
    //  79 80  -       95 63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = TCId[ihit] - 1;
    int kkk = 0;
    if (iTCId0 % 5 == 0) {
      kkk = (iTCId0 / 5) * 2;
      TCFire[kkk]   = TCId[ihit];
      TCFire[kkk + 1] = TCId[ihit];
    } else {
      kkk = iTCId0 / 5;
      switch (iTCId0 % 5) {
        case 1 :
          TCFire[32 + 2 * kkk]   = TCId[ihit]; break;
        case 2 :
          TCFire[64 + 2 * kkk]   = TCId[ihit]; break;
        case 3 :
          TCFire[64 + 2 * kkk + 1] = TCId[ihit]; break;
        case 4 :
          TCFire[32 + 2 * kkk + 1] = TCId[ihit]; break;
        default:
          break;
      }
    }
  }
  // for(int iTCId = 0; iTCId <80;iTCId++){
  //   if(TCFireEnergy[iTCId]0=0||TCFireTiming[iTCId]==0){continue;}
  //   int PhiId =  _TCMap->getTCPhiIdFromTCId(iTCId+1);
  //   int ThetaID = _TCMap->getTCThetaIdFromTCId(iTCId+1);

  //   if(ThetaId ==1){ // mostinner part
  //     TempCluster[0] = iTCId+1; //center



  //     if(PhiId==10){
  //  TempCluster[1] = TCFire[31]; // top
  //  TempCluster[2] = 0;// right top

  //     }
  //     TempCluster[3] = 0;
  //     TempCluster[4] = 0;
  //     TempCluster[5] = TCFire[iii + 2]; //bottom



  //   }




  // }


  for (int iii = 0 ; iii < 96 ; iii++) {
    int icn_flag = 0;
    if (iii < 32) {
      if (iii % 2 == 1) {
        continue;
      }
    }
    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (TCFire[iii] == 0) { continue; }
    if (iii < 32) { // most inner
      if (iii == 0) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[30]; // top
        TempCluster[2] = TCFire[63]; // left up
        TempCluster[3] = TCFire[iii + 32] ; // left 1
        TempCluster[4] = TCFire[iii + 33]; // left 2
        TempCluster[5] = TCFire[iii + 34]; // left bottom
        TempCluster[6] = TCFire[iii + 2]; // bottom
        TempCluster[7] = 0;
        TempCluster[8] = 0;

      } else if (iii == 30) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 2]; // top
        TempCluster[2] = TCFire[iii + 31]; // left up
        TempCluster[3] = TCFire[iii + 32] ; // left 1
        TempCluster[4] = TCFire[iii + 33]; // left 2
        TempCluster[5] = TCFire[32]; // left bottom
        TempCluster[6] = TCFire[0]; // bottom
        TempCluster[7] = 0;
        TempCluster[8] = 0;

      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 2]; // top
        TempCluster[2] = TCFire[iii + 31]; // left up
        TempCluster[3] = TCFire[iii + 32] ; // left 1
        TempCluster[4] = TCFire[iii + 33]; // left 2
        TempCluster[5] = TCFire[iii + 34]; // left bottom
        TempCluster[6] = TCFire[iii + 2]; // bottom
        TempCluster[7] = 0;
        TempCluster[8] = 0;

      }
      if (!(TempCluster[1] != 0 || (TempCluster[3] != 0 || TempCluster[4] != 0))) {
        if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
          icn_flag = 1;


        }
      }

    } else if (iii > 31 && iii < 64) {
      if (iii == 32) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[63]; // up
        TempCluster[2] = TCFire[iii - 2]; // up right
        TempCluster[3] = TCFire[iii - 32]; // right
        TempCluster[4] = 0; //right bottom
        TempCluster[5] = TCFire[iii + 1]; // bottom
        TempCluster[6] = TCFire[iii + 33]; // left bottom
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[95]; // up left

      } else if (iii == 33) {

        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // up
        TempCluster[2] = 0; // up right
        TempCluster[3] = TCFire[iii - 32]; // right
        TempCluster[4] = TCFire[iii - 30]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; // bottom
        TempCluster[6] = TCFire[iii + 33]; // left bottom
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; // up left

      }

      else if (iii == 62) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // up
        TempCluster[2] = TCFire[iii - 34]; // up right
        TempCluster[3] = TCFire[iii - 32]; // right
        TempCluster[4] = 0; //right bottom
        TempCluster[5] = TCFire[iii + 1]; // bottom
        TempCluster[6] = TCFire[iii + 33]; // left bottom
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; // up left


      } else if (iii == 63) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // up
        TempCluster[2] = 0; // up right
        TempCluster[3] = TCFire[iii - 32]; // right
        TempCluster[4] = TCFire[1]; //right bottom
        TempCluster[5] = TCFire[32]; // bottom
        TempCluster[6] = TCFire[64]; // left bottom
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; // up left

      }

      else {

        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // up
        TempCluster[2] = TCFire[iii - 34]; // up right
        TempCluster[3] = TCFire[iii - 32]; // right
        TempCluster[4] = TCFire[iii - 30]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; // bottom
        TempCluster[6] = TCFire[iii + 33]; // left bottom
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; // up left
        if (iii % 2 == 0) {
          TempCluster[4] = 0;
        } else {
          TempCluster[2] = 0;
        }
      }
      if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
        if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
          icn_flag = 1;


        }
      }



    } else {
      if (iii == 64) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[95]; // top
        TempCluster[2] = TCFire[63];// right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      } else if (iii == 95) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[32]; //right bottom
        TempCluster[5] = TCFire[64]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left

      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      }
      if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
        if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
          icn_flag = 1;

        }
      }


    }
    //    cout <<" "<< TempCluster[0] << "  "<< TempCluster[1] << "  "<< TempCluster[2] << "  "<< TempCluster[3] << "  "<< TempCluster[4] << "  "<< TempCluster[5] << "  "<< TempCluster[6] << "  "<< TempCluster[7] << "  "<< TempCluster[8] <<endl;

    if (icn_flag == 1) {
      TempICNTCId = TCFire[iii];

      if (Method == 1) { //for cluster method2
        int maxTCid = 0;
        double maxTCEnergy = 0;
        for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
          if (TempCluster[iTC] == 0) {continue;}
          if (maxTCEnergy <  TCFireEnergy[TempCluster[iTC] - 1]) {
            maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 1];
            maxTCid = TempCluster[iTC];
          }
        }
        TempCluster.resize(9, 0);
        TempCluster[0] = maxTCid;
        int kkk = (TempCluster[0] - 1) / 5;
        if ((TempCluster[0] - 1) % 5 == 0) {
          kkk = kkk * 2;
        } else if ((TempCluster[0] - 1) % 5 == 1) {
          kkk = 32 +  2 * kkk;
        } else if ((TempCluster[0] - 1) % 5 == 2) {
          kkk = 64 +  2 * kkk;
        } else if ((TempCluster[0] - 1) % 5 == 3) {
          kkk = 64 + 1 +  2 * kkk;
        } else if ((TempCluster[0] - 1) % 5 == 4) {
          kkk = 32 + 1 +  2 * kkk;
        }

        if (kkk < 32) { // most inner
          if (kkk == 0) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[30]; // top
            TempCluster[2] = TCFire[63]; // left up
            TempCluster[3] = TCFire[kkk + 32] ; // left 1
            TempCluster[4] = TCFire[kkk + 33]; // left 2
            TempCluster[5] = TCFire[kkk + 34]; // left bottom
            TempCluster[6] = TCFire[kkk + 2]; // bottom
            TempCluster[7] = 0;
            TempCluster[8] = 0;

          } else if (kkk == 30) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 2]; // top
            TempCluster[2] = TCFire[kkk + 31]; // left up
            TempCluster[3] = TCFire[kkk + 32] ; // left 1
            TempCluster[4] = TCFire[kkk + 33]; // left 2
            TempCluster[5] = TCFire[32]; // left bottom
            TempCluster[6] = TCFire[0]; // bottom
            TempCluster[7] = 0;
            TempCluster[8] = 0;

          } else {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 2]; // top
            TempCluster[2] = TCFire[kkk + 31]; // left up
            TempCluster[3] = TCFire[kkk + 32] ; // left 1
            TempCluster[4] = TCFire[kkk + 33]; // left 2
            TempCluster[5] = TCFire[kkk + 34]; // left bottom
            TempCluster[6] = TCFire[kkk + 2]; // bottom
            TempCluster[7] = 0;
            TempCluster[8] = 0;

          }

        } else if (kkk > 31 && kkk < 64) {
          if (kkk == 32) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[63]; // up
            TempCluster[2] = TCFire[kkk - 2]; // up right
            TempCluster[3] = TCFire[kkk - 32]; // right
            TempCluster[4] = 0; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; // bottom
            TempCluster[6] = TCFire[kkk + 33]; // left bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[95]; // up left

          } else if (kkk == 33) {

            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // up
            TempCluster[2] = 0; // up right
            TempCluster[3] = TCFire[kkk - 32]; // right
            TempCluster[4] = TCFire[kkk - 30]; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; // bottom
            TempCluster[6] = TCFire[kkk + 33]; // left bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; // up left

          }

          else if (kkk == 62) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // up
            TempCluster[2] = TCFire[kkk - 34]; // up right
            TempCluster[3] = TCFire[kkk - 32]; // right
            TempCluster[4] = 0; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; // bottom
            TempCluster[6] = TCFire[kkk + 33]; // left bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; // up left


          } else if (kkk == 63) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // up
            TempCluster[2] = 0; // up right
            TempCluster[3] = TCFire[kkk - 32]; // right
            TempCluster[4] = TCFire[1]; //right bottom
            TempCluster[5] = TCFire[32]; // bottom
            TempCluster[6] = TCFire[64]; // left bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; // up left

          }

          else {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // up
            TempCluster[2] = TCFire[kkk - 34]; // up right
            TempCluster[3] = TCFire[kkk - 32]; // right
            TempCluster[4] = TCFire[kkk - 30]; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; // bottom
            TempCluster[6] = TCFire[kkk + 33]; // left bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; // up left
            if (kkk % 2 == 0) {
              TempCluster[4] = 0;
            } else {
              TempCluster[2] = 0;
            }

          }



        } else {
          if (kkk == 64) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[95]; // top
            TempCluster[2] = TCFire[63];// right top
            TempCluster[3] = TCFire[kkk - 32]; //right
            TempCluster[4] = TCFire[kkk - 31]; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; //bottom
            TempCluster[6] = 0; //bottom left
            TempCluster[7] = 0; // left
            TempCluster[8] = 0; //top left
          } else if (kkk == 95) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // top
            TempCluster[2] = TCFire[kkk - 33]; // right top
            TempCluster[3] = TCFire[kkk - 32]; //right
            TempCluster[4] = TCFire[32]; //right bottom
            TempCluster[5] = TCFire[64]; //bottom
            TempCluster[6] = 0; //bottom left
            TempCluster[7] = 0; // left
            TempCluster[8] = 0; //top left

          } else {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // top
            TempCluster[2] = TCFire[kkk - 33]; // right top
            TempCluster[3] = TCFire[kkk - 32]; //right
            TempCluster[4] = TCFire[kkk - 31]; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; //bottom
            TempCluster[6] = 0; //bottom left
            TempCluster[7] = 0; // left
            TempCluster[8] = 0; //top left
          }

        }
      }
      for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
        for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
          if (TempCluster[iNearTC] == 0) {continue;}
          if (iNearTC == jNearTC)continue;
          if (TempCluster[iNearTC] == TempCluster[jNearTC]) {
            TempCluster[jNearTC] = 0;
          }
        }
      }

      int maxTCId = 999;
      double clusterenergy = 0;
      double clustertiming = 0;
      double clusterpositionX = 0;
      double clusterpositionY = 0;
      double clusterpositionZ = 0;
      int noftcincluster = 0;
      for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
        if (TempCluster[iNearTC] == 0) {continue;}
        else {noftcincluster++;}
        clusterenergy +=   TCFireEnergy[TempCluster[iNearTC] - 1];
        clustertiming +=   TCFireEnergy[TempCluster[iNearTC] - 1] * TCFireTiming[TempCluster[iNearTC] - 1];
        clusterpositionX += TCFireEnergy[TempCluster[iNearTC] - 1] * TCFirePosition[TempCluster[iNearTC] - 1][0];
        clusterpositionY += TCFireEnergy[TempCluster[iNearTC] - 1] * TCFirePosition[TempCluster[iNearTC] - 1][1];
        clusterpositionZ += TCFireEnergy[TempCluster[iNearTC] - 1] * TCFirePosition[TempCluster[iNearTC] - 1][2];


        // if (maxTC < TCFireEnergy[TempCluster[iNearTC] - 1]) {
        //   maxTC = TCFireEnergy[TempCluster[iNearTC]  - 1];
        //   maxTCId = TempCluster[iNearTC] ;
        // }
      }
      //
      maxTCId = TempCluster[0];
      //

      clustertiming /= clusterenergy;
      if (_Position == 1) {
        clusterpositionX /= clusterenergy;
        clusterpositionY /= clusterenergy;
        clusterpositionZ /= clusterenergy;
      } else if (_Position == 0) {
        clustertiming =  TCFireTiming[maxTCId - 1];
        clusterpositionX = TCFirePosition[maxTCId - 1][0];
        clusterpositionY = TCFirePosition[maxTCId - 1][1];
        clusterpositionZ = TCFirePosition[maxTCId - 1][2];
      }
      if (clustertiming == 0 && clusterenergy == 0) {continue;}
      //        _FwCluster.push_back(TempCluster);

      TempClusterEnergy.push_back(clusterenergy);
      TempClusterTiming.push_back(clustertiming);
      TempClusterPositionX.push_back(clusterpositionX);
      TempClusterPositionY.push_back(clusterpositionY);
      TempClusterPositionZ.push_back(clusterpositionZ);
      TempNofTCinCluster.push_back(noftcincluster);
      TempMaxTCId.push_back(maxTCId);
      sortTCId.push_back(TempICNTCId);
    }

  }


  sort(sortTCId.begin(), sortTCId.end());
  const int clustersize = sortTCId.size();
  for (int itc = 0; itc < clustersize ; itc++) {
    //    cout << sortTCId[itc] << endl;;

    for (int jtc = 0; jtc < clustersize; jtc++) {
      //      if(sortTCId[itc] != TempMaxTCId[jtc]){continue;}
      ClusterEnergy[1].push_back(TempClusterEnergy[jtc]);
      ClusterTiming[1].push_back(TempClusterTiming[jtc]);
      ClusterPositionX[1].push_back(TempClusterPositionX[jtc]);
      ClusterPositionY[1].push_back(TempClusterPositionY[jtc]);
      ClusterPositionZ[1].push_back(TempClusterPositionZ[jtc]);
      NofTCinCluster[1].push_back(TempNofTCinCluster[jtc]);
      MaxTCId[1].push_back(TempMaxTCId[jtc]);


    }


  }




  _FWDNofCluster = MaxTCId[1].size();

}
//
//
//
void TrgEclCluster::setBackwardICN(int Method)
{
  std::vector<int> TCFire;
  std::vector<double> TCFireEnergy;
  std::vector<double> TCFireTiming;
  std::vector<std::vector<double>> TCFirePosition;


  std::vector<double> TempClusterEnergy;
  std::vector<double> TempClusterTiming;
  std::vector<double> TempClusterPositionX;
  std::vector<double> TempClusterPositionY;
  std::vector<double> TempClusterPositionZ;
  std::vector<int> TempNofTCinCluster;
  std::vector<int> TempMaxTCId;
  std::vector<int> sortTCId;
  int TempICNTCId = 0;

  TempClusterEnergy.clear();
  TempClusterTiming.clear();
  TempClusterPositionX.clear();
  TempClusterPositionY.clear();
  TempClusterPositionZ.clear();
  TempNofTCinCluster.clear();
  TempMaxTCId.clear();
  sortTCId.clear();



  TCFire.clear();
  TCFireEnergy.clear();
  TCFireTiming.clear();
  TCFirePosition.clear();

  TCFire.resize(64, 0);
  TCFireEnergy.resize(64, 0.);
  TCFireTiming.resize(64, 0.);
  TCFirePosition.resize(64, std::vector<double>(3, 0.));



  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] < 513) {continue;}
    TCFireEnergy[TCId[ihit] - 513] = Energy[ihit];
    TCFireTiming[TCId[ihit] - 513] = Timing[ihit];
    TCFirePosition[TCId[ihit] - 513][0] = (_TCMap->getTCPosition(TCId[ihit])).X();
    TCFirePosition[TCId[ihit] - 513][1] = (_TCMap->getTCPosition(TCId[ihit])).Y();
    TCFirePosition[TCId[ihit] - 513][2] = (_TCMap->getTCPosition(TCId[ihit])).Z();

    //    TCFire[TCId[ihit] - 513] = TCId[ihit];
    //------------------------------------
    // To rearrange the hitted map
    //
    //   orignal       converted
    //  (<- Theta)    (<- Theta)
    //
    //     516  515       32  0
    //     513  514       33  1
    //     520  519  =>   34  2
    //     517  518       35  3
    //     ...            .
    //     576  575       62 30
    //     573  574       63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = TCId[ihit] - 1;
    int kkk = 0;
    if ((iTCId0 - 512) % 4 == 2) {
      kkk = (iTCId0 - 512) / 2 - 1;
    }
    if ((iTCId0 - 512) % 4 == 1) {
      kkk = ((iTCId0 - 512) + 1) / 2;
    }
    if ((iTCId0 - 512) % 4 == 3) {
      kkk =  32 + ((iTCId0 - 512) - 3) / 2;
    }
    if ((iTCId0 - 512) % 4 == 0) {
      kkk =  33 + ((iTCId0 - 512)) / 2;
    }

    TCFire[kkk] = iTCId0 + 1;

    //    cout <<kkk << " "  << TCFire[kkk]<< endl;
  }


  for (int iii = 0 ; iii < 64 ; iii ++) {

    if (TCFire[iii] == 0) { continue; }

    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (iii < 32) {
      TempCluster[0] = TCFire[iii];
      TempCluster[2] = 0;// right top
      TempCluster[3] = 0; //right
      TempCluster[4] = 0; //right bottom
      TempCluster[5] = TCFire[iii + 1]; //bottom
      TempCluster[7] = TCFire[iii + 32]; // left
      TempCluster[8] = TCFire[iii + 31]; //top left


      if (iii == 0) {
        TempCluster[1] = TCFire[31]; // top
        TempCluster[8] = TCFire[63]; //top left
      } else {
        TempCluster[1] = TCFire[iii - 1]; // top
      }
      if (iii == 31) {
        TempCluster[5] = TCFire[0]; //bottom
        TempCluster[6] = TCFire[32]; //bottom left
      } else {
        TempCluster[6] = TCFire[iii + 33]; //bottom left
      }

    }


    if (iii > 31) {
      TempCluster[0] = TCFire[iii];
      TempCluster[1] = TCFire[iii - 1]; // top
      TempCluster[3] = TCFire[iii - 32]; //right
      TempCluster[4] = TCFire[iii - 31]; //right bottom
      TempCluster[6] = 0; //bottom left
      TempCluster[7] = 0; // left
      TempCluster[8] = 0; //top left
      if (iii == 32) {
        TempCluster[1] = TCFire[63]; // top
        TempCluster[2] = TCFire[31];// right top

      } else {
        TempCluster[2] = TCFire[iii - 33]; // right top
      }
      if (iii == 63) {
        TempCluster[5] = TCFire[32]; //bottom
        TempCluster[4] = TCFire[0]; //right bottom

      } else {
        TempCluster[5] = TCFire[iii + 1]; //bottom
      }
    }



    if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
      if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
        TempICNTCId = TCFire[iii];

        if (Method == 1) {
          int maxTCid = 0;
          double maxTCEnergy = 0;
          for (int iTC = 0; iTC < 9; iTC++) { //find center of Cluster
            if (TempCluster[iTC] == 0) {continue;}
            if (maxTCEnergy <       TCFireEnergy[TempCluster[iTC] - 513]) {
              maxTCEnergy = TCFireEnergy[TempCluster[iTC] - 513];
              maxTCid = TempCluster[iTC];
            }
          }
          int kkk = 0;
          TempCluster[0] = maxTCid;
          if ((TempCluster[0] - 513) % 4 == 2) {
            kkk = (TempCluster[0] - 513) / 2 - 1;
          }
          if ((TempCluster[0] - 513) % 4 == 1) {
            kkk = ((TempCluster[0] - 513) + 1) / 2;
          }
          if ((TempCluster[0] - 513) % 4 == 3) {
            kkk =  32 + ((TempCluster[0] - 513) - 3) / 2;
          }
          if ((TempCluster[0] - 513) % 4 == 0) {
            kkk =  33 + ((TempCluster[0] - 513)) / 2;
          }

          if (kkk < 32) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[2] = 0;// right top
            TempCluster[3] = 0; //right
            TempCluster[4] = 0; //right bottom
            TempCluster[5] = TCFire[kkk + 1]; //bottom
            TempCluster[7] = TCFire[kkk + 32]; // left
            TempCluster[8] = TCFire[kkk + 31]; //top left


            if (kkk == 0) {
              TempCluster[1] = TCFire[31]; // top
              TempCluster[8] = TCFire[63]; //top left
            } else {
              TempCluster[1] = TCFire[kkk - 1]; // top
            }
            if (kkk == 31) {
              TempCluster[5] = TCFire[0]; //bottom
              TempCluster[6] = TCFire[32]; //bottom left
            } else {
              TempCluster[6] = TCFire[kkk + 33]; //bottom left
            }

          } else if (kkk > 31) {
            TempCluster[0] = TCFire[kkk];
            TempCluster[1] = TCFire[kkk - 1]; // top
            TempCluster[3] = TCFire[kkk - 32]; //right
            TempCluster[4] = TCFire[kkk - 31]; //right bottom
            TempCluster[6] = 0; //bottom left
            TempCluster[7] = 0; // left
            TempCluster[8] = 0; //top left
            if (kkk == 32) {
              TempCluster[1] = TCFire[63]; // top
              TempCluster[2] = TCFire[31];// right top

            } else {
              TempCluster[2] = TCFire[kkk - 33]; // right top
            }
            if (kkk == 63) {
              TempCluster[5] = TCFire[32]; //bottom
              TempCluster[4] = TCFire[0]; //right bottom

            } else {
              TempCluster[5] = TCFire[kkk + 1]; //bottom
            }
          }

        }
        for (int iNearTC = 1; iNearTC < 9; iNearTC ++) {
          for (int jNearTC = 1; jNearTC < 9; jNearTC ++) {
            if (TempCluster[iNearTC] == 0) {continue;}
            if (iNearTC == jNearTC) {continue;}
            if (TempCluster[iNearTC] == TempCluster[jNearTC]) {
              TempCluster[jNearTC] = 0;
            }
          }
        }

        int maxTCId = 999;
        double clusterenergy = 0;
        double clustertiming = 0;
        double clusterpositionX = 0;
        double clusterpositionY = 0;
        double clusterpositionZ = 0;
        int noftcincluster = 0;
        for (int iNearTC = 0; iNearTC < 9; iNearTC ++) {
          if (TempCluster[iNearTC] == 0) {continue;}
          else {noftcincluster++;}
          //     if (iNearTC == 2 || iNearTC == 3 || iNearTC == 4) {continue;}

          clusterenergy +=   TCFireEnergy[TempCluster[iNearTC] - 513];
          clustertiming +=   TCFireEnergy[TempCluster[iNearTC] - 513] * TCFireTiming[TempCluster[iNearTC] - 513];
          clusterpositionX += TCFireEnergy[TempCluster[iNearTC] - 513] * TCFirePosition[TempCluster[iNearTC] - 513][0];
          clusterpositionY += TCFireEnergy[TempCluster[iNearTC] - 513] * TCFirePosition[TempCluster[iNearTC] - 513][1];
          clusterpositionZ += TCFireEnergy[TempCluster[iNearTC] - 513] * TCFirePosition[TempCluster[iNearTC] - 513][2];

          // if (maxTC < TCFireEnergy[TempCluster[iNearTC] - 513]) {
          //   maxTC = TCFireEnergy[TempCluster[iNearTC]  - 513];
          //   maxTCId = TempCluster[iNearTC] ;
          // }



        }
        //
        maxTCId = TempCluster[0];
        //

        clustertiming /= clusterenergy;
        if (_Position == 1) {
          clusterpositionX /= clusterenergy;
          clusterpositionY /= clusterenergy;
          clusterpositionZ /= clusterenergy;
        } else if (_Position == 0) {
          clustertiming =  TCFireTiming[maxTCId - 513];
          clusterpositionX = TCFirePosition[maxTCId - 513][0];
          clusterpositionY = TCFirePosition[maxTCId - 513][1];
          clusterpositionZ = TCFirePosition[maxTCId - 513][2];
        }

        if (clustertiming == 0 && clusterenergy == 0) {continue;}
        //        _FwCluster.push_back(TempCluster);
        TempClusterEnergy.push_back(clusterenergy);
        TempClusterTiming.push_back(clustertiming);
        TempClusterPositionX.push_back(clusterpositionX);
        TempClusterPositionY.push_back(clusterpositionY);
        TempClusterPositionZ.push_back(clusterpositionZ);
        TempNofTCinCluster.push_back(noftcincluster);
        TempMaxTCId.push_back(maxTCId);
        sortTCId.push_back(TempICNTCId);
      }

    }
  }
  //  std::vector<int> sortTCId = TempICNTCId;
  sort(sortTCId.begin(), sortTCId.end());
  const int clustersize = sortTCId.size();
  for (int itc = 0; itc < clustersize ; itc++) {
    //    cout << sortTCId[itc] << endl;;
    for (int jtc = 0; jtc < clustersize; jtc++) {
      //      if(sortTCId[itc] != TempMaxTCId[jtc]){continue;}
      ClusterEnergy[2].push_back(TempClusterEnergy[jtc]);
      ClusterTiming[2].push_back(TempClusterTiming[jtc]);
      ClusterPositionX[2].push_back(TempClusterPositionX[jtc]);
      ClusterPositionY[2].push_back(TempClusterPositionY[jtc]);
      ClusterPositionZ[2].push_back(TempClusterPositionZ[jtc]);
      NofTCinCluster[2].push_back(TempNofTCinCluster[jtc]);
      MaxTCId[2].push_back(TempMaxTCId[jtc]);


    }


  }




  _BWDNofCluster = MaxTCId[2].size();



}

//
//
//
int
TrgEclCluster::setBarrelICN()
{

  std::vector<int> TCFire;

  TCFire.clear();

  TCFire.resize(432, 0);

  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] >= 81 && TCId[ihit] <= 512) {
      TCFire[TCId[ihit] - 81] = TCId[ihit];
    }
  }

  //
  //
  //
  int tc_upper = 0; // check upper TC
  int tc_upper_right = 0; // check right TC
  int tc_right = 0; // check right TC
  int tc_lower_right = 0;
  int tc_lower = 0; // check lower TC
  int tc_lower_left = 0; // check lower TC
  int tc_left = 0;
  int tc_upper_left = 0;



  for (int iii = 0 ; iii < 432 ; iii++) {
    if (TCFire[iii] == 0) { continue; }

    if (iii < 12) {
      tc_upper      = TCFire[iii + 420] ;
      tc_upper_right = TCFire[iii + 419] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_lower_left = TCFire[iii + 13] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii + 421] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }
    if (iii > 11 && iii < 420) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii + 11] ;
      tc_lower      = TCFire[iii + 12] ;
      tc_lower_left = TCFire[iii + 13] ;
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii - 11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;

      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }

    }
    if (iii > 419) {
      tc_upper      = TCFire[iii - 12] ;
      tc_upper_right = TCFire[iii - 13] ;
      tc_right      = TCFire[iii - 1] ;
      tc_lower_right = TCFire[iii - 421] ;
      tc_lower      = TCFire[iii - 420] ;
      tc_lower_left = TCFire[iii  - 419];
      tc_left = TCFire[iii + 1] ;
      tc_upper_left = TCFire[iii  - 11] ;
      if (iii % 12 == 0) {
        tc_upper_right = 0;
        tc_right      = 0;
        tc_lower_right = 0;
      }
      if (iii % 12 == 11) {
        tc_lower_left = 0;
        tc_left = 0;
        tc_upper_left = 0;

      }
    }


    TempCluster[0] = iii + 80 + 1; //middle of ICN
    TempCluster[1] = tc_upper; // upper
    TempCluster[2] = tc_upper_right; //right
    TempCluster[3] = tc_right; //right
    TempCluster[4] = tc_lower_right; //lower
    TempCluster[5] = tc_lower; //lower
    TempCluster[6] = tc_lower_left; //lower
    TempCluster[7] = tc_left; //lower
    TempCluster[8] = tc_upper_left; //lower right;



    if (!(tc_upper != 0 || tc_left != 0)) {
      if (!(tc_lower != 0 && tc_lower_left != 0)) {
        _BRICN++;
        int phiid = _TCMap -> getTCPhiIdFromTCId(iii + 80 + 1);

        if (phiid == 36 || (phiid > 0 && phiid < 11)) {
          _Quadrant[1][0]++;
        }
        if (phiid > 8 && phiid < 20) {
          _Quadrant[1][1]++;
        }
        if (phiid > 17 && phiid < 29) {
          _Quadrant[1][2]++;
        }
        if ((phiid > 26 && phiid < 37) || phiid == 1) {
          _Quadrant[1][3]++;
        }

      }
    }// iii loop
  }
  return _BRICN;

}
//
//
//
int
TrgEclCluster::setForwardICN()
{

  std::vector<int> TCFire;


  TCFire.clear();

  TCFire.resize(96, 0);


  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] > 80) {continue;}


    //------------------------------------
    // To rearrange the hitted map
    //
    //   orignal       converted
    //  (<- Theta)    (<- Theta)
    //
    //   3  2  1       64 32  0
    //   4  5  -       65 33  1
    //   8  7  6  =>   66 34  2
    //   9 10  -       67 35  3
    //  ....           ...
    //  78 77 76       94 62 30
    //  79 80  -       95 63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = TCId[ihit] - 1;
    int kkk = 0;
    if (iTCId0 % 5 == 0) {
      kkk = (iTCId0 / 5) * 2;
      TCFire[kkk]   = TCId[ihit];
      TCFire[kkk + 1] = TCId[ihit];
    } else {
      kkk = iTCId0 / 5;
      switch (iTCId0 % 5) {
        case 1 :
          TCFire[32 + 2 * kkk]   = TCId[ihit]; break;
        case 2 :
          TCFire[64 + 2 * kkk]   = TCId[ihit]; break;
        case 3 :
          TCFire[64 + 2 * kkk + 1] = TCId[ihit]; break;
        case 4 :
          TCFire[32 + 2 * kkk + 1] = TCId[ihit]; break;
        default:
          break;
      }
    }
  }


  for (int iii = 32 ; iii < 96 ; iii++) {
    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (TCFire[iii] == 0) { continue; }
    if (iii < 64) {
      if (iii == 32) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[63]; // top
        TempCluster[2] = 0;// right top
        TempCluster[3] = 0; //right
        TempCluster[4] = 0; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = TCFire[iii + 33]; //bottom left
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[95]; //top left
      } else if (iii == 63) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = 0;                         // right top
        TempCluster[3] = 0;                         //right
        TempCluster[4] = 0; //right bottom
        TempCluster[5] =  TCFire[32]; //bottom
        TempCluster[6] = TCFire[64]; //bottom left
        TempCluster[7] = TCFire[iii +        32]; // left
        TempCluster[8] = TCFire[iii +    31]; //top left
      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = 0; // right top
        TempCluster[3] = 0; //right
        TempCluster[4] = 0; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = TCFire[iii + 33]; //bottom left
        TempCluster[7] = TCFire[iii + 32]; // left
        TempCluster[8] = TCFire[iii + 31]; //top left
      }
    } else {
      if (iii == 64) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[95]; // top
        TempCluster[2] = TCFire[63];// right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      } else if (iii == 95) {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[32]; //right bottom
        TempCluster[5] = TCFire[64]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left

      } else {
        TempCluster[0] = TCFire[iii];
        TempCluster[1] = TCFire[iii - 1]; // top
        TempCluster[2] = TCFire[iii - 33]; // right top
        TempCluster[3] = TCFire[iii - 32]; //right
        TempCluster[4] = TCFire[iii - 31]; //right bottom
        TempCluster[5] = TCFire[iii + 1]; //bottom
        TempCluster[6] = 0; //bottom left
        TempCluster[7] = 0; // left
        TempCluster[8] = 0; //top left
      }

    }
    if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
      if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
        _FWDICN++;
        int phiid = _TCMap -> getTCPhiIdFromTCId(TCFire[iii]);

        if (phiid == 32 || (phiid > 0 && phiid < 10)) {
          _Quadrant[0][0]++;
        }
        if (phiid > 7 && phiid < 18) {
          _Quadrant[0][1]++;
        }
        if (phiid > 15 && phiid < 26) {
          _Quadrant[0][2]++;
        }
        if ((phiid > 22 && phiid < 33) || phiid == 1) {
          _Quadrant[0][3]++;
        }


      }
    }
  }
  return _FWDICN;
}
//
//
//
int TrgEclCluster::setBackwardICN()
{
  std::vector<int> TCFire;


  TCFire.clear();

  TCFire.resize(64, 0);



  const int  hit_size  = TCId.size();
  for (int ihit = 0 ; ihit < hit_size ; ihit++) {
    if (TCId[ihit] < 513) {continue;}

    //    TCFire[TCId[ihit] - 513] = TCId[ihit];
    //------------------------------------
    // To rearrange the hitted map
    //
    //   orignal       converted
    //  (<- Theta)    (<- Theta)
    //
    //     516  515       32  0
    //     513  514       33  1
    //     520  519  =>   34  2
    //     517  518       35  3
    //     ...            .
    //     576  575       62 30
    //     573  574       63 31
    //
    // Here, TCId-1 becomes TCId=0 and 1.
    //------------------------------------
    int iTCId0 = TCId[ihit] - 1;
    int kkk = 0;
    if ((iTCId0 - 512) % 4 == 2) {
      kkk = (iTCId0 - 512) / 2 - 1;
    }
    if ((iTCId0 - 512) % 4 == 1) {
      kkk = ((iTCId0 - 512) + 1) / 2;
    }
    if ((iTCId0 - 512) % 4 == 3) {
      kkk =  32 + ((iTCId0 - 512) - 3) / 2;
    }
    if ((iTCId0 - 512) % 4 == 0) {
      kkk =  33 + ((iTCId0 - 512)) / 2;
    }

    TCFire[kkk] = iTCId0 + 1;

  }


  for (int iii = 0 ; iii < 64 ; iii ++) {

    if (TCFire[iii] == 0) { continue; }

    for (int iinit = 0; iinit < 9; iinit ++) {TempCluster[iinit] = 0;}
    if (iii < 32) {
      TempCluster[0] = TCFire[iii];
      TempCluster[2] = 0;// right top
      TempCluster[3] = 0; //right
      TempCluster[4] = 0; //right bottom
      TempCluster[5] = TCFire[iii + 1]; //bottom
      TempCluster[7] = TCFire[iii + 32]; // left
      TempCluster[8] = TCFire[iii + 31]; //top left


      if (iii == 0) {
        TempCluster[1] = TCFire[31]; // top
        TempCluster[8] = TCFire[63]; //top left
      } else {
        TempCluster[1] = TCFire[iii - 1]; // top
      }
      if (iii == 31) {
        TempCluster[5] = TCFire[0]; //bottom
        TempCluster[6] = TCFire[32]; //bottom left
      } else {
        TempCluster[6] = TCFire[iii + 33]; //bottom left
      }

    }


    if (iii > 31) {
      TempCluster[0] = TCFire[iii];
      TempCluster[1] = TCFire[iii - 1]; // top
      TempCluster[3] = TCFire[iii - 32]; //right
      TempCluster[4] = TCFire[iii - 31]; //right bottom
      TempCluster[6] = 0; //bottom left
      TempCluster[7] = 0; // left
      TempCluster[8] = 0; //top left
      if (iii == 32) {
        TempCluster[1] = TCFire[63]; // top
        TempCluster[2] = TCFire[31];// right top

      } else {
        TempCluster[2] = TCFire[iii - 33]; // right top
      }
      if (iii == 63) {
        TempCluster[5] = TCFire[32]; //bottom
        TempCluster[4] = TCFire[0]; //right bottom

      } else {
        TempCluster[5] = TCFire[iii + 1]; //bottom
      }
    }



    if (!(TempCluster[1] != 0 || TempCluster[7] != 0)) {
      if (!(TempCluster[5] != 0 && TempCluster[6] != 0)) {
        _BWDICN ++;
        int phiid = _TCMap -> getTCPhiIdFromTCId(TCFire[iii]);

        if (phiid == 32 || (phiid > 0 && phiid < 10)) {
          _Quadrant[2][0]++;
        }
        if (phiid > 7 && phiid < 18) {
          _Quadrant[2][1]++;
        }
        if (phiid > 15 && phiid < 26) {
          _Quadrant[2][2]++;
        }
        if ((phiid > 22 && phiid < 33) || phiid == 1) {
          _Quadrant[2][3]++;
        }


      }
    }

  }




  return _BWDICN;

}
//
//===<END>
//
