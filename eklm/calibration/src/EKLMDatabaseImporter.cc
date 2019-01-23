/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cmath>

/* External headers. */
#include <TFile.h>
#include <TTree.h>

/* Belle2 headers. */
#include <eklm/calibration/EKLMDatabaseImporter.h>
#include <eklm/dataobjects/ElementNumbersSingleton.h>
#include <eklm/dbobjects/EKLMDigitizationParameters.h>
#include <eklm/dbobjects/EKLMElectronicsMap.h>
#include <eklm/dbobjects/EKLMReconstructionParameters.h>
#include <eklm/dbobjects/EKLMSimulationParameters.h>
#include <eklm/geometry/AlignmentChecker.h>
#include <eklm/geometry/GeometryData.h>
#include <framework/database/IntervalOfValidity.h>
#include <framework/database/DBImportObjPtr.h>
#include <framework/database/DBObjPtr.h>
#include <framework/gearbox/GearDir.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>
#include <klm/dbobjects/KLMTimeConversion.h>

using namespace Belle2;

EKLMDatabaseImporter::EKLMDatabaseImporter() :
  m_Displacement("EKLMDisplacement")
{
  m_ExperimentLow = 0;
  m_RunLow = 0;
  m_ExperimentHigh = -1;
  m_RunHigh = -1;
}

EKLMDatabaseImporter::~EKLMDatabaseImporter()
{
}

void EKLMDatabaseImporter::setIOV(int experimentLow, int runLow,
                                  int experimentHigh, int runHigh)
{
  m_ExperimentLow = experimentLow;
  m_RunLow = runLow;
  m_ExperimentHigh = experimentHigh;
  m_RunHigh = runHigh;
}

void EKLMDatabaseImporter::importDigitizationParameters()
{
  DBImportObjPtr<EKLMDigitizationParameters> digPar;
  digPar.construct();
  GearDir dig("/Detector/DetectorComponent[@name=\"EKLM\"]/"
              "Content/DigitizationParams");
  digPar->setADCRange(dig.getInt("ADCRange"));
  digPar->setADCSamplingFrequency(dig.getDouble("ADCSamplingFrequency"));
  digPar->setNDigitizations(dig.getInt("nDigitizations"));
  digPar->setADCPedestal(dig.getDouble("ADCPedestal"));
  digPar->setADCPEAmplitude(dig.getDouble("ADCPEAmplitude"));
  digPar->setADCThreshold(dig.getInt("ADCThreshold"));
  digPar->setADCSaturation(dig.getInt("ADCSaturation"));
  digPar->setNPEperMeV(dig.getDouble("nPEperMeV"));
  digPar->setMinCosTheta(cos(dig.getDouble("MaxTotalIRAngle") / 180.0 * M_PI));
  digPar->setMirrorReflectiveIndex(dig.getDouble("MirrorReflectiveIndex"));
  digPar->setScintillatorDeExcitationTime(dig.getDouble("ScintDeExTime"));
  digPar->setFiberDeExcitationTime(dig.getDouble("FiberDeExTime"));
  digPar->setFiberLightSpeed(dig.getDouble("FiberLightSpeed"));
  digPar->setAttenuationLength(dig.getDouble("AttenuationLength"));
  digPar->setPEAttenuationFrequency(dig.getDouble("PEAttenuationFreq"));
  digPar->setMeanSiPMNoise(dig.getDouble("MeanSiPMNoise"));
  digPar->setEnableConstBkg(dig.getDouble("EnableConstBkg") > 0);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  digPar.import(iov);
}

