/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Andreas Moll, Hiroyuki Nakayama                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef BFIELDCOMPONENTQUAD_H
#define BFIELDCOMPONENTQUAD_H

#include <geometry/bfieldmap/BFieldComponentAbs.h>

#include <boost/array.hpp>
#include <string>

namespace Belle2 {

  /**
   * The BFieldComponentQuad class.
   *
   * This class represents a magnetic field map from quadrupole magnets QC[1/2]*.
   * Only the field from QC[1/2]* inside beam pipes is described.
   * Leak field inside the beam pipe from the quadruple magnet on the other beam line is also included,
   * however, leak field outside both beam pipe are not described.
   * Therefore, we should turn on this field component only when we use
   * TouschekTURTLEReader and Synchrotron radiation study,
   * which needs accurate propagation of beam particles.
   * Field strength are calculated from the magnet parameters (K0,K1,SK0,SK1) used in accelerator simulation.
   * These parameters are provided for each 4cm slice in the beam direction.
   */
  class BFieldComponentQuad : public BFieldComponentAbs {

  public:

    struct ApertPoint {
      double s; // in [mm]
      double r; // in [mm]
    };

    struct ParamPoint {
      double s;   //in [m]
      double L;   //in [m]
      double K0;  // in [dimensionless]
      double K1;  // in [1/m]
      double SK0; // in [dimensionless]
      double SK1; // in [1/m]
      double ROTATE; // in [radian]
      double DX;   //in [m]
      double DY;   //in [m]
      /* Note that K parameters used in SAD is multiplied by the element length.
       * Popular definitions are:  K0,SK0[1/m] and K1,SK1[1/m^2]
       */
    };

    /** The BFieldComponentQuad constructor. */
    BFieldComponentQuad();

    /** The BFieldComponentQuad destructor. */
    virtual ~BFieldComponentQuad();

    /**
     * Initializes the magnetic field component.
     * This method opens the magnetic field map file.
     */
    virtual void initialize();

    /**
     * Calculates the magnetic field vector at the specified space point.
     *
     * @param point The space point in Cartesian coordinates (x,y,z) in [cm] at which the magnetic field vector should be calculated.
     * @return The magnetic field vector at the given space point in [T]. Returns a zero vector TVector(0,0,0) if the space point lies outside the region described by the component.
     */
    virtual TVector3 calculate(const TVector3& point) const;

    /**
     * Terminates the magnetic field component.
     * This method closes the magnetic field map file.
     */
    virtual void terminate();

    /**
     * Returns the HER beam pipe aperture at given position.
     * @param s The position in HER beam-axis coordinate.
     * @return The beam pipe aperture at given position.
     */
    double getApertureHER(double s) const;

    /**
     * Returns the LER beam pipe aperture at given position.
     * @param s The position in LER beam-axis coordinate.
     * @return The beam pipe aperture at given position.
     */
    double getApertureLER(double s) const;

    /**
     * Sets the filename of the magnetic field map.
     * @param filenameHER The filename of the magnetic field map for HER.
     * @param filenameHER The filename of the magnetic field map for LER.
     */
    void setMapFilename(const std::string& filenameHER, const std::string& filenameLER) { m_mapFilenameHER = filenameHER; m_mapFilenameLER = filenameLER; };

    /**
     * Sets the filename of aperture definition file.
     * @param filenameHER The filename of the aperture definition for HER.
     * @param filenameLER The filename of the aperture definition for LER.
     */
    void setApertFilename(const std::string& filenameHER, const std::string& filenameLER) { m_apertFilenameHER = filenameHER; m_apertFilenameLER = filenameLER; };

    /**
     * Sets the size of the magnetic field map.
     * @param mapSizeHER The number of points in the HER field parameters.
     * @param mapSizeLER The number of points in the LER field parameters.
     */
    void setMapSize(int sizeHER, int sizeLER) { m_mapSizeHER = sizeHER; m_mapSizeLER = sizeLER; }

    /**
     * Sets the size of the aperture map
     * @param apertSizeHER The number of points in the HER aperture file.
     * @param apertSizeLER The number of points in the LER aperture file.
     */
    void setApertSize(int sizeHER, int sizeLER) { m_apertSizeHER = sizeHER; m_apertSizeLER = sizeLER; }

    /**
     * Sets the standard beam energy used for B calculation from K parameters
     * @param beamEnergyHER The beam energy of HER.
     * @param beamEnergyLER The beam energy of LER.
     */
    void setBeamEnergy(double beamEnergyHER, double beamEnergyLER) { m_beamEnergyHER = beamEnergyHER; m_beamEnergyLER = beamEnergyLER; }

  protected:

  private:

    std::string m_mapFilenameHER, m_mapFilenameLER;    /**< The filename of the magnetic field map */
    std::string m_apertFilenameHER, m_apertFilenameLER; /**< The filename of the aperture*/
    int m_mapSizeHER, m_mapSizeLER;                /**< The size of the map*/
    int m_apertSizeHER, m_apertSizeLER;            /**< The size of the aperture*/
    double m_beamEnergyHER, m_beamEnergyLER;       /**< The beamEnergy*/

    ParamPoint* m_mapBufferHER, *m_mapBufferLER;    /**< The memory buffer for the field parameters. */
    ApertPoint* m_apertBufferHER, *m_apertBufferLER; /**< The memory buffer for the aperture parameters. */

  };

} //end of namespace Belle2

#endif /* BFIELDCOMPONENTQUAD_H */
