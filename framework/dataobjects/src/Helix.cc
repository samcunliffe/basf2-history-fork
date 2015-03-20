/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <framework/dataobjects/Helix.h>
#include <TMath.h>

#include <framework/logging/Logger.h>

#include <boost/math/special_functions/sign.hpp>
#include <boost/math/special_functions/sinc.hpp>
#include <boost/math/tools/precision.hpp>

#include <assert.h>

using namespace Belle2;
using namespace HelixParameterIndex;

ClassImp(Helix);

Helix::Helix()
{
}

Helix::Helix(const TVector3& position,
             const TVector3& momentum,
             const short int charge,
             const double bZ)
{
  setCartesian(position, momentum, charge, bZ);
}

Helix::Helix(const double& d0,
             const double& phi0,
             const double& omega,
             const double& z0,
             const double& tanLambda)
  : m_d0(d0),
    m_phi0(phi0),
    m_omega(omega),
    m_z0(z0),
    m_tanLambda(tanLambda)
{
}

double Helix::getPerigeeX() const
{
  return getD0() * getSinPhi0();
}

double Helix::getPerigeeY() const
{
  return -getD0() * getCosPhi0();
}

double Helix::getPerigeeZ() const
{
  return getZ0();
}

TVector3 Helix::getPerigee() const
{
  return TVector3(getPerigeeX(), getPerigeeY(), getPerigeeZ());
}

double Helix::getMomentumX(const double bZ) const
{
  return getCosPhi0() * getTransverseMomentum(bZ);
}

double Helix::getMomentumY(const double bZ) const
{
  return getSinPhi0() * getTransverseMomentum(bZ);
}

double Helix::getMomentumZ(const double bZ) const
{
  return getTanLambda() * getTransverseMomentum(bZ);
}

TVector3 Helix:: getMomentum(const double bZ) const
{
  return TVector3(getMomentumX(bZ), getMomentumY(bZ), getMomentumZ(bZ));
}

double Helix::getTransverseMomentum(const double bZ) const
{
  return 1 / std::fabs(getAlpha(bZ) * getOmega());
}

double Helix::getKappa(const double bZ) const
{
  return getOmega() * getAlpha(bZ);
}

double Helix::getAlpha(const double bZ)
{
  return 1.0 / (bZ * TMath::C()) * 1E11;
}

short Helix::getChargeSign() const
{
  return boost::math::sign(getOmega());
}

void Helix::reverse()
{
  // All except z0 have to be taken to their opposites
  m_d0 = -m_d0; //d0
  m_phi0 = reversePhi(m_phi0);
  m_omega = -m_omega;
  m_tanLambda = -m_tanLambda;
}

double Helix::getArcLength2DAtCylindricalR(const double& cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  const double dr = getD0();
  const double deltaCylindricalR = cylindricalR;
  const double absArcLength2D = calcArcLength2DAtDeltaCylindricalRAndDr(deltaCylindricalR, dr);
  return absArcLength2D;
}

double Helix::getArcLength2DAtXY(const double& x, const double& y) const
{
  double dr = 0;
  double arcLength2D = 0;
  calcArcLength2DAndDrAtXY(x, y, arcLength2D, dr);
  return arcLength2D;
}

TVector3 Helix::getPositionAtArcLength2D(const double& arcLength2D) const
{
  /*
    /   \     /                      \     /                              \
    | x |     | cos phi0   -sin phi0 |     |    - sin(chi)  / omega       |
    |   |  =  |                      |  *  |                              |
    | y |     | sin phi0    cos phi0 |     | -(1 - cos(chi)) / omega - d0 |
    \   /     \                      /     \                              /

    and

    z = tanLambda * arclength + z0;

    where chi = -arcLength2D * omega

    Here arcLength2D means the arc length of the circle in the xy projection
    traversed in the forward direction.

    // Old definition identical?
    x =  d0 * sin(phi0) + charge / omega * (sin(phi0 + chi) - sin(phi0));
    y = -d0 * cos(phi0) + charge / omega * (-cos(phi0 + chi) + cos(phi0));
    // Actually no - have to talk back to Markus about this.
  */

  // First calculating the position assuming the circle center lies on the y axes (phi0 = 0)
  // Rotate to the right phi position afterwards
  // Using the sinus cardinalis yields expressions that are smooth in the limit of omega -> 0

  // Do calculations in double
  const double chi = -arcLength2D * getOmega();
  const double chiHalf = chi / 2.0;

  using boost::math::sinc_pi;

  const double x = arcLength2D * sinc_pi(chi);
  const double y = arcLength2D * sinc_pi(chiHalf) * sin(chiHalf) - getD0();

  // const double z = s * tan lambda + z0
  const double z = fma((double)arcLength2D, getTanLambda(),  getZ0());

  // Unrotated position
  TVector3 position(x, y, z);

  // Rotate to the right phi0 position
  position.RotateZ(getPhi0());

  return position;
}

