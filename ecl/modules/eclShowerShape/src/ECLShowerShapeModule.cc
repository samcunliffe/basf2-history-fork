/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module calculates shower shape variables.                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (ferber@physics.ubc.ca)                    *
 *               Guglielmo De Nardo (denardo@na.infn.it)                  *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// THIS MODULE
#include <ecl/modules/eclShowerShape/ECLShowerShapeModule.h>

// FRAMEWORK
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ECL
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>
#include <ecl/geometry/ECLGeometryPar.h>

//Root
#include <TMath.h>

using namespace Belle2;
using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLShowerShape)
REG_MODULE(ECLShowerShapePureCsI)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLShowerShapeModule::ECLShowerShapeModule() : Module()
{
  // Set description
  setDescription("ECLShowerShapeModule: Calculate ECL shower shape variable (e.g. E9E25)");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("rho0", m_rho0,
           "Scaling factor for radial distances in a plane perpendicular to direction to shower. Distances greated than rho0 will be ignored in Zernike moment calculations",
           15.0 * Unit::cm);

}

ECLShowerShapeModule::~ECLShowerShapeModule()
{
}

void ECLShowerShapeModule::initialize()
{
}

void ECLShowerShapeModule::beginRun()
{
  // Do not use this for Database updates, they will not follow the concept of a "run"
  ;
}

void ECLShowerShapeModule::event()
{
  StoreArray<ECLShower> eclShowers(eclShowerArrayName());
  for (auto& eclShower : eclShowers)
    eclShower.setLateralEnergy(computeLateralEnergy(eclShower));

  //---------------------------------------------------------------------
  //Calculate shower shape variables in a plane perpendicular to the shower direction
  //---------------------------------------------------------------------
  for (auto& shower : eclShowers) {
    //Project the digits on the plane perpendicular to the shower direction
    std::vector<ProjectedECLDigit> projectedECLDigits = projectECLDigits(shower);

    //Compute shower shape variables in the plane
    double absZernike20 = computeAbsZernikeMoment(projectedECLDigits, shower.getEnergy(), 2, 0, m_rho0);
    double absZernike40 = computeAbsZernikeMoment(projectedECLDigits, shower.getEnergy(), 4, 0, m_rho0);
    double absZernike42 = computeAbsZernikeMoment(projectedECLDigits, shower.getEnergy(), 4, 2, m_rho0);
    double absZernike51 = computeAbsZernikeMoment(projectedECLDigits, shower.getEnergy(), 5, 1, m_rho0);
    double absZernike53 = computeAbsZernikeMoment(projectedECLDigits, shower.getEnergy(), 5, 3, m_rho0);
    double secondMoment = computeSecondMoment(projectedECLDigits, shower.getEnergy());

    //Set shower shape variables
    shower.setAbsZernike20(absZernike20);
    shower.setAbsZernike40(absZernike40);
    shower.setAbsZernike42(absZernike42);
    shower.setAbsZernike51(absZernike51);
    shower.setAbsZernike53(absZernike53);
    shower.setSecondMoment(secondMoment);
  }
}

