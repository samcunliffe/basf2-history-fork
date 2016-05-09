/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <top/dbobjects/TOPGeoBarSegment.h>

namespace Belle2 {

  /**
   * Geometry parameters of a mirror segment
   */
  class TOPGeoMirrorSegment: public TOPGeoBarSegment {
  public:

    /**
     * Default constructor
     */
    TOPGeoMirrorSegment()
    {}

    /**
     * Useful constructor
     * @param width mirror segment width
     * @param thickness mirror segment thickness
     * @param length mirror segment length
     * @param material mirror segment material name
     */
    TOPGeoMirrorSegment(double width, double thickness, double length,
                        const std::string& material):
      TOPGeoBarSegment(width, thickness, length, material)
    {}

    /**
     * Sets spherical mirror radius of curvature
     * @param radius radius
     */
    void setRadius(double radius) {m_radius = radius;}

    /**
     * Sets spherical mirror center of curvature
     * @param xc x-coordinate
     * @param yc y-coordinate
     */
    void setCenterOfCurvature(double xc, double yc)
    {
      m_xc = xc;
      m_yc = yc;
    }

    /**
     * Sets parameters of reflective coating
     * @param thickness coating thickness
     * @param material material name
     * @param surface optical surface properties
     */
    void setCoating(double thickness,
                    const std::string& material,
                    const GeoOpticalSurface& surface)
    {
      m_coatingThickness = thickness;
      m_coatingMaterial = material;
      m_coatingSurface = surface;
    }

    /**
     * Returns spherical mirror radius of curvature
     * @return radius
     */
    double getRadius() const {return m_radius / s_unit;}

    /**
     * Returns spherical mirror center of curvature in x
     * @return center of curvature in x
     */
    double getXc() const {return m_xc / s_unit;}

    /**
     * Returns spherical mirror center of curvature in y
     * @return center of curvature in y
     */
    double getYc() const {return m_yc / s_unit;}

    /**
     * Returns reflective coating thickness
     * @return thickness
     */
    double getCoatingThickness() const {return m_coatingThickness / s_unit;}

    /**
     * Returns reflective coating material
     * @return material name
     */
    const std::string& getCoatingMaterial() const {return m_coatingMaterial;}

    /**
     * Returns reflective coating optical surface
     * @return optical surface
     */
    const GeoOpticalSurface& getCoatingSurface() const {return m_coatingSurface;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const;

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "Mirror segment geometry parameters") const;

  private:

    float m_radius = 0; /**< spherical mirror radius */
    float m_xc = 0;     /**< spherical mirror center-of-curvature in x */
    float m_yc = 0;     /**< spherical mirror center-of-curvature in y */
    float m_coatingThickness = 0;       /**< reflective coating thickness */
    std::string m_coatingMaterial;      /**< reflective coating material */
    GeoOpticalSurface m_coatingSurface; /**< reflective coating optical surface */

    ClassDef(TOPGeoMirrorSegment, 1); /**< ClassDef */

  };

} // end namespace Belle2
