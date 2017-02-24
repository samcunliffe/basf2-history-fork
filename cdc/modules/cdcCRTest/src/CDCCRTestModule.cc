/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Dong Van Thanh                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "cdc/modules/cdcCRTest/CDCCRTestModule.h"
#include <TTree.h>
#include <framework/gearbox/Const.h>
#include <framework/core/HistoModule.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>

#include <mdst/dataobjects/TrackFitResult.h>
#include <mdst/dataobjects/Track.h>
#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitStatus.h>
#include <genfit/KalmanFitterInfo.h>
#include <genfit/MeasurementOnPlane.h>
#include <genfit/MeasuredStateOnPlane.h>
#include <genfit/StateOnPlane.h>
#include <set>
#include <boost/format.hpp>
#include <boost/foreach.hpp>

#include <cdc/translators/RealisticCDCGeometryTranslator.h>
#include <cdc/translators/RealisticTDCCountTranslator.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include "TDirectory.h"
#include "TMath.h"
#include <Math/ProbFuncMathCore.h>
#include "iostream"

using namespace std;
using namespace Belle2;
using namespace CDC;
using namespace genfit;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(CDCCRTest)

//                 Implementation

CDCCRTestModule::CDCCRTestModule() : HistoModule()
{
  setDescription("CDC Cosmic ray test module");
  setPropertyFlags(c_ParallelProcessingCertified);  // specify this flag if you need parallel processing
  addParam("CorrectToF", m_ToF, "if true, time of flight will take account in t", true);
  addParam("CorrectToP", m_ToP, "if true, time of Propagation will take account in t", true);
  addParam("RecoTracksColName", m_recoTrackArrayName, "Name of collection hold genfit::Track", std::string(""));
  addParam("histogramDirectoryName", m_histogramDirectoryName,
           "Track fit results histograms will be put into this directory", std::string("trackfit"));
  addParam("NameOfTree", m_treeName, "name of tree in output file", string("tree"));
  addParam("fillExpertHistograms", m_fillExpertHistos, "Fill additional histograms", false);
  addParam("noBFit", m_noBFit, "If true -> #Params ==4, #params ==5 for calculate P-Val", true);
  addParam("plotResidual", m_plotResidual, "plot biased residual, normalized res and xtplot for all layer", false);
  addParam("calExpectedDriftTime", m_calExpectedDriftTime, "if true module will calculate expected drift time, it take a time",
           false);
  addParam("hitEfficiency", m_hitEfficiency, "calculate hit efficiency(Not work now) true:yes false:No", false);
  addParam("TriggerPos", m_TriggerPos, "Trigger position use for cut and reconstruct Trigger image", std::vector<double> { -0.6, -13.25, 17.3});
  addParam("NormTriggerPlaneDirection", m_TriggerPlaneDirection, "Normal trigger plane direction and reconstruct Trigger image",
           std::vector<double> { 0, 1, 0});
  addParam("TriggerSize", m_TriggerSize, "Trigger Size, (Width x length)", std::vector<double> {100, 50});
  addParam("IwireLow", m_low, "Lower boundary of hit dist. Histogram", std::vector<int> {0, 0, 0, 0, 0, 0, 0, 0, 0});
  addParam("IwireUpper", m_up, "Upper boundary of hit dist. Histogram", std::vector<int> {161, 161, 193, 225, 257, 289, 321, 355, 385});
  addParam("StoreCDCSimHitInfo", m_StoreCDCSimHitInfo, "Store simulation info related to hit, driftLeng, flight time,z_onwire",
           false);
  addParam("EstimateResultForUnFittedLayer", m_EstimateResultForUnFittedLayer,
           "Calculate residual for Layer that is set unUseInFit", true);
  addParam("SmallerOutput", m_SmallerOutput, "If true, trigghit position, residual cov,absRes, will not be stored", true);
  addParam("StoreTrackParams", m_StoreTrackParams, "Store Track Parameter or not, it will be multicount for each hit", true);
  addParam("StoreHitDistribution", m_MakeHitDist, "Make hit distribution or not", false);
}

