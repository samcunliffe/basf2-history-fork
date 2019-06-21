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
#include <vxd/dataobjects/VxdID.h>

#include <tracking/dataobjects/SVDIntercept.h>
#include <tracking/dataobjects/RecoTrack.h>

#include "TString.h"
#include "TH1D.h"
#include "TH2D.h"

namespace Belle2 {

  /**
   * Creates the basic histograms for SVD Efficiency DQM
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


    bool isGoodIntercept(SVDIntercept* inter); /**< returns true if the track related to the intercept passes the selection cuts */

    VXD::GeoCache& m_geoCache; /**< BelleII Geometry*/

    std::string m_svdClustersName; /**< SVDClusters StoreArray name*/
    std::string m_interceptsName; /**< SVDIntercepts StoreArray name*/

    StoreArray<SVDCluster> m_svdClusters; /**< SVDCluster StoreArray*/
    StoreArray<SVDIntercept> m_intercepts; /**< SVDIntercept StoreArray*/

    float m_fiducialU; /**< stay away from the U border by m_fiducialU (in cm)*/
    float m_fiducialV; /**< stay away from the U border by m_fiducialU (in cm)*/

    float m_maxResidU; /**< max distance cut in cm U side */
    float m_maxResidV; /**< max distance cut in cm V side */

    //    double m_d0cut; /**<d0-Cut for tracks */
    //    double m_z0cut; /**<z0-Cut for tracks */
    double m_pcut; /**<pValue-Cut for tracks */
    double m_momCut; /**<Cut on fitted track momentum */
    double m_ptCut; /**<Cut on fitted track pt */
    unsigned int m_minSVDHits; /**<Required hits in SVD strips for tracks */
    unsigned int m_minCDCHits; /**<Required hits in CDC for tracks */

    /* Histograms to later determine efficiency */

    std::string m_histogramDirectoryName; /**< name of the directory where to store the histograms*/

    bool m_saveExpertHistos; /**< save additional histograms id set True*/

    int m_u_bins; /**< number of U-bins for expert histogram*/
    int m_v_bins; /**< number of V-bins for expert histogram*/

    std::map<VxdID, TH2D*> m_h_track_hits; /**< track hits histogram map to sensorID*/
    std::map<VxdID, TH2D*> m_h_matched_clusterU; /**< matched U-hits histogram map to sensorID*/
    std::map<VxdID, TH2D*> m_h_matched_clusterV; /**< matched V-hits histogram map to sensorID*/
    SVDSummaryPlots* m_TrackHits = nullptr; /**< track hits summary plot*/
    SVDSummaryPlots* m_MatchedHits = nullptr; /**< matched hits summary plot*/

  };
}
