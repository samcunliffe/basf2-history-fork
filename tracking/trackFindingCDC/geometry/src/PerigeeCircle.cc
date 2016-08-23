/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <tracking/trackFindingCDC/geometry/PerigeeCircle.h>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/numerics/SpecialFunctions.h>
#include <boost/math/special_functions/sinc.hpp>

#include <tracking/trackFindingCDC/numerics/SinEqLine.h>

#include <cmath>

using namespace std;
using namespace boost::math;

using namespace Belle2;
using namespace TrackFindingCDC;

PerigeeCircle::PerigeeCircle()
  : GeneralizedCircle()
{
  invalidate();
}

PerigeeCircle::PerigeeCircle(double curvature, const Vector2D& phi0Vec, double impact)
  : GeneralizedCircle(GeneralizedCircle::fromPerigeeParameters(curvature, phi0Vec, impact))
  , m_curvature(curvature)
  , m_phi0(phi0Vec.phi())
  , m_phi0Vec(phi0Vec)
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(double curvature, double phi0, double impact)
  : GeneralizedCircle(GeneralizedCircle::fromPerigeeParameters(curvature, phi0, impact))
  , m_curvature(curvature)
  , m_phi0(phi0)
  , m_phi0Vec(Vector2D::Phi(phi0))
  , m_impact(impact)
{
}

PerigeeCircle::PerigeeCircle(const PerigeeParameters& parameters)
  : PerigeeCircle(parameters(EPerigeeParameter::c_Curv),
                  parameters(EPerigeeParameter::c_Phi0),
                  parameters(EPerigeeParameter::c_I))
{
}

PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123,
                             double curvature,
                             double phi0,
                             const Vector2D& phi0Vec,
                             double impact)
  : GeneralizedCircle(n0123)
  , m_curvature(curvature)
  , m_phi0(phi0)
  , m_phi0Vec(phi0Vec)
  , m_impact(impact)
{
  /// Nothing to do here
}

PerigeeCircle::PerigeeCircle(const Line2D& n012)
  : GeneralizedCircle(n012)
{
  setN(n012);
}

PerigeeCircle::PerigeeCircle(const GeneralizedCircle& n0123)
  : GeneralizedCircle(n0123)
{
  setN(n0123);
}

PerigeeCircle::PerigeeCircle(const Circle2D& circle)
{
  setCenterAndRadius(circle.center(), circle.absRadius(), circle.orientation());
}

PerigeeCircle PerigeeCircle::fromN(double n0, double n1, double n2, double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n1, n2, n3);
  return circle;
}

PerigeeCircle PerigeeCircle::fromN(double n0, const Vector2D& n12, double n3)
{
  PerigeeCircle circle;
  circle.setN(n0, n12, n3);
  return circle;
}

PerigeeCircle PerigeeCircle::fromCenterAndRadius(const Vector2D& center,
                                                 double absRadius,
                                                 const ERotation orientation)
{
  PerigeeCircle circle;
  circle.setCenterAndRadius(center, absRadius, orientation);
  return circle;
}

Vector2D PerigeeCircle::atArcLength(double arcLength) const
{
  double chi = arcLength * curvature();
  double chiHalf = chi / 2.0;

  using boost::math::sinc_pi;
  double atX = arcLength * sinc_pi(chi);
  double atY = arcLength * sinc_pi(chiHalf) * sin(chiHalf) + impact();
  return Vector2D::compose(phi0Vec(), atX, atY);
}

void PerigeeCircle::reverse()
{
  m_curvature = -m_curvature;
  m_phi0 = AngleUtil::reversed(m_phi0);
  m_phi0Vec.reverse();
  m_impact = -m_impact;
  GeneralizedCircle::reverse();
}

PerigeeCircle PerigeeCircle::reversed() const
{
  return PerigeeCircle(GeneralizedCircle::reversed(),
                       -m_curvature,
                       AngleUtil::reversed(m_phi0),
                       -m_phi0Vec,
                       -m_impact
                      );
}

void PerigeeCircle::conformalTransform()
{
  double denominator = 2 + curvature() * impact();
  std::swap(m_impact, m_curvature);
  m_curvature *= denominator;
  m_impact /= denominator;
  // Also properly fixing the orientation to the opposite.
  reverse();

  GeneralizedCircle::conformalTransform();
  GeneralizedCircle::reverse();
}

PerigeeCircle PerigeeCircle::conformalTransformed() const
{
  double denominator = 2 + curvature() * impact();
  // Also properly fixing the orientation to the opposite.
  double newCurvature = -impact() * denominator;
  Vector2D newPhi0Vec = -phi0Vec();
  double newImpact = -curvature() / denominator;
  return PerigeeCircle(newCurvature, newPhi0Vec, newImpact);
}

void PerigeeCircle::receivePerigeeParameters()
{
  m_curvature = GeneralizedCircle::curvature();
  m_phi0Vec = GeneralizedCircle::tangential();
  m_phi0 = GeneralizedCircle::tangentialPhi();
  m_impact = GeneralizedCircle::impact();
}

void PerigeeCircle::invalidate()
{
  m_curvature = 0.0;
  m_phi0 = NAN;
  m_phi0Vec = Vector2D(0.0, 0.0);
  m_impact = 0;
  GeneralizedCircle::invalidate();
}

void PerigeeCircle::passiveMoveBy(const Vector2D& by)
{
  double arcLength = arcLengthTo(by);
  m_impact = distance(by);
  m_phi0 = m_phi0 + curvature() * arcLength;
  AngleUtil::normalise(m_phi0);
  m_phi0Vec = Vector2D::Phi(m_phi0);
  GeneralizedCircle::passiveMoveBy(by);
}

