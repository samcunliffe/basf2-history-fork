/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: tadeas                                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <alignment/modules/MillepedeCollector/MillepedeCollectorModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/pcore/ProcHandler.h>
#include <framework/core/FileCatalog.h>

#include <alignment/dataobjects/MilleData.h>

#include <genfit/Track.h>
#include <genfit/GblFitter.h>

#include <analysis/dataobjects/ParticleList.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>
#include <framework/database/DBObjPtr.h>
#include <framework/dbobjects/BeamParameters.h>
#include <mdst/dataobjects/Track.h>
#include <genfit/Track.h>

#include <alignment/GlobalLabel.h>
#include <framework/core/FileCatalog.h>
#include <framework/dataobjects/FileMetaData.h>

#include <TMath.h>
#include <TH1F.h>

#include <genfit/FullMeasurement.h>
#include <tracking/trackFitting/fitter/base/TrackFitter.h>
#include <tracking/trackFitting/measurementCreator/adder/MeasurementAdder.h>

#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>
#include <alignment/reconstruction/AlignableSVDRecoHit.h>

#include <genfit/PlanarMeasurement.h>

#include <alignment/reconstruction/AlignableCDCRecoHit.h>
#include <alignment/reconstruction/AlignablePXDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit.h>
#include <alignment/reconstruction/AlignableSVDRecoHit2D.h>
#include <alignment/reconstruction/BKLMRecoHit.h>
#include <alignment/reconstruction/AlignableEKLMRecoHit.h>

#include <alignment/Hierarchy.h>

#include <alignment/dbobjects/VXDAlignment.h>

using namespace Belle2;
using namespace std;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(MillepedeCollector)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

