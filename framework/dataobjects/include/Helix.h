/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Markus Prim                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <framework/gearbox/Const.h>
#include <framework/datastore/RelationsObject.h>
#include <framework/logging/Logger.h>

#include <TVector3.h>
#include <TMatrixD.h>
#include <TMatrixDSym.h>

#include <cstdlib>
#include <vector>

namespace Belle2 {

  /** This class represents an ideal helix in perigee parameterization.
   *  The used perigee parameters are:
   *
   *  1. **@f$ d_0 @f$** - the signed distance from the origin to the perigee. The sign is positive (negative),
                           if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
                           @f$d_0@F$ has the same sign as `getPerigee().Cross(getMomentum()).Z()`.
   *  2. **@f$ \phi_0 @f$** - the angle in the xy projection between the transverse momentum and the x axis, which is in [-pi, pi]
   *  3. **@f$ \omega @f$** - the signed curvature of the track where the sign is given by the charge of the particle
   *  4. **@f$ z_0 @f$** - z coordinate of the perigee
   *  5. **@f$ \tan \lambda @f$** - the slope of the track in the sz plane (dz/ds)
   *
   *  Each point on the helix can adressed by the arc length s, which has to be traversed to get to it from the perigee.
   *  More precisely the arc length means the transverse part of the particles travel distance,
   *  hence the arc length of the circle in the xy projection.
   *
   *  If you need different kind of methods / interfaces to the Helix please do not hesitate to contract oliver.frost@desy.de
   *  Contributions are always welcome.
   *
   */
  class Helix : public RelationsObject {
  public:

    /** Constructor initializing all perigee parameters to zero. */
    Helix();

    /** Constructor initializing class with a fit result.
     *
     *  The give n position and momentum are extrapolated to the perigee given a homogeneous magnetic field in the z direction
     *
     *  @param position      Position of the track at the perigee.
     *  @param momentum      Momentum of the track at the perigee.
     *  @param charge        Charge of the particle.
     *  @param bZ            Magnetic field to be used for the calculation of the curvature.
                             It is assumed, that the B-field is homogeneous parallel to the z axis.
     */
    Helix(const TVector3& position,
          const TVector3& momentum,
          const short int charge,
          const float bZ);

    /** Constructor initializing class with perigee parameters.
     *
     *  @param d0            The signed distance from the origin to the perigee. The sign is positive (negative),
     *                       if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
     *                       d0 has the same sign as `getPosition().Cross(getMomentum()).Z()`.
     *  @param phi0          The angle between the transverse momentum and the x axis and in [-pi, pi]
     *  @param omega         The signed curvature of the track where the sign is given by the charge of the particle
     *  @param z0            The z coordinate of the perigee.
     *  @param tanLambda     The slope of the track in the sz plane (dz/ds)
     */
    Helix(const float& d0,
          const float& phi0,
          const float& omega,
          const float& z0,
          const float& tanLambda);

    /** Output operator for debugging and the generation of unittest error messages.*/
    friend std::ostream& operator<<(std::ostream& output, const Helix& helix);


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for cartesian parameters of the perigee
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Calculates the x coordinate of the perigee point.*/
    double getPerigeeX() const;

    /** Calculates the y coordinate of the perigee point.*/
    double getPerigeeY() const;

    /** Calculates the z coordinate of the perigee point.*/
    double getPerigeeZ() const;

    /** Getter for the perigee position. */
    TVector3 getPerigee() const;

    /** Calculates the x momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumX(const float bZ) const;

    /** Calculates the y momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumY(const float bZ) const;

    /** Calculates the z momentum of the particle at the perigee point.
     *  @param bZ            Z component of the magnetic field in Tesla
     */
    double getMomentumZ(const float bZ) const;

    /** Getter for vector of momentum at the perigee position
     *
     *  As we calculate recalculate the momentum from a geometric helix, we need an estimate
     *  of the magnetic field along the z-axis to give back the momentum.
     *  @param bZ            Magnetic field at the perigee.
     */
    TVector3 getMomentum(const float bZ = 1.5) const;

    /** Getter for the absolute value of the transverse momentum at the perigee.
     *
     *  @param bZ            Magnetic field at the perigee
     */
    float getTransverseMomentum(const float bZ = 1.5) const;

