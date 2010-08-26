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
#include <svd/geosvd/B2GeomSVDLayer.h>
using namespace Belle2;
using namespace boost;
#else
#include "B2GeomSVDLayer.h"
#endif

using namespace std;

B2GeomSVDLayer::B2GeomSVDLayer()
{
  B2GeomSVDLayer(-1);
}

B2GeomSVDLayer::B2GeomSVDLayer(Int_t iLay)
{
  volSVDLayer = 0;
  iLayer = iLay;
  char text[200];
  sprintf(text, "SVD_Layer_%i", iLayer);
  path = string(text);
  printf("Layer %i created! \n", iLayer);

}

B2GeomSVDLayer::~B2GeomSVDLayer()
{

}

#ifdef B2GEOM_BASF2
Bool_t B2GeomSVDLayer::init(GearDir& content)
{
  layerContent = GearDir(content);
  layerContent.append((format("Layers/Layer[@id=\'SVD_Layer_%1%\']/") % (iLayer)).str());
  fPhi0 = layerContent.getParamAngle("Phi0");
  nLadders = int(layerContent.getParamNumValue("NumberOfLadders"));
  b2gSVDLadders.resize(nLadders);
  fPhi.resize(nLadders);
  fLadderOffsetY = layerContent.getParamLength("OffsetY");
  fRadius = layerContent.getParamLength("Radius");
  for (int iLadder = 0; iLadder < nLadders; iLadder++) {
    fPhi[iLadder] = iLadder * (360. / nLadders) + fPhi0;
  }

  b2gLadderOffsets.resize(nLadders);
  if (initOffsets()) {
    return true;
  } else {
    return false;
  }
}

Bool_t B2GeomSVDLayer::initOffsets()
{
  // create GearDir object for parameter access
  GearDir offsetsDir;

  for (Int_t iLadder = 0; iLadder < nLadders; iLadder++) {
    // go in XML file to section where the parameters for the corresponding sensor are stored
    printf("(%i / %i)\n", nLadders, iLadder);
    offsetsDir.setDirPath((format("//LadderOffset[@id=\'SVD_Offsets_Layer_%1%_Ladder_%2%\']/") % iLayer % iLadder).str());
    // init parameters
    b2gLadderOffsets[iLadder] = new B2GeomOffset();
    b2gLadderOffsets[iLadder]->init(offsetsDir);
  }
  return true;
}
#else
Bool_t B2GeomSVDLayer::init()
{

}
#endif

Bool_t B2GeomSVDLayer::make()
{
  printf("make B2GeomSVDLayer %i\n", iLayer);
  volSVDLayer = new TGeoVolumeAssembly(path.c_str());
  for (int iLadder = 0; iLadder < nLadders; ++iLadder) {
    putLadder(iLadder);
  }
  return true;
}

void B2GeomSVDLayer::putLadder(Int_t iLadder)
{
  // position the ladder at radius
  TGeoTranslation traX = TGeoTranslation("name", fRadius, 0., 0.);
  // create the wind mill structure
  TGeoTranslation traY = TGeoTranslation("name", 0., fLadderOffsetY, 0.);
  // shift the ladder in z diretion
  TGeoTranslation traZ = TGeoTranslation("name", 0., 0., 0);
  // rotate around z
  TGeoRotation rotPhi = TGeoRotation("name", fPhi[iLadder], 0, 0);

  // compose the transformation to position the ladder
  TGeoHMatrix hmaHelp;
  hmaHelp = gGeoIdentity;
  hmaHelp = traX * hmaHelp;
  hmaHelp = traY * hmaHelp;
  hmaHelp = traZ * hmaHelp;
  hmaHelp = rotPhi * hmaHelp;

  // this transformation is finally used for positioning of the ladder
  TGeoHMatrix* hmaLadderPosition = new TGeoHMatrix(hmaHelp);
  // create Ladder
  b2gSVDLadders[iLadder] = new B2GeomSVDLadder(iLayer, iLadder);

#ifdef B2GEOM_BASF2
  b2gSVDLadders[iLadder]->init(layerContent);
#else
  b2gSVDLadders[iLadder]->init();
#endif
  b2gSVDLadders[iLadder]->make();

  volSVDLayer->AddNode(b2gSVDLadders[iLadder]->getVol(), 1, hmaLadderPosition);

}



