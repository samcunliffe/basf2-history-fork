/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Laura Zani, Giulia Casarosa                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <svd/modules/svdCalibration/SVDLocalCalibrationsMonitorModule.h>
#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SVDLocalCalibrationsMonitor)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SVDLocalCalibrationsMonitorModule::SVDLocalCalibrationsMonitorModule() : Module()
{
  // Set module properties
  setDescription("Module to produce a list of histograms showing the uploaded calibration constants");

  // Parameter definitions
  addParam("outputFileName", m_rootFileName, "Name of output root file.", std::string("SVDLocalCalibrationMonitor_output.root"));
}

void SVDLocalCalibrationsMonitorModule::beginRun()
{

  // create new root file
  m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");

  //tree initialization
  m_tree = new TTree("calibLocal", "RECREATE");
  b_run = m_tree->Branch("run", &m_run, "run/i");
  b_layer = m_tree->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_tree->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_tree->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_tree->Branch("side", &m_side, "side/i");
  b_maskAVE = m_tree->Branch("maskAVE", &m_maskAVE, "maskAVE/F");
  b_pedestalAVE = m_tree->Branch("pedestalAVE", &m_pedestalAVE, "pedestalAVE/F");
  b_pedestalRMS = m_tree->Branch("pedestalRMS", &m_pedestalRMS, "pedestalRMS/F");
  b_noiseAVE = m_tree->Branch("noiseAVE", &m_noiseAVE, "noiseAVE/F");
  b_noiseRMS = m_tree->Branch("noiseRMS", &m_noiseRMS, "noiseRMS/F");
  b_gainAVE = m_tree->Branch("gainAVE", &m_gainAVE, "gainAVE/F");
  b_gainRMS = m_tree->Branch("gainRMS", &m_gainRMS, "gainRMS/F");
  b_peakTimeAVE = m_tree->Branch("peakTimeAVE", &m_peakTimeAVE, "peakTimeAVE/F");
  b_peakTimeRMS = m_tree->Branch("peakTimeRMS", &m_peakTimeRMS, "peakTimeRMS/F");
  b_pulseWidthAVE = m_tree->Branch("pulseWidthAVE", &m_pulseWidthAVE, "pulseWidthAVE/F");
  b_pulseWidthRMS = m_tree->Branch("pulseWidthRMS", &m_pulseWidthRMS, "pulseWidthRMS/F");

  m_treeDetailed = new TTree("calibLocalDetailed", "RECREATE");
  b_run = m_treeDetailed->Branch("run", &m_run, "run/i");
  b_layer = m_treeDetailed->Branch("layer", &m_layer, "layer/i");
  b_ladder = m_treeDetailed->Branch("ladder", &m_ladder, "ladder/i");
  b_sensor = m_treeDetailed->Branch("sensor", &m_sensor, "sensor/i");
  b_side = m_treeDetailed->Branch("side", &m_side, "side/i");
  b_strip = m_treeDetailed->Branch("strip", &m_strip, "strip/i");
  b_mask = m_treeDetailed->Branch("mask", &m_mask, "mask/F");
  b_noise = m_treeDetailed->Branch("noise", &m_noise, "noise/F");
  b_noiseEl = m_treeDetailed->Branch("noiseEl", &m_noiseEl, "noiseEl/F");
  b_gain = m_treeDetailed->Branch("gain", &m_gain, "gain/F");
  b_pedestal = m_treeDetailed->Branch("pedestal", &m_pedestal, "pedestal/F");
  b_peakTime = m_treeDetailed->Branch("peakTime", &m_peakTime, "peakTime/F");
  b_pulseWidth = m_treeDetailed->Branch("pulseWidth", &m_pulseWidth, "pulseWidth/F");


  if (!m_MaskedStr.isValid())
    B2WARNING("No valid SVDFADCMaskedStrip for the requested IoV");
  if (!m_NoiseCal.isValid())
    B2WARNING("No valid SVDNoiseCalibration for the requested IoV");
  if (!m_PedestalCal.isValid())
    B2WARNING("No valid SVDPedestalCalibration for the requested IoV");
  if (! m_PulseShapeCal.isValid())
    B2WARNING("No valid SVDPulseShapeCalibrations for the requested IoV");
  /*
  if(!m_OccCal.isValid())
    B2WARNING("No valid SVDOccupancyCalibrations for the requested IoV");
  if(!m_HotStripsCal.isValid())
    B2WARNING("No valid SVDHotStripsCalibrations for the requested IoV");
  */


  ///MASKS
  TH1F hMask("masked_L@layerL@ladderS@sensor@view",
             "masked strip in @layer.@ladder.@sensor @view/@side",
             2, -0.5, 1.5);
  hMask.GetXaxis()->SetTitle("isMasked");
  m_hMask = new SVDHistograms<TH1F>(hMask);

  TH2F h2Mask_512("masked2D_512_L@layerL@ladderS@sensor@view",
                  "masked strip in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 4, -0.5, 128 * 4 - 0.5, 2, -0.5, 1.5);
  h2Mask_512.GetYaxis()->SetTitle("isMasked");
  h2Mask_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Mask_768("masked2D_768_L@layerL@ladderS@sensor@view",
                  "masked strip in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 6, -0.5, 128 * 6 - 0.5, 2, -0.5, 1.5);
  h2Mask_768.GetYaxis()->SetTitle("isMasked");
  h2Mask_768.GetXaxis()->SetTitle("cellID");

  m_h2Mask = new SVDHistograms<TH2F>(h2Mask_768, h2Mask_768, h2Mask_768, h2Mask_512);

  ///NOISE ADC
  TH1F hNoise("noiseADC_L@layerL@ladderS@sensor@view",
              "noise in ADC in @layer.@ladder.@sensor @view/@side",
              80, -0.5, 9.5);
  hNoise.GetXaxis()->SetTitle("strip noise (ADC)");
  m_hNoise = new SVDHistograms<TH1F>(hNoise);

  TH2F h2Noise_512("noise2D_512_L@layerL@ladderS@sensor@view",
                   "noise in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                   128 * 4, -0.5, 128 * 4 - 0.5, 80, -0.5, 9.5);
  h2Noise_512.GetYaxis()->SetTitle("strip noise (ADC)");
  h2Noise_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Noise_768("noise2D_768_L@layerL@ladderS@sensor@view",
                   "noise in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                   128 * 6, -0.5, 128 * 6 - 0.5, 80, -0.5, 9.5);
  h2Noise_768.GetYaxis()->SetTitle("strip noise (ADC)");
  h2Noise_768.GetXaxis()->SetTitle("cellID");

  m_h2Noise = new SVDHistograms<TH2F>(h2Noise_768, h2Noise_768, h2Noise_768, h2Noise_512);


  ///NOISE e-
  TH1F hNoiseEl("noiseEl_L@layerL@ladderS@sensor@view",
                "noise in e- in @layer.@ladder.@sensor @view/@side",
                600, -199.5, 1499.5);
  hNoiseEl.GetXaxis()->SetTitle("strip noise (e-)");
  m_hNoiseEl = new SVDHistograms<TH1F>(hNoiseEl);

  TH2F h2NoiseEl_512("noiseEl2D_512_L@layerL@ladderS@sensor@view",
                     "noise in e- in @layer.@ladder.@sensor @view/@side VS strip number",
                     128 * 4, -0.5, 128 * 4 - 0.5, 600, -199.5, 1499.5);
  h2NoiseEl_512.GetYaxis()->SetTitle("strip noise (e-)");
  h2NoiseEl_512.GetXaxis()->SetTitle("cellID");

  TH2F h2NoiseEl_768("noiseEl2D_768_L@layerL@ladderS@sensor@view",
                     "noise in e- in @layer.@ladder.@sensor @view/@side VS strip number",
                     128 * 6, -0.5, 128 * 6 - 0.5, 600, -199.5, 1499.5);
  h2NoiseEl_768.GetYaxis()->SetTitle("strip noise (e-)");
  h2NoiseEl_768.GetXaxis()->SetTitle("cellID");

  m_h2NoiseEl = new SVDHistograms<TH2F>(h2NoiseEl_768, h2NoiseEl_768, h2NoiseEl_768, h2NoiseEl_512);


  ///PEDESTAL ADC
  TH1F hPedestal("pedestalADC_L@layerL@ladderS@sensor@view",
                 "pedestal in ADC in @layer.@ladder.@sensor @view/@side",
                 200, -199.5, 599.5);
  hPedestal.GetXaxis()->SetTitle("strip pedestal (ADC)");
  m_hPedestal = new SVDHistograms<TH1F>(hPedestal);

  TH2F h2Pedestal_512("pedestal2D_512_L@layerL@ladderS@sensor@view",
                      "pedestal in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 4, -0.5, 128 * 4 - 0.5, 200, -199.5, 599.5);
  h2Pedestal_512.GetYaxis()->SetTitle("strip pedestal (ADC)");
  h2Pedestal_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Pedestal_768("pedestal2D_768_L@layerL@ladderS@sensor@view",
                      "pedestal in ADC in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 6, -0.5, 128 * 6 - 0.5, 200, -199.5, 599.5);
  h2Pedestal_768.GetYaxis()->SetTitle("strip pedestal (ADC)");
  h2Pedestal_768.GetXaxis()->SetTitle("cellID");

  m_h2Pedestal = new SVDHistograms<TH2F>(h2Pedestal_768, h2Pedestal_768, h2Pedestal_768, h2Pedestal_512);

  /// 1/GAIN (e-/ADC)
  TH1F hGain("gainADC_L@layerL@ladderS@sensor@view",
             "1/gain in @layer.@ladder.@sensor @view/@side",
             300, -0.5, 499.5);
  hGain.GetXaxis()->SetTitle("strip 1/gain (e-/ADC)");
  m_hGain = new SVDHistograms<TH1F>(hGain);

  TH2F h2Gain_512("gain2D_512_L@layerL@ladderS@sensor@view",
                  "1/gain in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 4, -0.5, 128 * 4 - 0.5, 300, -0.5, 499.5);
  h2Gain_512.GetYaxis()->SetTitle("strip 1/gain (e-/ADC)");
  h2Gain_512.GetXaxis()->SetTitle("cellID");

  TH2F h2Gain_768("gain2D_768_L@layerL@ladderS@sensor@view",
                  "1/gain in @layer.@ladder.@sensor @view/@side VS strip number",
                  128 * 6, -0.5, 128 * 6 - 0.5, 300, -0.5, 499.5);
  h2Gain_768.GetYaxis()->SetTitle("strip 1/gain (e-/ADC)");
  h2Gain_768.GetXaxis()->SetTitle("cellID");

  m_h2Gain = new SVDHistograms<TH2F>(h2Gain_768, h2Gain_768, h2Gain_768, h2Gain_512);

  // PEAKTIME (ns)
  TH1F hPeakTime("peakTime_L@layerL@ladderS@sensor@view",
                 "peakTime in @layer.@ladder.@sensor @view/@side",
                 255, -0.5, 254.5);
  hPeakTime.GetXaxis()->SetTitle("strip peakTime (ns)");
  m_hPeakTime = new SVDHistograms<TH1F>(hPeakTime);

  TH2F h2PeakTime_512("peakTime2D_512_L@layerL@ladderS@sensor@view",
                      "peakTime in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 4, -0.5, 128 * 4 - 0.5, 255, -0.5, 254.5);
  h2PeakTime_512.GetYaxis()->SetTitle("strip peakTime (ns)");
  h2PeakTime_512.GetXaxis()->SetTitle("cellID");

  TH2F h2PeakTime_768("peakTime2D_768_L@layerL@ladderS@sensor@view",
                      "peakTime in @layer.@ladder.@sensor @view/@side VS strip number",
                      128 * 6, -0.5, 128 * 6 - 0.5, 255, -0.5, 254.5);
  h2PeakTime_768.GetYaxis()->SetTitle("strip peakTime (ns)");
  h2PeakTime_768.GetXaxis()->SetTitle("cellID");

  m_h2PeakTime = new SVDHistograms<TH2F>(h2PeakTime_768, h2PeakTime_768, h2PeakTime_768, h2PeakTime_512);

  // PULSE WIDTH (ns)
  TH1F hPulseWidth("pulseWidth_L@layerL@ladderS@sensor@view",
                   "pulseWidth in @layer.@ladder.@sensor @view/@side",
                   255, -0.5, 254.5);
  hPulseWidth.GetXaxis()->SetTitle("strip pulseWidth (ns)");
  m_hPulseWidth = new SVDHistograms<TH1F>(hPulseWidth);

  TH2F h2PulseWidth_512("pulseWidth2D_512_L@layerL@ladderS@sensor@view",
                        "pulseWidth in @layer.@ladder.@sensor @view/@side VS strip number",
                        128 * 4, -0.5, 128 * 4 - 0.5, 255, -0.5, 254.5);
  h2PulseWidth_512.GetYaxis()->SetTitle("strip pulseWidth (ns)");
  h2PulseWidth_512.GetXaxis()->SetTitle("cellID");

  TH2F h2PulseWidth_768("pulseWidth2D_768_L@layerL@ladderS@sensor@view",
                        "pulseWidth in @layer.@ladder.@sensor @view/@side VS strip number",
                        128 * 6, -0.5, 128 * 6 - 0.5, 255, -0.5, 254.5);
  h2PulseWidth_768.GetYaxis()->SetTitle("strip pulseWidth (ns)");
  h2PulseWidth_768.GetXaxis()->SetTitle("cellID");

  m_h2PulseWidth = new SVDHistograms<TH2F>(h2PulseWidth_768, h2PulseWidth_768, h2PulseWidth_768, h2PulseWidth_512);

}

void SVDLocalCalibrationsMonitorModule::event()
{

  StoreObjPtr<EventMetaData> meta;
  m_run = meta->getRun();


  //call for a geometry instance
  VXD::GeoCache& aGeometry = VXD::GeoCache::getInstance();
  std::set<Belle2::VxdID> svdLayers = aGeometry.getLayers(VXD::SensorInfoBase::SVD);
  std::set<Belle2::VxdID>::iterator itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        int layer = itSvdSensors->getLayerNumber();
        int ladder =  itSvdSensors->getLadderNumber();
        int sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(layer, ladder, sensor);
        const SVD::SensorInfo* currentSensorInfo = dynamic_cast<const SVD::SensorInfo*>(&VXD::GeoCache::get(theVxdID));

        m_layer = layer;
        m_ladder = ladder;
        m_sensor = sensor;

        for (m_side = 0; m_side < 2; m_side++) {

          int Ncells = currentSensorInfo->getUCells();
          if (m_side == 0)
            Ncells = currentSensorInfo->getVCells();

          for (m_strip = 0; m_strip < Ncells; m_strip++) {

            m_mask = -1;
            if (m_MaskedStr.isValid())
              m_mask = m_MaskedStr.isMasked(theVxdID, m_side, m_strip);
            m_hMask->fill(theVxdID, m_side, m_mask);
            m_h2Mask->fill(theVxdID, m_side, m_strip, m_mask);

            m_noise = -1;
            m_noiseEl = -1;
            if (m_NoiseCal.isValid()) {
              m_noise = m_NoiseCal.getNoise(theVxdID, m_side, m_strip);
              m_noiseEl = m_NoiseCal.getNoiseInElectrons(theVxdID, m_side, m_strip);
            }
            m_hNoise->fill(theVxdID, m_side, m_noise);
            m_h2Noise->fill(theVxdID, m_side, m_strip, m_noise);
            m_hNoiseEl->fill(theVxdID, m_side, m_noiseEl);
            m_h2NoiseEl->fill(theVxdID, m_side, m_strip, m_noiseEl);

            m_pedestal = -1;
            if (m_PedestalCal.isValid())
              m_pedestal = m_PedestalCal.getPedestal(theVxdID, m_side, m_strip);
            m_hPedestal->fill(theVxdID, m_side, m_pedestal);
            m_h2Pedestal->fill(theVxdID, m_side, m_strip, m_pedestal);

            m_gain = -1;
            if (m_PulseShapeCal.isValid()) {
              m_gain = m_PulseShapeCal.getChargeFromADC(theVxdID, m_side, m_strip, 1/*ADC*/);
              m_peakTime = m_PulseShapeCal.getPeakTime(theVxdID, m_side, m_strip);
              m_pulseWidth = m_PulseShapeCal.getWidth(theVxdID, m_side, m_strip);
            }
            m_hGain->fill(theVxdID, m_side, m_gain);
            m_h2Gain->fill(theVxdID, m_side, m_strip, m_gain);
            m_hPeakTime->fill(theVxdID, m_side, m_peakTime);
            m_h2PeakTime->fill(theVxdID, m_side, m_strip, m_peakTime);
            m_hPulseWidth->fill(theVxdID, m_side, m_pulseWidth);
            m_h2PulseWidth->fill(theVxdID, m_side, m_strip, m_pulseWidth);

            m_treeDetailed->Fill();

          }
        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }

  B2INFO("now computing Mean and RMS of local calibration constants");

  //compute averages and RMS

  itSvdLayers = svdLayers.begin();

  while ((itSvdLayers != svdLayers.end()) && (itSvdLayers->getLayerNumber() != 7)) { //loop on Layers

    std::set<Belle2::VxdID> svdLadders = aGeometry.getLadders(*itSvdLayers);
    std::set<Belle2::VxdID>::iterator itSvdLadders = svdLadders.begin();

    while (itSvdLadders != svdLadders.end()) { //loop on Ladders

      std::set<Belle2::VxdID> svdSensors = aGeometry.getSensors(*itSvdLadders);
      std::set<Belle2::VxdID>::iterator itSvdSensors = svdSensors.begin();
      B2DEBUG(1, "    svd sensor info " << * (svdSensors.begin()));

      while (itSvdSensors != svdSensors.end()) { //loop on sensors
        B2DEBUG(1, "    svd sensor info " << *itSvdSensors);

        m_layer = itSvdSensors->getLayerNumber();
        m_ladder =  itSvdSensors->getLadderNumber();
        m_sensor = itSvdSensors->getSensorNumber();
        Belle2::VxdID theVxdID(m_layer, m_ladder, m_sensor);


        for (m_side = 0; m_side < 2; m_side++) {
          m_maskAVE = (m_hMask->getHistogram(theVxdID, m_side))->GetMean();
          m_pedestalAVE = (m_hPedestal->getHistogram(theVxdID, m_side))->GetMean();
          m_pedestalRMS = (m_hPedestal->getHistogram(theVxdID, m_side))->GetRMS();
          m_noiseAVE = (m_hNoise->getHistogram(theVxdID, m_side))->GetMean();
          m_noiseRMS = (m_hNoise->getHistogram(theVxdID, m_side))->GetRMS();
          m_gainAVE = (m_hGain->getHistogram(theVxdID, m_side))->GetMean();
          m_gainRMS = (m_hGain->getHistogram(theVxdID, m_side))->GetRMS();
          m_peakTimeAVE = (m_hPeakTime->getHistogram(theVxdID, m_side))->GetMean();
          m_peakTimeRMS = (m_hPeakTime->getHistogram(theVxdID, m_side))->GetRMS();
          m_pulseWidthAVE = (m_hPulseWidth->getHistogram(theVxdID, m_side))->GetMean();
          m_pulseWidthRMS = (m_hPulseWidth->getHistogram(theVxdID, m_side))->GetRMS();
          m_tree->Fill();

        }
        ++itSvdSensors;
      }
      ++itSvdLadders;
    }
    ++itSvdLayers;
  }


}

void SVDLocalCalibrationsMonitorModule::endRun()
{
  B2RESULT("******************************************");
  B2RESULT("** UNIQUE IDs of calibration DB objects **");
  B2RESULT("");

  if (m_MaskedStr.isValid())
    B2RESULT("   - SVDFADCMaskedStrips:" << m_MaskedStr.getUniqueID());
  else
    B2WARNING("No valid SVDFADCMaskedStrips for the requested IoV");

  if (m_NoiseCal.isValid())
    B2RESULT("   - SVDNoiseCalibrations:" << m_NoiseCal.getUniqueID());
  else
    B2WARNING("No valid SVDNoiseCalibrations for the requested IoV");

  if (m_PedestalCal.isValid())
    B2RESULT("   - SVDPedestalCalibrations:" << m_PedestalCal.getUniqueID());
  else
    B2WARNING("No valid SVDPedestalCalibrations for the requested IoV");

  if (m_PulseShapeCal.isValid())
    B2RESULT("   - SVDPulseShapeCalibrations:" << m_PulseShapeCal.getUniqueID());
  else
    B2WARNING("No valid SVDPulseShapeCalibrations for the requested IoV");
  //}

  //void SVDLocalCalibrationsMonitorModule::terminate()
  //{

  if (m_rootFilePtr != NULL) {

    m_rootFilePtr->cd();

    //write the tree
    m_treeDetailed->Write();
    m_tree->Write();

    m_rootFilePtr->mkdir("masked_strips");
    m_rootFilePtr->mkdir("pedestal_ADCunits");
    m_rootFilePtr->mkdir("noise_ADCunits");
    m_rootFilePtr->mkdir("noise_electronsCharge");
    m_rootFilePtr->mkdir("gain_electronsCharge");
    m_rootFilePtr->mkdir("peakTime");
    m_rootFilePtr->mkdir("pulseWidth");


    VXD::GeoCache& geoCache = VXD::GeoCache::getInstance();

    for (auto layer : geoCache.getLayers(VXD::SensorInfoBase::SVD))
      for (auto ladder : geoCache.getLadders(layer))
        for (Belle2::VxdID sensor :  geoCache.getSensors(ladder))
          for (int view = SVDHistograms<TH1F>::VIndex ; view < SVDHistograms<TH1F>::UIndex + 1; view++) {

            //writing the histogram list for the masks in ADC units
            m_rootFilePtr->cd("masked_strips");
            (m_hMask->getHistogram(sensor, view))->Write();
            (m_h2Mask->getHistogram(sensor, view))->Write();

            //writing the histogram list for the pedestals in ADC units
            m_rootFilePtr->cd("pedestal_ADCunits");
            (m_hPedestal->getHistogram(sensor, view))->Write();
            (m_h2Pedestal->getHistogram(sensor, view))->Write();

            //writing the histogram list for the noises in ADC units
            m_rootFilePtr->cd("noise_ADCunits");
            (m_hNoise->getHistogram(sensor, view))->Write();
            (m_h2Noise->getHistogram(sensor, view))->Write();

            //writing the histogram list for the noises in electron charge
            m_rootFilePtr->cd("noise_electronsCharge");
            (m_hNoiseEl->getHistogram(sensor, view))->Write();
            (m_h2NoiseEl->getHistogram(sensor, view))->Write();

            //writing the histogram list for the gains in electron charge
            m_rootFilePtr->cd("gain_electronsCharge");
            (m_hGain->getHistogram(sensor, view))->Write();
            (m_h2Gain->getHistogram(sensor, view))->Write();

            //writing the histogram list for the peak times in ns
            m_rootFilePtr->cd("peakTime");
            (m_hPeakTime->getHistogram(sensor, view))->Write();
            (m_h2PeakTime->getHistogram(sensor, view))->Write();

            //writing the histogram list for the pulse widths in ns
            m_rootFilePtr->cd("pulseWidth");
            (m_hPulseWidth->getHistogram(sensor, view))->Write();
            (m_h2PulseWidth->getHistogram(sensor, view))->Write();

          }

    m_rootFilePtr->mkdir("expert");

    m_rootFilePtr->cd("expert");
    m_h2Noise->Write("h2Noise");
    m_h2PulseWidth->Write("h2PulseShape");
    m_h2Pedestal->Write("h2Pedestal");
    m_h2Gain->Write("h2Gain");
    m_h2PeakTime->Write("h2PeakTime");

    m_rootFilePtr->Close();
    B2RESULT("The rootfile containing the list of histograms has been filled and closed [Local].");


  }
}
