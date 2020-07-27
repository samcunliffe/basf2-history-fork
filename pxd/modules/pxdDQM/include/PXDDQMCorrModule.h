/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <vector>
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Corr Module */
  class PXDDQMCorrModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMCorrModule();

  private:
    /** Module functions */
    void initialize() override final;
    void beginRun() override final;
    void event() override final;

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    void defineHisto() override final;

  private:
    /** PXDClusters StoreArray name */
    std::string m_storeClustersName;
    /** Name of the histogram directory in ROOT file */
    std::string m_histogramDirectoryName;

    /** Storearray for clusters   */
    StoreArray<PXDCluster> m_storeClusters;

    // +1 in dimensions to protect against noisy VXDID values.
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationU{};
    /** Correlation Sensor 1 vs 2 */
    TH2F* m_CorrelationV{};
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaU{};
    /** Correlation Sensor 1 vs 2 */
    TH1F* m_DeltaV{};
  };

}