TVector3 Helix::getTangentialAtArcLength2D(const double& arcLength2D) const
{
  const double omega = getOmega();
  const double phi0 = getPhi0();
  const double tanLambda = getTanLambda();

  const double chi = - omega * arcLength2D;

  const double tx = cos(chi + phi0);
  const double ty = sin(chi + phi0);
  const double tz = tanLambda;

  TVector3 tangential(tx, ty, tz);
  return tangential;
}


TVector3 Helix::getUnitTangentialAtArcLength2D(const double& arcLength2D) const
{
  TVector3 unitTangential = getTangentialAtArcLength2D(arcLength2D);
  const double norm = hypot(1, getTanLambda());
  const double invNorm = 1 / norm;
  unitTangential *= invNorm;
  return unitTangential;
}

TVector3 Helix::getMomentumAtArcLength2D(const double& arcLength2D, const double& bz) const
{
  TVector3 momentum = getTangentialAtArcLength2D(arcLength2D);
  const double pr = getTransverseMomentum(bz);
  momentum *= pr;

  return momentum;
}

double Helix::passiveMoveBy(const TVector3& by)
{
  // First calculate the distance of the new origin to the helix in the xy projection
  double new_d0 = 0;
  double arcLength2D = 0;
  calcArcLength2DAndDrAtXY(by.X(), by.Y(), arcLength2D, new_d0);

  // Third the new phi0 and z0 can be calculated from the arc length
  double chi = -arcLength2D * getOmega();
  double new_phi0 = m_phi0 + chi;
  double new_z0 = getZ0() - by.Z() + getTanLambda() * arcLength2D;

  /// Update the parameters inplace. Omega and tan lambda are unchanged
  m_d0 = new_d0;
  m_phi0 = new_phi0;
  m_z0 = new_z0;

  return arcLength2D;
}

TMatrixD Helix::calcPassiveMoveByJacobian(const TVector3& by, const double expandBelowChi) const
{
  TMatrixD jacobian(5, 5);
  calcPassiveMoveByJacobian(by, jacobian, expandBelowChi);
  return jacobian;
}


