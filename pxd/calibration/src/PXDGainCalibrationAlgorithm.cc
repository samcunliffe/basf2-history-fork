/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Benjamin Schwenker                                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/calibration/PXDGainCalibrationAlgorithm.h>
#include <pxd/dbobjects/PXDGainMapPar.h>


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

using namespace std;
using boost::format;
using namespace Belle2;


// Anonymous namespace for data objects used by PXDGainCalibrationAlgorithm class
namespace {

  /** Gain of collected clusters */
  float m_gain;
  /** Signal in ADU of collected clusters */
  int m_signal;
  /** Flag for MC data  */
  bool m_isMC;

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


PXDGainCalibrationAlgorithm::PXDGainCalibrationAlgorithm():
  CalibrationAlgorithm("PXDGainCollector"),
  minClusters(1000), noiseSigma(1.0), safetyFactor(2.0)
{
  setDescription(
    " -------------------------- PXDGainCalibrationAlgorithm ---------------------------------\n"
    "                                                                                         \n"
    "  Algorithm for estimating pxd gains (conversion factor from charge to ADU)              \n"
    " ----------------------------------------------------------------------------------------\n"
  );
}




CalibrationAlgorithm::EResult PXDGainCalibrationAlgorithm::calibrate()
{

  // Get counter histograms for MC and Data
  auto counter_MC = getObjectPtr<TH1I>("PXDMCCounter");
  auto counter_Data = getObjectPtr<TH1I>("PXDDataCounter");

  // Make some consistency checks here to assure the Data and MC were collected for
  // the same geometry.

  if (getNumberOfSensors(counter_Data) != getNumberOfSensors(counter_MC)) {
    B2FATAL("Number of sensors in Data and MC collector outputs are different.");
  }

  if (counter_Data->GetXaxis()->GetNbins() != counter_MC->GetXaxis()->GetNbins()) {
    B2FATAL("Number of grid bins in Data and MC collector outputs are different.");
  }

  // Extract number of sensors from counter histograms
  auto nSensors = getNumberOfSensors(counter_Data);

  // Extract the number of grid bins from counter histograms
  unsigned short nBinsU = 0;
  unsigned short nBinsV = 0;
  getNumberOfBins(counter_Data, nBinsU, nBinsV);

  // Check that we have collected enough MC
  if (counter_MC->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    B2INFO("Not enough MC: Only " <<  counter_MC->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
           nSensors  * nBinsU * nBinsV) << " needed!");
    return c_NotEnoughData;
  }

  // Check that we have collected enough Data
  if (counter_Data->GetEntries() < int(safetyFactor * minClusters * nSensors * nBinsU * nBinsV)) {
    B2INFO("Not enough Data: Only " <<  counter_Data->GetEntries() << " hits were collected but " << int(safetyFactor * minClusters *
           nSensors * nBinsU * nBinsV) << " needed!");
    return c_NotEnoughData;
  }

  B2INFO("Start calibration using a " << nBinsU << "x" << nBinsV << " grid per sensor.");

  // This is the PXD gain correction payload for conditions DB
  PXDGainMapPar* gainMapPar = new PXDGainMapPar(nBinsU, nBinsV);

  // Loop over all bins of input histo
  for (auto histoBin = 1; histoBin <= counter_Data->GetXaxis()->GetNbins(); histoBin++) {
    // The bin label contains the vxdid, uBin and vBin
    string label = counter_Data->GetXaxis()->GetBinLabel(histoBin);

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
    int numberOfDataHits = counter_Data->GetBinContent(histoBin);
    int numberOfMCHits = counter_MC->GetBinContent(histoBin);

    // Only perform fitting, when enough data is available
    if (numberOfDataHits >= minClusters && numberOfMCHits >= minClusters) {

      // Estimate the gain on a certain part of PXD
      auto gain = EstimateGain(sensorID, uBin, vBin);

      // Store the gain
      gainMapPar->setContent(sensorID.getID(), uBin, vBin, gain);
    } else {
      if (numberOfMCHits < minClusters) {
        B2WARNING(label << ": Number of mc hits too small for fitting (" << numberOfMCHits << " < " << minClusters <<
                  "). Use default gain.");
      }
      if (numberOfDataHits < minClusters) {
        B2WARNING(label << ": Number of data hits too small for fitting (" << numberOfDataHits << " < " << minClusters <<
                  "). Use default gain.");
      }
    }
  }

  // Save the gain map to database. Note that this will set the database object name to the same as the collector but you
  // are free to change it.
  saveCalibration(gainMapPar, "PXDGainMapPar");

  B2INFO("PXD Gain Calibration Successful");
  return c_OK;
}


double PXDGainCalibrationAlgorithm::EstimateGain(VxdID sensorID, unsigned short uBin, unsigned short vBin)
{

  // Construct a tree name for requested part of PXD
  auto layerNumber = sensorID.getLayerNumber();
  auto ladderNumber = sensorID.getLadderNumber();
  auto sensorNumber = sensorID.getSensorNumber();
  const string treename = str(format("tree_%1%_%2%_%3%_%4%_%5%") % layerNumber % ladderNumber % sensorNumber % uBin % vBin);

  // Vector with cluster signals from collected data
  vector<double> data_signals;

  // Fill data_signal vector from input data
  auto tree_Data = getObjectPtr<TTree>(treename);
  tree_Data->SetBranchAddress("gain", &m_gain);
  tree_Data->SetBranchAddress("signal", &m_signal);
  tree_Data->SetBranchAddress("isMC", &m_isMC);

  // Loop over tree_Data
  const auto nEntries_Data = tree_Data->GetEntries();
  for (int i = 0; i < nEntries_Data; ++i) {
    tree_Data->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    if (m_isMC) {
      //Found MC cluster in data tree. This can happen when using SingleIoV strategy
    } else {
      data_signals.push_back(m_signal + noise);
    }
  }

  // Vector with cluster signals from collected mc
  vector<double> mc_signals;

  vector<Calibration::ExpRun> vecMCRuns;
  // Push in whichever extra (exp,run) you need for this execution.
  vecMCRuns.push_back(std::make_pair(0, 0));
  // You will only get data from these ExpRuns in the tree. NOT necessarily any data from the runs your are executing over.
  auto tree_MC = getObjectPtr<TTree>(treename, vecMCRuns);
  tree_MC->SetBranchAddress("gain", &m_gain);
  tree_MC->SetBranchAddress("signal", &m_signal);
  tree_MC->SetBranchAddress("isMC", &m_isMC);

  // Loop over tree_MC
  const auto nEntries_MC = tree_MC->GetEntries();
  for (int i = 0; i < nEntries_MC; ++i) {
    tree_MC->GetEntry(i);

    double noise = gRandom->Gaus(0.0, noiseSigma);
    if (m_isMC) {
      mc_signals.push_back(m_signal + noise);
    } else {
      B2WARNING("Found data cluster in mc tree. This is very fishy and points to a mistake in your CAF script.");
    }
  }

  auto dataMedian = CalculateMedian(data_signals);
  auto mcMedian = CalculateMedian(mc_signals);

  double gain =  dataMedian / mcMedian;
  return gain;
}


double PXDGainCalibrationAlgorithm::CalculateMedian(vector<double>& signals)
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