std::vector<ECLShowerShapeModule::ProjectedECLDigit> ECLShowerShapeModule::projectECLDigits(const ECLShower& shower) const
{
  std::vector<ProjectedECLDigit> tmpProjectedECLDigits; //Will be returned at the end of the function

  //---------------------------------------------------------------------
  //Get shower parameters
  //---------------------------------------------------------------------
  double showerR = shower.getR();
  double showerTheta = shower.getTheta();
  double showerPhi = shower.getPhi();

  TVector3 showerPosition;
  showerPosition.SetMagThetaPhi(showerR, showerTheta, showerPhi);

  //Unit vector pointing in shower direction
  const TVector3 showerDirection = (1.0 / showerPosition.Mag()) * showerPosition;

  //---------------------------------------------------------------------
  //Calculate axes that span the perpendicular plane
  //---------------------------------------------------------------------
  //xPrimeDirection = showerdirection.cross(zAxis)
  TVector3 xPrimeDirection = TVector3(showerPosition.y(), -showerPosition.x(), 0.0);
  xPrimeDirection *= 1.0 / xPrimeDirection.Mag();

  TVector3 yPrimeDirection = xPrimeDirection.Cross(showerDirection);
  yPrimeDirection *= 1.0 / yPrimeDirection.Mag();

  //---------------------------------------------------------------------
  //Loop on CalDigits in shower and calculate the projection
  //---------------------------------------------------------------------
  auto showerDigitRelations = shower.getRelationsTo<ECLCalDigit>();
  ECLGeometryPar* geometry = ECLGeometryPar::Instance();

  for (unsigned int iRelation = 0; iRelation < showerDigitRelations.size(); ++iRelation) {
    const auto calDigit = showerDigitRelations.object(iRelation);

    ProjectedECLDigit tmpProjectedDigit;

    //---------------------------------------------------------------------
    //Projected digit energy
    //---------------------------------------------------------------------
    const auto weight = showerDigitRelations.weight(iRelation);
    tmpProjectedDigit.energy = weight * calDigit->getEnergy();

    //---------------------------------------------------------------------
    //Projected digit radial distance
    //---------------------------------------------------------------------
    int cellId = calDigit->getCellId();
    TVector3 calDigitPosition = geometry->GetCrystalPos(cellId - 1);

    //Angle between vector pointing to shower and vector pointing to CalDigit,
    //where the orgin is the detector origin (implicitly assuming IP = detector origin)
    double angleDigitShower = calDigitPosition.Angle(showerPosition);
    tmpProjectedDigit.rho = showerR * TMath::Tan(angleDigitShower);

    //---------------------------------------------------------------------
    //Projected digit polar angle
    //---------------------------------------------------------------------
    //Vector perpendicular to the vector pointing to the shower position, pointing to the CalDigit.
    //It's length is not rho. Not normalized!!! We only care about the angle between in and xPrime.
    TVector3 projectedDigitDirection = calDigitPosition - calDigitPosition.Dot(showerDirection) * showerDirection;
    tmpProjectedDigit.alpha = projectedDigitDirection.Angle(xPrimeDirection);

    //adjust so that alpha spans 0..2pi
    if (projectedDigitDirection.Angle(yPrimeDirection) > TMath::Pi() / 2.0)
      tmpProjectedDigit.alpha = 2.0 * TMath::Pi() - tmpProjectedDigit.alpha;
    tmpProjectedECLDigits.push_back(tmpProjectedDigit);
  }

  return tmpProjectedECLDigits;
}

void ECLShowerShapeModule::endRun()
{
}

void ECLShowerShapeModule::terminate()
{
}

double ECLShowerShapeModule::computeLateralEnergy(const ECLShower& shower) const
{

  auto relatedDigitsPairs = shower.getRelationsTo<ECLCalDigit>();
  if (shower.getNofCrystals() < 3.0) return 0;

  // Find the two digits with the maximum energy
  double maxEnergy(0), secondMaxEnergy(0);
  unsigned int iMax(0), iSecondMax(0);
  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
    const auto weight = relatedDigitsPairs.weight(iRel);
    double energy = weight * aECLCalDigit->getEnergy();
    if (energy > maxEnergy) {
      maxEnergy = energy;
      iMax = iRel;
      secondMaxEnergy = maxEnergy;
      iSecondMax = iMax;
    } else if (energy > secondMaxEnergy) {
      secondMaxEnergy = energy;
      iSecondMax = iRel;
    }
  }
  double sumE = 0;
  TVector3 cryCenter;
  cryCenter.SetMagThetaPhi(shower.getR(), shower.getTheta(), shower.getPhi());

  for (unsigned int iRel = 0; iRel < relatedDigitsPairs.size(); iRel++) {
    if (iRel != iMax && iRel != iSecondMax) {
      const auto aECLCalDigit = relatedDigitsPairs.object(iRel);
      const auto weight = relatedDigitsPairs.weight(iRel);
      int cId = aECLCalDigit->getCellId();
      ECLGeometryPar* geometry = ECLGeometryPar::Instance();
      TVector3 pos = geometry->GetCrystalPos(cId - 1);
      TVector3 deltaPos = pos - cryCenter;
      double r = deltaPos.Mag();
      double r2 = r * r;
      sumE += weight * aECLCalDigit->getEnergy() * r2;
    }
  }
  const double r0sq = 5.*5. ; // average crystal dimension
  return sumE / (sumE + r0sq * (maxEnergy + secondMaxEnergy));
}

