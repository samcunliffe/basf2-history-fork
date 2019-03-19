/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/HistoModule.h>
#include <vxd/geometry/GeoCache.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawFTSWFormat_latest.h>
#include <pxd/dataobjects/PXDRawHit.h>
#include <TH1.h>
#include <string>
#include <vector>
#include <map>

namespace Belle2 {

  namespace PXD {
    /** The PXD Occ after Injection DQM module.
     *
     * Occ after Injection (taken from TTD packet)
     */
    class PXDInjectionDQMModule : public HistoModule {

    public:

      /** Constructor defining the parameters */
      PXDInjectionDQMModule();

      void initialize() override final;

      void beginRun() override final;

      void event() override final;

    private:
      std::string m_histogramDirectoryName; /**< Name of the histogram directory in ROOT file */
      std::string m_PXDRawHitsName;  /**< The name of the StoreArray of PXDRawHits to be generated */

      bool m_eachModule{false};// create a histo per module

      /** Input array for DAQ Status. */
      StoreArray<RawFTSW> m_rawTTD;

      /** Input array for PXD Raw Hits. */
      StoreArray<PXDRawHit> m_storeRawHits;

      //the geometry
      VXD::GeoCache& m_vxdGeometry;

      TH1F* hOccAfterInjLER{};          /** Occupancy after LER injection */
      TH1F* hOccAfterInjHER{};          /** Occupancy after HER injection */
      std::map<VxdID, TH1F*> hOccModAfterInjLER;/** Occupancy after LER injection */
      std::map<VxdID, TH1F*> hOccModAfterInjHER;/** Occupancy after HER injection */

      TH1F* hEOccAfterInjLER{};          /** Occupancy after LER injection */
      TH1F* hEOccAfterInjHER{};          /** Occupancy after HER injection */
      std::map<VxdID, TH1F*> hEOccModAfterInjLER;/** Occupancy after LER injection */
      std::map<VxdID, TH1F*> hEOccModAfterInjHER;/** Occupancy after HER injection */

      void defineHisto() override final;

    };//end class declaration


  } //end PXD namespace;
} // end namespace Belle2
