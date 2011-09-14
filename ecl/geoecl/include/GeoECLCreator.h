/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Poyuan Chen                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef GEOECLCREATOR_H
#define GEOECLCREATOR_H

#include <geometry/CreatorBase.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <ecl/simecl/ECLSensitiveDetector.h>

class G4LogicalVolume;
class G4VPhysicalVolume;

namespace Belle2 {

  namespace ecl {

    //!  The GeoECLCreator class.
    /*!
       The creator for the ECL geometry of the Belle II detector.
    */
    class GeoECLCreator : public geometry::CreatorBase {

    public:

      //! Constructor of the GeoECLCreator class.
      GeoECLCreator();

      //! The destructor of the GeoECLCreator class.
      ~GeoECLCreator();

      //! Creates the ROOT Objects for the ECL geometry.
      /*!
        \param content A reference to the content part of the parameter description, which should to be used to create the ROOT objects.
      */
      virtual void create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes type);


    protected:

    private:

      G4LogicalVolume* logical_ecl;
      G4VPhysicalVolume* physical_ecl;

      //! Sensitive detector
      ECLSensitiveDetector* m_sensitive;

    };

  } // end of ecl namespace
} // end of Belle2 namespace

#endif /* GEOECLBELLEII_H */
