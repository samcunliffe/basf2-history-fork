/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Ritter, Igal Jaegle                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <beast/csi/geometry/CsiCreator.h>
#include <beast/csi/simulation/SensitiveDetector.h>

#include <geometry/Materials.h>
#include <geometry/CreatorFactory.h>
#include <geometry/utilities.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
//#include <csi/simulation/SensitiveDetector.h>

#include <cmath>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>

//Shapes
#include <G4Trap.hh>
#include <G4Box.hh>
#include <G4Polycone.hh>
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include <G4UserLimits.hh>
#include <G4RegionStore.hh>
#include "G4Tubs.hh"

using namespace std;
using namespace boost;

namespace Belle2 {

  /** Namespace to encapsulate code needed for simulation and reconstrucion of the CSI detector */
  namespace csi {

    // Register the creator
    /** Creator creates the CSI geometry */
    geometry::CreatorFactory<CsiCreator> CsiFactory("CSICreator");

    CsiCreator::CsiCreator(): m_sensitive(0)
    {
      m_sensitive = new SensitiveDetector();
    }

    CsiCreator::~CsiCreator()
    {
      if (m_sensitive) delete m_sensitive;
    }

    void CsiCreator::create(const GearDir& content, G4LogicalVolume& topVolume, geometry::GeometryTypes /* type */)
    {
      //lets get the stepsize parameter with a default value of 5 µm
      double stepSize = content.getLength("stepSize", 5 * Unit::um);

      //no get the array. Notice that the default framework unit is cm, so the
      //values will be automatically converted
      vector<double> bar = content.getArray("bar");
      B2INFO("Contents of bar: ");
      BOOST_FOREACH(double value, bar) {
        B2INFO("value: " << value);
      }
      int detID = 0;
      //Lets loop over all the Active nodes
      BOOST_FOREACH(const GearDir & activeParams, content.getNodes("Active")) {

        //create csi volume
        G4Trap* s_CSI = new G4Trap("s_CSI",
                                   activeParams.getLength("cDz") / 2.*Unit::mm ,
                                   activeParams.getLength("cDtheta") ,
                                   activeParams.getLength("cDphi") ,
                                   activeParams.getLength("cDy1") / 2.*Unit::mm ,
                                   activeParams.getLength("cDx2") / 2.*Unit::mm ,
                                   activeParams.getLength("cDx1") / 2.*Unit::mm , 0,
                                   activeParams.getLength("cDy2") / 2.*Unit::mm ,
                                   activeParams.getLength("cDx4") / 2.*Unit::mm ,
                                   activeParams.getLength("cDx3") / 2.*Unit::mm , 0);

        G4LogicalVolume* l_CSI = new G4LogicalVolume(s_CSI, geometry::Materials::get("CSI"), "l_CSI", 0, m_sensitive);

        //Lets limit the Geant4 stepsize inside the volume
        l_CSI->SetUserLimits(new G4UserLimits(stepSize));

        //position csi volume

        G4Transform3D theta_init = G4RotateX3D(- activeParams.getLength("cDtheta"));
        G4Transform3D phi_init = G4RotateZ3D(activeParams.getLength("k_phi_init"));
        G4Transform3D tilt_z = G4RotateY3D(activeParams.getLength("k_z_TILTED"));
        G4Transform3D tilt_phi = G4RotateZ3D(activeParams.getLength("k_phi_TILTED"));
        G4Transform3D position = G4Translate3D(activeParams.getLength("k_zC") * tan(activeParams.getLength("k_z_TILTED")) * Unit::cm, 0,
                                               activeParams.getLength("k_zC") * Unit::cm);
        G4Transform3D pos_phi = G4RotateZ3D(activeParams.getLength("k_phiC"));
        G4Transform3D Tr = pos_phi * position * tilt_phi * tilt_z * phi_init * theta_init;
        //cout << "rotation  " << Tr.getRotation() << " translation " << Tr.getTranslation() << endl;

        new G4PVPlacement(Tr, l_CSI, "p_CSI", &topVolume, false, detID);

        detID++;
      }
    }
  } // csi namespace
} // Belle2 namespace
