#ifndef SVDDQMMODULE_H_
#define SVDDQMMODULE_H_

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
#include "TH1F.h"

namespace Belle2 {

  /** SVD DQM Module */
  class SVDDQMModule : public HistoModule {  // <- derived from HistoModule class

  public:

    /** Number of SVD planes and their numbers.
     * The actual (layer, ladder, sensor numbers are (i,1,i), i = 3,4,5,6
     */
    enum {
      c_nSVDPlanes = 4,
      c_firstSVDPlane = 3,
      c_lastSVDPlane = 6,
    };

    /** Constructor */
    SVDDQMModule();
    /* Destructor */
    virtual ~SVDDQMModule();

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
    /** Utility functions to convert indices to plane numbers and v.v.,
     * and to protect against range errors.
     */
    inline int indexToPlane(int index) const {
      return c_firstSVDPlane + index;
    }
    inline int planeToIndex(int iPlane) const {
      return iPlane - c_firstSVDPlane;
    }
    /** This is a shortcut to getting SVD::SensorInfo from the GeoCache.
     * @param index Index of the sensor (0,1,2,3), _not_ layer number!
     * @return SensorInfo object for the desired plane.
     */
    inline const SVD::SensorInfo& getInfo(int index) const;

    std::string m_storeDigitsName;      /**< SVDDigits StoreArray name */
    std::string m_storeClustersName;    /**< SVDClusters StoreArray name */
    std::string m_relClusterDigitName;  /**< SVDClustersToSVDDigits RelationArray name */

    // +1 in dimensions to protect against noisy VXDID values.
    TH1F* m_firedU[c_nSVDPlanes];       /**< Fired u strips per event by plane */
    TH1F* m_firedV[c_nSVDPlanes];       /**< Fired v strips per event by plane */
    TH1F* m_clustersU[c_nSVDPlanes];      /**< u clusters per event by plane */
    TH1F* m_clustersV[c_nSVDPlanes];      /**< v clusters per event by plane */
    TH1F* m_hitMapU[c_nSVDPlanes];        /**< Hitmaps for u-strips by plane */
    TH1F* m_hitMapV[c_nSVDPlanes];        /**< Hitmaps for v-strips by plane */
  };

  inline const SVD::SensorInfo& SVDDQMModule::getInfo(int index) const
  {
    int iPlane = indexToPlane(index);
    VxdID sensorID(iPlane, 1, iPlane);
    return dynamic_cast<const SVD::SensorInfo&>(VXD::GeoCache::get(sensorID));
  }
}
#endif

