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

#include <top/dbobjects/TOPGeoBase.h>

namespace Belle2 {

  /**
   * Displacement parameters of MCP-PMT array.
   * To displace: rotation first, then translation
   */
  class TOPGeoPMTArrayDisplacement: public TOPGeoBase {
  public:

    /**
     * Default constructor
     */
    TOPGeoPMTArrayDisplacement()
    {}

    /**
     * Full constructor
     * @param x translation in x
     * @param y translation in y
     * @param alpha rotation angle around z
     */
    TOPGeoPMTArrayDisplacement(double x, double y, double alpha):
      m_x(x), m_y(y), m_alpha(alpha)
    {}

    /**
     * Returns translation in x
     * @return translation in x
     */
    double getX() const {return m_x / s_unit;}

    /**
     * Returns translation in y
     * @return translation in y
     */
    double getY() const {return m_y / s_unit;}

    /**
     * Returns rotation angle (around z)
     * @return rotation angle
     */
    double getAlpha() const {return m_alpha;}

    /**
     * Check for consistency of data members
     * @return true if values consistent (valid)
     */
    bool isConsistent() const {return true;}

    /**
     * Print the content of the class
     * @param title title to be printed
     */
    void print(const std::string& title = "PMT array displacement parameters") const;

  private:

    float m_x = 0; /**< translation in x */
    float m_y = 0; /**< translation in y */
    float m_alpha = 0; /**< rotation angle around z */

    ClassDef(TOPGeoPMTArrayDisplacement, 1); /**< ClassDef */

  };

} // end namespace Belle2