CDCCRTestModule::~CDCCRTestModule()
{
  m_allHistos.clear();
}
//------------------------------------------------------------------
// Function to define histograms
//-----------------------------------------------------------------
void CDCCRTestModule::defineHisto()
{
  m_tree  = new TTree(m_treeName.c_str(), "tree");
  m_tree->Branch("x_mea", &x_mea, "x_mea/D");
  m_tree->Branch("x_u", &x_u, "x_u/D");
  m_tree->Branch("x_b", &x_b, "x_b/D");
  m_tree->Branch("z", &z, "z/D");
  m_tree->Branch("alpha", &alpha, "alpha/D");
  m_tree->Branch("theta", &theta, "theta/D");
  m_tree->Branch("z_prop", &z_prop, "z_prop/D");
  m_tree->Branch("t", &t, "t/D");
  m_tree->Branch("tdc", &tdc, "tdc/I");
  m_tree->Branch("adc", &adc, "adc/s");
  m_tree->Branch("boardID", &boardID, "boardID/I");
  m_tree->Branch("lay", &lay, "lay/I");
  m_tree->Branch("weight", &weight, "weight/D");
  m_tree->Branch("IWire", &IWire, "IWire/I");
  m_tree->Branch("Pval", &Pval, "Pval/D");
  m_tree->Branch("ndf", &ndf, "ndf/D");
  m_tree->Branch("trighit", &trighit, "trighit/I");
  if (m_StoreTrackParams) {
    m_tree->Branch("d0", &d0, "d0/D");
    m_tree->Branch("z0", &z0, "z0/D");
    m_tree->Branch("phi0", &phi0, "phi0/D");
    m_tree->Branch("tanL", &tanL, "tanL/D");
    m_tree->Branch("omega", &omega, "omega/D");
  }
  if (m_StoreCDCSimHitInfo) {
    m_tree->Branch("z_sim", &z_sim, "z_sim/D");
    m_tree->Branch("x_sim", &x_sim, "x_sim/D");
    m_tree->Branch("dt_flight_sim", &dt_flight_sim, "dt_flight_sim/D");
  }
  if (m_calExpectedDriftTime) { // expected drift time, calculated form xfit
    m_tree->Branch("t_fit", &t_fit, "t_fit/D");
  }
  if (!m_SmallerOutput) {
    m_tree->Branch("res_b", &res_b, "res_b/D");
    m_tree->Branch("res_u", &res_u, "res_u/D");
    m_tree->Branch("lr", &lr, "lr/I");
    m_tree->Branch("trigHitPos_x", &trigHitPos_x, "trigHitPos_x/D");
    m_tree->Branch("trigHitPos_z", &trigHitPos_z, "trigHitPos_z/D");
    m_tree->Branch("numhits", &numhits, "numhits/I");
    m_tree->Branch("res_b_err", &res_b_err, "res_b_err/D");
    m_tree->Branch("res_u_err", &res_u_err, "res_u_err/D");
    m_tree->Branch("absRes_u", &absRes_u, "absRes_u/D");
    m_tree->Branch("absRes_b", &absRes_b, "absRes_b/D");
    m_tree->Branch("dt_prop", &dt_prop, "dt_prop/D");
    m_tree->Branch("dt_flight", &dt_flight, "dt_flight/D");
  }

  // int N =m_Nchannel;//Number of Wire per Layer used;
  TDirectory* oldDir = gDirectory;
  TDirectory* histDir = oldDir->mkdir(m_histogramDirectoryName.c_str());
  histDir->cd();
  m_hNTracks = getHist("hNTracks", "number of tracks", 3, 0, 3);
  m_hNTracks->GetXaxis()->SetBinLabel(1, "fitted, converged");
  m_hNTracks->GetXaxis()->SetBinLabel(2, "fitted, not converged");
  m_hNTracks->GetXaxis()->SetBinLabel(3, "TrackCand, but no Track");

  m_hNDF = getHist("hNDF", "NDF of fitted track;NDF;Tracks", 71, -1, 70);
  m_hNHits = getHist("hNHits", "#hit of fitted track;#hit;Tracks", 61, -1, 70);
  m_hNHits_trackcand = getHist("hNHits_trackcand", "#hit of track candidate;#hit;Tracks", 71, -1, 70);
  m_hNTracksPerEvent = getHist("hNTracksPerEvent", "#tracks/Event;#Tracks;Event", 20, 0, 20);
  m_hNTracksPerEventFitted = getHist("hNTracksPerEventFitted", "#tracks/Event After Fit;#Tracks;Event", 20, 0, 20);
  m_hE1Dist = getHist("hE1Dist", "Energy Dist. in case 1track/evt; E(Gev);Tracks", 100, 0, 20);
  m_hE2Dist = getHist("hE2Dist", "Energy Dist. in case 2track/evt; E(Gev);Tracks", 100, 0, 20);
  m_hChi2 = getHist("hChi2", "#chi^{2} of tracks;#chi^{2};Tracks", 400, 0, 400);
  m_hPhi0 = getHist("hPhi0", "#Phi_{0} of tracks;#phi_{0} (Degree);Tracks", 400, -190, 190);
  m_hAlpha = getHist("hAlpha", "#alpha Dist.;#alpha (Degree);Hits", 360, -90, 90);
  m_hTheta = getHist("hTheta", "#theta Dist.;#theta (Degree);Hits", 360, 0, 180);
  m_hPval = getHist("hPval", "p-values of tracks;pVal;Tracks", 1000, 0, 1);

  m_hTriggerHitZX =  getHist("TriggerHitZX", "Hit Position on trigger counter;z(cm);x(cm)", 300, -100, 100, 120, -15, 15);
  if (m_MakeHitDist) {
    m_h2DHitDistInCDCHit =  getHist("2DHitDistInCDCHit", " CDCHit;WireID;LayerID",
                                    m_up[8] - m_low[0], m_low[0], m_up[8], 56, 0, 56);
    m_h2DHitDistInTrCand =  getHist("2DHitDistInTrCand", "Track Cand ;WireID;LayerID",
                                    m_up[8] - m_low[0], m_low[0], m_up[8], 56, 0, 56);
    m_h2DHitDistInTrack =   getHist("2DHitDistInTrack", "Fitted Track ;WireID;LayerID",
                                    m_up[8] - m_low[0], m_low[0], m_up[8], 56, 0, 56);
  }
  if (m_fillExpertHistos) {
    m_hNDFChi2 = getHist("hNDFChi2", "#chi^{2} of tracks;NDF;#chi^{2};Tracks", 8, 0, 8, 800, 0, 200);
    m_hNDFPval = getHist("hNDFPval", "p-values of tracks;NDF;pVal;Tracks", 8, 0, 8, 100, 0, 1);
  }
  int sl;
  for (int i = 0; i < 56; ++i) {
    if (m_hitEfficiency) {
      m_hHitEff_soft[i] = getHistProfile(Form("hHitEff_soft_L%d", i),
                                         Form("hit efficiency(soft) of Layer %d ;Drift distance;Software Efficiency", i), 200, -1, 1);
    }
    if (m_MakeHitDist) {
      if (i < 8) {sl = 0;} else { sl = floor((i - 8) / 6) + 1;}
      m_hHitDistInCDCHit[i] = getHist(Form("hHitDistInCDCHit_layer%d", i), Form("Hit Dist. ICLayer_%d;WireID;#Hits", i),
                                      m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
      m_hHitDistInCDCHit[i]->SetLineColor(kGreen);
      m_hHitDistInTrCand[i] = getHist(Form("hHitDistInTrCand_layer%d", i), Form("Hit Dist. ICLayer_%d;WireID;#Hits", i),
                                      m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
      m_hHitDistInTrCand[i]->SetLineColor(kRed);
      m_hHitDistInTrack[i] = getHist(Form("hHitDistInTrack_layer%d", i), Form("Hit Dist. ICLayer_%d;WireID;#Hits", i),
                                     m_up.at(sl) - m_low.at(sl), m_low.at(sl), m_up.at(sl));
    }
    const double normResRange = 20;
    const double residualRange = 0.3;
    std::string title, name;
    if (m_plotResidual) {
      name = (boost::format("hist_ResidualsU%1%") % i).str();
      title = (boost::format("unnormalized, unbiased residuals in layer %1%;cm;Tracks") % i).str();
      m_hResidualU[i] = getHist(name, title, 500, -residualRange, residualRange);

      name = (boost::format("hNormalizedResidualsU%1%") % i).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma (cm);Tracks") % i).str();
      m_hNormalizedResidualU[i] = getHist(name, title, 500, -normResRange, normResRange);

      name = (boost::format("DxDt%1%") % i).str();
      title = (boost::format("Drift Length vs Drift time at Layer_%1%;Drift Length (cm);Drift time (ns)") % i).str();
      m_hDxDt[i] = getHist(name, title, 200, -1, 1, 450, -50, 400);
    }
    if (m_fillExpertHistos) {
      name = (boost::format("hNDFResidualsU%1%") % i).str();
      title = (boost::format("unnormalized, unbiased residuals along U in layer %1%;NDF;cm;Tracks") % i).str();
      m_hNDFResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -residualRange, residualRange);

      name = (boost::format("hNDFNormalizedResidualsU%1%") % i).str();
      title = (boost::format("normalized, unbiased residuals in layer %1%;NDF;#sigma (cm);Tracks") % i).str();
      m_hNDFNormalizedResidualU[i] = getHist(name, title, 8, 0, 8, 1000, -normResRange, normResRange);
    }
  }
  oldDir->cd();
}

void CDCCRTestModule::initialize()
{
  REG_HISTOGRAM
  StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  StoreArray<RecoTrack> recoTracks(m_recoTrackArrayName);
  StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  RelationArray relRecoTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);
  //Store names to speed up creation later
  m_recoTrackArrayName = recoTracks.getName();
  m_trackFitResultArrayName = storeTrackFitResults.getName();
  m_relRecoTrackTrackName = relRecoTrackTrack.getName();

  for (size_t i = 0; i < m_allHistos.size(); ++i) {
    m_allHistos[i]->Reset();
  }
  B2ASSERT("Trigger Position (TriggerPos) must be 3 components.", m_TriggerPos.size() == 3);
  B2ASSERT("Normal vector of Trigger Plane (NormTriggerPlaneDirection) must be 3 components.", m_TriggerPlaneDirection.size() == 3);
  B2ASSERT("Trigger size (TriggerSize) must be 2 component width and length(z direction)", m_TriggerSize.size() == 2);
  B2ASSERT("List of Lower boundary (IWireLow) ( for histo must be 9 components, equivalent 9 supper layers", m_low.size() == 9);
  B2ASSERT("List of Upper boundary (IWireUp) for histo must be 9 components, equivalent 9 supper layers", m_low.size() == 9);
  B2INFO("Trigger Position (" << m_TriggerPos.at(0) << " ," << m_TriggerPos.at(1) << " ," << m_TriggerPos.at(2) << ")");
}

