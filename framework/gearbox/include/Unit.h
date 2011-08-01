/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Martin Heck, Martin Ritter                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef UNIT_H
#define UNIT_H

#include <string>
#include <map>

namespace Belle2 {

  /**
   * The Unit class.
   *
   * This class provides a set of units for the framework. Use these to
   * specify the unit of your value. In addition the class offers methods
   * to convert floating point numbers from a specified unit to the standard
   * unit of the framework.
   *
   * This class can either be used a singleton or by using its static methods.
   */
  class Unit {

  public:

    // standard units
    static const double cm;           /**< Standard of [length] */
    static const double ns;           /**< Standard of [time] */
    static const double rad;          /**< Standard of [angle] */
    static const double GeV;          /**< Standard of [energy, momentum, mass] */
    static const double K;            /**< Standard of [temperature] */
    static const double T;            /**< Standard of [magnetic field] */
    static const double e;            /**< Standard of [electric charge] */
    static const double g_cm3;        /**< Standard of [density] */

    // length units
    static const double km;           /**< [kilometers] */
    static const double m;            /**< [meters] */
    static const double mm;           /**< [millimeters] */
    static const double um;           /**< [micrometers] */
    static const double nm;           /**< [nanometers] */
    static const double fm;           /**< [femtometers] */

    // area units
    static const double m2;           /**< [square meters] */
    static const double cm2;          /**< [square centimeters] */
    static const double mm2;          /**< [square millimeters] */

    static const double b;            /**< [barn] */
    static const double mb;           /**< [millibarn] */
    static const double ub;           /**< [microbarn] */
    static const double nb;           /**< [nanobarn] */
    static const double pb;           /**< [picobarn] */
    static const double fb;           /**< [femtobarn] */
    static const double ab;           /**< [atobarn] */

    // volume units
    static const double m3;           /**< [cubic meters] */
    static const double cm3;          /**< [cubic centimeters] */
    static const double mm3;          /**< [cubic millimeters] */

    // time units
    static const double s;            /**< [second] */
    static const double ms;           /**< [millisecond] */
    static const double us;           /**< [microsecond] */
    static const double ps;           /**< [picosecond] */
    static const double fs;           /**< [femtosecond] */

    // angle units
    static const double mrad;         /**< [millirad] */
    static const double deg;          /**< degree to radians */

    // energy units
    static const double eV;           /**< [electronvolt] */
    static const double keV;          /**< [kiloelectronvolt] */
    static const double MeV;          /**< [megaelectronvolt] */
    static const double TeV;          /**< [megaelectronvolt] */
    static const double J;            /**< [joule] */
    static const double V;            /**< [voltage] */

    // charge units
    static const double C;            /**< [coulomb] */
    static const double fC;           /**< [femtocoulomb] */

    // density units
    static const double mg_cm3;       /**< [mg/cm^3] */
    static const double kg_cm3;       /**< [kg/cm^3] */
    static const double g_mm3;        /**< [g/mm^3] */
    static const double mg_mm3;       /**< [mg/mm^3] */
    static const double kg_mm3;       /**< [kg/mm^3] */

    //Various constants
    static const double speedOfLight; /**< [cm/ns] */
    static const double kBoltzmann;   /**< Boltzmann constant in GeV/K. */
    static const double ehEnergy;     /**< Energy needed to create an electron-hole pair in Si at std. T. */
    static const double electronMass; /**< Electron mass in MeV. */
    static const double fineStrConst; /**< The fine structure constant. */
    static const double permSi;       /**< Permittivity of Silicon */
    static const double uTherm;       /**< Thermal Voltage at room temperature */
    static const double eMobilitySi;  /**< Electron mobility in intinsic Silicon at room temperature */

    //SuperKEKB and Belle II constants
    static const double ringCircumference; /**< The circumference of the SuperKEKB ring. */


    /**
     * Converts a floating point value to the standard framework unit.
     *
     * @param value The value which should be converted.
     * @param unitType The unit type of the specified value.
     * @param unitString The string of the unit of the specified value.
     * @return The value converted to the specified unit.
     */
    static double convertValue(double value, const std::string& unitString);

  protected:
    static double registerConversion(const std::string& name, double value);
    /** Map to be able to convert between units using string representations of the unit name */
    static std::map<std::string, double> m_conversionFactors;

  private:
    Unit();
    Unit(const Unit&);
    Unit& operator=(const Unit&);
    ~Unit() {};
  };

}

#endif /* UNIT_H */
