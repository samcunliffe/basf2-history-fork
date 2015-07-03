#include <tracking/dataobjects/RecoTrack.h>


#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <genfit/AbsKalmanFitter.h>
#include <genfit/RKTrackRep.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/KalmanFitStatus.h>

using namespace Belle2;

ClassImp(RecoTrack);

RecoTrack::RecoTrack(const TVector3& position, const TVector3& momentum, const short int charge,
                     const std::string& storeArrayNameOfCDCHits,
                     const std::string& storeArrayNameOfSVDHits,
                     const std::string& storeArrayNameOfPXDHits,
                     const std::string& storeArrayNameOfRecoHitInformation) :
  m_charge(charge),
  m_storeArrayNameOfCDCHits(storeArrayNameOfCDCHits),
  m_storeArrayNameOfSVDHits(storeArrayNameOfSVDHits),
  m_storeArrayNameOfPXDHits(storeArrayNameOfPXDHits),
  m_storeArrayNameOfRecoHitInformation(storeArrayNameOfRecoHitInformation),
  m_lastFitSucessfull(false)
{
  setStateSeed(position, momentum);

  // Create a measurement factory
  StoreArray<UsedPXDHit> pxdHits(m_storeArrayNameOfPXDHits);
  StoreArray<UsedSVDHit> svdHits(m_storeArrayNameOfSVDHits);
  StoreArray<UsedCDCHit> cdcHits(m_storeArrayNameOfCDCHits);

  // Create the related measurement factory
  if (pxdHits.isOptional())
    m_measurementFactory.addProducer(Const::PXD, new genfit::MeasurementProducer<RecoTrack::UsedPXDHit, PXDRecoHit>(pxdHits.getPtr()));
  if (svdHits.isOptional())
    m_measurementFactory.addProducer(Const::SVD, new genfit::MeasurementProducer<RecoTrack::UsedSVDHit, SVDRecoHit>(svdHits.getPtr()));
  if (cdcHits.isOptional())
    m_measurementFactory.addProducer(Const::CDC, new genfit::MeasurementProducer<RecoTrack::UsedCDCHit, CDCRecoHit>(cdcHits.getPtr()));
}

RecoTrack* RecoTrack::createFromTrackCand(const genfit::TrackCand* trackCand,
                                          const std::string& storeArrayNameOfRecoTracks,
                                          const std::string& storeArrayNameOfCDCHits,
                                          const std::string& storeArrayNameOfSVDHits,
                                          const std::string& storeArrayNameOfPXDHits,
                                          const std::string& storeArrayNameOfRecoHitInformation
                                         )
{

  StoreArray<RecoTrack> recoTracks(storeArrayNameOfRecoTracks);
  StoreArray<UsedCDCHit> cdcHits(storeArrayNameOfCDCHits);
  StoreArray<UsedSVDHit> svdHits(storeArrayNameOfSVDHits);
  StoreArray<UsedPXDHit> pxdHits(storeArrayNameOfPXDHits);

  // Set the tracking parameters
  const TVector3& position = trackCand->getPosSeed();
  const TVector3& momentum = trackCand->getMomSeed();
  const short int charge = trackCand->getChargeSeed();

  RecoTrack* newRecoTrack = recoTracks.appendNew(position, momentum, charge,
                                                 storeArrayNameOfCDCHits, storeArrayNameOfSVDHits,
                                                 storeArrayNameOfPXDHits, storeArrayNameOfRecoHitInformation);

  for (unsigned int hitIndex = 0; hitIndex < trackCand->getNHits(); hitIndex++) {
    genfit::TrackCandHit* trackCandHit = trackCand->getHit(hitIndex);
    const int detID = trackCandHit->getDetId();
    const int hitID = trackCandHit->getHitId();
    const double sortingParameter = trackCandHit->getSortingParameter();
    if (detID == Const::CDC) {
      UsedCDCHit* cdcHit = cdcHits[hitID];
      newRecoTrack->addCDCHit(cdcHit, sortingParameter);
    } else if (detID == Const::SVD) {
      UsedSVDHit* svdHit = svdHits[hitID];
      newRecoTrack->addSVDHit(svdHit, sortingParameter);
    } else if (detID == Const::PXD) {
      UsedPXDHit* pxdHit = pxdHits[hitID];
      newRecoTrack->addPXDHit(pxdHit, sortingParameter);
    }
  }

  return newRecoTrack;
}

