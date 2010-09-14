/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Oswald, Andreas Moll, Zbynek Drasal            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#define B2GEOM_BASF2

#ifdef B2GEOM_BASF2
#include <pxd/geopxd/B2GeomPXDLadder.h>
using namespace Belle2;
using namespace boost;
#else
#include "B2GeomPXDLadder.h"
#endif
using namespace std;

B2GeomPXDLadder::B2GeomPXDLadder()
{
  B2GeomPXDLadder(-1, -1);
}

B2GeomPXDLadder::B2GeomPXDLadder(Int_t iLay, Int_t iLad)
{
  iLayer = iLay;
  iLadder = iLad;
  char text[200];
  sprintf(text, "PXD_Layer_%i_Ladder_%i", iLayer, iLadder);
  path = string(text);
}

B2GeomPXDLadder::~B2GeomPXDLadder()
{
}

#ifdef B2GEOM_BASF2
Bool_t B2GeomPXDLadder::init(GearDir& content)
{
  ladderContent = GearDir(content);
  ladderContent.append("Ladder/");
  nSensors = int(ladderContent.getParamNumValue("NumberOfSensors"));
  b2gPXDSensors = new B2GeomPXDSensor*[nSensors];
  fVPosition.resize(nSensors);
  for (Int_t iSensor = 0; iSensor < nSensors; iSensor++) {
    GearDir sensorPositionContent(ladderContent);
    sensorPositionContent.append((format("Sensors/Sensor[@id=\'PXD_Layer_%1%_Ladder_Sensor_%2%\']/Position/") % iLayer % iSensor).str());
    fVPosition[iSensor] = sensorPositionContent.getParamLength("V");
  }

  TGeoMaterial* matVacuum = new TGeoMaterial("Vacuum", 0, 0, 0);
  medAir = new TGeoMedium("medAir", 1, matVacuum);
  return true;
}
#else
Bool_t B2GeomPXDLadder::init()
{

}
#endif

Bool_t B2GeomPXDLadder::make()
{
  volPXDLadder = new TGeoVolumeAssembly(path.c_str());
  putSensors();
  return true;
}

void B2GeomPXDLadder::putSensors()
{

  for (int iSensor = 0; iSensor < nSensors; ++iSensor) {
    b2gPXDSensors[iSensor] = new B2GeomPXDSensor(iLayer, iLadder, iSensor);
#ifdef B2GEOM_BASF2
    b2gPXDSensors[iSensor]->init(ladderContent);
#else
    b2gPXDSensors[iSensor]->init();
#endif
    b2gPXDSensors[iSensor]->make();
    TGeoTranslation tra(0.0, 0.0, fVPosition[iSensor] + 0.5 * b2gPXDSensors[iSensor]->getLengthSilicon());
    TGeoHMatrix hmaHelp;
    hmaHelp = gGeoIdentity;
    hmaHelp = b2gPXDSensors[iSensor]->getSurfaceCenterPosition() * hmaHelp;
    hmaHelp = tra * hmaHelp;

    volPXDLadder->AddNode(b2gPXDSensors[iSensor]->getVol(), 1, new TGeoHMatrix(hmaHelp));
  }
}