void EKLMDatabaseImporter::importReconstructionParameters()
{
  DBImportObjPtr<EKLMReconstructionParameters> recPar;
  recPar.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/DigitizationParams");
  recPar->setTimeResolution(gd.getDouble("TimeResolution"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  recPar.import(iov);
}

void EKLMDatabaseImporter::importSimulationParameters()
{
  DBImportObjPtr<EKLMSimulationParameters> simPar;
  simPar.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/SensitiveDetector");
  simPar->setHitTimeThreshold(
    Unit::convertValue(gd.getDouble("HitTimeThreshold") , "ns"));
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  simPar.import(iov);
}

void EKLMDatabaseImporter::loadDefaultDisplacement()
{
  EKLMAlignmentData alignmentData(0., 0., 0.);
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  m_Displacement.construct();
  int iEndcap, iLayer, iSector, iPlane, iSegment, segment, sector;
  for (iEndcap = 1; iEndcap <= geoDat->getNEndcaps(); iEndcap++) {
    for (iLayer = 1; iLayer <= geoDat->getNDetectorLayers(iEndcap);
         iLayer++) {
      for (iSector = 1; iSector <= geoDat->getNSectors(); iSector++) {
        sector = geoDat->sectorNumber(iEndcap, iLayer, iSector);
        m_Displacement->setSectorAlignment(sector, &alignmentData);
        for (iPlane = 1; iPlane <= geoDat->getNPlanes(); iPlane++) {
          for (iSegment = 1; iSegment <= geoDat->getNSegments(); iSegment++) {
            segment = geoDat->segmentNumber(iEndcap, iLayer, iSector, iPlane,
                                            iSegment);
            m_Displacement->setSegmentAlignment(segment, &alignmentData);
          }
        }
      }
    }
  }
}

void EKLMDatabaseImporter::setSectorDisplacement(
  int endcap, int layer, int sector, float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData sectorAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  int sectorGlobal;
  sectorGlobal = geoDat->sectorNumber(endcap, layer, sector);
  if (!alignmentChecker.checkSectorAlignment(endcap, layer, sector,
                                             &sectorAlignment)) {
    B2ERROR("Incorrect displacement data (overlaps exist). "
            "The displacement is not changed");
    return;
  }
  m_Displacement->setSectorAlignment(sectorGlobal, &sectorAlignment);
}

void EKLMDatabaseImporter::setSegmentDisplacement(
  int endcap, int layer, int sector, int plane, int segment,
  float dx, float dy, float dalpha)
{
  const EKLM::GeometryData* geoDat = &(EKLM::GeometryData::Instance());
  EKLMAlignmentData segmentAlignment(dx, dy, dalpha);
  EKLM::AlignmentChecker alignmentChecker(false);
  const EKLMAlignmentData* sectorAlignment;
  int sectorGlobal, segmentGlobal;
  sectorGlobal = geoDat->sectorNumber(endcap, layer, sector);
  sectorAlignment = m_Displacement->getSectorAlignment(sectorGlobal);
  if (sectorAlignment == nullptr)
    B2FATAL("Incomplete alignment data.");
  segmentGlobal = geoDat->segmentNumber(endcap, layer, sector, plane, segment);
  if (!alignmentChecker.checkSegmentAlignment(endcap, layer, sector, plane,
                                              segment, sectorAlignment,
                                              &segmentAlignment, false)) {
    B2ERROR("Incorrect displacement data (overlaps exist). "
            "The displacement is not changed");
    return;
  }
  m_Displacement->setSegmentAlignment(segmentGlobal, &segmentAlignment);
}

void EKLMDatabaseImporter::importDisplacement()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_Displacement.import(iov);
}

void EKLMDatabaseImporter::loadDefaultElectronicsMap()
{
  m_ElectronicsMap.construct();
}

void EKLMDatabaseImporter::addSectorLane(
  int endcap, int layer, int sector, int copper, int dataConcentrator, int lane)
{
  m_ElectronicsMap->addSectorLane(endcap, layer, sector,
                                  copper, dataConcentrator, lane);
}

void EKLMDatabaseImporter::importElectronicsMap()
{
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  m_ElectronicsMap.import(iov);
}

void EKLMDatabaseImporter::importTimeConversion()
{
  DBImportObjPtr<KLMTimeConversion> timeConversion;
  timeConversion.construct();
  GearDir gd("/Detector/DetectorComponent[@name=\"EKLM\"]/"
             "Content/TimeConversion");
  timeConversion->setTDCFrequency(gd.getDouble("TDCFrequency"));
  timeConversion->setTimeOffset(gd.getDouble("TimeOffset"));
  timeConversion->setCTimeShift(0);
  IntervalOfValidity iov(m_ExperimentLow, m_RunLow,
                         m_ExperimentHigh, m_RunHigh);
  timeConversion.import(iov);
}

