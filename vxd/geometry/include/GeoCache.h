/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXD_GEOCACHE_H
#define VXD_GEOCACHE_H

#include <vxd/dataobjects/VxdID.h>
#include <vxd/geometry/SensorInfoBase.h>
#include <vector>
#include <set>
#include <map>

#include <TMath.h>

#ifndef __CINT__
#include <boost/unordered_map.hpp>
#endif

#include <Geant4/G4Transform3D.hh>

class G4VPhysicalVolume;

namespace Belle2 {
  /** Namespace to provide code needed by both Vertex Detectors, PXD and SVD, and also testbeam telescopes */
  namespace VXD {
    /**
     * Class to faciliate easy access to sensor information of the VXD like
     * coordinate transformations or pitch size.
     */
    class GeoCache {
    public:
      /** destructor to clean up structures */
      ~GeoCache() { clear(); };


      /** Clean up internal structures */
      void clear();

      /** Search a given Geometry for Sensors.
       * This method will search a given Physical Volume and add all
       * sensitive Volumes where the SensitiveDetector implementation
       * inherits from vxd::SensitiveDetector to the cache.
       *
       * The given volume *must be* a volume placed directly in the top
       * volume, otherwise the transformations cannot be determined
       * correctly.
       *
       * This method should be called by the geometry creator once all
       * sensitive volumes are placed
       *
       * @param envelope Geometry tree to search, usually the Envelope of PXD or SVD
       */
      void findVolumes(G4VPhysicalVolume* envelope);

      /** Add a SensorInfo instance to the list of known sensors
       * This method will manually add a SensorInfo instance to the list of
       * known sensors. It should not be needed except for testing purposes.
       *
       * @param sensorinfo SensorInfoBase instance to add to the list of known sensors.
       */
      void addSensor(SensorInfoBase* sensorinfo);

      /** Return a set of all known Layers.
       * @param sensortype Wether to only return the layer IDs for PXD, SVD or all VXD layers (default)
       * @return a set containing all existing Layers with the requested type
       */
      const std::set<Belle2::VxdID>  getLayers(SensorInfoBase::SensorType sensortype = SensorInfoBase::VXD);
      /** Return a set of all ladder IDs belonging to a given layer */
      const std::set<Belle2::VxdID>& getLadders(Belle2::VxdID layer) const;
      /** Return a set of all sensor IDs belonging to a given ladder */
      const std::set<Belle2::VxdID>& getSensors(Belle2::VxdID ladder) const;

      /** Check that id is a valid sensor number.
       * @param id sensor id to be checked against current GeoCache information
       * @return true if there is a SensorInfo associated with id, otherwise false.
       */
      bool validSensorID(Belle2::VxdID id) const;

      /** Get list of all sensors.
       * Intended mostly for Python, where std::sets don't work.
       * @return vector of VxdIDs of all sensors in GeoCache.
       */
      const std::vector<VxdID> getListOfSensors() const;

      /** Return a referecne to the SensorInfo of a given SensorID */
      /* If sensor not found, it returns a warning and the first sensor!*/
      const SensorInfoBase& getSensorInfo(Belle2::VxdID id) const;

      // ------------------ Alignment constants in reconstruction + hierarchy ------------------------------

      /// Retrieve stored half-shell placements into world volume
      /// @return vector of pairs, each pair contains a VxdID for half-shell (0.0.0#1-4) and its placement
      const std::vector<std::pair<VxdID, TGeoHMatrix>>& getHalfShellPlacements() const;

      /// Retrieve stored ladder placements into half-shell
      /// @return vector of pairs, each pair contains a VxdID for ladder (layer.ladder.0) and its placement
      const std::vector<std::pair<VxdID, TGeoHMatrix>>& getLadderPlacements(VxdID halfShell) const;

      /// Retrieve stored sensor placements into ladders
      /// @return vector of pairs, each pair contains a VxdID for sensor and its placement
      const std::vector<std::pair<VxdID, TGeoHMatrix>>& getSensorPlacements(VxdID ladder) const;

      /// Remember how half-shell is placed into world volume
      void addHalfShellPlacement(VxdID halfShell, G4Transform3D placement);

      /// Remember how ladder is placed into half-shell
      void addLadderPlacement(VxdID halfShell, VxdID ladder, G4Transform3D placement);

      /// Remember how sensor is placed into ladder
      void addSensorPlacement(VxdID ladder, VxdID sensor, G4Transform3D placement);

      /// Initialize from DB for reconstruction
      /// Updates all SensorInfo transformations for reconstruction from DB object(s)
      /// (recalculating new global positions) and registers itself for subsequent updates
      /// of DB objects to keep the hierarchy up-to-date.
      void setupReconstructionTransformations();

      /// Covenient function to convert G4Transform3D to TGeoHMatrix
      static TGeoHMatrix g4Transform3DToTGeo(G4Transform3D g4);

      /// Convert 6 rigid body params (alignment corrections) to corresponding TGeoHMatrix
      /// Angles in radians, length units in centimeters.
      static TGeoHMatrix getTGeoFromRigidBodyParams(double dU, double dV, double dW, double dAlpha, double dBeta, double dGamma);

      // --------------------------------------------------------------------------------------------------

      /** Return a reference to the SensorInfo of a given SensorID.
       * This function is a shorthand for GeoCache::getInstance().getSensorInfo
       */
      static const SensorInfoBase& get(Belle2::VxdID id) { return getInstance().getSensorInfo(id); }

      /** Return a reference to the singleton instance */
      static GeoCache& getInstance();

    private:
#ifndef __CINT__
      /** Hash map to store pointers to all existing SensorInfos with constant lookup complexity */
      typedef boost::unordered_map<VxdID::baseType, SensorInfoBase*> SensorInfoMap;
#endif
      /** Map to store a set of unique VxdIDs belonging to one VxdID. Like ladders belong to layers, etc. */
      typedef std::map<Belle2::VxdID, std::set<Belle2::VxdID> > SensorHierachy;

      /** Singleton class, hidden constructor */
      GeoCache() {};
      /** Singleton class, hidden copy constructor */
      GeoCache(const GeoCache&) = delete;
      /** Singleton class, hidden assignment operator */
      GeoCache& operator=(const GeoCache&) = delete;

      /** Set of all PXD layer IDs */
      std::set<Belle2::VxdID> m_pxdLayers;
      /** Set of all SVD layer IDs */
      std::set<Belle2::VxdID> m_svdLayers;
      /** Set of all Tel layer IDs */
      std::set<Belle2::VxdID> m_telLayers;
      /** Map of all Ladder IDs belonging to a given Layer ID */
      SensorHierachy m_ladders;
      /** Map of all Sensor IDs belonging to a given Ladder ID */
      SensorHierachy m_sensors;

      /// vector of shell ids and their placements in top volume
      std::vector<std::pair<VxdID, TGeoHMatrix>> m_halfShellPlacements {};
      /// Map of shell ids and their associated ladder ids and their placements
      std::map<VxdID, std::vector<std::pair<VxdID, TGeoHMatrix>>> m_ladderPlacements {};
      /// Map of ladder ids and their associated sensor ids and their placements
      std::map<VxdID, std::vector<std::pair<VxdID, TGeoHMatrix>>> m_sensorPlacements {};

#ifndef __CINT__
      /** Map to find the SensorInfo for a given Sensor ID */
      SensorInfoMap m_sensorInfo;
#endif
    };
  }
} //Belle2 namespace
#endif
