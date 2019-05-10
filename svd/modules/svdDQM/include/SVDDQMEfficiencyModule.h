/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Gaetano De Marino, Giulia Casarosa       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#pragma once

#include <framework/core/HistoModule.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <svd/dataobjects/SVDCluster.h>
#include <svd/dataobjects/SVDSummaryPlots.h>

#include <vxd/geometry/GeoCache.h>
#include <vxd/geometry/SensorInfoBase.h>

#include <tracking/dataobjects/RecoTrack.h>

//root stuff
#include "TTree.h"
#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TVector3.h"


namespace Belle2 {

  /**
   * Creates the basic histograms for SVD Efficiency DQM
   * Freely inspired from PXDDQMEfficiencyModuleSimplified
   */
  class SVDDQMEfficiencyModule : public HistoModule {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDDQMEfficiencyModule();

  private:

    /**
     * main function which fills trees and histograms
     */
    void event() override final;

    /**
     * initializes the need store arrays, trees and histograms
     */
    void initialize() override final;

    /**
     * actually defines the trees and histograms
     */
    void defineHisto() override final;



  private:
    /** helper functions to do some of the calculations*/
    /* returns the space point in local coordinates where the track hits the sensor:
      sensorInfo: info of the sensor under investigation
      aTrack: the track to be tested
      isgood: flag which is false if some error occured (do not use the point if false)
      du and dv are the uncertainties in u and v on the sensor plane of the fit (local coordinates)
     */
    TVector3 getTrackInterSec(VXD::SensorInfoBase& svdSensorInfo, const RecoTrack& aTrack, bool& isgood, double& du, double& dv);

    int findClosestCluster(VxdID& vxdid, TVector3 intersection, bool isU);

    bool isCloseToBorder(int u, int v, int checkDistance);

    bool isMaskedStripClose(int u, int v, int checkDistance, VxdID& moduleID);

    //if true alignment will be used!
    bool m_useAlignment;

    bool m_maskStrips;

    bool m_cutBorders;

    bool m_saveExpertHistos;
    //the geometry
    VXD::GeoCache& m_vxdGeometry;

    //Where to save the histograms too
    std::string m_histogramDirectoryName;

    std::string m_svdClustersName;
    std::string m_tracksName;

    int m_u_bins;
    int m_v_bins;

    StoreArray<SVDCluster> m_svdclusters;
    StoreArray<RecoTrack> m_tracks;

    double m_distcut; //distance cut in cm!
    double m_pcut; //pValue-Cut for tracks
    double m_momCut; //Cut on fitted track momentum
    double m_ptCut; //Cut on fitted track pt
    unsigned int m_minSVDHits; //Required hits in SVD strips for tracks
    unsigned int m_minPXDHits; //Required hits in PXD for tracks
    unsigned int m_minCDCHits; //Required hits in CDC for tracks
    int m_maskedDistance; //Distance inside which no dead pixel or module border is allowed

    //Histograms to later determine efficiency
    std::map<VxdID, TH2D*> m_h_track_hits;
    std::map<VxdID, TH2D*> m_h_matched_clusterU;
    std::map<VxdID, TH2D*> m_h_matched_clusterV;
    SVDSummaryPlots* m_TrackHits = nullptr;
    SVDSummaryPlots* m_MatchedHits = nullptr;

  };
}
