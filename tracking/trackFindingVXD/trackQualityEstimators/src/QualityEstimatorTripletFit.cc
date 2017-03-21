/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Felix Metzner, Jonas Wagner                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include "tracking/trackFindingVXD/trackQualityEstimators/QualityEstimatorTripletFit.h"
#include <math.h>

using namespace Belle2;

float QualityEstimatorTripletFit::calcChiSquared(std::vector<Measurement> const& measurements)
{
  const int nTriplets = measurements.size() - 2;

  if (nTriplets < 1) return NAN;

  double combinedChi2 = 0.;

  // values required for pt calculation
  m_R3Ds.clear();
  m_R3Ds.reserve(nTriplets);
  m_sigmaR3DSquareds.clear();
  m_sigmaR3DSquareds.reserve(nTriplets);
  m_alphas.clear();
  m_alphas.reserve(nTriplets);
  m_thetas.clear();
  m_thetas.reserve(nTriplets);

  // looping over all triplets
  for (int i = 0; i < nTriplets; i++) {

    // Three hits relevant for curent triplet
    const TVector3 hit0 = measurements.at(i).position;
    const TVector3 hit1 = measurements.at(i + 1).position;
    const TVector3 hit2 = measurements.at(i + 2).position;

    const double d01sq = pow(hit1.X() - hit0.X(), 2) + pow(hit1.Y() - hit0.Y(), 2);
    const double d12sq = pow(hit2.X() - hit1.X(), 2) + pow(hit2.Y() - hit1.Y(), 2);
    const double d02sq = pow(hit2.X() - hit0.X(), 2) + pow(hit2.Y() - hit0.Y(), 2);

    const double d01 = sqrt(d01sq);
    const double d12 = sqrt(d12sq);
    const double d02 = sqrt(d02sq);

    const double z01 = hit1.Z() - hit0.Z();
    const double z12 = hit2.Z() - hit1.Z();

    const double R_C = (d01 * d12 * d02) / sqrt(-d01sq * d01sq - d12sq * d12sq - d02sq * d02sq + 2 * d01sq * d12sq + 2 * d12sq * d02sq +
                                                2 *
                                                d02sq * d01sq);

    const double Phi1C = 2. * asin(d01 / (2. * R_C));
    const double Phi2C = 2. * asin(d12 / (2. * R_C));
    // TODO Phi1C and Phi2C have 2 solutions (<Pi and >Pi), each, of which the correct one must be chosen!

    const double R3D1C = sqrt(R_C * R_C + (z01 * z01) / (Phi1C * Phi1C));
    const double R3D2C = sqrt(R_C * R_C + (z12 * z12) / (Phi2C * Phi2C));

    const double theta1C = acos(z01 / (Phi1C * R3D1C));
    const double theta2C = acos(z12 / (Phi2C * R3D2C));
    const double theta = (theta1C + theta2C) / 2.;

    double alpha1 = R_C * R_C * Phi1C * Phi1C + z01 * z01;
    alpha1 *= 1. / (0.5 * R_C * R_C * Phi1C * Phi1C * Phi1C / tan(Phi1C / 2.) + z01 * z01);
    double alpha2 = R_C * R_C * Phi2C * Phi2C + z12 * z12;
    alpha2 *= 1. / (0.5 * R_C * R_C * Phi2C * Phi2C * Phi2C / tan(Phi2C / 2.) + z12 * z12);

    const double PhiTilde = - 0.5 * (Phi1C * alpha1 + Phi2C * alpha2);
    const double eta = 0.5 * Phi1C * alpha1 / R3D1C + 0.5 * Phi2C * alpha2 / R3D2C;
    const double ThetaTilde = theta2C - theta1C - (1 - alpha2) / tan(theta2C) + (1 - alpha1) / tan(theta1C);
    const double beta = (1 - alpha2) / (R3D2C * tan(theta2C)) - (1 - alpha1) / (R3D1C * tan(theta1C));

    // Calculation of sigmaMS
    //double bField = getMagneticField();

    /** Using average material budged of SVD sensors for approximation of radiation length
     *  Belle II TDR page 156 states a value of 0.57% X_0.
     *  This approximation is a first approach to the problem and must be checked.
     */
    const double XoverX0 = 0.0057 / sin(theta1C);

    double R3D = - (eta * PhiTilde * sin(theta) * sin(theta) + beta * ThetaTilde);
    R3D *= 1. / (eta * eta * sin(theta) * sin(theta) + beta * beta);
    const double b = 4.5 / m_magneticFieldZ * sqrt(XoverX0);
    const double sigmaMS = 50 * b / R3D;

    double sigmaR3DSquared = pow(sigmaMS, 2) / (pow(eta * sin(theta), 2) + pow(beta, 2));

    double Chi2min = pow(beta * PhiTilde - eta * ThetaTilde, 2);
    Chi2min *= 1. / (sigmaMS * sigmaMS * (eta * eta + beta * beta / pow(sin(theta), 2)));

    // bias correction as proposed in section 2.4 of the paper describing this fit. (see above)
    double delta = (beta * PhiTilde - eta * ThetaTilde) / (eta * PhiTilde * sin(theta) * sin(theta) + beta * ThetaTilde);
    if (8 * delta * delta * sin(theta) * sin(theta) <= 1) {
      R3D *= (0.75 + sqrt(1 - 8 * delta * delta * sin(theta) * sin(theta)) / 4.);
    }

    // required for optional pt calculation
    m_thetas.push_back(theta);
    m_alphas.push_back((alpha1 + alpha2) / 2.);

    // store values for combination
    m_R3Ds.push_back(R3D);
    m_sigmaR3DSquareds.push_back(sigmaR3DSquared);

    combinedChi2 += Chi2min;
  }

  // Calculate average R3D
  double numerator = 0;
  double denominator = 0;
  for (short i = 0; i < nTriplets; ++i) {
    numerator += m_R3Ds.at(i) / m_sigmaR3DSquareds.at(i);
    denominator += 1. / m_sigmaR3DSquareds.at(i);
  }
  m_averageR3D = numerator / denominator;

  // Compare individual R3Ds with average R3D to improve chi2 as presented at:
  // Connecting the Dots, Vienna, Feb 2016 by A. Schoening
  double globalCompatibilityOfR3Ds = 0;
  for (short i = 0; i < nTriplets; ++i) {
    globalCompatibilityOfR3Ds += pow(m_R3Ds.at(i) - m_averageR3D, 2) / m_sigmaR3DSquareds.at(i);
  }

  double const finalChi2 = combinedChi2 + globalCompatibilityOfR3Ds;

  return finalChi2;
}


