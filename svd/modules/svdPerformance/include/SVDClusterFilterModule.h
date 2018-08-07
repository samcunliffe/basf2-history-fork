/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Lueck                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef REMOVESVDCLUSTERSMODULE_H
#define REMOVESVDCLUSTERSMODULE_H

#include <framework/core/Module.h>
#include <framework/datastore/SelectSubset.h>

#include <svd/dataobjects/SVDCluster.h>


namespace Belle2 {
  /**
   * generates a new StoreArray from the input StoreArray which has all specified Clusters removed
   *
   * generates a new StoreArray from the input StoreArray which has all specified Clusters removed
   *
   */
  class SVDClusterFilterModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDClusterFilterModule();

    /** if required */
    virtual ~SVDClusterFilterModule();

    /** initializes the module */
    virtual void beginRun();

    /** processes the event */
    virtual void event();

    /** end the run */
    virtual void endRun();

    /** terminates the module */
    virtual void terminate();

    /** init the module */
    virtual void initialize();

  private:

    std::string m_inputArrayName;  /**< StoreArray with the input clusters */
    std::string m_outputINArrayName;  /**< StoreArray with the selectd output clusters */
    std::string m_outputOUTArrayName;  /**< StoreArray with the NOT selected output clusters */
    int m_layerNum;  /** the layer number from which the clusters should be excluded  m_sensorID*/

    SelectSubset<SVDCluster> m_selectedClusters; /** all clusters NOT on the layer with m_layerNum */
    SelectSubset<SVDCluster> m_notSelectedClusters; /** all clusters on the layer with m_layerNum */
  };
}

#endif /* REMOVESVDCLUSTERSMODULE_H */
