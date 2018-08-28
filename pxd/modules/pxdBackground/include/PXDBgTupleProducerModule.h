/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/geometry/GeoCache.h>
#include <string>
#include <vector>
#include <memory>
#include <map>


namespace Belle2 {

  namespace PXD {

    /** PXD Background Tuple Producer.
     *
     * This module produces PXD tuples for PXD background studies. The output is a
     * root TFile with a TTree containing one second time stamps together with
     * different PXD observables like occupancy, charged particle flux, photon flux
     * and exposition.
     *
     * falls nicht gesetzt, dann SetMetaTimeModule einbinden.
     */
    class PXDBgTupleProducerModule: public Module {

    public:

      /** Struct to hold data of an PXD sensor */
      struct SensorData {
        /** Exposition (energy deposited per cm2 and second) */
        double m_expo;
        /** Dose (Gy per second) */
        double m_dose;
        /** Photon like clusters per cm and second */
        double m_photonFlux;
        /** Charged particle like clusters per cm and second */
        double m_chargedFlux;
        /** Occupancy*/
        double m_occupancy;
      };

      /** Constructor */
      PXDBgTupleProducerModule();
      /** Destructor */
      virtual ~PXDBgTupleProducerModule();

      /* Initialize module */
      virtual void initialize();
      /* Start-of-run initializations */
      virtual void beginRun();
      /* Event processing */
      virtual void event();
      /* End-of-run tasks */
      virtual void endRun();
      /* Final summary and cleanup */
      virtual void terminate();

    private:

      // General
      const double c_densitySi = 2.3290 * Unit::g_cm3; /**< Density of crystalline Silicon */


      /** This is a shortcut to getting PXD::SensorInfo from the GeoCache.
       * @param sensorID VxdID of the sensor
       * @return SensorInfo object for the desired sensor.
       */
      inline const PXD::SensorInfo& getInfo(VxdID sensorID) const;
      /** Return thickness of the sensor with the given sensor ID */
      inline double getSensorThickness(VxdID sensorID) const;
      /** Return mass of the sensor with the given sensor ID */
      inline double getSensorMass(VxdID sensorID) const;
      /** Return area of the sensor with the given sensor ID */
      inline double getSensorArea(VxdID sensorID) const;

      /** Convert local sensor coordinates to global */
      const TVector3& pointToGlobal(VxdID sensorID, const TVector3& local);
      /** Convert local vector coordinates to global */
      const TVector3& vectorToGlobal(VxdID sensorID, const TVector3& local);

      /** Get number of sensors in a layer */
      inline int getNumSensors(int layerNum);
      /** Get total number of sensors */
      inline int getTotalSensors();

      // Output directory
      std::string m_outputDirectoryName; /**< Path to directory where output data will be stored */

      // StoreArrays
      std::string m_storeClustersName; /**< PXDClusters StoreArray name */
      std::string m_storeDigitsName; /**< PXDDigits StoreArray name */

      double m_integrationTime; /**< Integration time of PXD. */

      std::map<VxdID, SensorData> m_sensorData; /**< Struct to hold sensor-wise background data. */
    };

    inline const PXD::SensorInfo& PXDBgTupleProducerModule::getInfo(VxdID sensorID) const
    {
      return dynamic_cast<const PXD::SensorInfo&>(VXD::GeoCache::get(sensorID));
    }

    inline  double PXDBgTupleProducerModule::getSensorThickness(VxdID sensorID) const
    {
      return getInfo(sensorID).getThickness();
    }

    inline  double PXDBgTupleProducerModule::getSensorMass(VxdID sensorID) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength() * info.getThickness() * c_densitySi;
    }

    inline  double PXDBgTupleProducerModule::getSensorArea(VxdID sensorID) const
    {
      const PXD::SensorInfo& info = getInfo(sensorID);
      return info.getWidth() * info.getLength();
    }

    inline int PXDBgTupleProducerModule::getNumSensors(int layerNum)
    {
      VxdID layerID;
      layerID.setLayerNumber(layerNum);
      int result = 0;
      for (auto ladderID : VXD::GeoCache::getInstance().getLadders(layerID))
        result += VXD::GeoCache::getInstance().getSensors(ladderID).size();
      return result;
    }

    /** Get total number of sensors */
    inline int PXDBgTupleProducerModule::getTotalSensors()
    {
      int result = 0;
      for (auto layerID : VXD::GeoCache::getInstance().getLayers(VXD::SensorInfoBase::PXD))
        result += getNumSensors(layerID.getLayerNumber());
      return result;
    }
  } // namespace PXD
} // namespace Belle2