void Helix::calcPassiveMoveByJacobian(const TVector3& by, TMatrixD& jacobian, const double expandBelowChi) const
{
  // 0. Preparations
  // Initialise the return value to a unit matrix
  jacobian.UnitMatrix();
  assert(jacobian.GetNrows() == jacobian.GetNcols());
  assert(jacobian.GetNrows() == 5 or jacobian.GetNrows() == 6);

  // Fetch the helix parameters
  const double omega = getOmega();
  const double cosPhi0 = getCosPhi0();
  const double sinPhi0 = getSinPhi0();
  const double d0 = getD0();
  const double tanLambda = getTanLambda();

  // Prepare a delta vector, which is the vector from the perigee point to the new origin
  // Split it in component parallel and a component orthogonal to tangent at the perigee.
  const double x = by.X();
  const double y = by.Y();

  const double deltaParallel = x * cosPhi0 + y * sinPhi0;
  const double deltaOrthogonal = y * cosPhi0 - x * sinPhi0 + d0;
  const double deltaCylindricalR = hypot(deltaOrthogonal, deltaParallel);
  const double deltaCylindricalRSquared = deltaCylindricalR * deltaCylindricalR;

  // Some commonly used terms - compare Karimaki 1990
  const double A = 2 * deltaOrthogonal + omega * deltaCylindricalRSquared;
  const double USquared = 1 + omega * A;
  const double U = sqrt(USquared);
  const double UOrthogonal = 1 + omega * deltaOrthogonal; // called nu in the Karimaki paper.
  const double UParallel = omega * deltaParallel;
  // Note U is a vector pointing from the middle of the projected circle scaled by a factor omega.
  const double u = 1 + omega * d0; // just called u in the Karimaki paper.

  // ---------------------------------------------------------------------------------------
  // 1. Set the parts related to the xy coordinates
  // Fills the upper left 3x3 corner of the jacobain

  // a. Calculate the row related to omega
  // The row related to omega is not different from the unit jacobian initialised above.
  // jacobian(iOmega, iOmega) = 1.0; // From UnitMatrix above.

  // b. Calculate the row related to d0
  const double new_d0 = A / (1 + U);

  jacobian(iD0, iOmega) = (deltaCylindricalR + new_d0) * (deltaCylindricalR - new_d0) / 2 / U;
  jacobian(iD0, iPhi0) = - u * deltaParallel / U;
  jacobian(iD0, iD0) =  UOrthogonal / U;

  // c. Calculate the row related to phi0
  // Also calculate the derivatives of the arc length
  // which are need for the row related to z.
  const double dArcLength2D_dD0 = - UParallel / USquared;
  const double dArcLength2D_dPhi0 = (omega * deltaCylindricalRSquared + deltaOrthogonal - d0 * UOrthogonal) / USquared;

  jacobian(iPhi0, iD0) = - dArcLength2D_dD0 * omega;
  jacobian(iPhi0, iPhi0) = u * UOrthogonal / USquared;
  jacobian(iPhi0, iOmega) =  -deltaParallel / USquared;

  // For jacobian(iPhi0, iOmega) we have to dig deeper
  // since the normal equations have a divergence for omega -> 0.
  // This hinders not only the straight line case but extrapolations between points which are close together,
  // like it happens when the current perigee is very close the new perigee.
  // To have a smooth transition in this limit we have to carefully
  // factor out the divergent quotents and approximate them with their taylor series.

  const double chi = -atan2(UParallel, UOrthogonal);
  double arcLength2D = 0;
  double dArcLength2D_dOmega = 0;

  if (fabs(chi) < std::min(expandBelowChi, M_PI / 2.0)) {
    // Never expand for the far side of the circle by limiting the expandBelow to maximally half a pi.
    // Close side of the circle
    double principleArcLength2D = deltaParallel / UOrthogonal;
    const double dPrincipleArcLength2D_dOmega = - principleArcLength2D * deltaOrthogonal / UOrthogonal;

    const double x = principleArcLength2D * omega;
    const double f = calcATanXDividedByX(x);
    const double df_dx = calcDerivativeOfATanXDividedByX(x);

    arcLength2D = principleArcLength2D * f;
    dArcLength2D_dOmega = (f + x * df_dx) * dPrincipleArcLength2D_dOmega + principleArcLength2D * df_dx * principleArcLength2D;
  } else {
    // Far side of the circle
    // If the far side of the circle is a well definied concept, omega is high enough that
    // we can divide by it.
    // Otherwise nothing can rescue us since the far side of the circle is so far away that no reasonable extrapolation can be made.
    arcLength2D = - chi / omega;
    dArcLength2D_dOmega = (-arcLength2D - jacobian(iPhi0, iOmega)) / omega;
  }

  // ---------------------------------------------------------------------------------------
  // 2. Set the parts related to the z coordinate
  // Since tanLambda stays the same there are no entries for tanLambda in the jacobian matrix
  // For the new z0' = z0 + arcLength2D * tanLambda
  jacobian(iZ0, iD0) = tanLambda * dArcLength2D_dD0;
  jacobian(iZ0, iPhi0) = tanLambda * dArcLength2D_dPhi0;
  jacobian(iZ0, iOmega) = tanLambda * dArcLength2D_dOmega;
  //jacobian(iZ0, iZ0) = 1.0; // From UnitMatrix above.
  jacobian(iZ0, iTanLambda) = arcLength2D;

  if (jacobian.GetNrows() == 6) {
    // Also write the derivates of arcLength2D to the jacobian if the matrix size has space for them.
    jacobian(iArcLength2D, iD0) = dArcLength2D_dD0;
    jacobian(iArcLength2D, iPhi0) = dArcLength2D_dPhi0;
    jacobian(iArcLength2D, iOmega) = dArcLength2D_dOmega;
    jacobian(iArcLength2D, iArcLength2D) = 0.0;
    jacobian(iArcLength2D, iTanLambda) = 0;
    jacobian(iArcLength2D, iArcLength2D) = 1.0;
  }
}

