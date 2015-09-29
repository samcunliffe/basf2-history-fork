/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once

#include <cmath>
#include <TMatrixD.h>


#include <tracking/trackFindingCDC/geometry/Helix.h>
#include <tracking/trackFindingCDC/geometry/HelixCovariance.h>

#include <tracking/trackFindingCDC/geometry/UncertainPerigeeCircle.h>
#include <tracking/trackFindingCDC/geometry/UncertainSZLine.h>

namespace Belle2 {

  namespace TrackFindingCDC {

    /// A general helix class including a covariance matrix.
    class UncertainHelix : public Helix {

    public:
      /// Default constructor for ROOT compatibility.
      UncertainHelix() :
        Helix(),
        m_helixCovariance(),
        m_chi2(0.0),
        m_ndf(0)
      {}

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainHelix(const double curvature,
                     const double tangentialPhi,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixCovariance(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0) :
        Helix(curvature, tangentialPhi, impact, tanLambda, z0),
        m_helixCovariance(helixCovariance),
        m_chi2(chi2),
        m_ndf(ndf)
      {}


      explicit UncertainHelix(const TVectorD& parameters,
                              const HelixCovariance& helixCovariance = HelixCovariance(),
                              const double chi2 = 0.0,
                              const size_t& ndf = 0) :
        Helix(parameters),
        m_helixCovariance(helixCovariance),
        m_chi2(chi2),
        m_ndf(ndf)
      {}

      /// Composes an uncertain perigee circle from the  perigee parameters and a 3x3 covariance matrix. Covariance matrix defaults to a zero matrix
      UncertainHelix(const double curvature,
                     const Vector2D& tangential,
                     const double impact,
                     const double tanLambda,
                     const double z0,
                     const HelixCovariance& helixCovariance = HelixCovariance(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0) :
        Helix(curvature, tangential, impact, tanLambda, z0),
        m_helixCovariance(helixCovariance),
        m_chi2(chi2),
        m_ndf(ndf)
      {}



      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const Helix& helix,
                     const HelixCovariance& helixCovariance = HelixCovariance(),
                     const double chi2 = 0.0,
                     const size_t& ndf = 0) :
        Helix(helix),
        m_helixCovariance(helixCovariance),
        m_chi2(chi2),
        m_ndf(ndf)
      {}


      /// Augments a plain helix with a covariance matrix. Covariance defaults to zero.
      UncertainHelix(const UncertainPerigeeCircle& uncertainPerigeeCircle,
                     const UncertainSZLine& uncertainSZLine) :

        Helix(uncertainPerigeeCircle, uncertainSZLine), //copies line and circle without uncertainties
        m_helixCovariance(uncertainPerigeeCircle.perigeeCovariance(), uncertainSZLine.szCovariance()),
        m_chi2(uncertainPerigeeCircle.chi2() + uncertainSZLine.chi2()),
        m_ndf(uncertainPerigeeCircle.ndf() + uncertainSZLine.ndf())
      {}









    public:
      /// Projects the helix into the xy plain carrying over the relevant parts of the convariance matrix.
      UncertainPerigeeCircle uncertainCircleXY() const
      { return UncertainPerigeeCircle(circleXY(), helixCovariance().perigeeCovariance());}

      /// Reduces the helix to an sz line carrying over the relevant parts of the convariance matrix.
      UncertainSZLine uncertainSZLine() const
      { return UncertainSZLine(lineSZ(), helixCovariance().szCovariance());}

      /// Setter for the whole covariance matrix of the perigee parameters
      inline void setHelixCovariance(const HelixCovariance& helixCovariance)
      { m_helixCovariance = helixCovariance; }

      /// Getter for the whole covariance matrix of the perigee parameters
      const HelixCovariance& helixCovariance() const
      { return m_helixCovariance; }

      /// Getter for individual elements of the covariance matrix
      double covariance(const HelixParameterIndex& iRow, const HelixParameterIndex& iCol) const
      { return helixCovariance()(iRow, iCol); }

      /// Getter for individual diagonal elements of the covariance matrix
      double variance(const HelixParameterIndex& i) const
      { return helixCovariance()(i, i); }

      /// Getter for the chi square value of the helix fit
      double chi2() const
      { return m_chi2; }

      /// Setter for the chi square value of the helix fit
      void setChi2(const double chi2)
      { m_chi2 = chi2; }

      /// Getter for the number of degrees of freediom used in the helix fit
      const size_t& ndf() const
      { return m_ndf; }

      /// Setter for the number of degrees of freediom used in the helix fit
      void setNDF(const size_t& ndf)
      { m_ndf = ndf; }

      /// Sets all circle parameters to zero including the covariance matrix
      inline void invalidate()
      {
        Helix::invalidate();
        m_helixCovariance.invalidate();
        m_chi2 = 0.0;
        m_ndf = 0;
      }

    public:
      /// Flips the orientation of the circle in place
      inline void reverse()
      {
        Helix::reverse();
        m_helixCovariance.reverse();
      }

      /// Returns a copy of the circle with opposite orientation.
      inline UncertainHelix reversed() const
      { return UncertainHelix(Helix::reversed(), m_helixCovariance.reversed(), chi2(), ndf()); }

    public:
      /// Moves the coordinate system by the vector by and calculates the new perigee and its covariance matrix. Change is inplace.
      void passiveMoveBy(const Vector3D& by)
      {
        // Move the covariance matrix first to have access to the original parameters
        TMatrixD jacobian = passiveMoveByJacobian(by);
        m_helixCovariance.similarityTransform(jacobian);
        Helix::passiveMoveBy(by);
      }

    private:
      /// Memory for the 5x5 covariance matrix of the helix parameters.
      HelixCovariance m_helixCovariance;

      /// Memory for the chi square value of the fit of this helix.
      double m_chi2;

      /// Memory for the number of degrees of freedim of the fit of this helix.
      size_t m_ndf;


    }; //class

  } // namespace TrackFindingCDC
} // namespace Belle2
