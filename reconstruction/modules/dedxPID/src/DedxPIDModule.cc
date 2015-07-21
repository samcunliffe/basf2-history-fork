/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/dedxPID/DedxPIDModule.h>
#include <reconstruction/modules/dedxPID/LineHelper.h>
#include <reconstruction/modules/dedxPID/HelixHelper.h>

#include <reconstruction/dataobjects/DedxTrack.h>
#include <reconstruction/dataobjects/DedxLikelihood.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <svd/dataobjects/SVDCluster.h>
#include <pxd/dataobjects/PXDCluster.h>

#include <cdc/translators/LinearGlobalADCCountTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <vxd/geometry/GeoCache.h>
#include <tracking/gfbfield/GFGeant4Field.h>

#include <genfit/TrackCand.h>
#include <genfit/Track.h>
#include <genfit/AbsTrackRep.h>
#include <genfit/Exception.h>
#include <genfit/MaterialEffects.h>
#include <genfit/StateOnPlane.h>

#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>

#include <memory>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>

using namespace Belle2;
using namespace CDC;
using namespace Dedx;

REG_MODULE(DedxPID)

DedxPIDModule::DedxPIDModule() : Module(), m_pdfs()
{
  setPropertyFlags(c_ParallelProcessingCertified);

  //Set module properties
  setDescription("Extract dE/dx and corresponding log-likelihood from fitted tracks and hits in the CDC, SVD and PXD.");

  //Parameter definitions
  addParam("useIndividualHits", m_useIndividualHits,
           "Include PDF value for each hit in likelihood. If false, the truncated mean of dedx values for the detectors will be used.", true);
  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.05));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.25));

  addParam("onlyPrimaryParticles", m_onlyPrimaryParticles, "Only save data for primary particles (as determined by MC truth)", false);
  addParam("usePXD", m_usePXD, "Use PXDClusters for dE/dx calculation", false);
  addParam("useSVD", m_useSVD, "Use SVDClusters for dE/dx calculation", true);
  addParam("useCDC", m_useCDC, "Use CDCHits for dE/dx calculation", true);

  addParam("trackDistanceThreshold", m_trackDistanceThreshhold,
           "Use a faster helix parametrisation, with corrections as soon as the approximation is more than ... cm off.", double(4.0));
  addParam("enableDebugOutput", m_enableDebugOutput, "Option to write out debugging information to DedxTracks (DataStore objects).",
           false);

  addParam("pdfFile", m_pdfFile, "The dE/dx:momentum PDF file to use. Use an empty string to disable classification.",
           std::string("/data/reconstruction/dedxPID_PDFs_r17648_400k_events.root"));
  addParam("ignoreMissingParticles", m_ignoreMissingParticles, "Ignore particles for which no PDFs are found", false);

  m_eventID = -1;
  m_trackID = 0;

}

DedxPIDModule::~DedxPIDModule() { }

