/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/calibration/TOPDatabaseImporter.h>

// framework - Database
#include <framework/database/Database.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportArray.h>

// framework - xml
#include <framework/gearbox/GearDir.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DB objects
#include <top/dbobjects/TOPSampleTime.h>

#include <iostream>
#include <fstream>

using namespace std;
using namespace Belle2;

void TOPDatabaseImporter::importSampleTimeCalibration(std::string fileName)
{


  if (!m_topgp->isInitialized()) {
    B2ERROR("TOPGeometryPar was not initialized");
    return;
  }

  auto& mapper = m_topgp->getChannelMapper();
  auto syncTimeBase = m_topgp->getSyncTimeBase();

  std::ifstream stream;
  stream.open(fileName.c_str());
  if (!stream) {
    B2ERROR("openFile: " << fileName << " *** failed to open");
    return;
  }
  B2INFO(fileName << ": open for reading");

  DBImportArray<TOPSampleTime> timeBases;

  for (int moduleID = 3; moduleID < 5; moduleID++) {
    for (int boardStack = 0; boardStack < 4; boardStack++) {
      for (int carrierBoard = 0; carrierBoard < 4; carrierBoard++) {
        for (int asic = 0; asic < 4; asic++) {
          for (int chan = 0; chan < 8; chan++) {
            std::vector<double> sampleTimes;
            for (int sample = 0; sample < 256; sample++) {
              double data = 0;
              stream >> data;
              if (!stream.good()) {
                B2ERROR("Error reading sample time calibration constants");
                return;
              }
              sampleTimes.push_back(data);
            }
            auto channel = mapper.getChannel(boardStack, carrierBoard, asic, chan);
            auto* timeBase = timeBases.appendNew(moduleID, channel, syncTimeBase);
            timeBase->setTimeAxis(sampleTimes, syncTimeBase);
          }
        }
      }
    }
  }

  stream.close();
  int ndataRead = timeBases.getEntries() * 256;
  B2INFO(fileName << ": file closed, " << ndataRead << " constants read from file");

  IntervalOfValidity iov(0, 0, -1, -1); // all experiments and runs
  timeBases.import(iov);

  B2RESULT("Sample time calibration constants imported to database");

}


void TOPDatabaseImporter::printSampleTimeCalibration()
{

  DBArray<TOPSampleTime> sampleTimes;

  for (const auto& timeBase : sampleTimes) {
    cout << timeBase.getModuleID() << " " << timeBase.getChannel() << endl;
    for (const auto& time : timeBase.getTimeAxis()) {
      cout << time << " ";
    }
    cout << endl;
  }

}
