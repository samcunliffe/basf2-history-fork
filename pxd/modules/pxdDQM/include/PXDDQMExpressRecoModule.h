/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Phase 2 and Belle II geometry                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/dataobjects/VxdID.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** PXD DQM Module */
  class PXDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    PXDDQMExpressRecoModule();
    /* Destructor */
    virtual ~PXDDQMExpressRecoModule();

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

    /** cut for accepting to hitmap histogram, using pixels only, default = 0 */
    float m_CutPXDCharge = 0.0;

    std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */

    /** PXDDigits StoreArray name */
    std::string m_storePXDDigitsName;
    /** PXDClusters StoreArray name */
    std::string m_storePXDClustersName;
    /** PXDClustersToPXDDigits RelationArray name */
    std::string m_relPXDClusterDigitName;
    /** Frames StoreArray name */
    std::string m_storeFramesName;

    /** Hitmaps of Digits */
    TH1I* m_hitMapCounts;
    /** Hitmaps of Clusters*/
    TH1I* m_hitMapClCounts;
    /** Hitmaps of digits on chips */
    TH1I* m_hitMapCountsChip;
    /** Hitmaps of clusters on chips */
    TH1I* m_hitMapClCountsChip;
    /** Fired pixels per event */
    TH1F** m_fired;
    /** Clusters per event */
    TH1F** m_clusters;
    /** Start row distribution */
    TH1F** m_startRow;
    /** Cluster seed charge by distance from the start row */
    TH1F** m_chargStartRow;
    /** counter for Cluster seed charge by distance from the start row */
    TH1F** m_startRowCount;
    /** Charge of clusters */
    TH1F** m_clusterCharge;
    /** Charge of pixels */
    TH1F** m_pixelSignal;
    /** u cluster size */
    TH1F** m_clusterSizeU;
    /** v cluster size */
    TH1F** m_clusterSizeV;
    /** Cluster size */
    TH1F** m_clusterSizeUV;

    /** Number of PXD chips per sensor in u (DCD) (=4) on Belle II */
    int c_nPXDChipsU = 4;
    /** Number of PXD chips per sensor in v (Switchers) (=6) on Belle II */
    int c_nPXDChipsV = 6;
    /** Number of PXD chips per sensor on Belle II */
    int c_nPXDChips;
    /** Number of VXD layers on Belle II */
    int c_nVXDLayers;
    /** Number of PXD layers on Belle II */
    int c_nPXDLayers;
    /** First VXD layer on Belle II */
    int c_firstVXDLayer;
    /** Last VXD layer on Belle II */
    int c_lastVXDLayer;
    /** First PXD layer on Belle II */
    int c_firstPXDLayer;
    /** Last PXD layer on Belle II */
    int c_lastPXDLayer;
    /** Number of PXD sensors on Belle II */
    int c_nPXDSensors;

    /** Function return index of chip in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @param Chip Chip position on sensor - DCDs or Switchers
       * @param IsU Info if Chip is on u direction (DCD)
       * @return Index of sensor in plots.
       */
    int getChipIndex(const int Layer, const int Ladder, const int Sensor, const int Chip, const int IsU) const;
    /** Function return position indexes of chipID in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       * @param Chip return Chip position on sensor - DCDs or Switchers
       * @param IsU return info if Chip is on u direction (DCD)
       */
    void getIDsFromChipIndex(const int Index, int& Layer, int& Ladder, int& Sensor, int& Chip, int& IsU) const;

    /** Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(const int Layer, const int Ladder, const int Sensor) const;
    /** Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(const int Index, int& Layer, int& Ladder, int& Sensor) const;

  };

}