void DedxPIDModule::initialize()
{

  // check for a pdf file - necessary for likelihood calculations
  if (!m_pdfFile.empty()) {

    std::string fullPath = FileSystem::findFile(m_pdfFile);
    if (fullPath.empty()) {
      B2ERROR("PDF file " << m_pdfFile << " not found!");
    }
    m_pdfFile = fullPath;
  }
  if (!m_enableDebugOutput and m_pdfFile.empty()) {
    B2ERROR("No PDFFile given and debug output disabled. This module will produce no output!");
  }

  // required inputs
  StoreArray<Track> tracks;
  StoreArray<genfit::Track> gfTracks;
  StoreArray<TrackFitResult> trackfitResults;
  StoreArray<genfit::TrackCand> trackCandidates;

  tracks.isRequired();
  gfTracks.isRequired();
  trackfitResults.isRequired();
  trackCandidates.isRequired();

  //optional inputs
  StoreArray<MCParticle> mcparticles;
  mcparticles.isOptional();
  tracks.optionalRelationTo(mcparticles);
  if (m_useCDC)
    StoreArray<CDCHit>::required();
  else
    StoreArray<CDCHit>::optional();
  if (m_useSVD)
    StoreArray<SVDCluster>::required();
  else
    StoreArray<SVDCluster>::optional();
  if (m_usePXD)
    StoreArray<PXDCluster>::required();
  else
    StoreArray<PXDCluster>::optional();

  // register outputs
  if (m_enableDebugOutput) {
    StoreArray<DedxTrack> dedxTracks;
    dedxTracks.registerInDataStore();
    tracks.registerRelationTo(dedxTracks);
  }

  if (!m_pdfFile.empty()) {
    StoreArray<DedxLikelihood> dedxLikelihoods;
    dedxLikelihoods.registerInDataStore();
    tracks.registerRelationTo(dedxLikelihoods);

    //load pdfs
    TFile* pdf_file = new TFile(m_pdfFile.c_str(), "READ");
    if (!pdf_file->IsOpen())
      B2FATAL("Couldn't open pdf file: " << m_pdfFile);

    //load dedx:momentum PDFs
    const char* suffix = (!m_useIndividualHits) ? "_trunc" : "";
    for (int detector = 0; detector < c_num_detectors; detector++) {
      int nBinsX, nBinsY;
      double xMin, xMax, yMin, yMax;
      nBinsX = nBinsY = -1;
      xMin = xMax = yMin = yMax = 0.0;
      for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
        const int pdg_code = Const::chargedStableSet.at(iPart).getPDGCode();
        m_pdfs[detector][iPart] =
          dynamic_cast<TH2F*>(pdf_file->Get(TString::Format("hist_d%i_%i%s", detector, pdg_code, suffix)));

        if (!m_pdfs[detector][iPart]) {
          if (m_ignoreMissingParticles)
            continue;
          B2FATAL("Couldn't find PDF for PDG " << pdg_code << ", detector " << detector << suffix);
        }

        //check that PDFs have the same dimensions and same binning
        const double eps_factor = 1e-5;
        if (nBinsX == -1 and nBinsY == -1) {
          nBinsX = m_pdfs[detector][iPart]->GetNbinsX();
          nBinsY = m_pdfs[detector][iPart]->GetNbinsY();
          xMin = m_pdfs[detector][iPart]->GetXaxis()->GetXmin();
          xMax = m_pdfs[detector][iPart]->GetXaxis()->GetXmax();
          yMin = m_pdfs[detector][iPart]->GetYaxis()->GetXmin();
          yMax = m_pdfs[detector][iPart]->GetYaxis()->GetXmax();
        } else if (nBinsX != m_pdfs[detector][iPart]->GetNbinsX()
                   or nBinsY != m_pdfs[detector][iPart]->GetNbinsY()
                   or fabs(xMin - m_pdfs[detector][iPart]->GetXaxis()->GetXmin()) > eps_factor * xMax
                   or fabs(xMax - m_pdfs[detector][iPart]->GetXaxis()->GetXmax()) > eps_factor * xMax
                   or fabs(yMin - m_pdfs[detector][iPart]->GetYaxis()->GetXmin()) > eps_factor * yMax
                   or fabs(yMax - m_pdfs[detector][iPart]->GetYaxis()->GetXmax()) > eps_factor * yMax) {
          B2FATAL("PDF for PDG " << pdg_code << ", detector " << detector << suffix << " has binning/dimensions differing from previous PDF.")
        }
      }
    }

    //leaking pdf_file so I can access the histograms
  }

  // create instances here to not confuse profiling
  CDCGeometryPar::Instance();
  VXD::GeoCache::getInstance();

  if (!genfit::MaterialEffects::getInstance()->isInitialized()) {
    B2FATAL("Need to have SetupGenfitExtrapolationModule in path before this one.");
  }
}