QualityEstimationResults QualityEstimatorTripletFit::calcCompleteResults(std::vector<Measurement> const& measurements)
{
  // calculate and store chiSquared(calcChiSquared) and curvature(calcCurvature) in m_reuslts.
  QualityEstimatorBase::calcCompleteResults(measurements);
  if (measurements.size() < 3) return m_results;

  m_results.curvatureSign = calcCurvatureSign(measurements);

  // calculate pt and pt_sigma
  double averageThetaPrime = 0;
  for (unsigned short i = 0; i < m_thetas.size(); ++i) {
    averageThetaPrime += m_thetas.at(i) - (m_averageR3D - m_R3Ds.at(i)) * (1 - m_alphas.at(i)) / (tan(m_thetas.at(i)) * m_R3Ds.at(i));
  }
  averageThetaPrime /= m_thetas.size();

  double sum = 0;
  for (unsigned short i = 0; i < m_sigmaR3DSquareds.size(); ++i) {
    sum += pow(m_R3Ds.at(i), 2) / m_sigmaR3DSquareds.at(i);
  }
  double sigmaAverageR3D = m_averageR3D / sqrt(sum);

  m_results.pt = calcPt(m_averageR3D * sin(averageThetaPrime));
  m_results.pt_sigma = calcPt(sigmaAverageR3D * sin(averageThetaPrime));

  return m_results;
}