void CDCCRTestModule::beginRun()
{
}

void CDCCRTestModule::event()
{
  const StoreArray<Belle2::Track> storeTrack(m_trackArrayName);
  const StoreArray<Belle2::TrackFitResult> storeTrackFitResults(m_trackFitResultArrayName);
  const StoreArray<Belle2::CDCHit> cdcHits(m_cdcHitArrayName);
  const StoreArray<Belle2::RecoTrack> recoTracks(m_recoTrackArrayName);
  const RelationArray relTrackTrack(recoTracks, storeTrack, m_relRecoTrackTrackName);

  /* CDCHit distribution */
  if (m_MakeHitDist) {
    for (int i = 0; i < cdcHits.getEntries(); ++i) {
      Belle2::CDCHit* hit = cdcHits[i];
      m_hHitDistInCDCHit[getICLayer(hit->getISuperLayer(), hit->getILayer())]->Fill(hit->getIWire());
      m_h2DHitDistInCDCHit->Fill(hit->getIWire(), getICLayer(hit->getISuperLayer(), hit->getILayer()));
    }
  }
  // Loop over Recotracks
  int nTr = recoTracks.getEntries();
  m_hNTracksPerEvent->Fill(nTr);

  int nfitted = 0;

  for (int i = 0; i < nTr; ++i) {
    RecoTrack* track = recoTracks[i];
    if (track->getDirtyFlag()) {B2INFO("Dirty flag was set for track: " << track->getPositionSeed().Y()); continue;}
    m_hNHits_trackcand->Fill(track->getNumberOfCDCHits());
    if (m_MakeHitDist) {
      getHitDistInTrackCand(track);
    }
    if (!track->getTrackFitStatus()->isFitted()) {
      m_hNTracks->Fill("TrackCand, but no Track", 1.0);
      continue;
    }
    const genfit::FitStatus* fs = track->getTrackFitStatus();
    if (!fs || !fs->isFitConverged()) {//not fully convergence
      m_hNTracks->Fill("fitted, not converged", 1.0);
      B2DEBUG(99, "------Fitted but not converged");
      continue;
    }

    m_hNTracks->Fill("fitted, converged", 1.0);
    B2DEBUG(99, "-------Fittted and Conveged");

    nfitted = nfitted + 1;
    /** find results in track fit results**/
    const Belle2::Track* b2track = track->getRelatedFrom<Belle2::Track>();
    if (!b2track) {B2DEBUG(99, "No relation found"); continue;}
    fitresult = b2track->getTrackFitResult(Const::muon);

    if (!fitresult) {
      B2WARNING("track was fitted but Relation not found");
      continue;
    }

    if (nTr == 1) m_hE1Dist->Fill(fitresult->getEnergy());
    if (nTr == 2) m_hE2Dist->Fill(fitresult->getEnergy());

    if (m_noBFit) {ndf = fs->getNdf() + 1;} // incase no Magnetic field, NDF=4;
    else {ndf = fs->getNdf();}

    double Chi2 = fs->getChi2();
    TrPval = std::max(0., ROOT::Math::chisquared_cdf_c(Chi2, ndf));
    d0 = fitresult->getD0();
    z0 = fitresult->getZ0();
    tanL = fitresult->getTanLambda();
    omega = fitresult->getOmega();
    phi0 = fitresult->getPhi0() * 180 / M_PI;

    m_hPhi0->Fill(phi0);
    m_hPval->Fill(TrPval);
    m_hNDF->Fill(ndf);
    m_hChi2->Fill(Chi2);
    if (m_hitEfficiency && track->getNumberOfCDCHits() > 30 && TrPval > 0.001) {
      HitEfficiency(track);
    }
    if (m_fillExpertHistos) {
      m_hNDFChi2->Fill(ndf, fs->getChi2());
      m_hNDFPval->Fill(ndf, TrPval);
    }
    try {
      plotResults(track);
    } catch (...) {
    }

    if (m_EstimateResultForUnFittedLayer) {
      try {
        getResidualOfUnFittedLayer(track);
        //      plotResults(track);
      } catch (...) {
      }
    }
  }

  m_hNTracksPerEventFitted->Fill(nfitted);
}