MillepedeCollectorModule::MillepedeCollectorModule() : CalibrationCollectorModule()
{
  setPropertyFlags(c_ParallelProcessingCertified);
  setDescription("Calibration data collector for Millepede Algorithm");

  addParam("tracks", m_tracks, "Names of collections of genfit::Tracks for calibration", vector<string>({""}));
  addParam("particles", m_particles, "Names of particle list of single particles", vector<string>());
  addParam("vertices", m_vertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex constraint", vector<string>());
  addParam("primaryVertices", m_primaryVertices,
           "Name of particle list of (mother) particles with daughters for calibration using vertex + IP profile constraint",
           vector<string>());
  addParam("doublePrecision", m_doublePrecision, "Use double (=true) or single/float (=false) precision for writing binary files",
           bool(false));
  addParam("calibrateVertex", m_calibrateVertex, "For primary vertices, beam spot calibration derivatives are added",
           bool(false));
  addParam("minPValue", m_minPValue, "Minimum p-value to write out a trejectory, <0 to write out all",
           double(-1.));
  addParam("useGblTree", m_useGblTree, "Store GBL trajectories in a tree instead of output to binary files",
           bool(true));
  addParam("useVXDHierarchy", m_useVXDHierarchy, "Use hierarchy for VXD alignment (adds alignment of half-shells, ladders)",
           bool(false));
}

void MillepedeCollectorModule::prepare()
{
  StoreObjPtr<EventMetaData>::required();

  if (m_tracks.empty() && m_particles.empty() && m_vertices.empty() && m_primaryVertices.empty())
    B2ERROR("You have to specify either arrays of single tracks or particle lists of single single particles or mothers with vertex constrained daughters.");

  if (!m_tracks.empty()) {
    for (auto arrayName : m_tracks)
      StoreArray<RecoTrack>::required(arrayName);
  }

  if (!m_particles.empty() || !m_vertices.empty() || !m_primaryVertices.empty()) {
    StoreArray<RecoTrack> recoTracks;
    StoreArray<Track> tracks;
    StoreArray<TrackFitResult> trackFitResults;

    recoTracks.isRequired();
    tracks.isRequired();
    trackFitResults.isRequired();
  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);
    list.isRequired();
  }

  // Register Mille output
  registerObject<MilleData>("mille", new MilleData(m_doublePrecision));

  auto gblDataTree = new TTree("GblDataTree", "GblDataTree");
  gblDataTree->Branch<std::vector<gbl::GblData>>("GblData", &m_currentGblData, 32000, 99);
  registerObject<TTree>("GblDataTree", gblDataTree);

  registerObject<TH1F>("chi2/ndf", new TH1F("chi2/ndf", "chi2/ndf", 200, 0., 50.));
  registerObject<TH1F>("pval", new TH1F("pval", "pval", 100, 0., 1.));

  auto& geo = VXD::GeoCache::getInstance();
  auto& hierarchy = Belle2::alignment::HierarchyManager::getInstance().getAlignmentHierarchy();

  if (m_useVXDHierarchy) {
    // Set-up hierarchy
    DBObjPtr<VXDAlignment> vxdAlignments;
    /**
    So the hierarchy is as follows:
                Belle 2
              / |     | \
          Ying  Yang Pat  Mat ... other sub-detectors
          / |   / |  |  \  | \
        ......  ladders ......
        / / |   / |  |  \  | \ \
      ......... sensors ........
    */

    for (auto& halfShellPlacement : geo.getHalfShellPlacements()) {
      TGeoHMatrix trafoHalfShell = halfShellPlacement.second;
      trafoHalfShell *= geo.getTGeoFromRigidBodyParams(
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dU),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dV),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dW),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dAlpha),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dBeta),
                          vxdAlignments->get(halfShellPlacement.first, VXDAlignment::dGamma)
                        );
      hierarchy.insertTGeoTransform<VXDAlignment, alignment::EmptyGlobaParamSet>(halfShellPlacement.first, 0, trafoHalfShell);

      for (auto& ladderPlacement : geo.getLadderPlacements(halfShellPlacement.first)) {
        // Updated trafo
        TGeoHMatrix trafoLadder = ladderPlacement.second;
        trafoLadder *= geo.getTGeoFromRigidBodyParams(
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dU),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dV),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dW),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dAlpha),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dBeta),
                         vxdAlignments->get(ladderPlacement.first, VXDAlignment::dGamma)
                       );
        hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(ladderPlacement.first, halfShellPlacement.first, trafoLadder);

        for (auto& sensorPlacement : geo.getSensorPlacements(ladderPlacement.first)) {
          // Updated trafo
          TGeoHMatrix trafoSensor = sensorPlacement.second;
          trafoSensor *= geo.getTGeoFromRigidBodyParams(
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dU),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dV),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dW),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dAlpha),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dBeta),
                           vxdAlignments->get(sensorPlacement.first, VXDAlignment::dGamma)
                         );
          hierarchy.insertTGeoTransform<VXDAlignment, VXDAlignment>(sensorPlacement.first, ladderPlacement.first, trafoSensor);


        }
      }
    }

  }

  Belle2::alignment::HierarchyManager::getInstance().writeConstraints("constraints.txt");

  // Add callback to itself. Callback are unique, so further calls should not change anything
  //vxdAlignments.addCallback(this, &VXD::GeoCache::setupReconstructionTransformations);
}

