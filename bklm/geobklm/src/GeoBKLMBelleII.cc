/*************************************************************************
*  BASF2 (Belle Analysis Framework 2)                                    *
*  Copyright(C) 2010 - Belle II Collaboration                            *
*                                                                        *
*  Author: The Belle II Collaboration                                    *
*  Contributors: Leo Piilonen                                            *
*                                                                        *
*  This software is provided "as is" without any warranty.               *
* ***********************************************************************/
#include <bklm/geobklm/GeoBKLMBelleII.h>
#include <bklm/geobklm/BKLMGeometryPar.h>
#include <bklm/simbklm/BKLMSensitiveDetector.h>

#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>

#include <cmath>

#include <TMath.h>
#include <TVector3.h>
#include <TGeoMatrix.h>
#include <TGeoManager.h>
#include <TGeoVolume.h>
#include <TGeoBBox.h>
#include <TGeoPcon.h>
#include <TGeoPgon.h>
#include "TGeoCompositeShape.h"
#include "TGeoTube.h"

#include <iostream>

using namespace std;
using namespace boost;
using namespace Belle2;
//-----------------------------------------------------------------
//                 Register the Creator
//-----------------------------------------------------------------

GeoBKLMBelleII regGeoBKLMBelleII;

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
GeoBKLMBelleII::GeoBKLMBelleII() : CreatorBase("BKLMBelleII")
{
  setDescription("Creates the TGeo objects for the BKLM geometry of the Belle II detector.");

  //We use the "BKLM_" prefix to flag our sensitive volumes
  addSensitiveDetector("BKLM_", new BKLMSensitiveDetector("BKLMSensitiveDetector"));
}

GeoBKLMBelleII::~GeoBKLMBelleII()
{
}