void CDCCRTestModule::endRun()
{
}

void CDCCRTestModule::terminate()
{
}

void CDCCRTestModule::plotResults(Belle2::RecoTrack* track)
{
  m_trigHitPos = getTriggerHitPosition(track);
  trigHitPos_x = m_trigHitPos.X();
  trigHitPos_z = m_trigHitPos.Z();
  m_hTriggerHitZX->Fill(m_trigHitPos.Z(), m_trigHitPos.X());
  bool hittrig = (sqrt((m_trigHitPos.X() - m_TriggerPos[0]) * (m_trigHitPos.X() - m_TriggerPos[0]) +
                       (m_trigHitPos.Y() - m_TriggerPos[1]) * (m_trigHitPos.Y() - m_TriggerPos[1])) < m_TriggerSize[0] / 2
                  && fabs(m_trigHitPos.Z() - m_TriggerPos[2]) < m_TriggerSize[1] / 2) ? true : false;
  if (hittrig) {trighit = 1;}
  else {trighit = 0;}
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  m_hNHits->Fill(track->getNumberOfCDCHits());

  std::vector<genfit::TrackPoint*> tps = track->getHitPointsWithMeasurement();
  numhits = tps.size();
  BOOST_FOREACH(genfit::TrackPoint * tp, tps) {
    if (!tp->hasRawMeasurements())
      continue;

    const genfit::AbsMeasurement* raw = tp->getRawMeasurement(0);
    const CDCRecoHit* rawCDC = dynamic_cast<const CDCRecoHit*>(raw);
    if (rawCDC) {
      WireID wireid = rawCDC->getWireID();
      const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
      if (!kfi) {B2DEBUG(199, "No Fitter Info: Layer " << wireid.getICLayer()); continue;}

      for (unsigned int iMeas = 0; iMeas < kfi->getNumMeasurements(); ++iMeas) {
        if ((kfi->getWeights().at(iMeas))  > 0.5) {
          const genfit::MeasurementOnPlane& residual_b = kfi->getResidual(iMeas, true);
          const genfit::MeasurementOnPlane& residual_u = kfi->getResidual(iMeas, false);
          lay = wireid.getICLayer();
          IWire = wireid.getIWire();
          if (m_MakeHitDist) {
            m_hHitDistInTrack[lay]->Fill(IWire);
            m_h2DHitDistInTrack->Fill(IWire, lay);
          }
          boardID = cdcgeo.getBoardID(wireid);
          Pval = TrPval;
          tdc = rawCDC->getCDCHit()->getTDCCount();
          adc = rawCDC->getCDCHit()->getADCCount();

          const genfit::MeasuredStateOnPlane& mop = kfi->getFittedState();
          const TVector3 pocaOnWire = mop.getPlane()->getO();//Local wire position
          const TVector3 pocaOnTrack = mop.getPlane()->getU();//residual direction
          const TVector3 pocaMom = mop.getMom();
          alpha = cdcgeo.getAlpha(pocaOnWire, pocaMom) ;
          theta = cdcgeo.getTheta(pocaMom);
          //Convert to outgoing
          x_mea = kfi->getMeasurementOnPlane(iMeas)->getState()(0);
          x_b = kfi->getFittedState(true).getState()(3);// x fit biased
          res_b = residual_b.getState()(0);
          x_u = kfi->getFittedState(false).getState()(3);//x fit unbiased
          res_u = residual_u.getState()(0);
          if (x_u > 0) lr = 1;
          else lr = 0;
          x_mea = copysign(x_mea, x_u);
          if (fabs(alpha) > M_PI / 2) {
            x_b *= -1;
            res_b *= -1;
            x_u *= -1;
            res_u *= -1;
          }

          lr = cdcgeo.getOutgoingLR(lr, alpha);
          theta = cdcgeo.getOutgoingTheta(alpha, theta);
          alpha = cdcgeo.getOutgoingAlpha(alpha);
          B2DEBUG(199, "x_unbiased " << x_u << " |left_right " << lr);
          if (m_calExpectedDriftTime) { t_fit = cdcgeo.getDriftTime(std::abs(x_u), lay, lr, alpha , theta);}
          alpha *= 180 / M_PI;
          theta *= 180 / M_PI;
          m_hAlpha->Fill(alpha);
          m_hTheta->Fill(theta);

          //B2INFO("resi V " <<residual.getState()(1));
          //    weight_res = residual.getWeight();
          absRes_b = std::abs(x_b + res_b) - std::abs(x_b);
          absRes_u = std::abs(x_u + res_u) - std::abs(x_u);
          weight = residual_u.getWeight();
          res_b_err = std::sqrt(residual_b.getCov()(0, 0));
          res_u_err = std::sqrt(residual_u.getCov()(0, 0));

          t = cdcgeo.getT0(wireid) - tdc * cdcgeo.getTdcBinWidth(); // - dt_flight - dt_prop;
          dt_flight = mop.getTime();
          if (dt_flight < 50 && m_ToF) {t -= dt_flight;}
          //          else{B2WARNING("Flight length larger than cdc volume :"<<r_flight); dt_flight}
          //estimate length for propagation
          z = pocaOnWire.Z();
          TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
          z_prop = z - m_backWirePos.Z();
          B2DEBUG(199, "z_prop = " << z_prop << " |z " << z << " |back wire poss: " << m_backWirePos.Z());
          dt_prop = z_prop * cdcgeo.getPropSpeedInv(lay);
          if (z_prop < 240 && m_ToP) {t -= dt_prop;}

          /*Time Walk*/
          t -= cdcgeo.getTimeWalk(wireid, adc);
          //    t = getCorrectedDriftTime(wireid, tdc, adc, z, z0);
          if (m_StoreCDCSimHitInfo) {
            CDCSimHit* simhit = rawCDC->getCDCHit()->getRelated<Belle2::CDCSimHit>();
            if (simhit) {
              x_sim = simhit->getDriftLength();
              z_sim = simhit->getPosWire().Z();
              dt_flight_sim = simhit->getFlightTime();
            }
          }
          m_tree->Fill();
          if (m_plotResidual) {
            m_hDxDt[lay]->Fill(x_u, t);
            m_hResidualU[lay]->Fill(res_b, weight);
            m_hNormalizedResidualU[lay]->Fill(res_b / sqrt(residual_b.getCov()(0, 0)), weight);
          }
          if (m_fillExpertHistos) {
            m_hNDFResidualU[lay]->Fill(ndf, res_b, weight);
            m_hNDFResidualU[lay]->Fill(ndf, res_b);
            m_hNDFNormalizedResidualU[lay]->Fill(ndf, res_b / std::sqrt(residual_b.getCov()(0, 0)), weight);
          }
        } //NDF
        // }//end of if isU
      }//end of for
    }//end of rawCDC
  }//end of for tp
}//end of func


