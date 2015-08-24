/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Oliver Frost                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <tracking/trackFindingCDC/numerics/BasicTypes.h>
#include <tracking/trackFindingCDC/numerics/numerics.h>

#include <cmath>

#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/geometry/Vector2D.h>
#include <tracking/trackFindingCDC/geometry/Vector3D.h>

namespace Belle2 {
  namespace TrackFindingCDC {
    /**
     * A three dimensional limited line represented by its closest approach to the z-axes (reference position ) and its skew parameter.
     * The representation is only suitable for lines out of the xy plane.
     * The endpoints are stored encoded by their z position.
     * The end point with the higher z value is called forward, the one with the smaller is called backward.
     * This class is mainly used to descripe the cdc wires with as few parameters as possible and
     * simplifies the retrival of the two dimensional track reference position, which is taken at
     * the closest approach to the beam z-axes.
     * @brief A three dimensional limited line
     *
     */
    class BoundSkewLine  {

    public:

      /// Default constructor for ROOT compatibility.
      BoundSkewLine() : m_referencePosition(), m_forwardZ(0.0) , m_backwardZ(0.0), m_skew(0.0) {;}

      /// Constuctor for a skew line between forward and backward point
      BoundSkewLine(const Vector3D& forwardIn , const Vector3D& backwardIn);

      /// Empty deconstructor
      ~BoundSkewLine() {;}

      /// Returns a copy of the skew line moved by a three dimensional offset
      inline BoundSkewLine movedBy(const Vector3D& offset) const
      { return BoundSkewLine(forward3D().add(offset), backward3D().add(offset)); }

      /// Returns a copy of the skew line moved by a two dimensional offset
      inline BoundSkewLine movedBy(const Vector2D& offset) const
      { return BoundSkewLine(forward3D().add(offset), backward3D().add(offset)); }

      /// Gives the three dimensional position of the line at the given z value
      inline Vector3D pos3DAtZ(const FloatType& z) const
      { return Vector3D(pos2DAtZ(z), z); }

      /// Gives the two dimensional position of the line at the given z value
      inline Vector2D pos2DAtZ(const FloatType& z) const
      { return refPos2D() + movePerZ() * (z - refZ()); }

      /// Gives the position of the forward point
      inline Vector3D forward3D() const
      { return pos3DAtZ(forwardZ()) ; }

      /// Gives the xy position of the forward point
      inline Vector2D forward2D() const
      { return pos2DAtZ(forwardZ()) ; }

      /// Gives the position of the backward point
      inline Vector3D backward3D() const
      { return pos3DAtZ(backwardZ()) ; }

      /// Gives the xy position of the backward point
      inline Vector2D backward2D() const
      { return pos2DAtZ(backwardZ()) ; }