PerigeeJacobian PerigeeCircle::passiveMoveByJacobian(const Vector2D& by) const
{
  PerigeeJacobian jacobian = PerigeeUtil::identity();
  passiveMoveByJacobian(by, jacobian);
  return jacobian;
}

void PerigeeCircle::passiveMoveByJacobian(const Vector2D& by, PerigeeJacobian& jacobian) const
{

  // In this frame of reference we have d=0,  phi= + or - M_PI
  Vector2D coordinateVector = phi0Vec();

  // Vector2D delta = perigee() - by;
  Vector2D delta = by - perigee();

  double deltaParallel = coordinateVector.unnormalizedParallelComp(delta);
  double deltaOrthogonal = coordinateVector.unnormalizedOrthogonalComp(delta);

  double halfA = fastDistance(by);
  double A = 2 * halfA;

  // B2INFO("A = " << A);
  // B2INFO("A = " << 2 * deltaOrthogonal + curvature() * delta.normSquared());

  Vector2D CB = gradient(by).orthogonal();
  // double C = CB.first();
  // double B = CB.second();

  // B2INFO("B = " << B);
  // B2INFO("C = " << C);

  double u = 1 + curvature() * impact(); //= n12().cylindricalR()

  double U = sqrt(1 + curvature() * A);

  // B2INFO("U = " << U);

  double nu = 1 + curvature() * deltaOrthogonal;

  // B2INFO("nu = " << nu);

  double xi = 1.0 / CB.normSquared();

  // B2INFO("xi = " << xi);

  double lambda = halfA / ((1 + U) * (1 + U) * U);
  double mu = 1.0 / (U * (U + 1)) + curvature() * lambda;

  // B2INFO("lambda = " << lambda);
  // B2INFO("mu = " << mu);

  double zeta = delta.normSquared();

  // B2INFO("zeta = " << zeta);

  using namespace NPerigeeParameterIndices;
  jacobian(c_Curv, c_Curv) = 1;
  jacobian(c_Curv, c_Phi0) = 0;
  jacobian(c_Curv, c_I) = 0;

  jacobian(c_Phi0, c_Curv) = xi * deltaParallel;
  jacobian(c_Phi0, c_Phi0) = xi * u * nu;
  jacobian(c_Phi0, c_I) = -xi * curvature() * curvature() * deltaParallel;

  jacobian(c_I, c_Curv) = mu * zeta - lambda * A;
  jacobian(c_I, c_Phi0) = 2 * mu * u * deltaParallel;
  jacobian(c_I, c_I) = 2 * mu * nu;
}

Vector2D PerigeeCircle::closest(const Vector2D& point) const
{
  Vector2D delta = point - perigee();
  Vector2D uVec = delta * curvature() - phi0Vec().orthogonal();
  return point - uVec.unit() * distance(point);
}

double PerigeeCircle::arcLengthTo(const Vector2D& point) const
{
  Vector2D closestToPoint = closest(point);
  double secantLength = perigee().distance(closestToPoint);
  double deltaParallel = phi0Vec().dot(point);
  return copysign(arcLengthAtSecantLength(secantLength), deltaParallel);
}

double PerigeeCircle::arcLengthToCylindricalR(double cylindricalR) const
{
  // Slight trick here
  // Since the sought point is on the helix we treat it as the perigee
  // and the origin as the point to extrapolate to.
  // We know the distance of the origin to the circle, which is just d0
  // The direct distance from the origin to the imaginary perigee is just the given cylindricalR.
  return arcLengthAtDeltaLength(cylindricalR, impact());
}

double PerigeeCircle::arcLengthAtDeltaLength(double delta, double dr) const
{
  const double secantLength = sqrt((delta + dr) * (delta - dr) / (1 + dr * curvature()));
  const double arcLength = arcLengthAtSecantLength(secantLength);
  return arcLength;
}

double PerigeeCircle::arcLengthAtSecantLength(double secantLength) const
{
  double x = secantLength * curvature() / 2.0;
  double arcLengthFactor = asinc(x);
  return secantLength * arcLengthFactor;
}

double PerigeeCircle::distance(double fastDistance) const
{
  if (fastDistance == 0.0 or isLine()) {
    // special case for unfitted state
    // and line
    return fastDistance;
  } else {
    double A = 2 * fastDistance;
    double U = std::sqrt(1 + A * curvature());
    return A / (1.0 + U);
  }
}

double PerigeeCircle::fastDistance(const Vector2D& point) const
{
  Vector2D delta = point - perigee();
  double deltaOrthogonal = phi0Vec().cross(delta);
  return -deltaOrthogonal + curvature() * delta.normSquared() / 2;
}

void PerigeeCircle::setCenterAndRadius(const Vector2D& center,
                                       double absRadius,
                                       ERotation orientation)
{
  m_curvature = orientation / std::fabs(absRadius);
  m_phi0Vec = center.orthogonal(NRotation::reversed(orientation));
  m_phi0Vec.normalize();
  m_phi0 = m_phi0Vec.phi();
  m_impact = (center.norm() - std::fabs(absRadius)) * orientation;
  GeneralizedCircle::setCenterAndRadius(center, absRadius, orientation);
}


void PerigeeCircle::setN(double n0, const Vector2D& n12, double n3)
{
  double normalization = sqrt(n12.normSquared() - 4 * n0 * n3);

  m_curvature = 2 * n3 / normalization;
  m_phi0Vec = n12.orthogonal();
  m_phi0Vec.normalize();
  m_phi0 = m_phi0Vec.phi();
  m_impact = distance(n0 / normalization); // Uses the new curvature
  GeneralizedCircle::setN(n0, n12, n3);
}