double CDCCRTestModule::getCorrectedDriftTime(Belle2::WireID wireid, unsigned short tdc, unsigned short adc , double z, double z0)
{
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  double t0 =  cdcgeo.getT0(wireid);
  double  tdcBinWidth = cdcgeo.getTdcBinWidth();
  double DriftT = t0 - tdcBinWidth * tdc;
  TVector3 m_backWirePos = cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned);
  const double* rinnerlayer = cdcgeo.innerRadiusWireLayer();
  const double* routerlayer = cdcgeo.outerRadiusWireLayer();
  double  rcell = (rinnerlayer[wireid.getICLayer()] + routerlayer[wireid.getICLayer()]) / 2;
  double  flightLength = sqrt(rcell * rcell + (z - z0) * (z - z0));
  if (flightLength > 200) {
    B2WARNING("flight Length from closet point to hit may be larger than det size " << flightLength
              << "| rcell :" << rcell
              << "| z: " << z);
  }
  B2DEBUG(99, "| rcell :" << rcell << "| z: " << z << " |flightLength" << flightLength);
  //subtract distance divided by speed of electric signal in the wire from the drift time.
  DriftT -= (z - m_backWirePos.Z()) * cdcgeo.getPropSpeedInv(wireid.getICLayer());
  //Correct time of Flight, tentative using distance form hit to IP divide V
  //  DriftT -= mop.getTime();//trackTime
  DriftT -= flightLength / 30; //assume vflight=30cm/ns; flight from IP to this wire;
  if (false) {DriftT -= cdcgeo.getTimeWalk(wireid, adc);}
  return DriftT;
}