void MillepedeCollectorModule::collect()
{
  if (!m_useGblTree) {
    // Open new file on request (at start or after being closed)
    auto mille = getObjectPtr<MilleData>("mille");
    if (!mille->isOpen())
      mille->open(getUniqueMilleName());
  }


  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());


  for (auto arrayName : m_tracks) {
    StoreArray<RecoTrack> recoTracks(arrayName);

    for (auto& recoTrack : recoTracks) {
      fitRecoTrack(recoTrack);
      auto& track = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);
      if (!track.hasFitStatus())
        continue;
      genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
      if (!fs)
        continue;

      if (!fs->isFittedWithReferenceTrack())
        continue;

      getObjectPtr<TH1F>("chi2/ndf")->Fill(fs->getChi2() / fs->getNdf());
      getObjectPtr<TH1F>("pval")->Fill(fs->getPVal());

      using namespace gbl;
      GblTrajectory trajectory(gbl->collectGblPoints(&track, track.getCardinalRep()), fs->hasCurvature());

      // if (fs->getPVal() > m_minPValue) mille.fill(trajectory);
      if (fs->getPVal() > m_minPValue) storeTrajectory(trajectory);

    }

  }

  for (auto listName : m_particles) {
    StoreObjPtr<ParticleList> list(listName);
    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      for (auto& track : getParticlesTracks({list->getParticle(iParticle)}, false)) {
        auto gblfs = dynamic_cast<genfit::GblFitStatus*>(track->getFitStatus());

        getObjectPtr<TH1F>("chi2/ndf")->Fill(gblfs->getChi2() / gblfs->getNdf());
        getObjectPtr<TH1F>("pval")->Fill(gblfs->getPVal());

        gbl::GblTrajectory trajectory(gbl->collectGblPoints(track, track->getCardinalRep()), gblfs->hasCurvature());
        //if (gblfs->getPVal() > m_minPValue) mille.fill(trajectory);
        if (gblfs->getPVal() > m_minPValue) storeTrajectory(trajectory);

      }
    }
  }

  for (auto listName : m_vertices) {
    StoreObjPtr<ParticleList> list(listName);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      for (auto& track : getParticlesTracks(mother->getDaughters()))
        daughters.push_back({
        gbl->collectGblPoints(track, track->getCardinalRep()),
        getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
      });

      if (daughters.size() > 1) {
        gbl::GblTrajectory combined(daughters);

        double chi2 = -1.;
        double lostWeight = -1.;
        int ndf = -1;

        combined.fit(chi2, ndf, lostWeight);
        B2INFO("Combined GBL fit with vertex constraint: NDF = " << ndf << " Chi2/NDF = " << chi2 / ndf);

        //if (TMath::Prob(chi2, ndf) > m_minPValue) mille.fill(combined);
        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

        getObjectPtr<TH1F>("chi2/ndf")->Fill(chi2 / ndf);
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      }
    }
  }


  for (auto listName : m_primaryVertices) {
    StoreObjPtr<ParticleList> list(listName);

    for (unsigned int iParticle = 0; iParticle < list->getListSize(); ++iParticle) {
      auto mother = list->getParticle(iParticle);
      std::vector<std::pair<std::vector<gbl::GblPoint>, TMatrixD> > daughters;

      TMatrixD extProjection(3, 5);

      bool first(true);
      for (auto& track : getParticlesTracks(mother->getDaughters())) {
        if (first) {
          // For first trajectory only
          extProjection = getLocalToGlobalTransform(track->getFittedState()).GetSub(0, 2, 0, 4);
          first = false;
        }
        daughters.push_back({
          gbl->collectGblPoints(track, track->getCardinalRep()),
          getGlobalToLocalTransform(track->getFittedState()).GetSub(0, 4, 0, 2)
        });
      }

      if (daughters.size() > 1) {
        DBObjPtr<BeamParameters> beam;

        TMatrixDSym vertexPrec(beam->getCovVertex().Invert());
        TVector3 vertexResidual = mother->getVertex() - beam->getVertex();

        TVectorD extMeasurements(3);
        extMeasurements[0] = vertexResidual[0];
        extMeasurements[1] = vertexResidual[1];
        extMeasurements[2] = vertexResidual[2];

        // Attach the external measurement to first point of first trajectory
        daughters[0].first[0].addMeasurement(extProjection, extMeasurements, vertexPrec);

        if (m_calibrateVertex) {
          TMatrixD globals(3, 3);
          globals.UnitMatrix();
          std::vector<int> labels;
          labels.push_back(GlobalLabel(BeamID(), BeamID::vertexX).label());
          labels.push_back(GlobalLabel(BeamID(), BeamID::vertexY).label());
          labels.push_back(GlobalLabel(BeamID(), BeamID::vertexZ).label());

          // Add derivatives for vertex calibration to first point of first trajectory
          daughters[0].first[0].addGlobals(labels, globals);
        }

        gbl::GblTrajectory combined(daughters);

        double chi2 = -1.;
        double lostWeight = -1.;
        int ndf = -1;

        combined.fit(chi2, ndf, lostWeight);
        B2INFO("Combined GBL fit with vertex + ip profile constraint: NDF = " << ndf << " Chi2/NDF = " << chi2 / ndf);

        // if (TMath::Prob(chi2, ndf) > m_minPValue) mille.fill(combined);
        if (TMath::Prob(chi2, ndf) > m_minPValue) storeTrajectory(combined);

        getObjectPtr<TH1F>("chi2/ndf")->Fill(chi2 / ndf);
        getObjectPtr<TH1F>("pval")->Fill(TMath::Prob(chi2, ndf));
      }
    }
  }
}

