/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Jake Bennett
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <reconstruction/modules/DedxCorrection/DedxCorrectionModule.h>
#include <reconstruction/dataobjects/DedxCell.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/TrackFitResult.h>

#include <framework/datastore/StoreArray.h>
#include <framework/gearbox/Const.h>
#include <framework/utilities/FileSystem.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <cassert>
#include <cmath>
#include <algorithm>
#include <utility>
#include <stdlib.h>
#include <time.h>

using namespace Belle2;
using namespace Dedx;

REG_MODULE(DedxCorrection)

DedxCorrectionModule::DedxCorrectionModule() : Module()
{

  setDescription("Apply hit level corrections to the dE/dx measurements.");

  addParam("runNumber", m_runNo, "run number", int(-1));

  addParam("removeLowest", m_removeLowest, "portion of events with low dE/dx that should be discarded", double(0.0));
  addParam("removeHighest", m_removeHighest, "portion of events with high dE/dx that should be discarded", double(0.2));

  // !!!!! At some point functionality to import the calibration constants
  // should be added here (for now just use some arbitrary numbers for testing)
  initializeParameters();

}

DedxCorrectionModule::~DedxCorrectionModule() { }

void DedxCorrectionModule::initialize()
{

  // required inputs
  StoreArray<DedxCell>::required();
  StoreArray<Track>::required();
  StoreArray<TrackFitResult>::required();

  // register outputs
  StoreArray<DedxCell>::registerPersistent();

}

void DedxCorrectionModule::event()
{

  // inputs
  StoreArray<DedxCell> dedxCells;
  StoreArray<Track> tracks;

  // outputs
  StoreArray<DedxCell> dedxArray;

  // **************************************************
  //
  //  LOOP OVER EACH DEDX MEASUREMENT (TRACK LEVEL)
  //
  // **************************************************

  for (int iTrack = 0; iTrack < tracks.getEntries(); ++iTrack) {
    Track* track = tracks[iTrack];
    DedxCell* dedxCell = track->getRelatedTo<DedxCell>();
    if (!dedxCell || dedxCell->size() != 0) {
      B2WARNING("No good hits on this track...");
      continue;
    }

    // get pion fit hypothesis for now
    const TrackFitResult* fitResult = track->getTrackFitResult(Const::pion);
    if (!fitResult) {
      B2WARNING("No related fit for this track...");
      continue;
    }


    // **************************************************
    //
    //  LOOP OVER EACH DEDX MEASUREMENT (HIT LEVEL)
    //
    // **************************************************

    int nhits = dedxCell->size();
    for (int i = 0; i < nhits; ++i) {
      double newdedx = StandardCorrection(m_runNo, dedxCell->getWireID(i), dedxCell->getCosTheta(), dedxCell->getDedx(i));
      dedxCell->setDedx(i, newdedx);
    } // end loop over hits

    calculateMeans(
      &(dedxCell->m_dedx_avg),
      &(dedxCell->m_dedx_avg_truncated),
      &(dedxCell->m_dedx_avg_truncated_err),
      dedxCell->m_dEdx);
  } // end loop over tracks
}

void DedxCorrectionModule::terminate()
{

  B2INFO("DedxCorrectionModule exiting...");
}

void DedxCorrectionModule::initializeParameters()
{

  B2INFO("DedxCorrectionModule: initializing calibration constants...");

  // For now just initialize the parameters to an arbitrary values for
  // debugging. Eventually, this should get the constants from the
  // calibration database.
  for (int i = 0; i < NRuns; ++i) {
    m_runGain[i] = 2.0;
  }
  for (int i = 0; i < NCDCWires; ++i) {
    m_wireGain[i] = 2.0;
    m_valid[i] = 1.0;
  }

  // these are arbitrary and should be extracted from the calibration
  m_alpha = 1.35630e-02;
  m_gamma = -6.78907e-04;
  m_delta = 1.18037e-02;
  m_power = 1.66268e+00;
  m_ratio = 9.94775e-01;
}

double DedxCorrectionModule::RunGainCorrection(int runNo, double& dedx) const
{

  if (runNo > 0 && runNo < NRuns && m_runGain[runNo] != 0) {
    double newDedx = dedx / m_runGain[runNo];
    return newDedx;
  } else
    return dedx;
}

double DedxCorrectionModule::WireGainCorrection(int wireID, double& dedx) const
{

  if (m_valid[wireID] && m_wireGain[wireID] != 0) {
    double newDedx = dedx / m_wireGain[wireID];
    return newDedx;
  } else
    return dedx;
}

double DedxCorrectionModule::HadronCorrection(double costheta, double dedx) const
{

  dedx = dedx / 550.00;

  return D2I(costheta, I2D(costheta, 1.00) / 1.00 * dedx) * 550;
}

double DedxCorrectionModule::StandardCorrection(int runNo, int wireID, double costheta, double dedx) const
{

  double temp = dedx;

  temp = RunGainCorrection(runNo, temp);

  temp = WireGainCorrection(wireID, temp);

  temp = HadronCorrection(costheta, temp);

  return temp;
}

double DedxCorrectionModule::D2I(const double& cosTheta, const double& D) const
{

  double absCosTheta   = fabs(cosTheta);
  double projection    = pow(absCosTheta, m_power) + m_delta;
  double chargeDensity = D / projection;
  double numerator     = 1 + m_alpha * chargeDensity;
  double denominator   = 1 + m_gamma * chargeDensity;

  double I = D * m_ratio * numerator / denominator;

  return I;
}

double DedxCorrectionModule::I2D(const double& cosTheta, const double& I) const
{

  double absCosTheta = fabs(cosTheta);
  double projection  = pow(absCosTheta, m_power) + m_delta;

  double a =  m_alpha / projection;
  double b =  1 - m_gamma / projection * (I / m_ratio);
  double c = -1.0 * I / m_ratio;

  if (b == 0 && a == 0) {
    B2WARNING("both a and b coefficiants for hadron correction are 0");
    return I;
  }

  double D = (a != 0) ? (-b + sqrt(b * b - 4.0 * a * c)) / (2.0 * a) : -c / b;
  if (D < 0) {
    B2WARNING("D is less 0! will try another solution");
    D = (a != 0) ? (-b - sqrt(b * b + 4.0 * a * c)) / (2.0 * a) : -c / b;
    if (D < 0) {
      B2WARNING("D is still less 0! just return uncorrectecd value");
      return I;
    }
  }

  return D;
}


void DedxCorrectionModule::calculateMeans(double* mean, double* truncatedMean, double* truncatedMeanErr, const std::vector<double>& dedx) const
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
    *truncatedMeanErr = sqrt(sum_of_squares / double(numValuesTrunc) - truncatedMeanTmp * truncatedMeanTmp) / double(numValuesTrunc - 1);
  } else {
    *truncatedMeanErr = 0;
  }
}
