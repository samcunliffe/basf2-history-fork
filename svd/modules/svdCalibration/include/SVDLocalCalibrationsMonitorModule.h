/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <vxd/dataobjects/VxdID.h>

#include <svd/calibration/SVDFADCMaskedStrips.h>
#include <svd/calibration/SVDPulseShapeCalibrations.h>
#include <svd/calibration/SVDNoiseCalibrations.h>
#include <svd/calibration/SVDPedestalCalibrations.h>
//#include <svd/calibration/SVDOccupancyCalibrations.h>
//#include <svd/calibration/SVDHotStripsCalibrations.h>

#include <svd/dataobjects/SVDHistograms.h>

#include <string>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TH1F.h>
#include <TH2F.h>

namespace Belle2 {
  /**
   * Module to produce a list of histogram showing the uploaded local calibration constants
   */
  class SVDLocalCalibrationsMonitorModule : public Module {

  public:

    /**
     * Constructor: Sets the description, the properties and the parameters of the module.
     */
    SVDLocalCalibrationsMonitorModule();

    /** initialize the TTrees */
    virtual void initialize() override;

    /**  check validities of payloads*/
    virtual void beginRun() override;

    /** fill trees and histograms */
    virtual void event() override;

    /** print the payloads uniqueID */
    virtual void endRun() override;

    /** write trees and histograms to the rootfile */
    virtual void terminate() override;


    /* ROOT file related parameters */
    TFile* m_rootFilePtr = nullptr; /**< pointer at root file used for storing histograms */
    TTree* m_tree = nullptr; /**<pointer at tree containing the mean and RMS of calibration constants */
    TTree* m_treeDetailed = nullptr; /**<pointer at tree containing the calibration constants of each strip*/

    //branches
    TBranch* b_run = nullptr; /**< run number*/
    TBranch* b_ladder = nullptr; /**< ladder number*/
    TBranch* b_layer = nullptr; /**< layer number*/
    TBranch* b_sensor = nullptr; /**< sensor number*/
    TBranch* b_side = nullptr; /**< sensor side */
    TBranch* b_strip = nullptr; /**< strip number*/
    TBranch* b_mask = nullptr; /**< strip mask 0/1*/
    TBranch* b_maskAVE = nullptr; /**< average sensor mask*/
    TBranch* b_pedestal = nullptr; /**< strip pedestal*/
    TBranch* b_pedestalAVE = nullptr; /**< average sensor pedestal*/
    TBranch* b_pedestalRMS = nullptr; /**< rms sensor pedestal*/
    TBranch* b_gain = nullptr; /**< strip gain*/
    TBranch* b_gainAVE = nullptr; /**< sensor gain average*/
    TBranch* b_gainRMS = nullptr; /**< sensor gain rms*/
    TBranch* b_noise = nullptr; /**< strip noise (ADC)*/
    TBranch* b_noiseEl = nullptr; /**< strip noise (e-)*/
    TBranch* b_noiseAVE = nullptr; /**< sensor noise average (ADC)*/
    TBranch* b_noiseRMS = nullptr; /**< sensor noise rms (ADC)*/
    TBranch* b_peakTime = nullptr; /**< strip peakTime*/
    TBranch* b_peakTimeAVE = nullptr; /**< sensor peakTime average*/
    TBranch* b_peakTimeRMS = nullptr; /**< sensor peakTime arm*/
    TBranch* b_pulseWidth = nullptr; /**< strip pulse width*/
    TBranch* b_pulseWidthAVE = nullptr; /**< sensor pulse width average*/
    TBranch* b_pulseWidthRMS = nullptr; /**< sensor pulse width rms*/

    //branch variables
    int m_run = -1; /**< run number*/
    int m_layer = -1; /**< layer number*/
    int m_ladder = -1; /**< ladder number */
    int m_sensor = -1; /**< sensor number*/
    int m_side = -1; /**< sensor side*/
    int m_strip = -1; /**< strip number*/
    float m_mask = -1; /**< strip mask 0/1*/
    float m_maskAVE = -1; /**< sensor mask average*/
    float m_noise = -1; /**< strip noise (ADC) */
    float m_noiseEl = -1; /**< strip noise (e-)*/
    float m_noiseAVE = -1; /**< sensor noise average (ADC)*/
    float m_noiseRMS = -1; /**< sensor noise rms (ADC)*/
    float m_pedestal = -1; /**< strip pedestal*/
    float m_pedestalAVE = -1; /**< sensor pedestal average*/
    float m_pedestalRMS = -1; /**< sensor pedestal rms*/
    float m_gain = -1; /**< strip gain*/
    float m_gainAVE = -1; /**< sensor gain average*/
    float m_gainRMS = -1; /**< sensor gain rms*/
    float m_peakTime = -1; /**< strip peak time*/
    float m_peakTimeAVE = -1; /**< sensor peak time average */
    float m_peakTimeRMS = -1; /**< sensor peak time rms*/
    float m_pulseWidth = -1; /**< strip pulse width */
    float m_pulseWidthAVE = -1; /**< sensor pulse width average*/
    float m_pulseWidthRMS = -1; /**< sensor pulse width rms*/

    std::string m_rootFileName = "SVDLocalCalibrationMonitor_output.root";   /**< root file name */

  private:

    SVDFADCMaskedStrips m_MaskedStr; /**< FADC masked strip payload*/
    SVDNoiseCalibrations m_NoiseCal; /**< noise payload*/
    SVDPulseShapeCalibrations m_PulseShapeCal; /**< pulse shape payload*/
    SVDPedestalCalibrations m_PedestalCal; /**< pedestal payload*/
    //    SVDOccupancyCalibrations m_OccCal; /**< occupancy payload*/
    //    SVDHotStripsCalibrations m_HotStripsCal; /**< hot strips calibration*/

    /** MASKS */
    SVDHistograms<TH1F>* m_hMask = nullptr; /**< masked strips histo */
    SVDHistograms<TH2F>* m_h2Mask = nullptr; /**< mask VS strip 2D histo */
    //    SVDHistograms<TProfile>* m_pMask = nullptr; /**< masked strips profile */

    //NOISE
    SVDHistograms<TH1F>* m_hNoise = nullptr; /**< noise (ADC) histo */
    SVDHistograms<TH2F>* m_h2Noise = nullptr; /**< noise (ADC) VS strip 2D histo */
    SVDHistograms<TH1F>* m_hNoiseEl = nullptr; /**< noise in e- histo */
    SVDHistograms<TH2F>* m_h2NoiseEl = nullptr; /**< noise in e- VS strip 2D histo */

    //PEDESTAL
    SVDHistograms<TH1F>* m_hPedestal = nullptr; /**< pedestal (ADC) histo */
    SVDHistograms<TH2F>* m_h2Pedestal = nullptr; /**< pedestal (ADC) VS strip 2D histo */

    //GAIN
    SVDHistograms<TH1F>* m_hGain = nullptr; /**< gain (e-/ADC) histo */
    SVDHistograms<TH2F>* m_h2Gain = nullptr; /**< gain (e-/ADC) VS strip 2D histo */

    //PEAKTIME
    SVDHistograms<TH1F>* m_hPeakTime = nullptr; /**< peakTime (ns) histo */
    SVDHistograms<TH2F>* m_h2PeakTime = nullptr; /**< peakTime (ns) VS strip 2D histo */

    //PULSEWIDTH
    SVDHistograms<TH1F>* m_hPulseWidth = nullptr; /**< peakTime (ns) histo */
    SVDHistograms<TH2F>* m_h2PulseWidth = nullptr; /**< peakTime (ns) VS strip 2D histo */

  };
}

