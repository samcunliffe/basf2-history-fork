/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Yinghui GUAN                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <bklm/calibration/BKLMDatabaseImporter.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>

#include <framework/gearbox/GearDir.h>
#include <framework/logging/Logger.h>

#include <framework/database/IntervalOfValidity.h>
#include <framework/database/Database.h>
#include <framework/database/DBArray.h>

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <TClonesArray.h>

using namespace std;
using namespace Belle2;

BKLMDatabaseImporter::BKLMDatabaseImporter()
{}

void BKLMDatabaseImporter::importBklmElectronicMapping()
{
  GearDir dir("/Detector/ElectronicsMapping/BKLM");

// define data array
  TClonesArray bklmMapping("Belle2::BKLMElectronicMapping");
  int index = 0;

// loop over xml files and extract the data
  for (GearDir& copper : dir.getNodes("COPPER")) {

    int copperId = copper.getInt("@id");

    for (GearDir& slot : copper.getNodes("Slot")) {
      int slotId = slot.getInt("@id");
      //B2INFO("slotid: " << slotId << endl;);
      for (GearDir& lane : slot.getNodes("Lane")) {
        int laneId = lane.getInt("@id");
        for (GearDir& axis : lane.getNodes("Axis")) {
          int axisId = axis.getInt("@id");
          int sector = axis.getInt("Sector");
          int isForward = axis.getInt("IsForward");
          int layer = axis.getInt("Layer");
          int plane = axis.getInt("Plane");
          B2INFO("reading xml file...");
          B2INFO(" copperId: " << copperId << " slotId: " << slotId << " laneId: " << laneId << " axisId: " << axisId);
          B2INFO(" sector: " << sector << " isforward: " << isForward << " layer: " << layer << " plane: " << plane << endl);
          // save data as an element of the array
          new(bklmMapping[index]) BKLMElectronicMapping(0, copperId, slotId, laneId, axisId, isForward, sector, layer, plane);
          index++;
        }
      }
    }

  }

  IntervalOfValidity iov(0, 0, -1, -1); // IOV (0,0,-1,-1) is valid for all runs and experiments

// store under default name:
//Database::Instance().storeData(&bklmMapping, iov);

// store under user defined name:
  Database::Instance().storeData("BKLMElectronicMapping", &bklmMapping, iov);

}

void BKLMDatabaseImporter::exportBklmElectronicMapping()
{

  DBArray<BKLMElectronicMapping> elements("BKLMElectronicMapping");
  elements.getEntries();

  // Print mapping info
  B2INFO("DBArray<BKLMElectronicMapping> entries " << elements.getEntries());

  for (const auto& element : elements) {
    B2INFO("Version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " << element.getCopperId() <<
           ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
           ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
           " plane(z/phi) = " << element.getPlane());
  }
}