void CDCCRTestModule::getHitDistInTrackCand(const RecoTrack* track)
{
  BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * cdchit, track->getCDCHitList()) {
    int iclay = getICLayer(cdchit->getISuperLayer(), cdchit->getILayer());
    B2DEBUG(99, "In TrackCand: ICLayer: " << iclay << "IWire: " << cdchit->getIWire());
    m_hHitDistInTrCand[iclay]->Fill(cdchit->getIWire());
    m_h2DHitDistInTrCand->Fill(cdchit->getIWire(), iclay);
  }
}

TVector3 CDCCRTestModule::getTriggerHitPosition(RecoTrack* track)
{
  TVector3 trigpos(m_TriggerPos.at(0), m_TriggerPos.at(1), m_TriggerPos.at(2));
  TVector3 trigDir(m_TriggerPlaneDirection.at(0), m_TriggerPlaneDirection.at(1), m_TriggerPlaneDirection.at(2));
  const genfit::AbsTrackRep* trackRepresentation = track->getCardinalRepresentation();
  genfit::MeasuredStateOnPlane mop = track->getMeasuredStateOnPlaneClosestTo(trigpos, trackRepresentation);
  TVector3 pos(-200, 200, 200);
  try {
    double l = mop.extrapolateToPlane(genfit::SharedPlanePtr(new genfit::DetPlane(trigpos, trigDir)));
    if (fabs(l) < 1000) pos = mop.getPos();
  } catch (const genfit::Exception& er) {
    B2WARNING("extrapolate to Trigger counter failure" << er.what());
  }
  return pos;
}
void CDCCRTestModule::HitEfficiency(const Belle2::RecoTrack* track)
{
  /*  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  for (int i = 0; i < 56; ++i) {
    double  rcell = (rinnerlayer[i] + routerlayer[i]) / 2;
    double arcL = h.getArcLength2DAtCylindricalR(rcell);
    const TVector3 hitpos = h.getPositionAtArcLength2D(arcL);
    int cellID = cdcgeo.cellId(i, hitpos);
    B2INFO("Hit at LayerID - CellID: " << i << "-" << cellID);
  }
  */
  ////
  BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * cdchit, track->getCDCHitList()) {
    WireID Wid = WireID(cdchit->getID());
    const genfit::TrackPoint* tp = track->getRecoHitInformation(cdchit)->getCreatedTrackPoint();
    //some hit didn't take account in fitting, so I use left/right info frm track finding results.
    int RLInfo = 0;
    RecoHitInformation::RightLeftInformation rightLeftHitInformation = track->getRecoHitInformation(cdchit)->getRightLeftInformation();
    if (rightLeftHitInformation == RecoHitInformation::RightLeftInformation::c_left) {
      RLInfo = -1;
    } else if (rightLeftHitInformation == RecoHitInformation::RightLeftInformation::c_right) {
      RLInfo = 1;
    } else continue;

    if (!tp->hasRawMeasurements())
      continue;
    const genfit::KalmanFitterInfo* kfi = tp->getKalmanFitterInfo();
    if (!kfi) continue;

    //    double max = std::max_element(kfi->getWeights(),kfi->getNumMeasurements());
    double max = 0.;
    unsigned short imea = 0;
    for (unsigned int iMeas = 0; iMeas < kfi->getNumMeasurements(); ++iMeas) {
      double ww = kfi->getWeights().at(iMeas);
      if (ww > max) {max = ww; imea = iMeas;}
    }
    double xx = kfi->getMeasurementOnPlane(imea)->getState()(0);
    m_hHitEff_soft[Wid.getICLayer()]->Fill(std::copysign(xx, RLInfo), max);
  }
  ////
}