void DedxPIDModule::event()
{
  // go through Tracks
  // get fitresult and gftrack and do extrapolations, save corresponding dE/dx and likelihood values
  //   get genfit::TrackCand through genfit::Track::getCand()
  //   get hit indices through genfit::TrackCand::getHit(...)
  //   create one DedxTrack per fitresult/gftrack
  //create one DedkLikelihood per Track (plus rel)
  m_eventID++;

  // inputs
  StoreArray<Track> tracks;
  StoreArray<MCParticle> mcparticles;
  const int num_mcparticles = mcparticles.getEntries();

  StoreArray<PXDCluster> pxdClusters;
  StoreArray<SVDCluster> svdClusters;

  // outputs
  StoreArray<DedxTrack> dedxArray;
  StoreArray<DedxLikelihood> likelihoodArray;

  // get the geometry of the cdc
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();

  // **************************************************
  //
  //  LOOP OVER TRACKS
  //
  // **************************************************

  for (const auto& track : tracks) {
    m_trackID++;

    std::shared_ptr<DedxTrack> dedxTrack = std::make_shared<DedxTrack>();
    dedxTrack->m_eventID = m_eventID;
    dedxTrack->m_trackID = m_trackID;

    // get pion fit hypothesis for now
    //  Should be ok in most cases, for MC fitting this will return the fit with the
    //  true PDG value. At some point, it might be worthwhile to look into using a
    //  different fit if the differences are large
    const TrackFitResult* fitResult = track.getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for track ...");
      continue;
    }

    if ((m_enableDebugOutput or m_onlyPrimaryParticles) and num_mcparticles != 0) {
      // find MCParticle corresponding to this track
      const MCParticle* mcpart = track.getRelatedTo<MCParticle>();

      if (mcpart) {
        if (m_onlyPrimaryParticles && !mcpart->hasStatus(MCParticle::c_PrimaryParticle)) {
          continue; //not a primary particle, ignore
        }

        //add some MC truths to DedxTrack object
        dedxTrack->m_pdg = mcpart->getPDG();
        const MCParticle* mother = mcpart->getMother();
        dedxTrack->m_mother_pdg = mother ? mother->getPDG() : 0;

        const TVector3 true_momentum = mcpart->getMomentum();
        dedxTrack->m_p_true = true_momentum.Mag();
      } else {
        B2WARNING("No MCParticle found for current track!");
      }
    }

    // get momentum (at origin) from fit result
    const TVector3& trackPos = fitResult->getPosition();
    const TVector3& trackMom = fitResult->getMomentum();
    dedxTrack->m_p = trackMom.Mag();
    dedxTrack->m_cosTheta = trackMom.CosTheta();
    dedxTrack->m_charge = fitResult->getChargeSign();

    // dE/dx values will be calculated using associated genfit::Track
    const genfit::Track* gftrack = fitResult->getRelatedFrom<genfit::Track>();
    if (!gftrack) {
      B2WARNING("No related track for this fit...");
      continue;
    }

    // Check to see if the track is pruned
    genfit::AbsTrackRep* trackrep = gftrack->getCardinalRep();
    if (gftrack->getFitStatus(trackrep)->isTrackPruned()) {
      B2ERROR("GFTrack is pruned, please run DedxPID only on unpruned tracks! Skipping this track.");
      continue;
    }

    //used for PXD/SVD hits
    const HelixHelper helixAtOrigin(trackPos, trackMom, dedxTrack->m_charge);

    // loop over all CDC hits from this track
    if (m_useCDC) {
      double layerdE = 0.0; // total charge in current layer
      double layerdx = 0.0; // total path length in current layer
      double cdcMom = 0.0; // momentum valid in the CDC

      // Get the TrackPoints, which contain the hit information we need.
      // Then iterate over each point.
      int tpcounter = 0;
      const std::vector< genfit::TrackPoint* > gftrackPoints = gftrack->getPointsWithMeasurement();
      for (std::vector< genfit::TrackPoint* >::const_iterator tp = gftrackPoints.begin();
           tp != gftrackPoints.end(); tp++) {
        tpcounter++;

        // should also be possible to use this for svd and pxd hits...
        genfit::AbsMeasurement* aAbsMeasurementPtr = (*tp)->getRawMeasurement(0);
        const CDCRecoHit* cdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtr);
        if (!cdcRecoHit) continue;
        const CDCHit* cdcHit = cdcRecoHit->getCDCHit();

        // get the poca on the wire and track momentum for this hit
        // make sure the fitter info exists
        const genfit::AbsFitterInfo* fi = (*tp)->getFitterInfo(trackrep);
        if (!fi) {
          B2WARNING("No fitter info, skipping...");
          continue;
        }

        // get the global wire ID (between 0 and 14336) and the layer info
        WireID wireID = cdcRecoHit->getWireID();
        const int wire = wireID.getIWire();
        int layer = cdcHit->getILayer();
        int superlayer = cdcHit->getISuperLayer();
        int currentLayer = (superlayer == 0) ? layer : (8 + (superlayer - 1) * 6 + layer);

        // if multiple hits in a layer, we may combine the hits
        const bool lastHit = (tp + 1 == gftrackPoints.end());
        bool lastHitInCurrentLayer = lastHit;
        if (!lastHit) {
          //peek at next hit
          genfit::AbsMeasurement* aAbsMeasurementPtr = (*(tp + 1))->getRawMeasurement(0);
          const CDCRecoHit* nextcdcRecoHit = dynamic_cast<const CDCRecoHit* >(aAbsMeasurementPtr);
          if (!nextcdcRecoHit) {
            lastHitInCurrentLayer = true;
            break;
          }
          const CDCHit* nextcdcHit = nextcdcRecoHit->getCDCHit();
          const int nextILayer = nextcdcHit->getILayer();
          const int nextSuperlayer = nextcdcHit->getISuperLayer();
          const int nextLayer = (nextSuperlayer == 0) ? nextILayer : (8 + (nextSuperlayer - 1) * 6 + nextILayer);
          lastHitInCurrentLayer = (nextLayer != currentLayer);
        }

        // find the position of the endpoints of the sense wire
        const TVector3& wirePosF = cdcgeo.wireForwardPosition(wireID);
        const TVector3& wirePosB = cdcgeo.wireBackwardPosition(wireID);
        const TVector3 wireDir = (wirePosB - wirePosF).Unit();

        int nWires = cdcgeo.nWiresInLayer(currentLayer);

        // radii of field wires for this layer
        double inner = cdcgeo.innerRadiusWireLayer()[currentLayer];
        double outer = cdcgeo.outerRadiusWireLayer()[currentLayer];

        double topHeight = outer - wirePosF.Perp();
        double bottomHeight = wirePosF.Perp() - inner;
        double cellHeight = topHeight + bottomHeight;
        double topHalfWidth = PI * outer / nWires;
        double bottomHalfWidth = PI * inner / nWires;
        double cellHalfWidth = PI * wirePosF.Perp() / nWires;

        // first construct the boundary lines, then create the cell
        const DedxPoint tl = DedxPoint(-topHalfWidth, topHeight);
        const DedxPoint tr = DedxPoint(topHalfWidth, topHeight);
        const DedxPoint br = DedxPoint(bottomHalfWidth, -bottomHeight);
        const DedxPoint bl = DedxPoint(-bottomHalfWidth, -bottomHeight);
        DedxDriftCell c = DedxDriftCell(tl, tr, br, bl);

        // make sure the MOP is reasonable (an exception is thrown for bad numerics)
        try {
          const genfit::MeasuredStateOnPlane& mop = fi->getFittedState();

          // use the MOP to determine the DOCA and entrance angle
          B2Vector3D fittedPoca = mop.getPos();
          const TVector3& pocaMom = mop.getMom();
          if (tp == gftrackPoints.begin() || cdcMom == 0) {
            cdcMom = pocaMom.Mag();
            dedxTrack->m_p_cdc = cdcMom;
          }

          // get the doca and entrance angle information.
          // constructPlane places the coordinate center in the POCA to the
          // wire.  Using this is the default behavior.  If this should be too
          // slow, as it has to re-evaluate the POCA
          //  B2Vector3D pocaOnWire = cdcRecoHit->constructPlane(mop)->getO();

          // uses the plane determined by the track fit.
          B2Vector3D pocaOnWire = mop.getPlane()->getO();

          // The vector from the wire to the track.
          B2Vector3D B2WireDoca = fittedPoca - pocaOnWire;

          // the sign of the doca is defined here to be positive in the +x dir
          double doca = B2WireDoca.Perp();
          if (B2WireDoca.X() < 0) doca = -1.0 * doca;

          // The opening angle of the track momentum direction
          const double px = pocaMom.x();
          const double py = pocaMom.y();
          const double wx = pocaOnWire.x();
          const double wy = pocaOnWire.y();
          const double cross = wx * py - wy * px;
          const double dot   = wx * px + wy * py;
          double entAng = atan2(cross, dot);

          LinearGlobalADCCountTranslator translator;
          double adcCount = cdcHit->getADCCount(); // pedestal subtracted?
          double hitCharge = translator.getCharge(adcCount, wireID, false, pocaOnWire.Z(), pocaMom.Phi());
          int driftT = cdcHit->getTDCCount();

          RealisticTDCCountTranslator realistictdc;
          double driftDRealistic = realistictdc.getDriftLength(driftT, wireID, 0, true, pocaOnWire.Z(), pocaMom.Phi(), pocaMom.Theta());
          double driftDRealisticRes = realistictdc.getDriftLengthResolution(driftDRealistic, wireID, true, pocaOnWire.Z(), pocaMom.Phi(),
                                      pocaMom.Theta());

          // now calculate the path length for this hit
          double celldx = c.dx(doca, entAng);
          if (!c.isValid()) continue;

          layerdE += adcCount;
          layerdx += celldx;

          // save individual hits
          double cellDedx = (adcCount / celldx) * sin(trackMom.Theta());
          if (m_enableDebugOutput)
            dedxTrack->addHit(wire, currentLayer, doca, entAng, adcCount, hitCharge, celldx, cellDedx, cellHeight, cellHalfWidth, driftT,
                              driftDRealistic, driftDRealisticRes);
        } catch (genfit::Exception) {
          B2WARNING("Event " << m_eventID << ", Track: " << m_trackID << ": genfit::MeasuredStateOnPlane exception...");
          continue;
        }

        // check if there are any more hits in this layer
        if (lastHitInCurrentLayer) {
          double totalDistance = layerdx / sin(trackMom.Theta());
          double layerDedx = layerdE / totalDistance;

          // save the information for this layer
          if (layerDedx > 0) {
            dedxTrack->addDedx(currentLayer, totalDistance, layerDedx);
            if (!m_pdfFile.empty() and m_useIndividualHits) {
              // use the momentum valid in the cdc
              saveLogLikelihood(dedxTrack->m_cdcLogl, dedxTrack->m_p_cdc, layerDedx, m_pdfs[c_CDC]);
            }
          }

          layerdE = 0;
          layerdx = 0;
        }
      } // end of loop over CDC hits for this track


      if (!m_useIndividualHits or m_enableDebugOutput) {
        calculateMeans(&(dedxTrack->m_dedx_avg[c_CDC]),
                       &(dedxTrack->m_dedx_avg_truncated[c_CDC]),
                       &(dedxTrack->m_dedx_avg_truncated_err[c_CDC]),
                       dedxTrack->dedx);
        const int numDedx = dedxTrack->dedx.size();
        dedxTrack->m_nHits = numDedx;
        const int lowEdgeTrunc = int(numDedx * m_removeLowest);
        const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest));
        dedxTrack->m_nHitsUsed = highEdgeTrunc - lowEdgeTrunc;
      }
    }

    // Now get the genfit::TrackCand to extract the VXD hits
    genfit::TrackCand* gftrackcand = fitResult->getRelatedFrom<genfit::TrackCand>();
    if (!gftrackcand || gftrackcand->getNHits() == 0) {
      B2WARNING("Track has no associated hits, skipping");
      continue;
    }
    gftrackcand->sortHits();

    if (m_usePXD) {
      const std::vector<int>& pxdClusterIDs = gftrackcand->getHitIDs(Const::PXD);
      saveSiHits(dedxTrack.get(), helixAtOrigin, pxdClusters, pxdClusterIDs);
    }

    if (m_useSVD) {
      const std::vector<int>& svdClusterIDs = gftrackcand->getHitIDs(Const::SVD);
      saveSiHits(dedxTrack.get(), helixAtOrigin, svdClusters, svdClusterIDs);
    }

    if (dedxTrack->dedx.empty()) {
      B2WARNING("Found track with no hits, ignoring.");
      continue;
    }

    // calculate likelihoods for truncated mean
    if (!m_useIndividualHits) {
      for (int detector = 0; detector < c_num_detectors; detector++) {
        if (!detectorEnabled(static_cast<Detector>(detector)))
          continue; //unwanted detector

        // for cdc hits, use the momentum valid in the cdc
        saveLogLikelihood((detector == c_CDC) ? dedxTrack->m_cdcLogl : dedxTrack->m_svdLogl,
                          (detector == c_CDC) ? dedxTrack->m_p_cdc : dedxTrack->m_p, dedxTrack->m_dedx_avg_truncated[detector], m_pdfs[detector]);
      }
    }

    if (m_enableDebugOutput) {
      // book the information for this track
      DedxTrack* newDedxTrack = dedxArray.appendNew(*dedxTrack);
      track.addRelationTo(newDedxTrack);
    }

    // save DedxLikelihood
    if (!m_pdfFile.empty()) {
      DedxLikelihood* likelihoodObj = likelihoodArray.appendNew(dedxTrack->m_cdcLogl, dedxTrack->m_svdLogl);
      track.addRelationTo(likelihoodObj);
    }

  } // end of loop over tracks
}

