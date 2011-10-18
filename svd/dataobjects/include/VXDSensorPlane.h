/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Martin Ritter                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef VXDSENSORPLANE_H_
#define VXDSENSORPLANE_H_

#include <pxd/vxd/VxdID.h>
#include <pxd/vxd/SensorInfoBase.h>
#ifndef __CINT__
#include <pxd/vxd/GeoCache.h>
#endif

#include <GFAbsFinitePlane.h>
#include <cmath>

namespace Belle2 {

  /**
   * A Finite plane of one VXD Sensor.
   * This class takes the SensorID of the sensor and gets the dimensions from the SensorInfo of that Sensor.
   */
  class VXDSensorPlane: public GFAbsFinitePlane {
  public:
    /**
     * Constructs the plane for a given VXD Sensor
     * @param sensorID SensorID of the Sensor for which this plane should be build
     */
    VXDSensorPlane(VxdID sensorID = 0): m_sensorID(sensorID), m_sensorInfo(0) {}

    /** Destructor. */
    virtual ~VXDSensorPlane() {}

    /** Return wether the given coordinates are inside the finite region.
     * @param u u-coordinate of the point.
     * @param v v-coordinate of the point.
     * @return true if (u,v) is within the sensor plane, otherwise false.
     */
    bool inActive(const double& u, const double& v) const {
#ifndef __CINT__
      if (!m_sensorInfo) {
        m_sensorInfo = &VXD::GeoCache::get(m_sensorID);
      }
#endif
      return m_sensorInfo->inside(u, v);
    }

    /** Prints object data. */
    void Print(const Option_t* option = "") const;

    /**
     * Deep copy of the object.
     * @return Pointer to a deep copy of the object.
     */
    virtual GFAbsFinitePlane* clone() const {
      return new VXDSensorPlane(*this);
    }

  private:
    unsigned short m_sensorID;
    mutable const VXD::SensorInfoBase* m_sensorInfo; //! transient member

    ClassDef(VXDSensorPlane, 1)
  };
} // end namespace

#endif /* VXDDETECTORPLANE_H_ */