      /// Gives the position of the point half way between forward and backward
      inline Vector3D center3D() const
      { return pos3DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the xy position of the point half way between forward and backward
      inline Vector2D center2D() const
      { return pos2DAtZ((forwardZ() + backwardZ()) / 2) ; }

      /// Gives the tangential vector to the line
      inline Vector3D tangential3D() const
      {
        FloatType deltaZ = forwardZ() - backwardZ();
        return Vector3D(movePerZ() * deltaZ, deltaZ);
      }

      /// Gives the tangential xy vector to the line
      inline Vector2D tangential2D() const
      {
        FloatType deltaZ = forwardZ() - backwardZ();
        return movePerZ() * deltaZ;
      }

      /// Gives the positional move in the xy projection per unit z.
      inline Vector2D movePerZ() const
      {
        return Vector2D(-m_skew * refY(),
                        m_skew * refX());
      }

      /// Gives the forward z coodinate
      inline FloatType forwardZ() const
      { return m_forwardZ; }

      /// Gives the backward z coodinate
      inline FloatType backwardZ() const
      { return m_backwardZ; }

      /// Gives the forward azimuth angle
      inline FloatType forwardPhi() const
      { return  forward2D().phi(); }

      /// Gives the backward azimuth angle
      inline FloatType backwardPhi() const
      { return backward2D().phi(); }

      /// Gives the cylindrical radius of the forward position
      inline FloatType forwardCylindricalR() const
      { return  forward2D().cylindricalR(); }

      /// Gives the cylindrical radius of the backward position
      inline FloatType backwardCylindricalR() const
      { return backward2D().cylindricalR(); }

      /// Gives the azimuth angle of the forward position relative to the reference position
      inline FloatType forwardPhiToRef() const
      { return  forward2D().angleWith(refPos2D()); }

      /// Gives the azimuth angle of the backward position relative to the reference position
      inline FloatType backwardPhiToRef() const
      { return backward2D().angleWith(refPos2D()); }

      /// Gives the azimuth angle difference from backward to forward position
      /* backwardToForwardAngle means how far the backward position has to be rotated in the xy projection
         in the mathematical positiv sense that it seems to be coaligned with the forward position. */
      inline FloatType backwardToForwardAngle() const
      { return backward2D().angleWith(forward2D()) ; }

      /// Returns the closest approach to the give point
      /** This calculates the point of closest approach on the line. It does not care about the boundaries of the line. */
      inline Vector3D closest3D(const Vector3D& point) const
      { return  refPos3D() - (point - refPos3D()).parallelVector(tangential3D()); }

      /// Returns the point of closest approach to the origin on the line
      inline Vector3D closestToOrigin3D() const
      { return refPos3D() - refPos3D().parallelVector(tangential3D()); }

      /// Calculates the distance of the given point to the line
      inline FloatType distance(const Vector3D& pos3D) const
      { return (pos3D - refPos3D()).orthogonalComp(tangential3D()); }

      /** Returns the tan lambda of the line
       *  Also know as dz / ds
       */
      inline FloatType tanLambda() const
      { return 1 / movePerZ().norm(); }

      /// Returns the lambda.
      inline FloatType lambda() const
      { return std::atan(tanLambda()); }

      /** Returns the tangent of the opening angle between tangential vector and the z axes
       *  Also know as ds / dz
       */
      inline FloatType tanTheta() const
      { return std::atan(movePerZ().norm()); }

      /// Returns the opening angle between tangential vector and the z axes.
      inline FloatType theta() const
      { return std::atan(movePerZ().norm()); }

      /// Returns the z coordinate of the point of closest approach to the z axes.
      inline FloatType perigeeZ() const
      { return -refPos2D().dot(movePerZ()) / movePerZ().normSquared(); }

      /// Returns the point of closest approach to the z axes.
      inline Vector3D perigee3D() const
      { return pos3DAtZ(perigeeZ()); }

      /// Returns the point of closest approach to the z axes.
      inline Vector2D perigee2D() const
      { return refPos2D().orthogonalVector(movePerZ()); }

      /// Returns the the x coordinate of the reference point.
      inline const FloatType& refX() const
      { return m_referencePosition.x(); }

      /// Returns the the y coordinate of the reference point.
      inline const FloatType& refY() const
      { return m_referencePosition.y(); }

      /// Returns the the z coordinate of the reference point.
      inline const FloatType& refZ() const
      { return m_referencePosition.z(); }

      /// Returns the cylindrical radius of the reference position
      inline FloatType refCylindricalRSquared() const
      { return m_referencePosition.cylindricalRSquared(); }

      /// Returns the xy vector of the reference position
      inline const Vector2D& refPos2D() const
      { return m_referencePosition.xy(); }

      /// Returns the reference position
      inline const Vector3D& refPos3D() const
      { return m_referencePosition; }

    private:
      /// Memory for the reference postion
      Vector3D m_referencePosition;

      /// Memory for the forward end z coordinate.
      FloatType m_forwardZ;

      /// Memory for the backward end z coordinate.
      FloatType m_backwardZ;

      /// Memory for the skew parameter
      FloatType m_skew;

    }; //class
  } // namespace TrackFindingCDC
} // namespace Belle2
