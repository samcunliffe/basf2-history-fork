/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jakob Lettenbichler                                      *
 *                                                                        *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

#include <vxd/dataobjects/VxdID.h>

#include <tracking/spacePointCreation/SpacePoint.h>
#include <testbeam/vxd/dataobjects/TelCluster.h>

#include <string>



namespace Belle2 {
  /**
   * Tester module for the validity of the TBSpacePointCreatorModule.
   *
   * The important thing compared to the SpacePointCreatorModule of the tracking package is:
   * both test for spacePoints,
   * but the TBSpacePointCreator does use a StoreArray of <TBSpcePint> and applies a StoreArray<SpacePoint> as mask on it.
   * This ensures that the TBSpacePoints can safely be treated as SpacePoints for track finding purposes
   * and do only need a separate Creator-Module in the testbeam package (TBSpacePointCreator instead of SpacePointCreator)
   *
   * Info Jakob (Aug 24, 2014)
   * TODO: at the moment, the genfit-output can only verified visually
   * (by checking, whether the detector types match the number of dimensions stored in the trackPoint)!
   * when full reco chain is working, this testerModule should be extended!
   * -> verification that input cluster(s) is/are converted to genfit-stuff shall be resilient!
   */
  class SpacePointCreatorTELTestModule : public Module {

  public:



    /** Constructor */
    SpacePointCreatorTELTestModule();



    /** Init the module.
    *
    * prepares all store- and relationArrays.
    */
    virtual void initialize();


    /** eventWise jobs (e.g. storing spacepoints */
    virtual void event();


    /** final output with mini-feedback */
    virtual void terminate() {}



  protected:


    // Data members
    std::string m_pxdClustersName; /**< PXDCluster collection name */

    StoreArray<PXDCluster>
    m_pxdClusters; /**< the storeArray for pxdClusters as member, is faster than recreating link for each event */

    std::string m_svdClustersName; /**< SVDCluster collection name */

    StoreArray<SVDCluster>
    m_svdClusters; /**< the storeArray for svdClusters as member, is faster than recreating link for each event */

    std::string m_telClustersName; /**< TelCluster collection name */

    StoreArray<TelCluster>
    m_telClusters; /**< the storeArray for telClusters as member, is faster than recreating link for each event */

    std::vector< StoreArray<SpacePoint> > m_allSpacePointStoreArrays; /**< a vector full of StoreArray carrying spacePoints. */

    std::vector< std::string > m_containerSpacePointsName; /**< intermediate storage for the names of the loaded storeArrays. */


    // modification parameters
    std::string
    m_nameOfInstance; /**< allows the user to set an identifier for this module. Usefull if one wants to use several instances of that module */

  };
} // end namespace Belle2
