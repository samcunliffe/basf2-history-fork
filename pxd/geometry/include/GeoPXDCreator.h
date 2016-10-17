/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Christian Oswald, Zbynek Drasal,           *
 *               Martin Ritter, Jozef Koval, Benjamin Schwenker           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOPXDCREATOR_H
#define GEOPXDCREATOR_H

#include <vector>
#include <vxd/geometry/GeoVXDCreator.h>
#include <vxd/dbobjects/PXDGeometryPar.h>


namespace Belle2 {
  /** Namespace to encapsulate code needed for simulation and reconstrucion of the PXD */
  namespace PXD {

    class SensorInfo;

    /** The creator for the PXD geometry of the Belle II detector.   */
    class GeoPXDCreator : public VXD::GeoVXDCreator {
    private:
      //! Create a parameter object from the Gearbox XML parameters.
      PXDGeometryPar createConfiguration(const GearDir& param)
      {
        PXDGeometryPar pxdGeometryPar;
        pxdGeometryPar.read(param);
        return pxdGeometryPar;
      };

      //! Create the geometry from a parameter object.
      void createGeometry(const PXDGeometryPar& parameters, G4LogicalVolume& topVolume, geometry::GeometryTypes type);
    public:
      /** Constructor of the GeoPXDCreator class. */
      GeoPXDCreator(): VXD::GeoVXDCreator("PXD") {};

      /** The destructor of the GeoPXDCreator class. */
      virtual ~GeoPXDCreator();

      /** Create the configuration objects and save them in the Database.  If
       * more than one object is needed adjust accordingly */
      virtual void createPayloads(const GearDir& content, const IntervalOfValidity& iov) override
      {
        DBImportObjPtr<PXDGeometryPar> importObj;
        importObj.construct(createConfiguration(content));
        importObj.import(iov);
      }

      /** Create the geometry from the Database */
      virtual void createFromDB(const std::string& name, G4LogicalVolume& topVolume, geometry::GeometryTypes type) override
      {
        DBObjPtr<PXDGeometryPar> dbObj;
        if (!dbObj) {
          // Check that we found the object and if not report the problem
          B2FATAL("No configuration for " << name << " found.");
        }
        createGeometry(*dbObj, topVolume, type);
      }

      /**
       * Create support structure for PXD Half Shell, that means everything
       * thagt does not depend on layer or sensor alignment
       * @param support Reference to the database containing the parameters
       */
      virtual VXD::GeoVXDAssembly createHalfShellSupport(GearDir support);

      /**
       * Read the sensor definitions from the database
       * @param sensors Reference to the database containing the parameters
       */
      virtual VXD::SensorInfoBase* createSensorInfo(const GearDir& sensor);

      /**
       * Return a SensitiveDetector implementation for a given sensor
       * @param sensorID  SensorID for the sensor
       * @param sensor    Information about the sensor to create the Sensitive Detector for
       * @param placement Information on how to place the sensor
       */
      virtual VXD::SensitiveDetectorBase* createSensitiveDetector(
        VxdID sensorID, const VXDGeoSensor& sensor, const VXDGeoSensorPlacement& placement);

    private:

      /** Vector of points to SensorInfo objects */
      std::vector<SensorInfo*> m_SensorInfo;

    }; // class GeoPXDCreator
  } // namespace PXD
} //namespace Belle2

#endif /* GEOPXDCREATOR_H */