void DedxPIDModule::terminate()
{

  B2INFO("DedxPIDModule exiting after processing " << m_trackID <<
         " tracks in " << m_eventID + 1 << " events.");
}

void DedxPIDModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr,
                                   const std::vector<double>& dedx) const
{
  // Calculate the truncated average by skipping the lowest & highest
  // events in the array of dE/dx values
  std::vector<double> sortedDedx = dedx;
  std::sort(sortedDedx.begin(), sortedDedx.end());

  double truncatedMeanTmp = 0.0;
  double meanTmp = 0.0;
  double sum_of_squares = 0.0;
  int numValuesTrunc = 0;
  const int numDedx = sortedDedx.size();
  const int lowEdgeTrunc = int(numDedx * m_removeLowest);
  const int highEdgeTrunc = int(numDedx * (1 - m_removeHighest));
  for (int i = 0; i < numDedx; i++) {
    meanTmp += sortedDedx[i];
    if (i >= lowEdgeTrunc and i < highEdgeTrunc) {
      truncatedMeanTmp += sortedDedx[i];
      sum_of_squares += sortedDedx[i] * sortedDedx[i];
      numValuesTrunc++;
    }
  }

  if (numDedx != 0) {
    meanTmp /= numDedx;
  }
  if (numValuesTrunc != 0) {
    truncatedMeanTmp /= numValuesTrunc;
  } else {
    truncatedMeanTmp = meanTmp;
  }

  *mean = meanTmp;
  *truncatedMean = truncatedMeanTmp;

  if (numValuesTrunc > 1) {
    *truncatedMeanErr = sqrt(sum_of_squares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(
                          numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}


double DedxPIDModule::getTraversedLength(const PXDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  const TVector3 local_pos(hit->getU(), hit->getV(), 0.0); //z-component is height over the center of the detector plane
  const TVector3& global_pos = sensor.pointToGlobal(local_pos);
  const TVector3& local_momentum = helix->momentum(helix->pathLengthToPoint(global_pos));

  const TVector3& sensor_normal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensor_normal.Angle(local_momentum); //includes theta and phi components

  //I'm assuming there's only one hit per sensor, there are _very_ rare exceptions to that (most likely curlers)
  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}


double DedxPIDModule::getTraversedLength(const SVDCluster* hit, const HelixHelper* helix)
{
  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();
  const VXD::SensorInfoBase& sensor = geo.get(hit->getSensorID());

  TVector3 a, b;
  if (hit->isUCluster()) {
    const float u = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(sensor.getBackwardWidth() / sensor.getWidth(0) * u, -0.5 * sensor.getLength(), 0.0));
    b = sensor.pointToGlobal(TVector3(sensor.getForwardWidth() / sensor.getWidth(0) * u, +0.5 * sensor.getLength(), 0.0));
  } else {
    const float v = hit->getPosition();
    a = sensor.pointToGlobal(TVector3(-0.5 * sensor.getWidth(v), v, 0.0));
    b = sensor.pointToGlobal(TVector3(+0.5 * sensor.getWidth(v), v, 0.0));
  }
  const double path_length = helix->pathLengthToLine(a, b);
  const TVector3& local_momentum = helix->momentum(path_length);

  const TVector3& sensor_normal = sensor.vectorToGlobal(TVector3(0.0, 0.0, 1.0));
  const double angle = sensor_normal.Angle(local_momentum); //includes theta and phi components

  return TMath::Min(sensor.getWidth(), sensor.getThickness() / fabs(cos(angle)));
}


template <class HitClass> void DedxPIDModule::saveSiHits(DedxTrack* track, const HelixHelper& helix,
                                                         const StoreArray<HitClass>& hits, const std::vector<int>& hit_indices) const
{
  const int num_hits = hit_indices.size();
  if (num_hits == 0)
    return;

  static VXD::GeoCache& geo = VXD::GeoCache::getInstance();

  //figure out which detector to assign hits to
  const int current_detector = geo.get(hits[hit_indices.at(0)]->getSensorID()).getType();
  assert(current_detector == VXD::SensorInfoBase::PXD or current_detector == VXD::SensorInfoBase::SVD);
  assert(current_detector <= 1); //used as array index

  std::vector<double> silicon_dedx; //used for averages
  silicon_dedx.reserve(num_hits);

  VxdID prevSensor;
  for (int i = 0; i < num_hits; i++) {
    const HitClass* hit = hits[hit_indices.at(i)];
    if (!hit) {
      B2ERROR(hits.getName() << " index out of bounds!");
      continue;
    }
    const VxdID& currentSensor = hit->getSensorID();
    int layer = -1;
    if (m_enableDebugOutput) {
      layer = -currentSensor.getLayerNumber();
      assert(layer >= -6 && layer < 0);
    }

    //active medium traversed, in cm (can traverse one sensor at most)
    //assumption: Si detectors are close enough to the origin that helix is still accurate
    const double total_distance = getTraversedLength(hit, &helix);
    const float charge = hit->getCharge();
    const float dedx = charge / total_distance;
    if (dedx <= 0) {
      B2WARNING("dE/dx is " << dedx << " in layer " << layer);
    } else if (i == 0 or prevSensor != currentSensor) { //only save once per sensor (u and v hits share charge!)
      prevSensor = currentSensor;
      //store data
      silicon_dedx.push_back(dedx);
      track->m_dedx_avg[current_detector] += dedx;
      track->addDedx(layer, total_distance, dedx);
      if (!m_pdfFile.empty() and m_useIndividualHits) {
        saveLogLikelihood(track->m_svdLogl, track->m_p, dedx, m_pdfs[current_detector]);
      }
    }

    if (m_enableDebugOutput) {
      track->addHit(currentSensor, layer, 0, 0, charge, charge, 0, dedx, 0, 0, 0, 0, 0);
    }
  }

  //save averages averages
  if (!m_useIndividualHits or m_enableDebugOutput) {
    calculateMeans(&(track->m_dedx_avg[current_detector]),
                   &(track->m_dedx_avg_truncated[current_detector]),
                   &(track->m_dedx_avg_truncated_err[current_detector]),
                   silicon_dedx);
  }
}


void DedxPIDModule::saveLogLikelihood(float(&logl)[Const::ChargedStable::c_SetSize], double p, double dedx,
                                      TH2F* const* pdf) const
{
  //all pdfs have the same dimensions
  const Int_t bin_x = pdf[0]->GetXaxis()->FindFixBin(p);
  const Int_t bin_y = pdf[0]->GetYaxis()->FindFixBin(dedx);


  for (unsigned int iPart = 0; iPart < Const::ChargedStable::c_SetSize; iPart++) {
    if (!pdf[iPart]) //might be NULL if m_ignoreMissingParticles is set
      continue;
    double probability = 0.0;

    //check if this is still in the histogram, take overflow bin otherwise
    if (bin_x < 1 or bin_x > pdf[iPart]->GetNbinsX()
        or bin_y < 1 or bin_y > pdf[iPart]->GetNbinsY()) {
      probability = pdf[iPart]->GetBinContent(bin_x, bin_y);
    } else {
      //in normal histogram range
      probability = pdf[iPart]->Interpolate(p, dedx);
    }

    if (probability != probability)
      B2ERROR("probability NAN for a track with p=" << p << " and dedx=" << dedx);

    //my pdfs aren't perfect...
    if (probability == 0.0)
      probability = m_useIndividualHits ? (1e-5) : (1e-3); //likelihoods for truncated mean are much higher

    logl[iPart] += log(probability);
  }
}
