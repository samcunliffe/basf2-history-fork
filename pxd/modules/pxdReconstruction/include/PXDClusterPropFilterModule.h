/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: bjoern.spruck@belle2.org                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>
#include <pxd/dataobjects/PXDCluster.h>

namespace Belle2 {

  /**
   * The module produce a StoreArray of PXDCluster with specific properties.
   *
   *    *
   */
  class PXDClusterPropFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    PXDClusterPropFilterModule();

  private:

    /**  */
    void initialize() override final;

    /**  */
    void event() override final;

    /**  all the actual work is done here */
    void filterClusters();

    bool m_CreateInside = true; /**< if set, create list of inside cuts */
    bool m_CreateOutside = false; /**< if set, create list of outside cuts */

    std::string m_PXDClustersName;  /**< The name of the StoreArray of PXDClusters to be filtered */
    std::string m_PXDClustersInsideCutsName;  /**< The name of the StoreArray of Filtered PXDClusters inside cuts */
    std::string m_PXDClustersOutsideCutsName;  /**< The name of the StoreArray of Filtered PXDClusters outside cuts */

    SelectSubset< PXDCluster > m_selectorIN; /**< selector of the subset of PXDClusters contained in the ROIs*/
    SelectSubset< PXDCluster > m_selectorOUT; /**< selector of the subset of PXDClusters NOT contained in the ROIs*/

    bool CheckCuts(const PXDCluster& thePXDCluster);/**< the actual check for cluster properties */

  };
}
