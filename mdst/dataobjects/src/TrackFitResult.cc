/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <mdst/dataobjects/TrackFitResult.h>
#include <TMath.h>

using namespace Belle2;

ClassImp(TrackFitResult);

TrackFitResult::TrackFitResult() : m_pdg(0), m_pValue(0), m_tau(), m_cov5(), m_hitPatternCDCInitializer(0), m_hitPatternCDCInitializer_part2(0), m_hitPatternVXDInitializer(0)
{
}

TrackFitResult::TrackFitResult(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                               const short int charge, const Const::ParticleType& particleType, const float pValue,
                               const float bField,
                               const long long int hitPatternCDCInitializer, const uint32_t hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer & 0xFFFFFFFF),
  m_hitPatternCDCInitializer_part2(hitPatternCDCInitializer >> 32),
  m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{
  cartesianToPerigee(position, momentum, covariance, charge, bField);
}

TrackFitResult::TrackFitResult(const std::vector<float>& tau, const std::vector<float>& cov5,
                               const Const::ParticleType& particleType, const float pValue,
                               const long long int hitPatternCDCInitializer, const uint32_t hitPatternVXDInitializer) :
  m_pdg(std::abs(particleType.getPDGCode())), m_pValue(pValue),
  m_tau(tau), m_cov5(cov5),
  m_hitPatternCDCInitializer(hitPatternCDCInitializer & 0xFFFFFFFF), m_hitPatternCDCInitializer_part2(hitPatternCDCInitializer >> 32), m_hitPatternVXDInitializer(hitPatternVXDInitializer)
{

}

// This class should be able to give back Helix information either in Perigee Parametrisation
// or as starting position + momentum.
TVector3 TrackFitResult::getPosition() const
{
  return TVector3(calcXFromPerigee(), calcYFromPerigee(), calcZFromPerigee());
}

TVector3 TrackFitResult::getMomentum(const float bField) const
{
  return TVector3(calcPxFromPerigee(bField), calcPyFromPerigee(bField), calcPzFromPerigee(bField));
}

float TrackFitResult::getTransverseMomentum(const float bField) const
{
  return std::fabs(1 / getAlpha(bField) / m_tau.at(2));
}

TMatrixDSym TrackFitResult::getCovariance5() const
{
  TMatrixDSym cov5(5);
  unsigned int counter = 0;
  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      cov5(i, j) = m_cov5[counter];
      ++counter;
    }
  }
  return cov5;
}

TMatrixDSym TrackFitResult::transformCov5ToCov6(const TMatrixDSym& cov5, const float bField) const
{
  const double d0 = getD0();
  const double phi = getPhi();
  const double omega = getOmega();
  const double cotTheta = getCotTheta();
  const double alpha = getAlpha(bField);

  TMatrixD B(6, 5); // Matrix is invoked with zeros
  const double alphaOmega = std::fabs(alpha * omega);
  const double alphaOmega2 = std::fabs(alpha * std::pow(omega, 2));

  B(0, 0) = std::sin(phi);
  B(0, 1) = d0 * std::cos(phi);
  B(1, 0) = -std::cos(phi);
  B(1, 1) = d0 * std::sin(phi);
  B(2, 3) = 1;
  B(3, 1) = -std::sin(phi) / alphaOmega;
  B(3, 2) = -std::cos(phi) / alphaOmega2;
  B(4, 1) = std::cos(phi) / alphaOmega;
  B(4, 2) = -std::sin(phi) / alphaOmega2;
  B(5, 2) = -cotTheta / alphaOmega2;
  B(5, 4) = 1.0 / alphaOmega;

  // FIXME why does this make the test fail?
//  TMatrixDSym cov6(cov5);
//  cov6.Similarity(B);
//  return cov6;
  // and this ugly code not, which should do in principal the same?
  TMatrixD cov6(6, 6);
  TMatrixD BT(5, 6);
  BT.Transpose(B);
  cov6 = B * cov5 * BT;
  TMatrixDSym cov6workaroud(6);
  for (unsigned int i = 0; i < 6; ++i) {
    for (unsigned int j = 0; j < 6; ++j) {
      cov6workaroud(i, j) = cov6(i, j);
    }
  }
  return cov6workaroud;
}


TMatrixDSym TrackFitResult::getCovariance6(const float bField) const
{
  return transformCov5ToCov6(getCovariance5(), bField);
}