void GeoBKLMBelleII::create(GearDir& content)
{

  //----------------------------------------
  //         Get Materials
  //----------------------------------------

  TGeoMedium* medIron     = gGeoManager->GetMedium("Iron");
  TGeoMedium* medAir      = gGeoManager->GetMedium("Air");
  TGeoMedium* medAluminum = gGeoManager->GetMedium("Aluminum");
  TGeoMedium* medCopper   = gGeoManager->GetMedium("Copper");
  TGeoMedium* medReadout  = gGeoManager->GetMedium("RPCReadout");
  TGeoMedium* medGlass    = gGeoManager->GetMedium("RPCGlass");
  TGeoMedium* medGas      = gGeoManager->GetMedium("RPCGas");

  //----------------------------------------
  //           Build BKLM
  //----------------------------------------

  BKLMGeometryPar* geoPar = BKLMGeometryPar::Instance();

  TGeoTranslation* noTranslation = new TGeoTranslation(0.0, 0.0, 0.0);
  TGeoRotation* globalRot = new TGeoRotation("BKLMGlobalRotation", 0.0, geoPar->rotation(), 0.0);
  TGeoVolumeAssembly* volGrpBKLM = addSubdetectorGroup("BKLM", new TGeoCombiTrans(0.0, 0.0, geoPar->offsetZ(), globalRot));

  double sectorDphi = 360.0 / geoPar->nsides();
  TVector3 size = geoPar->gapSize(0, false);
  double gapLength = size.Z() * 2.0;
  double ribShift0 = (geoPar->thickness_rib() / 2.0) / sin(M_PI / geoPar->nsides());
  TGeoTranslation* ribShift        = new TGeoTranslation("ribShift", -ribShift0, 0.0, 0.0);
  TGeoTranslation* ribShiftInverse = new TGeoTranslation(ribShift0, 0.0, 0.0);
  ribShift->RegisterYourself();
  double dx = geoPar->height_glass() + geoPar->height_gas() / 2.0;
  TGeoTranslation* innerGasTranslation = new TGeoTranslation(-dx, 0.0, 0.0);
  TGeoTranslation* outerGasTranslation = new TGeoTranslation(+dx, 0.0, 0.0);
  TGeoRotation* chimneyRot = new TGeoRotation();
  chimneyRot->RotateY(90.0);
  TGeoCombiTrans* chimneyTransform = new TGeoCombiTrans(0.0, 0.0, 0.0, chimneyRot);

  TGeoVolume* fbVol;
  TGeoCompositeShape* fbShape;
  TGeoVolume* sectorVol;
  TGeoCompositeShape* sectorShape;
  TGeoVolume* layerVol;
  TGeoPgon*   layerShape;
  TGeoVolume* gapVol;
  TGeoVolume* chimneyGapVol;
  TGeoVolume* chimneyVol;
  TGeoVolume* chimneyHousingVol;
  TGeoVolume* chimneyShieldVol;
  TGeoVolume* chimneyPipeVol;
  TGeoVolume* moduleVol;
  TGeoVolume* readoutVol;
  TGeoVolume* electrodeVol;
  TGeoVolume* gasVol;
  TGeoVolume* capVol;
  TGeoCompositeShape* capShape;
  TGeoVolume* kiteVol;
  TGeoVolume* cableVol;
  TGeoVolume* braceVol;
  TGeoCompositeShape* voidShape;
  TGeoVolume* voidVol;
  TGeoVolume* supportPlateVol;
  TGeoPgon*   temp;
  char name[4];
  char gasName[30];
  TGeoTube* solenoidCutoutShape = new TGeoTube(0.0, geoPar->rsolenoid(), geoPar->length() * 1.01);
  solenoidCutoutShape->SetName("solenoidCutout");
  for (int fb = 0; fb < 2; fb++) {
    temp = new TGeoPgon("BKLMpgon", geoPar->phi(), 360.0, geoPar->nsides(), 2);
    temp->DefineSection(0, 0.0,              0.0, geoPar->rmax());
    temp->DefineSection(1, geoPar->length(), 0.0, geoPar->rmax());
    fbShape = new TGeoCompositeShape((fb == 0 ? "BKLM_F" : "BKLM_B"), "BKLMpgon-solenoidCutout");
    fbVol = new TGeoVolume((fb == 0 ? "BKLM_F" : "BKLM_B"), fbShape, medAir);
    fbVol->SetAttVisibility(true);
    fbVol->SetLineColor(kBlue);
    TGeoRotation* fbRot = new TGeoRotation("BKLMfbRotation", 0.0, 0.0, 0.0);
    if (fb == 1) { fbRot->ReflectZ(true); }
    volGrpBKLM->AddNode(fbVol, fb, new TGeoCombiTrans(0.0, 0.0, 0.0, fbRot));
    for (int sect = 0; sect < geoPar->nsides(); sect++) {
      bool hasChimney = (fb == 1) && (sect == 2);
      sprintf(name, "S%d", sect);
      TGeoPgon* temp = new TGeoPgon("Sectorpgon", geoPar->phi(), sectorDphi, 1, 2);
      temp->DefineSection(0, 0.0,              0.0, geoPar->rmax());
      temp->DefineSection(1, geoPar->length(), 0.0, geoPar->rmax());
      sectorShape = new TGeoCompositeShape("Sector", "Sectorpgon-solenoidCutout");
      sectorVol = new TGeoVolume(name, sectorShape, medIron);
      sectorVol->SetAttVisibility(true);
      sectorVol->SetLineColor(kBlue - 9);
      TGeoRotation* sectorRot = new TGeoRotation("BKLMSectorRotation", 0.0, 0.0, sectorDphi * sect);
      fbVol->AddNode(sectorVol, sect, new TGeoCombiTrans(0.0, 0.0, 0.0, sectorRot));
      // Install layers in the sector
      for (int lyr = 0; lyr < geoPar->nlayer() ; lyr++) {
        sprintf(name, "L%02d", lyr);
        layerVol = gGeoManager->MakePgon(name, medIron, geoPar->phi(), sectorDphi, 1, 2);
        layerShape = (TGeoPgon*)layerVol->GetShape();
        layerShape->DefineSection(0, 0.0,       geoPar->layerRmin(lyr), geoPar->layerRmax(lyr));
        layerShape->DefineSection(1, gapLength, geoPar->layerRmin(lyr), geoPar->layerRmax(lyr));
        layerVol->SetAttVisibility(true);
        layerVol->SetLineColor(kGreen);
        sectorVol->AddNode(layerVol, lyr, noTranslation);
        size = geoPar->gapSize(lyr, hasChimney);
        double gapZ = size.Z();
        sprintf(name, "G%02d", lyr);
        gapVol = gGeoManager->MakeBox(name, medAir, size.X(), size.Y(), size.Z());
        gapVol->SetAttVisibility(true);
        gapVol->SetLineColor(kRed);
        layerVol->AddNode(gapVol, 0, new TGeoTranslation(geoPar->gapR(lyr), 0.0, size.Z()));
        if (hasChimney) {
          TVector3 sizeChimney = geoPar->size_chimney(lyr);
          TVector3 posChimney = geoPar->pos_chimney(lyr);
          size.SetY((size.Y() - sizeChimney.Y()) / 2.0);
          size.SetZ(sizeChimney.Z());
          sprintf(name, "H%02d", lyr);
          chimneyGapVol = gGeoManager->MakeBox(name, medAir, size.X(), size.Y(), size.Z());
          chimneyGapVol->SetAttVisibility(true);
          chimneyGapVol->SetLineColor(kGreen);
          layerVol->AddNode(chimneyGapVol, 1, new TGeoTranslation(geoPar->gapR(lyr), +(size.Y() + sizeChimney.Y()), gapLength - size.Z()));
          layerVol->AddNode(chimneyGapVol, 2, new TGeoTranslation(geoPar->gapR(lyr), -(size.Y() + sizeChimney.Y()), gapLength - size.Z()));
          sizeChimney.SetZ(sizeChimney.Z() - geoPar->cover_chimney() / 2.0);
          posChimney.SetZ(posChimney.Z() - geoPar->cover_chimney() / 2.0);
          sprintf(name, "C%02d", lyr);
          chimneyVol = gGeoManager->MakeBox(name, medAir, sizeChimney.X(), sizeChimney.Y(), sizeChimney.Z());
          chimneyVol->SetAttVisibility(true);
          chimneyVol->SetLineColor(kBlue);
          layerVol->AddNode(chimneyVol, 3, new TGeoTranslation(posChimney.X(), posChimney.Y(), posChimney.Z()));
          chimneyHousingVol = gGeoManager->MakeTube("chimneyHousing", medIron, geoPar->chimney_housing_rmin(), geoPar->chimney_housing_rmax(), sizeChimney.X());
          chimneyVol->AddNode(chimneyHousingVol, 0, chimneyTransform);
          chimneyShieldVol = gGeoManager->MakeTube("chimneyShield", medCopper, geoPar->chimney_shield_rmin(), geoPar->chimney_shield_rmax(), sizeChimney.X());
          chimneyVol->AddNode(chimneyShieldVol, 1, chimneyTransform);
          chimneyPipeVol = gGeoManager->MakeTube("chimneyPipe", medAluminum, geoPar->chimney_pipe_rmin(), geoPar->chimney_pipe_rmax(), sizeChimney.X());
          chimneyVol->AddNode(chimneyPipeVol, 2, chimneyTransform);
        }
        sprintf(name,  "M%02d", lyr);
        size = geoPar->moduleSize(lyr, hasChimney);
        moduleVol = gGeoManager->MakeBox(name, medAluminum, size.X(), size.Y(), size.Z());
        moduleVol->SetAttVisibility(true);
        moduleVol->SetLineColor(kBlack);
        gapVol->AddNode(moduleVol, 0, new TGeoTranslation(0.0, 0.0, size.Z() - gapZ));
        sprintf(name,  "R%02d", lyr);
        size = geoPar->readoutSize(lyr, hasChimney);
        readoutVol = gGeoManager->MakeBox(name, medReadout, size.X(), size.Y(), size.Z());
        readoutVol->SetAttVisibility(true);
        readoutVol->SetLineColor(kBlack);
        moduleVol->AddNode(readoutVol, 0, noTranslation);
        sprintf(name,  "E%02d", lyr);
        size = geoPar->electrodeSize(lyr, hasChimney);
        electrodeVol = gGeoManager->MakeBox(name, medGlass, size.X(), size.Y(), size.Z());
        electrodeVol->SetAttVisibility(true);
        electrodeVol->SetLineColor(kBlack);
        readoutVol->AddNode(electrodeVol, 0, noTranslation);
        sprintf(gasName,  "BKLM_barrel_gas_%d_%d_%02d_0", fb, sect, lyr);
        size = geoPar->gasSize(lyr, hasChimney);
        gasVol = gGeoManager->MakeBox(gasName, medGas, size.X(), size.Y(), size.Z());
        gasVol->SetAttVisibility(true);
        gasVol->SetLineColor(kBlack);
        electrodeVol->AddNode(gasVol, 0, innerGasTranslation);
        sprintf(gasName,  "BKLM_barrel_gas_%d_%d_%02d_1", fb, sect, lyr);
        electrodeVol->AddNode(gasVol, 1, outerGasTranslation);
      }
      // Install cap at the +Z end of the sector - this contains the support kites/bar and "cables"
      sprintf(name, "C%d", sect);
      temp = new TGeoPgon("Cappgon", geoPar->phi(), sectorDphi, 1, 2);
      temp->DefineSection(0, gapLength,        0.0, geoPar->rmax());
      temp->DefineSection(1, geoPar->length(), 0.0, geoPar->rmax());
      capShape = new TGeoCompositeShape("Cap", "Cappgon-solenoidCutout");
      capVol = new TGeoVolume(name, capShape, medAir);
      sectorVol->AddNode(capVol, geoPar->nlayer(), noTranslation);
      size = geoPar->gapSize(0, false);
      double rminKite = geoPar->layerRmin(1) + size.X() * 2.0;
      kiteVol = gGeoManager->MakePgon("barrelkite", medIron, geoPar->phi(), sectorDphi, 1, 2);
      TGeoPgon* kiteShape = (TGeoPgon*)kiteVol->GetShape();
      kiteShape->DefineSection(0, gapLength,        geoPar->layerRmin(1) + size.X() * 2.0, geoPar->rmax());
      kiteShape->DefineSection(1, geoPar->length(), geoPar->layerRmin(1) + size.X() * 2.0, geoPar->rmax());
      kiteVol->SetAttVisibility(true);
      kiteVol->SetLineColor(kGreen);
      capVol->AddNode(kiteVol, 0, noTranslation);
      size.SetX((geoPar->rmax() - rminKite) / 2.0);
      size.SetY(geoPar->width_cables() / 2.0);
      size.SetZ((geoPar->length() - gapLength) / 2.0);
      cableVol = gGeoManager->MakeBox("barrelcables", medAluminum, size.X(), size.Y(), size.Z());
      cableVol->SetAttVisibility(true);
      cableVol->SetLineColor(kRed);
      TGeoTranslation* cableTranslate = new TGeoTranslation((geoPar->rmax() + rminKite) / 2.0, 0.0, (gapLength + geoPar->length()) / 2.0);
      kiteVol->AddNode(cableVol, 0, cableTranslate);
      size.SetY((hasChimney ? geoPar->width_brace_chimney() : geoPar->width_brace()) / 2.0);
      braceVol = gGeoManager->MakeBox("barrelbrace", medIron, size.X(), size.Y(), size.Z());
      braceVol->SetAttVisibility(true);
      braceVol->SetLineColor(kBlue);
      cableVol->AddNode(braceVol, 0, noTranslation);
      // Install inner void in the sector - this contains the support structure for innermost detector
      // "ribShift" leaves radial solenoid-support iron ribs at low- and high-phi sides of the sector
      sprintf(name, "V%d", sect);
      temp = new TGeoPgon("Voidpgon", geoPar->phi(), sectorDphi, 1, 2);
      temp->DefineSection(0, 0.0,       0.0, geoPar->layerRmin(0) - ribShift0);
      temp->DefineSection(1, gapLength, 0.0, geoPar->layerRmin(0) - ribShift0);
      voidShape = new TGeoCompositeShape("Void", "Voidpgon-solenoidCutout:ribShift");
      voidVol = new TGeoVolume(name, voidShape, medAir);
      sectorVol->AddNode(voidVol, geoPar->nlayer() + 1, ribShiftInverse);
      if (sect <= geoPar->nsides() / 2) {
        size = geoPar->size_support_plate(hasChimney);
        supportPlateVol = gGeoManager->MakeBox("supportplate", medAluminum, size.X(), size.Y(), size.Z());
        voidVol->AddNode(supportPlateVol, 0, new TGeoTranslation(geoPar->layerRmin(0) - size.X() - ribShift0, 0.0, size.Z()));
        // *DIVOT* missing the iron brackets that hold this support plate
      }
    }
  }

/////////////////////////////////////////////////////////////////////////////////////////////

}