double Helix::reversePhi(const double& phi)
{
  return std::remainder(phi + M_PI, 2 * M_PI);
}

double Helix::calcArcLength2DFromSecantLength(const double& secantLength) const
{
  return secantLength * calcSecantLengthToArcLength2DFactor(secantLength);
}


double Helix::calcSecantLengthToArcLength2DFactor(const double& secantLength) const
{
  double x = secantLength * getOmega() / 2.0;
  return calcASinXDividedByX(x);
}

double Helix::calcASinXDividedByX(const double& x)
{
  // Approximation of asin(x) / x
  // Inspired by BOOST's sinc

  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    if (fabs(x) == 1) {
      return  M_PI / 2.0;

    } else {
      return asin(x) / x;

    }

  } else {
    // approximation by taylor series in x at 0 up to order 0
    double result = 1.0;

    double const taylor_0_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      double x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      result += x2 / 6.0;

      double const taylor_2_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x2 * (3.0 / 40.0);
      }
    }
    return result;
  }

}

double Helix::calcATanXDividedByX(const double& x)
{
  // Approximation of atan(x) / x
  // Inspired by BOOST's sinc

  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  if (abs(x) >= taylor_n_bound) {
    return atan(x) / x;

  } else {
    // approximation by taylor series in x at 0 up to order 0
    double result = 1.0;

    double const taylor_0_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      double x2 = x * x;
      // approximation by taylor series in x at 0 up to order 2
      result -= x2 / 3.0;

      double const taylor_2_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x2 * (1.0 / 5.0);
      }
    }
    return result;
  }
}

double Helix::calcDerivativeOfATanXDividedByX(const double& x)
{
  // Approximation of atan(x) / x
  // Inspired by BOOST's sinc

  BOOST_MATH_STD_USING;

  double const taylor_n_bound = boost::math::tools::forth_root_epsilon<double>();

  const double x2 = x * x;
  if (abs(x) >= taylor_n_bound) {
    const double chi = atan(x);
    return ((1 - chi / x) / x - chi) / (1 + x2);

  } else {
    // approximation by taylor series in x at 0 up to order 0
    double result = 1.0;

    double const taylor_0_bound = boost::math::tools::epsilon<double>();
    if (abs(x) >= taylor_0_bound) {
      // approximation by taylor series in x at 0 up to order 2
      result -= 2.0 * x / 3.0;

      double const taylor_2_bound = boost::math::tools::root_epsilon<double>();
      if (abs(x) >= taylor_2_bound) {
        // approximation by taylor series in x at 0 up to order 4
        result += x2 * x * (4.0 / 5.0);
      }
    }
    return result;
  }
}




void Helix::calcArcLength2DAndDrAtXY(const double& x, const double& y, double& arcLength2D, double& dr) const
{
  // Prepare common variables
  const double omega = getOmega();
  const double cosPhi0 = getCosPhi0();
  const double sinPhi0 = getSinPhi0();
  const double d0 = getD0();

  const double deltaParallel = x * cosPhi0 + y * sinPhi0;
  const double deltaOrthogonal = y * cosPhi0 - x * sinPhi0 + d0;
  const double deltaCylindricalR = hypot(deltaOrthogonal, deltaParallel);
  const double deltaCylindricalRSquared = deltaCylindricalR * deltaCylindricalR;

  const double A = 2 * deltaOrthogonal + omega * deltaCylindricalRSquared;
  const double U = sqrt(1 + omega * A);
  const double UOrthogonal = 1 + omega * deltaOrthogonal; // called nu in the Karimaki paper.
  const double UParallel = omega * deltaParallel;
  // Note U is a vector pointing from the middle of the projected circle scaled by a factor omega.

  // Calculate dr
  dr = A / (1 + U);

  // Calculate the absolute value of the arc length
  const double chi = -atan2(UParallel, UOrthogonal);

  if (fabs(chi) < M_PI / 8) { // Rough guess where the critical zone for approximations begins
    // Close side of the circle
    double principleArcLength2D = deltaParallel / UOrthogonal;
    arcLength2D = principleArcLength2D * calcATanXDividedByX(principleArcLength2D * omega);
  } else {
    // Far side of the circle
    // If the far side of the circle is a well definied concept meaning that we have big enough omega.
    arcLength2D = -chi / omega;
  }
}

