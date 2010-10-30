/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/geosvd/B2GeomSVDSensor.h>
#include <pxd/geopxd/B2GeomVolume.h>
#include <framework/gearbox/GearDir.h>
#include <framework/datastore/Units.h>
#include <boost/format.hpp>
#include "TROOT.h"
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
namespace Belle2 {
  class GearDir;

// ------------------------------------------------------------------------------------------------
// Classes describing the ribs of SVD
// ------------------------------------------------------------------------------------------------

  class B2GeomSVDLadderRibFoam : public B2GeomVolume {
  protected:
    Int_t iLayer;
  public:
    B2GeomSVDLadderRibFoam(Int_t iLay);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomSVDLadderRib : public B2GeomVolume {
  protected:
    Int_t iLayer;
  public:
    B2GeomSVDLadderRib(Int_t iLay);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

  class B2GeomSVDLadderRibs : public B2GeomVolume {
  protected:
    B2GeomSVDLadderRib* volRib;
    Int_t iLayer;
    Float_t fRib1UPosition;
    Float_t fRib2UPosition;
  public:
    B2GeomSVDLadderRibs(Int_t iLay);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

// ------------------------------------------------------------------------------------------------
// Classes describing the cooling pipe of SVD
// ------------------------------------------------------------------------------------------------

  class B2GeomSVDLadderCoolingpipe : public B2GeomVolume {
  protected:
    Int_t iLayer;
  public:
    B2GeomSVDLadderCoolingpipe(Int_t iLay);
    Bool_t init(GearDir& content);
    Bool_t make();
  };

// ------------------------------------------------------------------------------------------------
// Classes describing the SVD ladder itself
// ------------------------------------------------------------------------------------------------

  class B2GeomSVDLadder : public B2GeomVolume {
  private:

    //! path of this Ladder
    string path;
    GearDir ladderContent;

    //! The sensor objects
    B2GeomSVDSensor** b2gSVDSensors;

    // Parameters
    //! layer number
    Int_t iLayer;
    //! ladder number
    Int_t iLadder;
    //! number of barrel sensors
    Int_t nSensors;

    B2GeomSVDLadderRibs* volBarrelRibs;
    B2GeomSVDLadderRibs* volSlantedRibs;
    B2GeomSVDLadderCoolingpipe* volCoolingpipe;

  public:
    B2GeomSVDLadder();
    B2GeomSVDLadder(Int_t iLayer, Int_t iLadder);
    ~B2GeomSVDLadder();
    Bool_t init(GearDir& content);
    Bool_t make();
  };
}
#endif

