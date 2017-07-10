/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * Prepared for Belle II geometry                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SVDDQMExpressRecoMODULE_H_
#define SVDDQMExpressRecoMODULE_H_

#undef DQM
#ifndef DQM
#include <framework/core/HistoModule.h>
#else
#include <daq/dqm/modules/DqmHistoManagerModule.h>
#endif
#include <vxd/dataobjects/VxdID.h>
#include <svd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <vector>
#include "TH1I.h"
#include "TH1F.h"
#include "TH2F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class SVDDQMExpressRecoModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Constructor */
    SVDDQMExpressRecoModule();
    /* Destructor */
    virtual ~SVDDQMExpressRecoModule();

    /** Module functions */
    virtual void initialize();
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /**
     * Histogram definitions such as TH1(), TH2(), TNtuple(), TTree().... are supposed
     * to be placed in this function.
    */
    virtual void defineHisto();

  private:

    float m_CutSVDCharge = 22.0;           /**< cut for accepting to hitmap histogram, using strips only, default = 22 */

    std::string m_storeSVDDigitsName;      /**< SVDDigits StoreArray name */
    std::string m_storeSVDClustersName;    /**< SVDClusters StoreArray name */
    std::string m_relSVDClusterDigitName;  /**< SVDClustersToSVDDigits RelationArray name */

    TDirectory* m_oldDir;                  /**< Basic Directory in output file */

    /** Name of file contain reference histograms, default=VXD-ReferenceHistos */
    std::string m_RefHistFileName = "vxd/data/VXD-DQMReferenceHistos.root";
    int m_NoOfEvents;        /** Number of events */
    int m_NoOfEventsRef;     /** Number of events in reference histogram */

    TH1I* m_fHitMapCountsUFlag;    /**< Flags of u Hitmaps of Digits */
    TH1I* m_fHitMapCountsVFlag;    /**< Flags of v Hitmaps of Digits */
    TH1I* m_fHitMapClCountsUFlag;  /**< Flags of u Hitmaps of Clusters*/
    TH1I* m_fHitMapClCountsVFlag;  /**< Flags of v Hitmaps of Clusters*/
    TH1I* m_fFiredUFlag;           /**< Flags of Hitmaps in U of Digits */
    TH1I* m_fFiredVFlag;           /**< Flags of Hitmaps in V of Digits */
    TH1I* m_fClustersUFlag;        /**< Flags of u Clusters per event */
    TH1I* m_fClustersVFlag;        /**< Flags of v Clusters per event */
    TH1I* m_fClusterChargeUFlag;   /**< Flags of u Charge of clusters */
    TH1I* m_fClusterChargeVFlag;   /**< Flags of v Charge of clusters */
    TH1I* m_fStripSignalUFlag;     /**< Flags of u Charge of strips */
    TH1I* m_fStripSignalVFlag;     /**< Flags of v Charge of strips */
    TH1I* m_fClusterSizeUFlag;     /**< Flags of u cluster size */
    TH1I* m_fClusterSizeVFlag;     /**< Flags of v cluster size */
    TH1I* m_fClusterTimeUFlag;     /**< Flags of u cluster size */
    TH1I* m_fClusterTimeVFlag;     /**< Flags of v cluster size */

    TH1I* m_hitMapCountsU;         /**< Hitmaps u of Digits */
    TH1I* m_hitMapCountsV;         /**< Hitmaps v of Digits */
    TH1I* m_hitMapClCountsU;       /**< Hitmaps u of Clusters*/
    TH1I* m_hitMapClCountsV;       /**< Hitmaps v of Clusters*/
    TH1F** m_firedU;               /**< Fired u strips per event */
    TH1F** m_firedV;               /**< Fired v strips per event */
    TH1F** m_clustersU;            /**< u clusters per event */
    TH1F** m_clustersV;            /**< v clusters per event */
    TH1F** m_clusterChargeU;       /**< u charge of clusters */
    TH1F** m_clusterChargeV;       /**< v charge of clusters */
    TH1F** m_stripSignalU;         /**< u charge of strips */
    TH1F** m_stripSignalV;         /**< v charge of strips */
    TH1F** m_clusterSizeU;         /**< u size */
    TH1F** m_clusterSizeV;         /**< v size */
    TH1F** m_clusterTimeU;         /**< u time */
    TH1F** m_clusterTimeV;         /**< v time */

    int c_nVXDLayers;              /**< Number of VXD layers on Belle II */
    int c_nPXDLayers;              /**< Number of PXD layers on Belle II */
    int c_nSVDLayers;              /**< Number of SVD layers on Belle II */
    int c_firstVXDLayer;           /**< First VXD layer on Belle II */
    int c_lastVXDLayer;            /**< Last VXD layer on Belle II */
    int c_firstPXDLayer;           /**< First PXD layer on Belle II */
    int c_lastPXDLayer;            /**< Last PXD layer on Belle II */
    int c_firstSVDLayer;           /**< First SVD layer on Belle II */
    int c_lastSVDLayer;            /**< Last SVD layer on Belle II */
    int c_nSVDSensors;             /**< Number of SVD sensors on Belle II */

    /**< Function return index of sensor in plots.
       * @param Layer Layer position of sensor
       * @param Ladder Ladder position of sensor
       * @param Sensor Sensor position of sensor
       * @return Index of sensor in plots.
       */
    int getSensorIndex(int Layer, int Ladder, int Sensor);
    /**< Function return index of sensor in plots.
       * @param Index Index of sensor in plots.
       * @param Layer return Layer position of sensor
       * @param Ladder return Ladder position of sensor
       * @param Sensor return Sensor position of sensor
       */
    void getIDsFromIndex(int Index, int* Layer, int* Ladder, int* Sensor);
    /**< Function return flag histogram filled based on condition.
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flag Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1F* hist, TH1F* refhist, TH1I* flaghist);
    /**< Function return flag histogram filled based on condition.
       * @param Type Set type of condition for flag calculation.
       * 1: use counts, mean and RMS.
       * 2: use counts only.
       * 3: use mean only.
       * 4: use RMS only.
       * 5: use counts and mean.
       * 9: use bin content only.
       * 10: use Chi2 condition and pars[0] and pars[1].
       * 100: nothing do just fill flags as OK.
       * @param bin bin which is fill in flag histogram.
       * @param pars array of parameters need for condition.
       * @param ratio Ratio of acquired events to reference events.
       * @param hist Histogram of sources.
       * @param refhist Reference histogram.
       * @param flag Histogram of flags.
       * @return Indication of succes of realizing of condition, 1: OK.
       */
    int SetFlag(int Type, int bin, double* pars, double ratio, TH1I* hist, TH1I* refhist, TH1I* flaghist);

  };

}
#endif

