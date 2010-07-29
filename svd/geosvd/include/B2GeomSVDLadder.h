/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifdef B2GEOM_BASF2
#include <svd/geosvd/B2GeomSVDSensor.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#else
#include "B2GeomSVDSensor.h"
#endif

#include "TGeoMaterial.h"
#include "TGeoMedium.h"
#include "TGeoManager.h"
#include "TGeoMatrix.h"
#include "TGeoVolume.h"
#include "TMath.h"
#include <vector>
#include <string>


#ifndef B2GEOMSVDLADDER_H_
#define B2GEOMSVDLADDER_H_

using namespace std;

#ifdef B2GEOM_BASF2
namespace Belle2 {
  class GearDir;
#endif

  class B2GeomSVDLadder {
  private:

    //! path of this Ladder
    string path;
#ifdef B2GEOM_BASF2
    GearDir ladderContent;
#endif

    //! TGeoVolumeAssembly which contains all parts of this sensor
    TGeoVolumeAssembly* volSVDLadder;

    //! Volumes contained in the sensor
    vector<B2GeomSVDSensor*> b2gSVDSensors;

    // Parameters
    //! layer number
    Int_t iLayer;
    //! ladder number
    Int_t iLadder;
    //! number of barrel sensors
    Int_t nSensors;
    //! number of slanted sensors
    Int_t nSlantedSensors;

    //! Gap between two sensor boxes
    Double_t fGapLength;
    //! Angle of slanted sensor
    Double_t fTheta;

    //! Mediums contained in the sensor
    TGeoMedium* medAir;
    TGeoMedium* medGlue;


    //! Methods to place components
    void putSensors();
    void putGlue();

  public:
    B2GeomSVDLadder();
    B2GeomSVDLadder(Int_t iLayer , Int_t iLadder);
    ~B2GeomSVDLadder();
#ifdef B2GEOM_BASF2
    Bool_t init(GearDir& content);
#else
    Bool_t init();
#endif
    Bool_t make();
    TGeoVolumeAssembly* getVol() {
      return volSVDLadder;
    }
  };
#ifdef B2GEOM_BASF2
}
#endif
#endif
