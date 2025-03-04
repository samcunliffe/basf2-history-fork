/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <svd/calibration/SVDHitTimeSelection.h>

#include <tracking/spacePointCreation/SpacePoint.h>

#include <string>

#include <TFile.h>

namespace Belle2 {
  /**
   * Imports Clusters of the SVD detector and converts them to spacePoints.
   *
   */
  class SVDSpacePointCreatorModule : public Module {

  public:



    /** Constructor */
    SVDSpacePointCreatorModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize() override;


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event() override;


    /** final output with mini-feedback */
    virtual void terminate() override;


    /** initialize variables in constructor to avoid nondeterministic behavior */
    void InitializeCounters();


  protected:


    // Data members
    std::string m_svdClustersName = "SVDClusters"; /**< SVDCluster collection name */

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::string m_spacePointsName = ""; /**< SpacePoints collection name */

    StoreArray<SpacePoint>
    m_spacePoints; /**< the storeArray for spacePoints as member, is faster than recreating link for each event */

    std::string m_eventLevelTrackingInfoName = ""; /**< Name of the EventLevelTrackingInfo */

    float m_minClusterTime = -999; /**< clusters with time below this value are not considered to make spacePoints*/

    // modification parameters
    std::string m_nameOfInstance =
      ""; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

    bool m_onlySingleClusterSpacePoints =
      false; /**< standard is false. If activated, the module will not try to find combinations of U and V clusters for the SVD any more. Does not affect pixel-type Clusters */

    std::string m_inputPDF = ""; /**< File path of root file containing pdf histograms */

    TFile* m_calibrationFile = nullptr; /**< Pointer to root TFile containing PDF histograms */

    bool m_useLegacyNaming = true; /**< Choice between PDF naming conventions */

    bool m_useQualityEstimator = false; /**< Standard is true. Can be turned off in case accessing pdf root file is causing errors */
    //counters for testing
    unsigned int m_TESTERSVDClusterCtr = 0; /**< counts total number of SVDCluster occured */

    unsigned int m_TESTERSpacePointCtr = 0; /**< counts total number of SpacePoints occured */

    unsigned int m_numMaxSpacePoints = 7e4; /**< do not crete SPs if their number exceeds m_numMaxSpacePoints, tuned with BG19*/

    SVDHitTimeSelection m_HitTimeCut; /**< selection based on clustr time db object*/
  };
} // end namespace Belle2