void MillepedeCollectorModule::closeRun()
{
  // We close the file at end of run, producing
  // one file per run (and process id) which is more
  // convenient than one large binary block.
  auto mille = getObjectPtr<MilleData>("mille");
  if (mille->isOpen())
    mille->close();
}

void MillepedeCollectorModule::finish()
{

  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  if (!fileMetaData.isValid()) {
    B2ERROR("Cannot register binaries in FileCatalog.");
    return;
  }


  const std::vector<string> parents = {fileMetaData->getLfn()};
  for (auto binary : getObjectPtr<MilleData>("mille")->getFiles()) {
    FileMetaData milleMetaData(*fileMetaData);
    // We reset filename to be set directly by the registerFile procedure
    milleMetaData.setLfn("");
    milleMetaData.setParents(parents);
    FileCatalog::Instance().registerFile(binary, milleMetaData);
  }

}

void MillepedeCollectorModule::storeTrajectory(gbl::GblTrajectory& trajectory)
{
  if (m_useGblTree) {
    if (trajectory.isValid())
      m_currentGblData = trajectory.getData();
    else
      m_currentGblData.clear();

    if (!m_currentGblData.empty())
      getObjectPtr<TTree>("GblDataTree")->Fill();
  } else {
    getObjectPtr<MilleData>("mille")->fill(trajectory);
  }
}

std::string MillepedeCollectorModule::getUniqueMilleName()
{
  StoreObjPtr<EventMetaData> emd;
  string name = getName();

  name += "-e"   + to_string(emd->getExperiment());
  name += "-r"   + to_string(emd->getRun());
  name += "-ev"  + to_string(emd->getEvent());

  if (ProcHandler::parallelProcessingUsed())
    name += "-pid" + to_string(ProcHandler::EvtProcID());

  name += ".mille";

  return name;
}

