/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLM_LINE2D_H
#define EKLM_LINE2D_H

/* External headers. */
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Geometry/Vector3D.h>

/* belle2 headers. */
#include <eklm/geometry/Circle2D.h>

namespace Belle2 {

  namespace EKLM {

    /**
     * 2D line. Equation: m_Point + m_Vector * t
     */
    class Line2D {

    public:

      /**
       * Constructor.
       * @param[in] x    Initial point X coordinate.
       * @param[in] y    Initial point y coordinate.
       * @param[in] vecx Vector X component.
       * @param[in] vecy Vector Y component.
       */
      Line2D(double x, double y, double vecx, double vecy);

      /**
       * Destructor.
       */
      ~Line2D();

      /**
       * Find intersections with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2]) const;

    protected:

      /**
       * Find intersections with a circle.
       * @param[in]  circle        Circle.
       * @param[out] intersections Intersections.
       * @param[out] t             Values of t for intersection points.
       * @return Number of intersections (0, 1 or 2).
       */
      int findIntersection(const Circle2D& circle,
                           HepGeom::Point3D<double> intersections[2],
                           double t[2]) const;

      /** Initial point. */
      HepGeom::Point3D<double> m_Point;

      /** Vector. */
      HepGeom::Vector3D<double> m_Vector;

    };

  }

}

#endif