    /** Getter for kappa, which is charge / transverse momentum or equivalently omega * alpha */
    float getKappa(const float bZ = 1.5) const;

    /** Calculates the alpha value for a given magnetic field in Tesla */
    static double getAlpha(const float bZ);

    /** Return track charge sign (1 or -1).*/
    short getChargeSign() const {
      return getOmega() >= 0 ? 1 : -1;
    }
    /// @}


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Simple extrapolations of the ideal helix
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Calculates the transverse travel distance at the point the helix first reaches the given cylindrical radius.
     *
     *  Gives the circle arc length in the forward direction that is traversed until a certain cylindrical radius is reached.
     *  Returns NAN, if the cylindrical radius can not be reached, either because it is to far outside or inside of the perigee.
     *
     *  Forward the result to getPositionAtArcLength() or getMomentumAtArcLength() in order to extrapolate to the cylinder detector boundaries.
     *
     *  The result always has a positive sign. Hence it refers to the forward direction.
     *  Adding a minus sign yields the point at the same cylindrical radius but in the backward direction.
     *
     *  @param cylindricalR  The cylinder radius to extrapolate to.
     *  @return              The circle arc length traversed to reach the cylindrical radius. NAN if it can not be reached.
     */
    float getArcLengthAtCylindricalR(const float& cylindricalR) const;

    /** Calculates the circle arc length at which the circle in the xy projection is closest to the point
     *
     *  This calculates the arc length to the closest approach in xy projection.
     *  Hence, it only optimizes the distance in x and y.
     *  This is sufficent to extrapolate to an axial wire.
     *  For stereo wires this is not optimal, since the distance in the z direction also plays a role.
     *
     *  @param x        X coordinate of the point to which to extrapolate
     *  @param y        Y coordinate of the point to which to extrapolate
     *  @return         The circle arc length from the perigee at which the closest approach is reached
     */
    float getArcLengthAtXY(const float& x, const float& y) const;

    /** Calculates the position on the helix at the given arc length
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     */
    TVector3 getPositionAtArcLength(const float& arcLength) const;

    /** Calculates the tangential vector to the helix curve at the given circle arc length.
     *
     *  The tangential vector is the derivative of the position with respect to the circle arc length
     *  It is normalised such that the cylindrical radius of the result is 1
     *
     *  getTangentialAtArcLength(arcLength).Perp() == 1 holds.
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     *  @return                Tangential vector normalised to unit transverse component / cylindrical radius.
     */
    TVector3 getTangentialAtArcLength(const float& arcLength) const;

    /** Calculates the unit tangential vector to the helix curve at the given circle arc length
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     */
    TVector3 getUnitTangentialAtArcLength(const float& arcLength) const;

    /** Calculates the momentum vector at the given arc length.
     *
     *  @param arcLength       Transverse travel distance on the helix, which is the length of the circle arc as seen in the xy projection.
     *  @param bz              Magnetic field strength in the z direction.
     */
    TVector3 getMomentumAtArcLength(const float& arcLength, const float& bz) const;

    /** Moves origin of the coordinate system (passive transformation) by the given vector. Updates the helix inplace.
     *
     *  @param by              Vector by which the origin of the coordinate system should be moved.
     *  @return                The float value is the arc length which as the be traversed from the old perigee to the new.
     */
    float passiveMoveBy(const TVector3& by);

    /** Reverses the direction of travel of the helix in place.
     *
     *  The same points are located on the helix stay the same after the transformation,
     *  but have the opposite arc length.
     *  The momentum at each point is reversed.
     *  The charge sign is changed to its opposite by this transformation.
     */
    void reverse();

    /** Reverses an azimuthal angle to the opposite direction
     *
     *  @param phi             A angle in [-pi, pi]
     *  @return                The angle for the opposite direction in [-pi, pi]
     */
    static float reversePhi(const float& phi) {
      return phi < 0 ? phi + M_PI : phi - M_PI;
    }

    /** Helper function to calculate the circle arc length from the length of a secant.
     *
     *  Translates the direct length between two point on the circle in the xy projection to the arc length on the circle
     *  Behaves smoothly in the limit of vanishing curvature.
     */
    double calcArcLengthFromSecantLength(const double& secantLength) const;