double Helix::calcArcLength2DAtDeltaCylindricalRAndDr(const double& deltaCylindricalR, const double& dr) const
{
  const double omega = getOmega();
  double secantLength = sqrt((deltaCylindricalR + dr) * (deltaCylindricalR - dr) / (1 + dr * omega));
  return calcArcLength2DFromSecantLength(secantLength);
}

void Helix::setCartesian(const TVector3& position,
                         const TVector3& momentum,
                         const short int charge,
                         const double bZ)
{
  assert(abs(charge) == 1);  // Not prepared for doubly-charged particles.
  const double alpha = getAlpha(bZ);

#if 0
  // We allow for the case that position, momentum are not given
  // exactly in the perigee.  Therefore we have to solve the
  // equations.  Any point on the helix, using the perigee parameters
  // as in "Fast vertex fitting with a local parametrization of tracks
  // - Billoir, Pierre et al. Nucl.Instrum.Meth. A311 (1992) 139-150"
  // named here omega, phi0, d0, tanLambda, z0 together with an angle
  // chi, can be written:
  px = cos(phi0 + chi) / alpha / omega;
  py = sin(phi0 + chi) / alpha / omega;
  pz = charge * tanLambda / alpha / omega;
  x =  d0 * sin(phi0) + charge / omega * (sin(phi0 + chi) - sin(phi0));
  y = -d0 * cos(phi0) + charge / omega * (-cos(phi0 + chi) + cos(phi0));
  z = z0 + charge / omega * tanLambda * chi;
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
  const double x = position.X();
  const double y = position.Y();
  const double z = position.Z();

  const double px = momentum.X();
  const double py = momentum.Y();
  const double pz = momentum.Z();

  // We find the perigee parameters by inverting this system of
  // equations and solving for the six variables d0, phi0, omega, z0,
  // tanLambda, chi.

  const double ptinv = 1 / hypot(px, py);
  const double omega = charge * ptinv / alpha;
  const double tanLambda = ptinv * pz;

  // Helix center in the (x, y) plane:
  const double helX = x + charge * py * alpha;
  const double helY = y - charge * px * alpha;
  const double rhoHel = hypot(helX, helY);

  const double d0 = charge * rhoHel - 1 / omega;

  // phi0 is the azimuth of momentum, in the POCA it is orthogonal to
  // the vector pointing to the helix center.  The rotation by 90
  // degrees is achieved by (x, y) -> (-y, x) and the opposite
  // rotation for negative particles.
  const double phi0 = atan2(charge * helX, -charge * helY);

  const double cosPhi0 = (-charge * helY) / rhoHel;
  const double sinPhi0 = (charge * helX) / rhoHel;

  const double cosphi0chi = ptinv * px;  // cos(phi0 + chi)
  const double sinphi0chi = ptinv * py;  // sin(phi0 + chi)

  const double coschi =  cosphi0chi * cosPhi0 + sinphi0chi * sinPhi0;
  const double sinchi = -cosphi0chi * sinPhi0 + sinphi0chi * cosPhi0;

  const double chi = atan2(sinchi, coschi);

  const double arcLength2D = -chi / omega;
  const double z0 = z - arcLength2D * tanLambda;

  m_omega = omega;
  m_phi0 = phi0;
  m_d0 = d0;
  m_tanLambda = tanLambda;
  m_z0 = z0;
}

namespace Belle2 {
  /** Output operator for debugging and the generation of unittest error messages.*/
  std::ostream& operator<<(std::ostream& output, const Helix& helix)
  {
    return output
           << "Helix("
           << "d0=" << helix.getD0() << ", "
           << "phi0=" << helix.getPhi0() << ", "
           << "omega=" << helix.getOmega() << ", "
           << "z0=" << helix.getZ0() << ", "
           << "tanLambda=" << helix.getTanLambda() << ")";
  }
}