void MillepedeCollectorModule::fitRecoTrack(RecoTrack& recoTrack, Particle* particle)
{
  std::shared_ptr<genfit::GblFitter> gbl(new genfit::GblFitter());
  gbl->setOptions("", true, true, 1, 1);

  MeasurementAdder factory("", "", "", "", "");

  // We need the store arrays
  StoreArray <CDCHit> cdcHits("");
  StoreArray <PXDCluster> pxdHits("");
  StoreArray <SVDCluster> svdHits("");
  StoreArray<RecoHitInformation::UsedBKLMHit> bklmHits("");
  StoreArray<RecoHitInformation::UsedEKLMHit> eklmHits("");

  // Create the genfit::MeasurementFactory
  genfit::MeasurementFactory<genfit::AbsMeasurement> genfitMeasurementFactory;

  // Add producer for alignable RecoHits to factory
  if (pxdHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedPXDHit, AlignablePXDRecoHit>* PXDProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedPXDHit, AlignablePXDRecoHit> (pxdHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::PXD, PXDProducer);
  }

  if (svdHits.isOptional())  {
    genfit::MeasurementProducer <RecoHitInformation::UsedSVDHit, AlignableSVDRecoHit>* SVDProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedSVDHit, AlignableSVDRecoHit> (svdHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::SVD, SVDProducer);
  }

  if (cdcHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedCDCHit, AlignableCDCRecoHit>* CDCProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedCDCHit, AlignableCDCRecoHit> (cdcHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::CDC, CDCProducer);
  }

  if (bklmHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedBKLMHit, BKLMRecoHit>* BKLMProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedBKLMHit, BKLMRecoHit> (bklmHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::BKLM, BKLMProducer);
  }

  if (eklmHits.isOptional()) {
    genfit::MeasurementProducer <RecoHitInformation::UsedEKLMHit, AlignableEKLMRecoHit>* EKLMProducer =  new genfit::MeasurementProducer
    <RecoHitInformation::UsedEKLMHit, AlignableEKLMRecoHit> (eklmHits.getPtr());
    genfitMeasurementFactory.addProducer(Const::EKLM, EKLMProducer);
  }


  // Create the measurement creators
  std::vector<std::shared_ptr<PXDBaseMeasurementCreator>> pxdMeasurementCreators = { std::shared_ptr<PXDBaseMeasurementCreator>(new PXDCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<SVDBaseMeasurementCreator>> svdMeasurementCreators = { std::shared_ptr<SVDBaseMeasurementCreator>(new SVDCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  // TODO: Create a new MeasurementCreator based on SVDBaseMeasurementCreator (or on SVDCoordinateMeasurementCreator), which does the combination on the fly.

  std::vector<std::shared_ptr<CDCBaseMeasurementCreator>> cdcMeasurementCreators = { std::shared_ptr<CDCBaseMeasurementCreator>(new CDCCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<BKLMBaseMeasurementCreator>> bklmMeasurementCreators = { std::shared_ptr<BKLMBaseMeasurementCreator>(new BKLMCoordinateMeasurementCreator(genfitMeasurementFactory)) };
  std::vector<std::shared_ptr<EKLMBaseMeasurementCreator>> eklmMeasurementCreators = { std::shared_ptr<EKLMBaseMeasurementCreator>(new EKLMCoordinateMeasurementCreator(genfitMeasurementFactory)) };

  // TODO: Or put it in here and leave the svdMeasurementCreators empty.
  std::vector<std::shared_ptr<BaseMeasurementCreator>> additionalMeasurementCreators = {};
  factory.resetMeasurementCreators(pxdMeasurementCreators, svdMeasurementCreators, cdcMeasurementCreators, bklmMeasurementCreators,
                                   eklmMeasurementCreators, additionalMeasurementCreators);
  factory.addMeasurements(recoTrack);

  auto& gfTrack = RecoTrackGenfitAccess::getGenfitTrack(recoTrack);

  const int currentPdgCode = TrackFitter::createCorrectPDGCodeForChargedStable(Const::muon, recoTrack);
  genfit::AbsTrackRep* trackRep = new genfit::RKTrackRep(currentPdgCode);
  gfTrack.addTrackRep(trackRep);
  gfTrack.setCardinalRep(gfTrack.getIdForRep(trackRep));

  if (particle) {
    TVector3 vertexPos = particle->getVertex();
    TVector3 vertexMom = particle->getMomentum();
    gfTrack.setStateSeed(vertexPos, vertexMom);

    genfit::StateOnPlane vertexSOP(gfTrack.getCardinalRep());
    TVector3 vertexRPhiDir(vertexPos[0], vertexPos[1], 0);
    TVector3 vertexZDir(0, 0, vertexPos[2]);
    genfit::SharedPlanePtr vertexPlane(new genfit::DetPlane(vertexPos, vertexRPhiDir, vertexZDir));
    vertexSOP.setPlane(vertexPlane);
    vertexSOP.setPosMom(vertexPos, vertexMom);
    TMatrixDSym vertexCov(5);
    vertexCov.UnitMatrix();
    vertexCov *= -1.;
    genfit::MeasuredStateOnPlane mop(vertexSOP, vertexCov);
    genfit::FullMeasurement* vertex = new genfit::FullMeasurement(mop, Const::IR);
    gfTrack.insertMeasurement(vertex, 0);
  }

  try {
    for (unsigned int i = 0; i < gfTrack.getNumPoints() - 1; ++i) {
      //if (gfTrack.getPointWithMeasurement(i)->getNumRawMeasurements() != 1)
      //  continue;
      genfit::PlanarMeasurement* planarMeas1 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(
                                                 i)->getRawMeasurement(0));
      genfit::PlanarMeasurement* planarMeas2 = dynamic_cast<genfit::PlanarMeasurement*>(gfTrack.getPointWithMeasurement(
                                                 i + 1)->getRawMeasurement(0));

      if (planarMeas1 != NULL && planarMeas2 != NULL &&
          planarMeas1->getDetId() == planarMeas2->getDetId() &&
          planarMeas1->getPlaneId() != -1 &&   // -1 is default plane id
          planarMeas1->getPlaneId() == planarMeas2->getPlaneId()) {
        Belle2::AlignableSVDRecoHit* hit1 = dynamic_cast<Belle2::AlignableSVDRecoHit*>(planarMeas1);
        Belle2::AlignableSVDRecoHit* hit2 = dynamic_cast<Belle2::AlignableSVDRecoHit*>(planarMeas2);
        if (hit1 && hit2) {
          Belle2::AlignableSVDRecoHit* hitU(NULL);
          Belle2::AlignableSVDRecoHit* hitV(NULL);
          // We have to decide U/V now (else AlignableSVDRecoHit2D could throw FATAL)
          if (hit1->isU() && !hit2->isU()) {
            hitU = hit1;
            hitV = hit2;
          } else if (!hit1->isU() && hit2->isU()) {
            hitU = hit2;
            hitV = hit1;
          } else {
            continue;
          }
          Belle2::AlignableSVDRecoHit2D* hit = new Belle2::AlignableSVDRecoHit2D(*hitU, *hitV);
          // insert measurement before point i (increases number of currect point to i+1)
          gfTrack.insertMeasurement(hit, i);
          // now delete current point (at its original place, we have the new 2D recohit)
          gfTrack.deletePoint(i + 1);
          gfTrack.deletePoint(i + 1);
        }
      }
    }
  } catch (...) {
    B2ERROR("SVD Cluster combination failed.");
  }
  try {
    gbl->processTrack(&gfTrack, true);
  } catch (...) {
    B2ERROR("GBL fit failed.");
  }
}


std::vector< genfit::Track* > MillepedeCollectorModule::getParticlesTracks(std::vector<Particle*> particles, bool addVertexPoint)
{
  std::vector< genfit::Track* > tracks;
  for (auto particle : particles) {
    auto belle2Track = particle->getTrack();
    if (!belle2Track) {
      B2INFO("No Belle2::Track for particle");
      continue;
    }
//     auto trackFitResult = belle2Track->getTrackFitResult(Const::chargedStableSet.find(abs(particle->getPDGCode())));
//     if (!trackFitResult) {
//       B2INFO("No track fit result for track");
//       continue;
//     }
//    auto recoTrack = trackFitResult->getRelatedFrom<RecoTrack>();
    auto recoTrack = belle2Track->getRelatedTo<RecoTrack>();

    if (!recoTrack) {
      B2INFO("No related RecoTrack for Belle2::Track");
      continue;
    }
    auto& track = RecoTrackGenfitAccess::getGenfitTrack(*recoTrack);

    fitRecoTrack(*recoTrack, (addVertexPoint) ? particle : nullptr);

    if (!track.hasFitStatus()) {
      B2INFO("Track has no fit status");
      continue;
    }
    genfit::GblFitStatus* fs = dynamic_cast<genfit::GblFitStatus*>(track.getFitStatus());
    if (!fs) {
      B2INFO("Fit status is not GblFitStatus.");
      continue;
    }
    if (!fs->isFittedWithReferenceTrack()) {
      B2INFO("Track is not fitted with reference track.");
      continue;
    }

    tracks.push_back(&track);
  }

  return tracks;
}

TMatrixD MillepedeCollectorModule::getGlobalToLocalTransform(genfit::MeasuredStateOnPlane msop)
{
  auto state = msop;
  const TVector3& U(state.getPlane()->getU());
  const TVector3& V(state.getPlane()->getV());
  const TVector3& O(state.getPlane()->getO());
  const TVector3& W(state.getPlane()->getNormal());

  const double* state5 = state.getState().GetMatrixArray();

  double spu = 1.;

  const TVectorD& auxInfo = state.getAuxInfo();
  if (auxInfo.GetNrows() == 2
      || auxInfo.GetNrows() == 1) // backwards compatibility with old RKTrackRep
    spu = state.getAuxInfo()(0);

  TVectorD state7(7);

  state7[0] = O.X() + state5[3] * U.X() + state5[4] * V.X(); // x
  state7[1] = O.Y() + state5[3] * U.Y() + state5[4] * V.Y(); // y
  state7[2] = O.Z() + state5[3] * U.Z() + state5[4] * V.Z(); // z

  state7[3] = spu * (W.X() + state5[1] * U.X() + state5[2] * V.X()); // a_x
  state7[4] = spu * (W.Y() + state5[1] * U.Y() + state5[2] * V.Y()); // a_y
  state7[5] = spu * (W.Z() + state5[1] * U.Z() + state5[2] * V.Z()); // a_z

  // normalize dir
  double norm = 1. / sqrt(state7[3] * state7[3] + state7[4] * state7[4] + state7[5] * state7[5]);
  for (unsigned int i = 3; i < 6; ++i) state7[i] *= norm;

  state7[6] = state5[0]; // q/p

  const double AtU = state7[3] * U.X() + state7[4] * U.Y() + state7[5] * U.Z();
  const double AtV = state7[3] * V.X() + state7[4] * V.Y() + state7[5] * V.Z();
  const double AtW = state7[3] * W.X() + state7[4] * W.Y() + state7[5] * W.Z();

  // J_Mp matrix is d(q/p,u',v',u,v) / d(x,y,z,px,py,pz)       (in is 6x6)

  const double qop = state7[6];
  const double p = state.getCharge() / qop; // momentum

  TMatrixD J_Mp_6x5(6, 5);
  J_Mp_6x5.Zero();

  //d(u)/d(x,y,z)
  J_Mp_6x5(0, 3)  = U.X(); // [0][3]
  J_Mp_6x5(1, 3)  = U.Y(); // [1][3]
  J_Mp_6x5(2, 3) = U.Z(); // [2][3]
  //d(v)/d(x,y,z)
  J_Mp_6x5(0, 4)  = V.X(); // [0][4]
  J_Mp_6x5(1, 4)  = V.Y(); // [1][4]
  J_Mp_6x5(2, 4) = V.Z(); // [2][4]

  // d(q/p)/d(px,py,pz)
  double fact = (-1.) * qop / p;
  J_Mp_6x5(3, 0) = fact * state7[3]; // [3][0]
  J_Mp_6x5(4, 0) = fact * state7[4]; // [4][0]
  J_Mp_6x5(5, 0) = fact * state7[5]; // [5][0]
  // d(u')/d(px,py,pz)
  fact = 1. / (p * AtW * AtW);
  J_Mp_6x5(3, 1) = fact * (U.X() * AtW - W.X() * AtU); // [3][1]
  J_Mp_6x5(4, 1) = fact * (U.Y() * AtW - W.Y() * AtU); // [4][1]
  J_Mp_6x5(5, 1) = fact * (U.Z() * AtW - W.Z() * AtU); // [5][1]
  // d(v')/d(px,py,pz)
  J_Mp_6x5(3, 2) = fact * (V.X() * AtW - W.X() * AtV); // [3][2]
  J_Mp_6x5(4, 2) = fact * (V.Y() * AtW - W.Y() * AtV); // [4][2]
  J_Mp_6x5(5, 2) = fact * (V.Z() * AtW - W.Z() * AtV); // [5][2]

  return J_Mp_6x5;
}

TMatrixD MillepedeCollectorModule::getLocalToGlobalTransform(genfit::MeasuredStateOnPlane msop)
{
  auto state = msop;
  // get vectors and aux variables
  const TVector3& U(state.getPlane()->getU());
  const TVector3& V(state.getPlane()->getV());
  const TVector3& W(state.getPlane()->getNormal());

  const TVectorD& state5(state.getState());
  double spu = 1.;

  const TVectorD& auxInfo = state.getAuxInfo();
  if (auxInfo.GetNrows() == 2
      || auxInfo.GetNrows() == 1) // backwards compatibility with old RKTrackRep
    spu = state.getAuxInfo()(0);

  TVectorD pTilde(3);
  pTilde[0] = spu * (W.X() + state5(1) * U.X() + state5(2) * V.X()); // a_x
  pTilde[1] = spu * (W.Y() + state5(1) * U.Y() + state5(2) * V.Y()); // a_y
  pTilde[2] = spu * (W.Z() + state5(1) * U.Z() + state5(2) * V.Z()); // a_z

  const double pTildeMag = sqrt(pTilde[0] * pTilde[0] + pTilde[1] * pTilde[1] + pTilde[2] * pTilde[2]);
  const double pTildeMag2 = pTildeMag * pTildeMag;

  const double utpTildeOverpTildeMag2 = (U.X() * pTilde[0] + U.Y() * pTilde[1] + U.Z() * pTilde[2]) / pTildeMag2;
  const double vtpTildeOverpTildeMag2 = (V.X() * pTilde[0] + V.Y() * pTilde[1] + V.Z() * pTilde[2]) / pTildeMag2;

  //J_pM matrix is d(x,y,z,px,py,pz) / d(q/p,u',v',u,v)       (out is 6x6)

  const double qop = state5(0);
  const double p = state.getCharge() / qop; // momentum

  TMatrixD J_pM_5x6(5, 6);
  J_pM_5x6.Zero();

  // d(px,py,pz)/d(q/p)
  double fact = -1. * p / (pTildeMag * qop);
  J_pM_5x6(0, 3) = fact * pTilde[0]; // [0][3]
  J_pM_5x6(0, 4) = fact * pTilde[1]; // [0][4]
  J_pM_5x6(0, 5) = fact * pTilde[2]; // [0][5]
  // d(px,py,pz)/d(u')
  fact = p * spu / pTildeMag;
  J_pM_5x6(1, 3)  = fact * (U.X() - pTilde[0] * utpTildeOverpTildeMag2); // [1][3]
  J_pM_5x6(1, 4) = fact * (U.Y() - pTilde[1] * utpTildeOverpTildeMag2); // [1][4]
  J_pM_5x6(1, 5) = fact * (U.Z() - pTilde[2] * utpTildeOverpTildeMag2); // [1][5]
  // d(px,py,pz)/d(v')
  J_pM_5x6(2, 3) = fact * (V.X() - pTilde[0] * vtpTildeOverpTildeMag2); // [2][3]
  J_pM_5x6(2, 4) = fact * (V.Y() - pTilde[1] * vtpTildeOverpTildeMag2); // [2][4]
  J_pM_5x6(2, 5) = fact * (V.Z() - pTilde[2] * vtpTildeOverpTildeMag2); // [2][5]
  // d(x,y,z)/d(u)
  J_pM_5x6(3, 0) = U.X(); // [3][0]
  J_pM_5x6(3, 1) = U.Y(); // [3][1]
  J_pM_5x6(3, 2) = U.Z(); // [3][2]
  // d(x,y,z)/d(v)
  J_pM_5x6(4, 0) = V.X(); // [4][0]
  J_pM_5x6(4, 1) = V.Y(); // [4][1]
  J_pM_5x6(4, 2) = V.Z(); // [4][2]

  return J_pM_5x6;

}