void CDCCRTestModule::getResidualOfUnFittedLayer(Belle2::RecoTrack* track)
{
  B2INFO("Start estimate residual for un-fitted layer");
  B2INFO("position seed" << track->getPositionSeed().Y());
  static CDCGeometryPar& cdcgeo = CDCGeometryPar::Instance();
  static CDC::RealisticTDCCountTranslator* tdcTrans = new RealisticTDCCountTranslator(true);
  const genfit::AbsTrackRep* trackRepresentation = track->getCardinalRepresentation();
  x_b = 0; res_b = 0; res_b_err = 0;
  x_u = 0; res_u = 0; res_u_err = 0;
  z = 0; dt_flight_sim = 0; z_prop = 0; t = 0;
  dt_prop = 0; dt_flight = 0; alpha = 0;   theta = 0;
  tdc = 0; adc = 0; lay = 0; IWire = 0;
  //  ndf =0; Pval=0; numhits=0; trigHitPos_x= 0; trigHitPos_z=0;
  //  trighit=1; lr=-1;
  B2INFO("number of cdchit" << track->getCDCHitList().size());
  B2INFO("number of point use int fit" << ndf + 4);
  BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * cdchit, track->getCDCHitList()) {
    RecoHitInformation* recoHitInfo = track->getRecoHitInformation(cdchit);
    //    if (recoHitInfo->useInFit()) continue;
    if ((recoHitInfo->getCreatedTrackPoint())) continue;
    B2DEBUG(99, "HitID use in fit" << recoHitInfo->useInFit());
    int hitID = track->getRecoHitInformation(cdchit)->getSortingParameter();
    int hitID4extr = 0;
    int fsID = 0;
    int bsID = 0;
    //find closest hit to hit which do not fit
    //    if (hitID < track->getNumberOfCDCHits() / 2) { //case for first part of track, searching forward, stop at first choice
    BOOST_FOREACH(const RecoHitInformation::UsedCDCHit * hit, track->getCDCHitList()) {
      if (track->getRecoHitInformation(hit)->useInFit()) { //may be should check fit status of that hit, do it later.
        fsID = track->getRecoHitInformation(hit)->getSortingParameter();
        break;
      }
    }
    //}
    //    if (hitID > track->getNumberOfCDCHits() / 2) { //case for last part of track, searching backward, and stop at the first choice
    BOOST_REVERSE_FOREACH(const RecoHitInformation::UsedCDCHit * hit, track->getCDCHitList()) {
      if (track->getRecoHitInformation(hit)->useInFit()) {
        //    hitID4extr=sortid;
        bsID = track->getRecoHitInformation(hit)->getSortingParameter();
        break;
      }
    }
    B2DEBUG(99, "forward ID: " << fsID << "  |bkwID = " << bsID);
    if (std::fabs(fsID - hitID) < std::fabs(bsID - hitID)) {
      hitID4extr = fsID;
    } else {
      hitID4extr = bsID;
    }
    //}
    B2DEBUG(99, "hitID for extrpolate to " << hitID << " is " << hitID4extr);

    genfit::MeasuredStateOnPlane meaOnPlane = track->getMeasuredStateOnPlaneFromHit(hitID4extr, trackRepresentation);
    //start to extrapolation
    WireID wireid = WireID(cdchit->getID());
    double flightTime1 = meaOnPlane.getTime();
    //Now reconstruct plane for hit
    genfit::SharedPlanePtr plane = constructPlane(meaOnPlane, wireid);
    double segmentLength;
    try {
      segmentLength = meaOnPlane.extrapolateToPlane(plane);
    } catch (const genfit::Exception& e) {
      B2WARNING("Could not extrapolate the fit" << e.what());
      continue;
    }
    IWire = wireid.getIWire();
    lay = wireid.getICLayer();
    const TVector3 pocaOnWire = meaOnPlane.getPlane()->getO();//Local wire position
    const TVector3 pocaMom = meaOnPlane.getMom();
    x_u = meaOnPlane.getState()(3);
    alpha = cdcgeo.getAlpha(pocaOnWire, pocaMom) ;
    theta = cdcgeo.getTheta(pocaMom);
    z = pocaOnWire.Z();
    z_prop = z -  cdcgeo.wireBackwardPosition(wireid, CDCGeometryPar::c_Aligned).Z();
    dt_prop = z_prop / 27.25;
    //Convert to outgoing
    if (x_u > 0) lr = 1;
    else lr = 0;
    if (fabs(alpha) > M_PI / 2) {
      x_u *= -1;
    }
    lr = cdcgeo.getOutgoingLR(lr, alpha);
    theta = cdcgeo.getOutgoingTheta(alpha, theta);
    alpha = cdcgeo.getOutgoingAlpha(alpha);
    dt_flight = meaOnPlane.getTime();
    x_mea = tdcTrans->getDriftLength(cdchit->getTDCCount(), wireid, dt_flight, lr, pocaOnWire.Z(), alpha, theta, cdchit->getADCCount());
    x_mea = std::copysign(x_mea, x_u);
    res_u = x_mea - x_u;
    absRes_u = fabs(x_mea) - fabs(x_u);
    alpha *= 180 / M_PI;
    theta *= 180 / M_PI;
    m_hAlpha->Fill(alpha);
    m_hTheta->Fill(theta);
    if (m_StoreCDCSimHitInfo) {
      CDCSimHit* simhit = cdchit->getRelated<Belle2::CDCSimHit>();
      if (simhit) {
        x_sim = simhit->getDriftLength();
        z_sim = simhit->getPosWire().Z();
        dt_flight_sim = simhit->getFlightTime();
      }
    }
    B2DEBUG(199, "we calculate residua for lay - IWire: " << lay << " - " << IWire);
    B2DEBUG(199, "Layer use for extrapolate and flight time of that hit "
            << WireID(track->getCDCHitList().at(hitID4extr)->getID()).getICLayer()
            << " " << flightTime1);
    B2DEBUG(199, "distance between two hit" << segmentLength);
    B2DEBUG(199, "Flight Time (extra | sim)" << dt_flight << " - " << dt_flight_sim);
    B2DEBUG(199, "DriftLength (cal   | sim)" << x_mea << " - " << x_sim);
    m_tree->Fill();
  }//Boost_foreach.
}

