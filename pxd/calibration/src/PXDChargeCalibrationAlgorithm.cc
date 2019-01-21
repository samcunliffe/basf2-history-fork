/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDChargeCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDClusterChargeMapPar.h>

#include <string>
#include <algorithm>
#include <map>
#include <set>

#include <sstream>
#include <iostream>

#include <boost/format.hpp>
#include <cmath>


//ROOT
#include <TRandom.h>
#include <TH1I.h>
#include <TF1.h>


using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by PXDChargeCalibrationAlgorithm class
namespace {

  /** Signal in ADU of collected clusters */
  int m_signal;

  /** Helper function to extract number of bins along u side and v side from counter histogram labels. */
  void getNumberOfBins(const std::shared_ptr<TH1I>& histo_ptr, unsigned short& nBinsU, unsigned short& nBinsV)
  {
    set<unsigned short> uBinSet;
    set<unsigned short> vBinSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      getline(stream, token, '_');
      unsigned short uBin = std::stoi(token);

      getline(stream, token, '_');
      unsigned short vBin = std::stoi(token);

      uBinSet.insert(uBin);
      vBinSet.insert(vBin);
    }

    if (uBinSet.empty() || vBinSet.empty()) {
      B2FATAL("Not able to determine the grid size. Something is wrong with collected data.");
    } else {
      nBinsU = *uBinSet.rbegin() + 1;
      nBinsV = *vBinSet.rbegin() + 1;
    }
  }

  /** Helper function to extract number of sensors from counter histogram labels. */
  unsigned short getNumberOfSensors(const std::shared_ptr<TH1I>& histo_ptr)
  {
    set<unsigned short> sensorSet;

    // Loop over all bins of input histo
    for (auto histoBin = 1; histoBin <= histo_ptr->GetXaxis()->GetNbins(); histoBin++) {
      // The bin label contains the vxdid, uBin and vBin
      string label = histo_ptr->GetXaxis()->GetBinLabel(histoBin);

      // Parse label string format to read sensorID, uBin and vBin
      istringstream  stream(label);
      string token;
      getline(stream, token, '_');
      VxdID sensorID(token);
      sensorSet.insert(sensorID.getID());
    }

    return sensorSet.size();
  }

}


PXDChargeCalibrationAlgorithm::PXDChargeCalibrationAlgorithm():
  CalibrationAlgorithm("PXDClusterChargeCollector"),
  minClusters(5000), noiseSigma(0.6), safetyFactor(2.0), forceContinue(false), strategy(0)
{
  setDescription(
    " ------------------------------ PXDChargeCalibrationAlgorithm -----------------------------------\n"
    "                                                                                               \n"
    "  Algorithm for estimating pxd median/MPV cluster charges for different position on sensor in ADU \n"
    " ------------------------------------------------------------------------------------------------\n"
  );
}




CalibrationAlgorithm::EResult PXDChargeCalibrationAlgorithm::calibrate()
{

  // Get counter histogram
  auto cluster_counter = getObjectPtr<TH1I>("PXDClusterCounter");

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(cluster_counter);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(cluster_counter, nBinsU, nBinsV);

  // Check that we have collected enough Data
  if (cluster_counter->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    if (not forceContinue) {
      B2INFO("Not enough Data: Only " <<  cluster_counter->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
             nSensors * nBinsU * nBinsV) << " needed!");
      return c_NotEnoughData;
    } else {
      B2INFO("Continue despite low statistics: Only " <<  cluster_counter->GetEntries() << " hits were collected but" << int(
               safetyFactor * minClusters *
               nSensors * nBinsU * nBinsV) << " would be desirable!");
    }
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");

  // This is the PXD charge calibration payload for conditions DB
  PXDClusterChargeMapPar* chargeMapPar = new PXDClusterChargeMapPar(nBinsU, nBinsV);

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= cluster_counter->GetXaxis()->GetNbins(); histoBin++) {
    // The bin label contains the vxdid, uBin and vBin
    string label = cluster_counter->GetXaxis()->GetBinLabel(histoBin);

    // Parse label string format to read sensorID, uBin and vBin
    istringstream  stream(label);
    string token;
    getline(stream, token, '_');
    VxdID sensorID(token);

    getline(stream, token, '_');
    unsigned short uBin = std::stoi(token);

    getline(stream, token, '_');
    unsigned short vBin = std::stoi(token);

    // Read back the counters for number of collected clusters
    int numberOfDataHits = cluster_counter->GetBinContent(histoBin);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters) {

      // Compute the median cluster charge for the part of PXD
      auto Charge = EstimateCharge(sensorID, uBin, vBin);

      // Store the charge
      chargeMapPar->setContent(sensorID.getID(), uBin, vBin, Charge);
    } else {
      B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                "). Use default value.");
    }
  }

  // Save the charge map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(chargeMapPar, "PXDClusterChargeMapPar");

  B2INFO("PXD Cluster Charge Calibration Successful");
  return c_OK;
}


double PXDChargeCalibrationAlgorithm::EstimateCharge(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{

  // Construct a tree name for requested part of PXD
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();
  const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

  // Vector with cluster signals from collected data
  vector<double> signals;

  // Fill data_signal vector from input data
  auto tree = getObjectPtr<TTree>(treename);
  tree->SetBranchAddress("signal", &m_signal);

  // Loop over tree
  const auto nEntries = tree->GetEntries();
  for (int i = 0; i < nEntries; ++i) {
    tree->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    signals.push_back(m_signal + noise);
  }
  if (strategy == 0) return CalculateMedian(signals);
  if (strategy == 1) return FitLandau(signals);
  else {
    B2ERROR("strategy unavailable, use 0 for medians or 1 for landau fit!");
    return 1.0;
  }
}


double PXDChargeCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
{
  auto size = signals.size();

  if (size == 0) {
    return 0;  // Undefined, really.
  } else {
    sort(signals.begin(), signals.end());
    if (size % 2 == 0) {
      return (signals[size / 2 - 1] + signals[size / 2]) / 2;
    } else {
      return signals[size / 2];
    }
  }
}

double PXDChargeCalibrationAlgorithm::FitLandau(vector<double>& signals)
{
  auto size = signals.size();

  // get max and min values of vector
  int max = *max_element(signals.begin(), signals.end());
  int min = *min_element(signals.begin(), signals.end());


  // create histogram to hold signals
  TH1D* hist_signals = new TH1D("", "", max - min, min, max);
  // create fit function
  TF1* landau = new TF1("landau", "TMath::Landau(x,[0],[1])*[2]", min, max);
  landau->SetParNames("MPV", "sigma", "scale");
  landau->SetParameters(100, 1, 1000);
  // fill histrogram
  for (auto it = signals.begin(); it != signals.end(); ++it) {
    hist_signals->Fill(*it);
  }

  if (size == 0) {
    return 1.0; // Undefined, really.
  } else {
    // restrict fit range to exclude low charge peak
    Int_t status = hist_signals->Fit("landau", "Lq", "", 30, 350);
    double MPV = landau->GetParameter("MPV");

    delete hist_signals;
    delete landau;

    // check fit status
    if (status == 0) return MPV;

    else {
      B2WARNING("Fit failed! using default value.");
      return 1.0;
    }
  }
}
