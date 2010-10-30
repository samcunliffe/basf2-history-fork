/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#include <pxd/geopxd/B2GeomVolume.h>
#include <string>
#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TROOT.h"

#ifndef B2GEOMSVDSENSOR_H_
#define B2GEOMSVDSENSOR_H_

using namespace std;

namespace Belle2 {

  class GearDir;

  class B2GeomSVDSensorActive : public B2GeomVolume {
  private:
    string path;
    //! Dimensions of the active part of the sensor
    Int_t iLayer;
    Int_t iLadder;
    Int_t iSensor;
  public:
    B2GeomSVDSensorActive(Int_t iLay, Int_t iLad, Int_t iSen);
    Bool_t init(GearDir& content);
    Bool_t make();
  };


  class B2GeomSVDSensorSilicon : public B2GeomVolume {
  private:
    string path;
    Int_t iLayer;
    Int_t iLadder;
    Int_t iSensor;
    B2GeomSVDSensorActive* volActive;
  public:
    B2GeomSVDSensorSilicon(Int_t iLay, Int_t iLad, Int_t iSen);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomSVDSensorFoam : public B2GeomVolume {
  private:
  public:
    B2GeomSVDSensorFoam();
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomSVDSensorKapton : public B2GeomVolume {
  private:
  public:
    B2GeomSVDSensorKapton();
    Bool_t init(GearDir& content);
    Bool_t make();
  };


  class B2GeomSVDSensorSmds : public B2GeomVolume {
  private:
  public:
    B2GeomSVDSensorSmds();
    Bool_t init(GearDir& content);
    Bool_t make();
  };


  class B2GeomSVDSensor : public B2GeomVolume {
  private:

    //! path of this Sensor
    string path;

    //! Medium of the sensor container
    TGeoMedium* medAir;

    // Parameters
    //! Layer number of this sensor
    Int_t iLayer;
    //! Ladder number of this sensor
    Int_t iLadder;
    //! Number of this sensor
    Int_t iSensor;
    //! Sensor type (0 = small, 1 = normal, 2 = wedge, 9x = active sensor only)
    Int_t iSensorType;

    //! parameters for sensor components
    B2GeomSVDSensorSilicon* volSilicon;
    B2GeomSVDSensorFoam* volFoam;
    B2GeomSVDSensorSmds* volSMDs;
    B2GeomSVDSensorKapton* volKapton;

  public:
    B2GeomSVDSensor();
    B2GeomSVDSensor(Int_t iLay, Int_t iLad, Int_t iSen);
    ~B2GeomSVDSensor();
    Bool_t init(GearDir& content);
    Bool_t make();
  };
}
#endif