const genfit::SharedPlanePtr CDCCRTestModule::constructPlane(const genfit::MeasuredStateOnPlane& state, WireID m_wireID)
{
  // We reconstruct plane from measuedStateOnPlane from one fitted hit.
  // because I don't want to change state of this plane so I create other state to extrapolate.
  // first: extrapolate to wire (ideal geometry, nosag) to find z pos than get virtual wire pos due to sag,
  // extrapolate again to found z pos and U.
  static CDC::RealisticCDCGeometryTranslator* cdcgeoTrans = new RealisticCDCGeometryTranslator(true);
  const StateOnPlane stateOnPlane = StateOnPlane(state.getState(), state.getPlane(), state.getRep());
  genfit::StateOnPlane st(stateOnPlane);

  const TVector3& Wire1PosIdeal(cdcgeoTrans->getWireBackwardPosition(m_wireID));
  const TVector3& Wire2PosIdeal(cdcgeoTrans->getWireForwardPosition(m_wireID));

  // unit vector of wire direction
  TVector3 WireDirectionIdeal = Wire2PosIdeal - Wire1PosIdeal;
  WireDirectionIdeal.SetMag(1.);//normalized

  // extraplate to find z
  const genfit::AbsTrackRep* rep = state.getRep();
  rep->extrapolateToLine(st, Wire1PosIdeal, WireDirectionIdeal);
  const TVector3& PocaIdeal = rep->getPos(st);

  double zPOCA = (Wire1PosIdeal.Z()
                  + WireDirectionIdeal.Dot(PocaIdeal - Wire1PosIdeal) * WireDirectionIdeal.Z());

  // Now re-extrapolate to new wire direction, wire sag was taking account.
  const TVector3& wire1(cdcgeoTrans->getWireBackwardPosition(m_wireID, zPOCA));
  const TVector3& wire2(cdcgeoTrans->getWireForwardPosition(m_wireID, zPOCA));

  // unit vector of wire direction (include sag)
  TVector3 wireDirection = wire2 - wire1;
  wireDirection.SetMag(1.);

  // extraplate to find poca
  rep->extrapolateToLine(st, wire1, wireDirection);
  const TVector3& poca = rep->getPos(st);
  TVector3 dirInPoca = rep->getMom(st);
  dirInPoca.SetMag(1.);
  const TVector3& pocaOnWire = wire1 + wireDirection.Dot(poca - wire1) * wireDirection;
  if (fabs(wireDirection.Angle(dirInPoca)) < 0.01) {
    B2WARNING("cannot construct det plane, track parallel with wire");
  }
  // construct orthogonal (unit) vector for plane
  const TVector3& U = wireDirection.Cross(dirInPoca);
  genfit::SharedPlanePtr pl = genfit::SharedPlanePtr(new genfit::DetPlane(pocaOnWire, U, wireDirection));
  return pl;
}