genfit::TrackCand* RecoTrack::createGenfitTrackCand() const
{
  genfit::TrackCand* createdTrackCand = new genfit::TrackCand();

  // Set the trajectory parameters
  createdTrackCand->setPosMomSeed(getPosition(), getMomentum(), getCharge());

  // Add the hits
  mapOnHits<UsedCDCHit>(m_storeArrayNameOfCDCHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedCDCHit * const hit) {
    createdTrackCand->addHit(Const::CDC, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedSVDHit>(m_storeArrayNameOfSVDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedSVDHit * const hit) {
    createdTrackCand->addHit(Const::SVD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });
  mapOnHits<UsedPXDHit>(m_storeArrayNameOfPXDHits, [&createdTrackCand](const RecoHitInformation & hitInformation,
  const UsedPXDHit * const hit) {
    createdTrackCand->addHit(Const::PXD, hit->getArrayIndex(), -1, hitInformation.getSortingParameter());
  });

  // Add the hits
  return createdTrackCand;
}

template <class HitType>
void RecoTrack::addHitToGenfitTrack(Const::EDetector detector,
                                    RecoHitInformation& recoHitInformation, HitType* const cdcHit)
{
  genfit::TrackCandHit* trackCandHit = new genfit::TrackCandHit(detector, cdcHit->getArrayIndex(), -1,
      recoHitInformation.getSortingParameter());
  genfit::AbsMeasurement* measurement = m_measurementFactory.createOne(trackCandHit->getDetId(), trackCandHit->getHitId(),
                                        trackCandHit);
  genfit::TrackPoint* trackPoint = new genfit::TrackPoint(measurement, this);
  trackPoint->setSortingParameter(recoHitInformation.getSortingParameter());
  insertPoint(trackPoint);
};

void RecoTrack::calculateTimeSeed(TParticlePDG* particleWithPDGCode)
{
  const TVector3& momentum = getMomentum();

  // Set the timing seed
  // Particle velocity in cm / ns.
  const double m = particleWithPDGCode->Mass();
  const double p = momentum.Mag();
  const double E = hypot(m, p);
  const double beta = p / E;
  const double v = beta * Const::speedOfLight;

  // Arc length from IP to posSeed in cm.
  // Calculate the arc-length.  Helix doesn't provide a way of
  // obtaining this directly from the difference in z, as it
  // only provide arc-lengths in the transverse plane, so we do
  // it like this.
  const TVector3& perigeePosition = getPosition();
  const Helix h(perigeePosition, momentum, particleWithPDGCode->Charge() / 3, 1.5);
  const double s2D = h.getArcLength2DAtCylindricalR(perigeePosition.Perp());
  const double s = s2D * hypot(1, h.getTanLambda());

  // Time (ns) from trigger (= 0 ns) to posSeed assuming constant velocity.
  double timeSeed = s / v;

  if (!(timeSeed > -1000)) {
    // Guard against NaN or just something silly.
    B2WARNING("Fixing calculated seed Time " << timeSeed << " to zero.");
    timeSeed = 0;
  }
  setTimeSeed(timeSeed);
}

void RecoTrack::fit(const std::shared_ptr<genfit::AbsFitter>& fitter, int pdgCodeForFit)
{
  m_lastFitSucessfull = false;

  // Create a track representation
  // Set the pdg code accordingly if the user gave us the wrong charge sign
  TParticlePDG* particleWithPDGCode = TDatabasePDG::Instance()->GetParticle(pdgCodeForFit);
  // Note that for leptons positive PDG codes correspond to the
  // negatively charged particles.
  if (std::signbit(particleWithPDGCode->Charge()) != std::signbit(getCharge()))
    pdgCodeForFit *= -1;
  if (TDatabasePDG::Instance()->GetParticle(pdgCodeForFit)->Charge()
      != getCharge() * 3)
    B2FATAL("Charge of candidate and PDG particle don't match.  (Code assumes |q| = 1).");

  genfit::RKTrackRep* trackRep = new genfit::RKTrackRep(pdgCodeForFit);
  addTrackRep(trackRep);

  // TODO: It may be better to do this already when adding the hits to the reco track.
  // create TrackPoints
  // Loop over all hits and create a abs measurement with the factory.
  // then create a TrackPoint from that and set the sorting parameter
  mapOnHits<UsedCDCHit>(m_storeArrayNameOfCDCHits, std::bind(&RecoTrack::addHitToGenfitTrack<UsedCDCHit>, this, Const::CDC,
                                                             std::placeholders::_1, std::placeholders::_2));
  mapOnHits<UsedSVDHit>(m_storeArrayNameOfSVDHits, std::bind(&RecoTrack::addHitToGenfitTrack<UsedSVDHit>, this, Const::SVD,
                                                             std::placeholders::_1, std::placeholders::_2));
  mapOnHits<UsedPXDHit>(m_storeArrayNameOfPXDHits, std::bind(&RecoTrack::addHitToGenfitTrack<UsedPXDHit>, this, Const::PXD,
                                                             std::placeholders::_1, std::placeholders::_2));

  // TODO!
  // Set the covariance seed
  TMatrixDSym covSeed(6);
  covSeed(0, 0) = 1e-3;
  covSeed(1, 1) = 1e-3;
  covSeed(2, 2) = 4e-3;
  covSeed(3, 3) = 0.01e-3;
  covSeed(4, 4) = 0.01e-3;
  covSeed(5, 5) = 0.04e-3;
  setCovSeed(covSeed);

  calculateTimeSeed(particleWithPDGCode);

  // Fit the track
  fitter->processTrack(this);

  // Postprocessing of the fitted track
  bool fitSuccess = hasFitStatus(trackRep);
  if (fitSuccess) {
    genfit::FitStatus* fs = getFitStatus(trackRep);
    fitSuccess = fitSuccess && fs->isFitted();
    fitSuccess = fitSuccess && fs->isFitConverged();
    genfit::KalmanFitStatus* kfs = dynamic_cast<genfit::KalmanFitStatus*>(fs);
    fitSuccess = fitSuccess && kfs;
  }
  m_lastFitSucessfull = fitSuccess;
}