double ECLShowerShapeModule::Rnm(const int n, const int m, const double rho) const
{
  //Some explicit polynomials
  if (n == 1 && m == 1) return rho;
  if (n == 2 && m == 0) return 2.0 * rho * rho - 1.0;
  if (n == 2 && m == 2) return rho * rho;
  if (n == 3 && m == 1) return 3.0 * rho * rho * rho - 2.0 * rho;
  if (n == 3 && m == 3) return rho * rho * rho;
  if (n == 4 && m == 0) return 6.0 * rho * rho * rho * rho - 6.0 * rho * rho + 1.0;
  if (n == 4 && m == 2) return 4.0 * rho * rho * rho * rho - 3.0 * rho * rho;
  if (n == 4 && m == 4) return rho * rho * rho * rho;
  if (n == 5 && m == 1) return 10.0 * rho * rho * rho * rho * rho - 12.0 * rho * rho * rho + 3.0 * rho;
  if (n == 5 && m == 3) return 5.0 * rho * rho * rho * rho * rho - 4.0 * rho * rho * rho;
  if (n == 5 && m == 5) return rho * rho * rho * rho * rho;

  //Otherwise compute explicitely
  double returnVal = 0;
  for (int idx = 0; idx <= (n - std::abs(m)) / 2; ++idx)
    returnVal += std::pow(-1, idx) * TMath::Factorial(n - idx) / TMath::Factorial(idx)
                 / TMath::Factorial((n + std::abs(m)) / 2 - idx) / TMath::Factorial((n - std::abs(m)) / 2 - idx) * std::pow(rho, n - 2 * idx);

  return returnVal;
}

std::complex<double> ECLShowerShapeModule::zernikeValue(const double rho, const double alpha, const int n, const int m) const
{
  //Zernike moment defined only on the unit cercile (rho < 1)
  if (rho > 1.0) return std::complex<double>(0, 0);

  std::complex<double> i(0, 1);
  std::complex<double> exponent = std::exp(i * std::complex<double>(m * alpha, 0));
  return std::complex<double>(Rnm(n, m, rho), 0) * exponent;
}


double ECLShowerShapeModule::computeAbsZernikeMoment(const std::vector<ProjectedECLDigit>& projectedDigits,
                                                     const double totalEnergy, const int n, const int m, const double rho0) const
{
  //Make sure n,m are valid
  if (n < 0 || m < 0) return 0.0;
  if (m > n) return 0.0;

  std::complex<double> sum(0.0, 0.0);

  for (const auto projectedDigit : projectedDigits) {
    const double normalizedRho = projectedDigit.rho / rho0;     //Normailze radial distance according to rho0
    sum += projectedDigit.energy * std::conj(zernikeValue(normalizedRho, projectedDigit.alpha, n, m));
  }
  return (n + 1.0) / TMath::Pi() * std::abs(sum) / totalEnergy;
}

double ECLShowerShapeModule::computeSecondMoment(const std::vector<ProjectedECLDigit>& projectedDigits,
                                                 const double totalEnergy) const
{
  double sum = 0.0;

  for (const auto projectedDigit : projectedDigits) sum += projectedDigit.energy * projectedDigit.rho * projectedDigit.rho;

  return sum / totalEnergy;
}
