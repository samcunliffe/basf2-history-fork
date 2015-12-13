/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* System headers. */
#include <fcntl.h>
#include <unistd.h>

/* External headers. */
#include <TFile.h>
#include <TTree.h>
#include <TH1F.h>
#include <TF1.h>
#include <TCanvas.h>

/* Belle2 headers. */
#include <eklm/dataobjects/EKLMHit2d.h>
#include <eklm/modules/EKLMTimeCalibration/EKLMTimeCalibrationCollectorModule.h>
#include <framework/core/ModuleManager.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Unit.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/ExtHit.h>

using namespace Belle2;

REG_MODULE(EKLMTimeCalibrationCollector)

EKLMTimeCalibrationCollectorModule::EKLMTimeCalibrationCollectorModule() :
  CalibrationCollectorModule()
{
  setDescription("Module for EKLM time calibration (data collection).");
  setPropertyFlags(c_ParallelProcessingCertified);
  m_nStripDifferent = -1;
  m_ev = {0, 0};
  m_TransformData = NULL;
  m_GeoDat = NULL;
}

EKLMTimeCalibrationCollectorModule::~EKLMTimeCalibrationCollectorModule()
{
  if (m_TransformData != NULL)
    delete m_TransformData;
}

void EKLMTimeCalibrationCollectorModule::prepare()
{
  int i;
  char str[128];
  TTree* t;
  m_GeoDat = &(EKLM::GeometryData::Instance());
  m_nStripDifferent = m_GeoDat->getNStripsDifferentLength();
  StoreArray<EKLMHit2d>::required();
  StoreArray<EKLMDigit>::required();
  StoreArray<Track>::required();
  StoreArray<ExtHit>::required();
  m_TransformData = new EKLM::TransformData(true, NULL);
  for (i = 0; i < m_nStripDifferent; i++) {
    snprintf(str, 128, "t%d", i);
    t = new TTree(str, "");
    t->Branch("time", &m_ev.time, "time/F");
    t->Branch("dist", &m_ev.dist, "dist/F");
    registerObject<TTree>(str, t);
  }
}

void EKLMTimeCalibrationCollectorModule::collect()
{
  int i, j, k, n, n2, vol;
  double l, hitTime;
  char str[128];
  TVector3 hitPosition;
  HepGeom::Point3D<double> hitGlobal, hitLocal;
  StoreArray<Track> tracks;
  StoreArray<EKLMHit2d> hit2ds;
  std::multimap<int, ExtHit*> mapExtHit;
  std::multimap<int, ExtHit*>::iterator it, itLower, itUpper;
  ExtHit* extHit, *entryHit[2], *exitHit[2];
  const HepGeom::Transform3D* tr;
  n = tracks.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<ExtHit> extHits = tracks[i]->getRelationsTo<ExtHit>();
    n2 = extHits.size();
    for (j = 0; j < n2; j++) {
      if (extHits[j]->getDetectorID() != Const::EDetector::KLM)
        continue;
      if (!m_GeoDat->hitInEKLM(extHits[j]->getPosition().Z()))
        continue;
      mapExtHit.insert(std::pair<int, ExtHit*>(extHits[j]->getCopyID(),
                                               extHits[j]));
    }
  }
  n = hit2ds.getEntries();
  for (i = 0; i < n; i++) {
    RelationVector<EKLMDigit> digits = hit2ds[i]->getRelationsTo<EKLMDigit>();
    if (digits.size() != 2)
      B2FATAL("Wrong number of related EKLMDigits.");
    for (j = 0; j < 2; j++) {
      entryHit[j] = NULL;
      exitHit[j] = NULL;
      vol = digits[j]->getVolumeID();
      itLower = mapExtHit.lower_bound(vol);
      itUpper = mapExtHit.upper_bound(vol);
      for (it = itLower; it != itUpper; ++it) {
        extHit = it->second;
        switch (extHit->getStatus()) {
          case EXT_ENTER:
            if (entryHit[j] == NULL) {
              entryHit[j] = extHit;
            } else {
              if (extHit->getTOF() < entryHit[j]->getTOF())
                entryHit[j] = extHit;
            }
            break;
          case EXT_EXIT:
            if (exitHit[j] == NULL) {
              exitHit[j] = extHit;
            } else {
              if (extHit->getTOF() > exitHit[j]->getTOF())
                exitHit[j] = extHit;
            }
            break;
          default:
            break;
        }
      }
    }
    if (entryHit[0] == NULL || exitHit[0] == NULL ||
        entryHit[1] == NULL || exitHit[1] == NULL)
      continue;
    for (j = 0; j < 2; j++) {
      hitTime = 0.5 * (entryHit[j]->getTOF() + exitHit[j]->getTOF());
      hitPosition = 0.5 * (entryHit[j]->getPosition() +
                           exitHit[j]->getPosition());
      l = m_GeoDat->getStripLength(digits[j]->getStrip()) / CLHEP::mm *
          Unit::mm;
      hitGlobal.setX(hitPosition.X() / Unit::mm * CLHEP::mm);
      hitGlobal.setY(hitPosition.Y() / Unit::mm * CLHEP::mm);
      hitGlobal.setZ(hitPosition.Z() / Unit::mm * CLHEP::mm);
      tr = m_TransformData->getStripGlobalToLocal(digits[j]);
      hitLocal = (*tr) * hitGlobal;
      m_ev.time = digits[j]->getTime() - hitTime;
      m_ev.dist = 0.5 * l - hitLocal.x() / CLHEP::mm * Unit::mm;
      k = m_GeoDat->getStripLengthIndex(digits[j]->getStrip() - 1);
      snprintf(str, 128, "t%d", k);
      getObject<TTree>(str).Fill();
    }
  }
}

void EKLMTimeCalibrationCollectorModule::terminate()
{
}