void TrackFitResult::cartesianToPerigee(const TVector3& position, const TVector3& momentum, const TMatrixDSym& covariance,
                                        const short int charge, const float bField)
{
  const double alpha = getAlpha(bField);

  // We allow for the case that position, momentum are not given
  // exactly in the perigee.  Therefore we have to solve the
  // equations.  Any point on the helix, using the perigee parameters
  // as in "Fast vertex fitting with a local parametrization of tracks
  // - Billoir, Pierre et al. Nucl.Instrum.Meth. A311 (1992) 139-150"
  // named here d0, phi, omega, z0, cotTheta together with an angle
  // chi, can be written:
#if 0
  px = cos(phi + chi) / alpha / omega;
  py = sin(phi + chi) / alpha / omega;
  pz = charge * cotTheta / alpha / omega;
  x =  d0 * sin(phi) + charge / omega * (sin(phi + chi) - sin(phi));
  y = -d0 * cos(phi) + charge / omega * (-cos(phi + chi) + cos(phi));
  z = z0 + charge / omega * cotTheta * chi;
  // Expressed in terms of the track length from the POCA 's' and its
  // projection into the x-y plane
  S = s / hypot(1, cotTheta);
  // we have chi = S*omega, and from this the formulae valid also for
  // straight line tracks (for the constant direction of the
  // straight-line track just take chi = 0, and for straight-line
  // tracks charge just establishes a sign convention):
  x = (d0 * sin(phi)
       + charge * S * (sin(phi) * (cos(S * omega) - 1) / (S * omega)
                       + cos(phi) * sin(S * omega) / (S * omega)));
  y = (-d0 * cos(phi)
       + charge * S * (sin(phi) * sin(S * omega) / (S * omega)
                       - cos(phi) * (cos(S * omega) - 1) / (S * omega)));
  z = charge * S * cotTheta;
  // For omega -> 0, these reduce to straight lines, according to
  x = (d0 * sin(phi)
       + charge * S * cos(phi) + O(omega));
  y = (-d0 * cos(phi)
       + chargs * S * sin(phi) + O(omega));
  z = charge * S * cotTheta;
  // Billoir (loc.cit.) gives the first terms of a series in S.
#endif
  double x = position.X(); double y = position.Y(); double z = position.Z();
  double px = momentum.X(); double py = momentum.Y(); double pz = momentum.Z();

  // We find the perigee parameters by inverting this system of
  // equations and solving for the six variables d0, phi, omega, z0,
  // cotTheta, chi.

  const double ptinv = 1 / hypot(px, py);
  const double omega = charge * ptinv / alpha;
  const double cotTheta = ptinv * pz;

  const double cosphichi = charge * ptinv * px;  // cos(phi + chi)
  const double sinphichi = charge * ptinv * py;  // sin(phi + chi)

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  const double rhoHel = hypot(helX, helY);

  const double d0 = charge * hypot(helX, helY) - 1 / omega;
  // remainder(..., 2*M_PI) reduces range to [-pi,pi].
  const double phi = remainder(atan2(helY, helX) + charge * M_PI / 2, 2 * M_PI);
  const double sinphi = sin(phi);
  const double cosphi = cos(phi);
  const double sinchi = sinphichi * cosphi - cosphichi * sinphi;
  const double coschi = sinphichi * sinphi + cosphichi * cosphi; // Precision loss near IP!  See below.
  // We usually need to evaluate chi close to the IP, i.e. chi small.
  // Here the calculation of cos(chi) is fraught with numerical error.
  // Therefore we go with asin(sinchi).  On the other hand for Kshort
  // tracks, we may be farther from the IP.  There we need the full
  // range of the helix, i.e. chi in [-pi,pi], whereas asin only
  // returns angles in the range [-pi/2,pi/2].  The separation point
  // between the formulae was chosen arbitrarily.
  double chi = 0;
  if (fabs(sinchi) > .5) {
    chi = atan2(sinchi, coschi);
  } else {
    chi = asin(sinchi);
  }
  const double z0 = z + charge / omega * cotTheta * chi;

  m_tau.reserve(5);
  m_tau.push_back(d0);
  m_tau.push_back(phi);
  m_tau.push_back(omega);
  m_tau.push_back(z0);
  m_tau.push_back(cotTheta);

  // Derivative of the helix parameters WRT the 6D parameters.
  // We write down all the intermediate derivatives and then apply the
  // chain rule to piece them together.  These derivatives were
  // verified with numerical derivatives.
  //
  //tau, chi = d0, phi, omega, z0, cosTheta, chi

  // Derivatives of the various subexpressions, naming convention:
  // dab = d(a)/d(b)
  const double dptinvpx = -px * pow(ptinv, 3);
  const double dptinvpy = -py * pow(ptinv, 3);

  const double domegaptinv = charge / alpha;
  const double domegapx = domegaptinv * dptinvpx;
  const double domegapy = domegaptinv * dptinvpy;

  const double dcotThetaptinv = pz;
  const double dcotThetapx = dcotThetaptinv * dptinvpx;
  const double dcotThetapy = dcotThetaptinv * dptinvpy;
  const double dcotThetapz = ptinv;

  const double dcosphichiptinv = charge * px;
  const double dcosphichipx = dcosphichiptinv * dptinvpx + charge * ptinv;
  const double dcosphichipy = dcosphichiptinv * dptinvpy;

  const double dsinphichiptinv = charge * py;
  const double dsinphichipx = dsinphichiptinv * dptinvpx;
  const double dsinphichipy = dsinphichiptinv * dptinvpy + charge * ptinv;

  const double dhelXx = 1;
  const double dhelXpy = charge * alpha;
  const double dhelYy = 1;
  const double dhelYpx = -charge * alpha;

  const double dd0helX = charge * helX / rhoHel;
  const double dd0helY = charge * helY / rhoHel;
  const double dd0omega = 1 / omega / omega;
  const double dd0x = dd0helX * dhelXx;
  const double dd0y = dd0helY * dhelYy;
  const double dd0px = dd0helY * dhelYpx + dd0omega * domegapx;
  const double dd0py = dd0helX * dhelXpy + dd0omega * domegapy;

  const double dphihelX = -helY / rhoHel / rhoHel;
  const double dphihelY = helX / rhoHel / rhoHel;
  const double dphix = dphihelX * dhelXx;
  const double dphiy = dphihelY * dhelYy;
  const double dphipx = dphihelY * dhelYpx;
  const double dphipy = dphihelX * dhelXpy;

  const double dsinchisinphichi = cosphi;      // d(sin chi) / d(sin(phi+chi))
  const double dsinchiphi = -coschi;  // See above concerning cos(chi)
  const double dsinchicosphichi = -sinphi;     // d(sin chi) / d(cos(phi+chi))
  const double dsinchix = dsinchiphi * dphix;
  const double dsinchiy = dsinchiphi * dphiy;
  const double dsinchipx = dsinchisinphichi * dsinphichipx + dsinchiphi * dphipx + dsinchicosphichi * dcosphichipx;
  const double dsinchipy = dsinchisinphichi * dsinphichipy + dsinchiphi * dphipy + dsinchicosphichi * dcosphichipy;
  const double dchisinchi = 1 / sqrt(1 - sinchi * sinchi);
  const double dchix = dchisinchi * dsinchix;
  const double dchiy = dchisinchi * dsinchiy;
  const double dchipx = dchisinchi * dsinchipx;
  const double dchipy = dchisinchi * dsinchipy;

  const double dz0omega = -charge / omega / omega * cotTheta * chi;
  const double dz0cotTheta = charge / omega * chi;
  const double dz0chi = charge / omega * cotTheta;
  const double dz0x = dz0chi * dchix;
  const double dz0y = dz0chi * dchiy;
  const double dz0z = 1;
  const double dz0px = dz0omega * domegapx + dz0cotTheta * dcotThetapx + dz0chi * dchipx;
  const double dz0py = dz0omega * domegapy + dz0cotTheta * dcotThetapy + dz0chi * dchipy;
  const double dz0pz = dz0cotTheta * dcotThetapz;

  //tau, chi = d0, phi, omega, z0, cotTheta, chi
  TMatrixD A(5, 6); // Rows (tau), cols (x, p)
  A(0, 0) = dd0x;
  A(0, 1) = dd0y;
  A(0, 3) = dd0px;
  A(0, 4) = dd0py;

  A(1, 0) = dphix;
  A(1, 1) = dphiy;
  A(1, 3) = dphipx;
  A(1, 4) = dphipy;

  A(2, 3) = domegapx;
  A(2, 4) = domegapy;

  A(3, 0) = dz0x;
  A(3, 1) = dz0y;
  A(3, 2) = dz0z;
  A(3, 3) = dz0px;
  A(3, 4) = dz0py;
  A(3, 5) = dz0pz;

  A(4, 3) = dcotThetapx;
  A(4, 4) = dcotThetapy;
  A(4, 5) = dcotThetapz;
  /* For the sake of completeness, not used, omitted:
  A(5, 0) = dchix;
  A(5, 1) = dchiy;
  A(5, 3) = dchipx;
  A(5, 4) = dchipy;
  */

  TMatrixDSym cov5(covariance);
  cov5.Similarity(A);

  for (unsigned int i = 0; i < 5; ++i) {
    for (unsigned int j = i; j < 5; ++j) {
      m_cov5.push_back(cov5(i, j));
    }
  }

}

double TrackFitResult::getAlpha(const float bField) const
{
  return 1.0 / (bField * TMath::C()) * 1E11;
}

double TrackFitResult::calcXFromPerigee() const
{
  return m_tau.at(0) * std::sin((double)m_tau.at(1));
}

double TrackFitResult::calcYFromPerigee() const
{
  return -m_tau.at(0) * std::cos((double)m_tau.at(1));
}

double TrackFitResult::calcZFromPerigee() const
{
  return m_tau.at(3);
}

double TrackFitResult::calcPxFromPerigee(const float bField) const
{
  return std::cos((double)m_tau.at(1)) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}

double TrackFitResult::calcPyFromPerigee(const float bField) const
{
  return std::sin((double)m_tau.at(1)) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}

double TrackFitResult::calcPzFromPerigee(const float bField) const
{
  return m_tau.at(4) / (std::fabs(m_tau.at(2) * getAlpha(bField)));
}
