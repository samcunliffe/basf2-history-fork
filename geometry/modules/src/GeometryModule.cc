/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <geometry/modules/GeometryModule.h>
#include <geometry/GeometryManager.h>
#include <framework/gearbox/GearDir.h>

using namespace std;

namespace Belle2 {

  REG_MODULE(Geometry);

  GeometryModule::GeometryModule()
  {
    setDescription("Setup geometry description");
    addParam("geometryPath", m_geometryPath,
             "Path where the parameters for the Geometry can be found", string("/Detector"));
    addParam("geometryType", m_geometryType,
             "Type of geometry to build. Valid values: Full, Tracking, Display", 0);

  }

  void GeometryModule::initialize()
  {
    geometry::GeometryManager::getInstance().createGeometry(GearDir(m_geometryPath), geometry::FullGeometry);
  }

} //Belle2 namespace