    /** Helper function to calculate the factor between the secant length and the circle arc length as seen in xy projection of the helix
     */
    double calcSecantLengthToArcLengthFactor(const double& secantLength) const;
    /// @}

  private:
    /** Implementation of the function asin(x) / x which handles small x values smoothly. */
    static double calcASinXDividedByX(const double& x);

    /** Helper method to calculate the signed circle arc length and the signed distance to the circle of a point in the xy projection.
     *
     *  This function is an implementation detail that prevents some code duplication.
     *
     *  @param x                   X coordinate of the point to which to extrapolate
     *  @param y                   Y coordinate of the point to which to extrapolate
     *  @param arcLength[out]      The circle arc length from the perigee at which the closest approach is reached
     *  @param dr[out]             Signed distance of the point to circle in the xy projection.
     */
    void calcArcLengthAndDrAtXY(const float& x, const float& y, double& arcLength, double& dr) const;

    /** Helper method to calculate the circle arc length *from the perigee* to a point at cylindrical radius, which also has the distance dr from the circle in the xy projection
     *
     *  This function is an implementation detail that prevents some code duplication.
     *
     *  @param deltaCylindricalR             The absolute distance of the point in question to the perigee in the xy projection
     *  @param dr                            Signed distance of the point to circle in the xy projection.
     *  @return                              The absolute arc length from the perigee to the point.
     */
    double calcArcLengthAtDeltaCylindricalRAndDr(const double& deltaCylindricalR, const double& dr) const;


    //---------------------------------------------------------------------------------------------------------------------------
    /// @name Getters for perigee helix parameters
    /// @{
    //---------------------------------------------------------------------------------------------------------------------------
  public:
    /** Getter for d0, which is the signed distance to the perigee in the r-phi plane.
     *
     *  The signed distance from the origin to the perigee. The sign is positive (negative),
     *  if the angle from the xy perigee position vector to the transverse momentum vector is +pi/2 (-pi/2).
     *  d0 has the same sign as `getPerigee().Cross(getMomentum()).Z()`.
     */
    float getD0() const { return m_d0; }

    /** Getter for phi0, which is the azimuth angle of the transverse momentum at the perigee.
     *
     *  getMomentum().Phi() == getPhi0() holds.
     */
    float getPhi0() const { return m_phi0; }

    /** Getter for the cosine of the azimuth angle of travel direction at the perigee. */
    double getCosPhi0() const { return std::cos(double(getPhi0())); }

    /** Getter for the cosine of the azimuth angle of travel direction at the perigee. */
    double getSinPhi0() const { return std::sin(double(getPhi0())); }

    /** Getter for omega, which is a signed curvature measure of the track. The sign is equivalent to the charge of the particle. */
    float getOmega() const { return m_omega; }

    /** Getter for z0, which is the z coordinate of the perigee. */
    float getZ0() const { return m_z0; }

    /** Getter for tan lambda, which is the z over arc length slope of the track. */
    float getTanLambda() const { return m_tanLambda; }

    /** Getter for cot theta, which is the z over arc length slope of the track. Synomym to tan lambda. */
    float getCotTheta() const { return m_tanLambda; }
    /// @}

    ///--------------------------------------------------------------------------------------------------------------------------
  private:
    //---------------------------------------------------------------------------------------------------------------------------
    //--- Functions for internal conversions between cartesian and perigee helix parameters
    //--- This can be placed in a seperate header which handles all the conversion stuff
    //---------------------------------------------------------------------------------------------------------------------------

    /** Cartesian to Perigee conversion.
     */
    void setCartesian(const TVector3& position,
                      const TVector3& momentum,
                      const short int charge,
                      const float bZ);

    /** Memory for the signed distance to the perigee. The sign is the same as of the z component of getPerigee().Cross(getMomentum()).*/
    float m_d0;

    /** Memory for the azimuth angle between the transverse momentum and the x axis, which is in [-pi, pi]. */
    float m_phi0;

    /** Memory for the curvature of the signed curvature*/
    float m_omega;

    /** Memory for the z coordinate of the perigee. */
    float m_z0;

    /** Memory for the slope of the track in the z coordinate over the arclength (dz/ds)*/
    float m_tanLambda;

    /** Streamer version 1. */
    ClassDef(Helix, 1);
  };
}

