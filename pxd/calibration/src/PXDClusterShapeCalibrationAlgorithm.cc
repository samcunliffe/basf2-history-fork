/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kodys                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDClusterShapeCalibrationAlgorithm.h>

#include <TH1F.h>
#include <TH2F.h>
#include <TMath.h>
#include <TRandom.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;

PXDClusterShapeCalibrationAlgorithm::PXDClusterShapeCalibrationAlgorithm() : CalibrationAlgorithm("pxdClusterShapeCalibration")
{
  setDescription("Calibration of position/error corrections based on cluster shape in PXD");
}


Belle2::CalibrationAlgorithm::EResult PXDClusterShapeCalibrationAlgorithm::calibrate()
{
  // cluster will be check if:
  //   is with non-corrected cluster shape, otheway remove from corrections (to prevent multiple-corrections)
  //   is not close borders, otheway remove from corrections
  //   for givig kind of pixel type, u and v angle direction:
  //     is in in-pixel hitmap for giving cluster shape and giving angle, otheway if hitmap is fully empty:
  //        do those steps for full range simulation in-pixel hitmap
  //        if still not in in-pixel hitmap - remove from corrections
  //     look for bias and estimated error to angle in 3 most closes points in calibration tables
  //     apply correction with weigh dependent of distances to table points
  // Tables for every pixel type (8 types for PXD):
  //   2 sets of tables: - for selection of angles and shapes for real data
  //                     - for full range of angles and shapes from simulations
  //   in every set of tables are:
  //       - in-pixel 2D hitmaps for angles and shapes
  //       - u+v corrections of bias for angles and shapes
  //       - u+v error estimations for angles and shapes
  // so we expect u+v directions, 8 kinds of pixels, 18 x 18 angles, 15 shapes = 77760 cases
  //   for Bias correction
  //   for Error estimation (EstimError/Residual)
  // so we expect 8 kinds of pixels, 18 x 18 angles, 15 shapes = 38880 cases
  //   for in-pixel positions 2D maps, each map has 9x9 bins with binary value (0/1)
  // For ~1000 points per histogram we expect 25 millions events (1 event = 2x sensors, u+v coordinates)
  // We expect set of histograms mostly empty so no need any correction for giving parameters
  // Finaly we store for 2 sets of tables:
  //   - for Bias correction:      77760 short values
  //   - for Error estimation:     77760 short values
  //   - for in-pixel positions: 3149280 binary values
  //
  // Storring in database will be in TVectorT format
  // Using will be on boost/multi_array format in unordered map and hash table

  TString name_Case;
  TString name_SourceTree;
  TString name_OutFileCalibrations;
  name_Case = Form("_RealData%i_Track%i_Calib%i_Pixel%i",
                   (int)m_UseRealData, (int)m_UseTracks, m_CalibrationKind, m_PixelKind);
  name_SourceTree = Form("pxdCal");
  name_OutFileCalibrations = Form("pxdClShCal%s.root", name_Case.Data());

  // START - section for variables for DQM and expert histograms
  TString name_OutFileDQM;
  TString name_OutDoExpertHistograms;
  TFile* fDQM = NULL;
  TFile* fExpertHistograms = NULL;
  TString DirPixelKind;
  TString DirShape;
  TH2F** m_histBiasCorrectionU = NULL;
  TH2F** m_histBiasCorrectionV = NULL;
  TH2F** m_histResidualRMSU = NULL;
  TH2F** m_histResidualRMSV = NULL;
  TH2F** m_histBiasCorrectionErrorU = NULL;
  TH2F** m_histBiasCorrectionErrorV = NULL;
  TH2F** m_histErrorEstimationU = NULL;
  TH2F** m_histErrorEstimationV = NULL;
  TH2F** m_histnClusters = NULL;
  TH2F** m_histnClusterFraction = NULL;
  name_OutFileDQM = Form("pxdClShCalDQM%s.root", name_Case.Data());
  name_OutDoExpertHistograms = Form("pxdClShCalHistos%s.root", name_Case.Data());
  // END - section for variables for DQM and expert histograms


  B2INFO("Name of files: callibration: " << name_OutFileCalibrations.Data() << ", DQM: " << name_OutFileDQM.Data() <<
         ", ExpertHistograms: " << name_OutDoExpertHistograms.Data());

  if (m_DoExpertHistograms) {
// Histograms: m_shapes * m_pixelkinds = 15 x 8 = 120 u+v = 240 (1D)
// Binning: angleU x angleV (18 x 18)
// Monitoring values:      BiasCorrection U + V (residual mean)
//                         ResidualRMS U + V (residual RMS)
//                         BiasCorrectionError U + V (error of residual mean)
//                         ErrorEstimation U + V (RMS of (residual / estimated error))
//                         nClusters, nClusterFraction
//
// Special histograms case 2):     2D SigmaUTrack + SigmaVTrack
//                                 2D phiTrack - phiTrue + thetaTrack - thetaTrue
//                                 2D InPixU - InPixUTrue + InPixV - InPixVTrue
//                                 2D ResidUTrack - ResidUTrue + ResidVTrack - ResidVTrue
    fDQM = new TFile(name_OutFileDQM.Data(), "recreate");
    fDQM->mkdir("NoSorting");
    fExpertHistograms = new TFile(name_OutDoExpertHistograms.Data(), "recreate");
    fExpertHistograms->mkdir("NoSorting");

    m_histBiasCorrectionU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histBiasCorrectionV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histResidualRMSU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histResidualRMSV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histBiasCorrectionErrorU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histBiasCorrectionErrorV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histErrorEstimationU = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histErrorEstimationV = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histnClusters = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];
    m_histnClusterFraction = (TH2F**) new TH2F*[m_pixelkinds * m_shapes + 1];

    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      DirPixelKind = Form("PixelKind_%01i_Layer_%i_Sensor_%i_Size_%i", i_pk, (int)((i_pk % 4) / 2) + 1, (int)(i_pk / 4) + 1, i_pk % 2);
      fDQM->mkdir(DirPixelKind.Data());
      fExpertHistograms->mkdir(DirPixelKind.Data());
      for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
        DirShape = Form("%s/Shape_%02i_%s", DirPixelKind.Data(), i_shape,
                        Belle2::PXD::PXDClusterShape::pxdClusterShapeDescription[(Belle2::PXD::pxdClusterShapeType)i_shape].c_str());
        DirShape.ReplaceAll(":", "");
        DirShape.ReplaceAll(" ", "_");
        fDQM->mkdir(DirShape.Data());
        fDQM->cd(DirShape.Data());

        // Bias Correction
        TString HistoName = Form("BiasCorrectionU_PK%01i_Sh%02i", i_pk, i_shape);
        TString HistoTitle = Form("Bias Correction U, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                    m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                    m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("BiasCorrectionV_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Bias Correction V, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                    m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                    m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Residual RMS
        HistoName = Form("ResidualRMSU_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Residual RMS U, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histResidualRMSU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResidualRMSU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResidualRMSU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ResidualRMSV_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Residual RMS V, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histResidualRMSV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                                 m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                                 m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histResidualRMSV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histResidualRMSV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Bias Correction Error
        HistoName = Form("BiasCorrectionErrorU_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Bias Correction Error U, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("BiasCorrectionErrorV_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Bias Correction Error V, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // Error Estimation
        HistoName = Form("ErrorEstimationU_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Error Estimation U, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histErrorEstimationU[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histErrorEstimationU[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histErrorEstimationU[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("ErrorEstimationV_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Error Estimation V, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histErrorEstimationV[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histErrorEstimationV[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histErrorEstimationV[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        // nCluster, nClusterFraction
        HistoName = Form("nClusters_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Clusters, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histnClusters[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histnClusters[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histnClusters[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");
        HistoName = Form("nClusterFraction_PK%01i_Sh%02i", i_pk, i_shape);
        HistoTitle = Form("Cluster Fraction V, pixel kind %01i, shape %02i", i_pk, i_shape);
        m_histnClusterFraction[i_pk * m_shapes + i_shape] = new TH2F(HistoName.Data(), HistoTitle.Data(),
            m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
            m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
        m_histnClusterFraction[i_pk * m_shapes + i_shape]->GetXaxis()->SetTitle("Angle in u [deg]");
        m_histnClusterFraction[i_pk * m_shapes + i_shape]->GetYaxis()->SetTitle("Angle in v [deg]");

        fExpertHistograms->mkdir(DirShape.Data());
      }
    }
    fDQM->cd("NoSorting");

    // Bias Correction
    TString HistoName = Form("BiasCorrectionU");
    TString HistoTitle = Form("Bias CorrectionU, no sorting");
    m_histBiasCorrectionU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("BiasCorrectionV");
    HistoTitle = Form("Bias CorrectionV, no sorting");
    m_histBiasCorrectionV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                              m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                              m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Residual RMS
    HistoName = Form("ResidualRMSU");
    HistoTitle = Form("Residual RMS U, no sorting");
    m_histResidualRMSU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResidualRMSU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ResidualRMSV");
    HistoTitle = Form("Residual RMS V, no sorting");
    m_histResidualRMSV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                           m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                           m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histResidualRMSV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histResidualRMSV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Bias Correction Error
    HistoName = Form("BiasCorrectionErrorU");
    HistoTitle = Form("Bias Correction Error U, no sorting");
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionErrorU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("BiasCorrectionErrorV");
    HistoTitle = Form("Bias Correction Error V, no sorting");
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histBiasCorrectionErrorV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // Error Estimation
    HistoName = Form("ErrorEstimationU");
    HistoTitle = Form("Error Estimation U, no sorting");
    m_histErrorEstimationU[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("ErrorEstimationV");
    HistoTitle = Form("Error Estimation V, no sorting");
    m_histErrorEstimationV[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    // nCluster, nClusterFraction
    HistoName = Form("nClusters");
    HistoTitle = Form("Clusters, no sorting");
    m_histnClusters[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                        m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                        m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histnClusters[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histnClusters[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");
    HistoName = Form("nClusterFraction");
    HistoTitle = Form("Cluster Fraction, no sorting");
    m_histnClusterFraction[m_pixelkinds * m_shapes] = new TH2F(HistoName.Data(), HistoTitle.Data(),
                                                               m_anglesU, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg,
                                                               m_anglesV, -TMath::Pi() * 0.5 / Unit::deg, TMath::Pi() * 0.5 / Unit::deg);
    m_histnClusterFraction[m_pixelkinds * m_shapes]->GetXaxis()->SetTitle("Angle in u [deg]");
    m_histnClusterFraction[m_pixelkinds * m_shapes]->GetYaxis()->SetTitle("Angle in v [deg]");

    fDQM->cd();
    fExpertHistograms->cd();

  }

  int nEntries = getObject<TTree>(name_SourceTree.Data()).GetEntries();

  B2INFO("Entries: " << nEntries);
  B2INFO("UseTracks: " << m_UseTracks <<
         ", UseRealData: " << m_UseRealData <<
         ", CompareTruePointTracks: " << m_CompareTruePointTracks <<
         ", CalibrationKind: " << m_CalibrationKind <<
         ", PixelKind: " << m_PixelKind
        );

  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("event", &m_evt);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("run", &m_run);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("exp", &m_exp);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("pid", &m_procId);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("layer", &m_layer);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("sensor", &m_sensor);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("pixelKind", &m_pixelKind);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("closeEdge", &m_closeEdge);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("shape", &m_shape);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("phiTrack", &m_phiTrack);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("thetaTrack", &m_thetaTrack);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("phiTrue", &m_phiTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("thetaTrue", &m_thetaTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("signal", &m_signal);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("seed", &m_seed);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("InPixUTrue", &m_InPixUTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("InPixVTrue", &m_InPixVTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("InPixU", &m_InPixU);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("InPixV", &m_InPixV);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("ResidUTrue", &m_ResidUTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("ResidVTrue", &m_ResidVTrue);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("SigmaU", &m_SigmaU);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("SigmaV", &m_SigmaV);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("ResidUTrack", &m_ResidUTrack);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("ResidVTrack", &m_ResidVTrack);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("SigmaUTrack", &m_SigmaUTrack);
  getObject<TTree>(name_SourceTree.Data()).SetBranchAddress("SigmaVTrack", &m_SigmaVTrack);


  // create vector for storing on database:

  std::vector<TVectorD*> Correction_Bias(1);
  std::vector<TVectorD*> Correction_ErrorEstimation(1);
  std::vector<TVectorD*> Correction_BiasErr(1);
  std::vector<TVectorD*> InPixelPosition(1);

  Correction_Bias[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_ErrorEstimation[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  Correction_BiasErr[0] = new TVectorD(m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV);
  InPixelPosition[0] = new TVectorD(m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV);


  // create tables for filling and normal using:

  typedef boost::multi_array<double, 5> correction_table_type;
  typedef boost::multi_array<double, 6> inpixel_table_type;

  correction_table_type TCorrection_Bias(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  correction_table_type TCorrection_ErrorEstimation(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  correction_table_type TCorrection_BiasErr(boost::extents[m_shapes][m_pixelkinds][m_dimensions][m_anglesU][m_anglesV]);
  inpixel_table_type TInPixelPosition(boost::extents[m_shapes][m_pixelkinds][m_anglesU][m_anglesV][m_in_pixelU][m_in_pixelV]);


  // presets of vectors and tables:

  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 0.0;
  Correction_Bias[0]->SetElements(ValueCors);
  Correction_BiasErr[0]->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV; i++)
    ValueCors[i] = 1.0;
  Correction_ErrorEstimation[0]->SetElements(ValueCors);
  for (int i = 0; i < m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV; i++)
    ValueInPix[i] = 0.0;
  InPixelPosition[0]->SetElements(ValueInPix);

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            TCorrection_Bias[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
            TCorrection_ErrorEstimation[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 1.0;
            TCorrection_BiasErr[i_shape][i_pk][i_axis][i_angleU][i_angleV] = 0.0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++)
              TInPixelPosition[i_shape][i_pk][i_angleU][i_angleV][i_ipU][i_ipV] = 0.0;

  B2DEBUG(30, "--> Presets done. ");

  //std::unordered_map<key_type, double, key_hash, key_equal> TCorrection_BiasMap;
  map_typeCorrs TCorrection_BiasMap;
  map_typeCorrs TCorrection_ErrorEstimationMap;
  map_typeCorrs TCorrection_BiasMapErr;
  std::unordered_map<key_type2, double, key_hash2, key_equal2> TInPixelPositionMap;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            TCorrection_BiasMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
            TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 1.0;
            TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV)] = 0;
          }
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++)
              TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 0.0;

  B2DEBUG(30, "--> Ordering done. ");

  // presets of vectors and tables:

  //int n_Events = getObject<TTree>(name_SourceTree).GetEntries();
  int nSelRowsTemp = 0;
  for (int i_shape = 0; i_shape < m_shapes; i_shape++) {
    TString cCat;
    TCut c1;
    TCut c2;
    TCut c3a;
    TCut c3b;
    TCut c4a;
    TCut c4b;
    TCut c5;
    TCut cFin;
    cCat = Form("shape == %i", i_shape);
    c1.SetTitle(cCat.Data());
    if (!m_DoExpertHistograms) {
      getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrack:ResidVTrack", c1, "goff");
      nSelRowsTemp = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
      printf("--> sh %i rows %i\n", i_shape, nSelRowsTemp);
      if (nSelRowsTemp < 10000) continue;
    }
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++) {
      cCat = Form("pixelKind == %i", i_pk);
      c2.SetTitle(cCat.Data());
      if (!m_DoExpertHistograms) {
        getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrack:ResidVTrack", c1 + c2, "goff");
        nSelRowsTemp = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
        if (nSelRowsTemp < 1000) continue;
      }
      B2DEBUG(130, "--> Calibration for: " << i_shape << ", " << i_pk);
      printf(".");
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++) {
        double PhiMi = ((TMath::Pi() * i_angleU) / m_anglesU) - (TMath::Pi() / 2.0);
        double PhiMa = ((TMath::Pi() * (i_angleU + 1)) / m_anglesU) - (TMath::Pi() / 2.0);
        if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
          cCat = Form("phiTrack > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrack < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
          cCat = Form("phiTrue > %f", PhiMi);
          c3a.SetTitle(cCat.Data());
          cCat = Form("phiTrue < %f", PhiMa);
          c3b.SetTitle(cCat.Data());
        }
        if (!m_DoExpertHistograms) {
          getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b, "goff");
          nSelRowsTemp = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
          if (nSelRowsTemp < 500) continue;
        }
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
          B2DEBUG(130, "  --> AngleCalibration for: " << i_angleU << ", " << i_angleV);

// there are following combinations:
//   1) Real data:              phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   2) Simulation from tracks: phiTrack, thetaTrack, InPixU, InPixV, ResidUTrack, ResidVTrack, SigmaU, SigmaV, // SigmaUTrack, SigmaVTrack
//   3) Simulation from TrueP.: phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//   4) SpecialSimulation:      phiTrue, thetaTrue, InPixUTrue, InPixVTrue, ResidUTrue, ResidVTrue, SigmaU, SigmaV
//
//  1) m_UseRealData == kTRUE --> m_UseTracks == kTRUE
//  2) m_UseTracks == kTRUE
//  3) m_UseTracks == kFALSE
//  4) m_CalibrationKind == 2 --> m_UseTracks == kFALSE
// so the combination will be use: m_UseTracks == kTRUE / kFALSE
//
// For calibration need to have: InPix, Residual, Sigma
//
// There is general question if we use in-pixel information for selections:
//    - from true prack there is significant and useful information
//    - from tracking information has spread in level sigma 9 microns (preliminary)
//   in-pixel filter will be prepared but not use.
//
// All histograms are optional only for request.
//
// Histograms: m_shapes * m_pixelkinds = 15 x 8 = 120 u+v = 240 (1D)
// Binning: angleU x angleV (18 x 18)
// Monitoring values:      BiasCorrection U + V (residual mean)
//                         ResidualRMS U + V (residual RMS)
//                         BiasCorrectionError U + V (error of residual mean)
//                         ErrorEstimation U + V (RMS of (residual / estimated error))
//                         nClusters, nClusterFraction
//
// Special histograms case 2):     2D SigmaUTrack + SigmaVTrack
//                                 2D phiTrack - phiTrue + thetaTrack - thetaTrue
//                                 2D InPixU - InPixUTrue + InPixV - InPixVTrue
//                                 2D ResidUTrack - ResidUTrue + ResidVTrack - ResidVTrue

          double ThetaMi = ((TMath::Pi() * i_angleV) / m_anglesV) - (TMath::Pi() / 2.0);
          double ThetaMa = ((TMath::Pi() * (i_angleV + 1)) / m_anglesV) - (TMath::Pi() / 2.0);
          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            cCat = Form("thetaTrack > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrack < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            cCat = Form("thetaTrue > %f", ThetaMi);
            c4a.SetTitle(cCat.Data());
            cCat = Form("thetaTrue < %f", ThetaMa);
            c4b.SetTitle(cCat.Data());
          }
          if (!m_DoExpertHistograms) {
            getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrack:ResidVTrack", c1 + c2 + c3a + c3b + c4a + c4b, "goff");
            nSelRowsTemp = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
            if (nSelRowsTemp < 500) continue;
          }
          cCat = Form("closeEdge == 0");
          c5.SetTitle(cCat.Data());

          cFin = c1 && c2 && c3a && c3b && c4a && c4b && c5;

          B2DEBUG(130, "--> Selection criteria: " << cFin.GetTitle());
          if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
            getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrack:ResidVTrack:ResidUTrack/SigmaU:ResidVTrack/SigmaV", cFin, "goff");
          } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
            getObject<TTree>(name_SourceTree.Data()).Draw("ResidUTrue:ResidVTrue:ResidUTrue/SigmaU:ResidVTrue/SigmaV", cFin, "goff");
          }
          //getObject<TTree>(name_SourceTree).GetReadEntry();
          int nSelRows = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
          if (nSelRows) {
            B2DEBUG(30, "--> Selected raws " << nSelRows);
            double* Col1 = getObject<TTree>(name_SourceTree.Data()).GetV1();
            double* Col2 = getObject<TTree>(name_SourceTree.Data()).GetV2();
            double* Col3 = getObject<TTree>(name_SourceTree.Data()).GetV3();
            double* Col4 = getObject<TTree>(name_SourceTree.Data()).GetV4();

            double RetVal;
            double RetValError;
            double RetRMS;
            RetVal = 0;
            RetValError = 0;
            RetRMS = 0;

            if (CalculateCorrection(1, nSelRows, Col1, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetValError;
              if (m_DoExpertHistograms) {
                m_histBiasCorrectionU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            if (CalculateCorrection(1, nSelRows, Col2, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_BiasMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetVal;
              TCorrection_BiasMapErr[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetValError;
              if (m_DoExpertHistograms) {
                m_histBiasCorrectionV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetVal);
                m_histBiasCorrectionErrorV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetValError);
                m_histResidualRMSV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histBiasCorrectionV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histBiasCorrectionV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            if (CalculateCorrection(2, nSelRows, Col3, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 0, i_angleU, i_angleV)] = RetRMS;
              if (m_DoExpertHistograms) {
                m_histErrorEstimationU[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histErrorEstimationU[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histErrorEstimationU[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }
            if (CalculateCorrection(2, nSelRows, Col4, &RetVal, &RetValError, &RetRMS)) {
              TCorrection_ErrorEstimationMap[make_tuple(i_shape, i_pk, 1, i_angleU, i_angleV)] = RetRMS;
              if (m_DoExpertHistograms) {
                m_histErrorEstimationV[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, RetRMS);
                m_histErrorEstimationV[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                    m_histErrorEstimationV[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + 1);
              }
            }

            if (m_DoExpertHistograms) {
              m_histnClusters[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, nSelRows);
              m_histnClusters[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                                                                      m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) + nSelRows);
              m_histnClusterFraction[i_pk * m_shapes + i_shape]->SetBinContent(i_angleU + 1, i_angleV + 1, (float)nSelRows / nEntries);
              m_histnClusterFraction[m_pixelkinds * m_shapes]->SetBinContent(i_angleU + 1, i_angleV + 1,
                  (float)m_histnClusters[m_pixelkinds * m_shapes]->GetBinContent(i_angleU + 1, i_angleV + 1) / nEntries);
            }

            if (m_MinHitsInPixels > 0) if (nSelRows >= m_MinHitsInPixels) {
                double PixSizeU = 50.0; //Unit::um  (TODO use more general way to obtain clustersize?)
                double PixSizeV = 0;      //Unit::um
                if ((i_pk == 0) || (i_pk == 4)) {
                  PixSizeV = 55.0;
                } else if ((i_pk == 1) || (i_pk == 5)) {
                  PixSizeV = 60.0;
                } else if ((i_pk == 2) || (i_pk == 6)) {
                  PixSizeV = 70.0;
                } else if ((i_pk == 3) || (i_pk == 7)) {
                  PixSizeV = 85.0;
                }
                TString sVarexp;
                if ((m_UseRealData == kTRUE) || (m_UseTracks == kTRUE)) {
                  sVarexp = Form("((InPixU / %f) + 0.5) : ((InPixV / %f) + 0.5)", PixSizeU, PixSizeV);
                } else if ((m_CalibrationKind == 2) || (m_UseTracks == kFALSE)) {
                  sVarexp = Form("((InPixUTrue / %f) + 0.5):((InPixVTrue / %f) + 0.5)", PixSizeU, PixSizeV);
                }

                for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++) {
                  for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
                    B2DEBUG(130, "--> InPixCalibration for: " << i_ipU << ", " << i_ipV);

                    TCut c6a;
                    TCut c6b;
                    TCut c7a;
                    TCut c7b;

                    double MinValU = (double)i_ipU / m_in_pixelU;
                    double MaxValU = (double)(i_ipU + 1) / m_in_pixelU;
                    double MinValV = (double)i_ipV / m_in_pixelV;
                    double MaxValV = (double)(i_ipV + 1) / m_in_pixelV;
                    cCat = Form("((InPixU / %f) + 0.5) > %f", PixSizeU, MinValU);
                    c6a.SetTitle(cCat.Data());
                    cCat = Form("((InPixU / %f) + 0.5) < %f", PixSizeU, MaxValU);
                    c6b.SetTitle(cCat.Data());
                    cCat = Form("((InPixV / %f) + 0.5) > %f", PixSizeV, MinValV);
                    c7a.SetTitle(cCat.Data());
                    cCat = Form("((InPixV / %f) + 0.5) < %f", PixSizeV, MaxValV);
                    c7b.SetTitle(cCat.Data());

                    cFin = c6a && c6b && c7a && c7b && c1 && c2 && c3a && c3b && c4a && c4b && c5;
                    //                printf("------------------------------------------> %s <---\n", cFin.GetTitle());

                    getObject<TTree>(name_SourceTree.Data()).Draw(sVarexp.Data(), cFin, "goff");

                    int nSelRows2 = (int)getObject<TTree>(name_SourceTree.Data()).GetSelectedRows();
                    if (nSelRows2 >= m_MinHitsAcceptInPixels) {
                      TInPixelPositionMap[make_tuple(i_shape, i_pk, i_angleU, i_angleV, i_ipU, i_ipV)] = 1.0;
                    }
                    //                double* Col5 = getObject<TTree>(name_SourceTree).GetV1();
                    //                double* Col6 = getObject<TTree>(name_SourceTree).GetV2();
                    //                for (int i = 0; i < nSelRows2; i++) {
                    //                  printf("------------------------------------------> %i %f %f <---\n",i,Col5[i],Col6[i]);
                    //                }
                  }
                }
              }
          }
        }
      }
    }
  }

  B2DEBUG(30, "--> Presets2b done. ");

  auto key = make_tuple(1, 1, 0, 1, 0);
  double& data = TCorrection_BiasMap[key];
  cout << "Test - Old: table " << data << ", vector" << Correction_Bias[0]->GetMatrixArray()[3] << endl;

  Table2Vector(TCorrection_BiasMap, Correction_Bias[0]);
  Table2Vector(TCorrection_ErrorEstimationMap, Correction_ErrorEstimation[0]);
  Table2Vector(TCorrection_BiasMapErr, Correction_BiasErr[0]);
  Table2Vector(TInPixelPositionMap, InPixelPosition[0]);
//  Vector2Table(Correction_Bias[0], TCorrection_BiasMap);
//  Vector2Table(Correction_ErrorEstimation[0], TCorrection_ErrorEstimationMap);
//  Vector2Table(InPixelPosition[0], TInPixelPositionMap);

  cout << "Test - New: table " << data << " or " << TCorrection_BiasMap[key] << ", vector " << Correction_Bias[0]->GetMatrixArray()[3]
       << endl;

  B2DEBUG(30, "--> Save calibration to vectors done. ");

  /*
    printf("--> %f <--\n", Correction_Bias[55]->GetMatrixArray()[3]);
      if (i==55) printf("%f \n", Correction_Bias[i][3]);
    TFile *ff = new TFile("Test.root", "recreate");
    for (int i = 0; i < m_shapes * m_pixelkinds * m_dimensions; i++) {
    }
    ff->Close();

    printf("--> %i %i %i - %i\n",(int)Correction_Bias[55]->GetLwb(), Correction_Bias[55]->GetNrows(), Correction_Bias[55]->GetUpb(), Correction_Bias[55]->GetNoElements());
  */

  if (m_DoExpertHistograms) {
    //  getObject<TTree>(name_SourceTree).Write(name_SourceTree);
    //  getObject<shape>("shape");
    //  getObject<TTree>("shape").Branch();
    fDQM->Write();
    fDQM->Close();
    fExpertHistograms->Write();
    fExpertHistograms->Close();
  }

  // Here to save corrections in TVectorT format, could change later
  // Use TFile for temporary use,
  // Final use with saveCalibration and database.
  TString nameB = Form("Correction_Bias");
  TString nameEE = Form("Correction_ErrorEstimation");
  TString nameIP = Form("InPixelPosition");
  TFile* f = new TFile(name_OutFileCalibrations.Data(), "recreate");
  Correction_Bias[0]->Write(nameB.Data());
  Correction_ErrorEstimation[0]->Write(nameEE.Data());
  InPixelPosition[0]->Write(nameIP.Data());
  f->Close();

  saveCalibration(Correction_Bias[0], nameB.Data());
  saveCalibration(Correction_ErrorEstimation[0], nameEE.Data());
  saveCalibration(InPixelPosition[0], nameIP.Data());

  delete[] ValueCors;
  delete[] ValueInPix;
  return c_OK;
}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeCorrs& TCorrection, TVectorD* CorVector)
{
  Double_t* ValueCors = new Double_t[m_shapes * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            auto key = make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV);
            auto& data = TCorrection[key];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            ValueCors[i_vector] = data;
          }
  CorVector->SetElements(ValueCors);
//  B2DEBUG(30, "--> Inside Table2Vector " << TCorrection[1][0][0][0][8]);
//  B2DEBUG(30, "--> Inside Table2Vector " << CorVector->GetMatrixArray()[3]);

  delete[] ValueCors;

}

void PXDClusterShapeCalibrationAlgorithm::Table2Vector(map_typeInPics& TInPix, TVectorD* InPixVector)
{
  Double_t* ValueInPix = new Double_t[m_shapes * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV];
  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              ValueInPix[i_vector] = data;
            }
  InPixVector->SetElements(ValueInPix);
  delete[] ValueInPix;

}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* CorVector, map_typeCorrs& TCorrection)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_axis = 0; i_axis < m_dimensions; i_axis++)
        for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
          for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++) {
            auto key = make_tuple(i_shape, i_pk, i_axis, i_angleU, i_angleV);
            auto& data = TCorrection[key];
            int i_vector = i_shape * m_pixelkinds * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_pk * m_dimensions * m_anglesU * m_anglesV;
            i_vector += i_axis * m_anglesU * m_anglesV;
            i_vector += i_angleU * m_anglesV;
            i_vector += i_angleV;
            data = CorVector->GetMatrixArray()[i_vector];
          }

  B2DEBUG(30, "--> Inside Vector2Table " << CorVector->GetMatrixArray()[3]);
}

void PXDClusterShapeCalibrationAlgorithm::Vector2Table(TVectorD* InPixVector, map_typeInPics& TInPix)
{

  for (int i_shape = 0; i_shape < m_shapes; i_shape++)
    for (int i_pk = 0; i_pk < m_pixelkinds; i_pk++)
      for (int i_angleU = 0; i_angleU < m_anglesU; i_angleU++)
        for (int i_angleV = 0; i_angleV < m_anglesV; i_angleV++)
          for (int i_ipU = 0; i_ipU < m_in_pixelU; i_ipU++)
            for (int i_ipV = 0; i_ipV < m_in_pixelV; i_ipV++) {
              auto key = make_tuple(i_shape, i_pk, i_angleU, i_angleV, m_in_pixelU, m_in_pixelV);
              auto& data = TInPix[key];
              int i_vector = i_shape * m_pixelkinds * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_pk * m_anglesU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleU * m_anglesV * m_in_pixelU * m_in_pixelV;
              i_vector += i_angleV * m_in_pixelU * m_in_pixelV;
              i_vector += i_ipU * m_in_pixelV;
              i_vector += i_ipV;
              data = InPixVector->GetMatrixArray()[i_vector];
            }

  B2DEBUG(30, "--> Inside Vector2Table " << InPixVector->GetMatrixArray()[3]);
}

int PXDClusterShapeCalibrationAlgorithm::getPhiIndex(float phi)
{
  int ind = (int)(phi + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesU);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::getThetaIndex(float theta)
{
  int ind = (int)(theta + TMath::Pi() / 2.0) / (TMath::Pi() / m_anglesV);
  return ind;
}

int PXDClusterShapeCalibrationAlgorithm::CalculateCorrection(int CorCase, int n, double* array, double* val,
    double* valError, double* rms)
{
  // CorCase: correction case for:
  //   1: Correction_Bias:            MinSamples: 100,  preset = 0, minCorrection = 0.5 micron, MinDistanceInErrors = 3
  //   2: Correction_ErrorEstimation  MinSamples: 100,  preset = 1, minCorrection = 0.05,       MinDistanceInErrors = 3

  int ret = 1;

  int MinSamples;
  double preset;
  double minCorrection;
  double MinDistanceInErrors;
  MinSamples = 100;
  MinDistanceInErrors = 3.0;
  if (CorCase == 1) {
    preset = 0.0;
    minCorrection = 0.5 * Unit::um;
  } else if (CorCase == 2) {
    preset = 1.0;
    minCorrection = 0.05;
  } else {
    return 0;
  }

  *val = preset;
  *valError = 0.0;
  *rms = 1;
//printf("----->n %i MinSamples %i\n",n,MinSamples);

  if (n < MinSamples) return 0;

  float QuantCut = 0.9; // TODO this parameter can be change...
  double quantiles[2];
  double prob[2];
  int nprob = 2;
  Bool_t isSorted = kFALSE;
  prob[0] = (1. - QuantCut) / 2.;
  prob[1] = 1.0 - prob[0];
  TMath::Quantiles(n, nprob, array, quantiles, prob, isSorted, 0, 7);
  for (int iD = 0; iD < n; iD++) {
    if ((array[iD] < quantiles[0]) || (array[iD] > quantiles[1])) {
      for (int iD2 = iD; iD2 < n - 1; iD2++) {
        array[iD2] = array[iD2 + 1];
      }
      iD--;
      n--;
    }
  }
  // TODO method of bootstrap resampling is best one, for medium time we use simplification

  double DevNew = TMath::Mean(n, array);
  double ErrNew = TMath::RMS(n, array);
  ErrNew /= 0.79; // Correction becouse we cut out 10% of array
  double SEM = ErrNew / sqrt(n);

  // condition for using correction:
  if (DevNew < minCorrection) ret = 0;
  if (CorCase == 1) if (DevNew < (MinDistanceInErrors * SEM)) ret = 0;  // apply only for bias

  if (CorCase == 1) printf("----->dev %f err %f sem %f \n", DevNew / Unit::um, ErrNew / Unit::um, SEM / Unit::um);
  else printf("----->dev %f err %f sem %f \n", DevNew, ErrNew, SEM);
  *val = DevNew;
  *valError = SEM;
  *rms = ErrNew;

  return ret;
}

