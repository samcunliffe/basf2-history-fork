/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Peter Kvasnicka, Zbynek Drasal                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PXDCHEATER_H
#define PXDCHEATER_H

#include <framework/gearbox/Unit.h>

namespace Belle2 {

  /**
   * PXD cheater class.
   *
   * The class provides detector pitch values which are currently nowhere to read
   * from, and provides simulation resolutions for the Gaussian-smearing
   * digitizer. It is a temporary class.
   * The class uses spline interpolants based on MC estimates of sensor
   * resolutions. Sensor resolutions are parametrized by a single parameter,
   * the theta angle of impact. Interpolants are spline fits to the data. Their
   * codes are automatically generated by the ROOT TSpline class.
   */

  class PXDCheater {

  public:
    /**
     * The constructor just sets defaults.
     */
    PXDCheater() {
      m_pitchRPhi1 = 50;
      m_pitchRPhi2 = 50;
      m_pitchZ1 = 50;
      m_pitchZ2 = 75;
    }

    ~PXDCheater() {
      // Does nothing.
    }
    /** Return z resolution estimate for a specific sensor.*/
    double getResolutionZ(int iLayer, int iLadder, int iSensor, double theta) const;

    /** Return r-phi resolution estimate for a specific sensor.*/
    double getResolutionRPhi(int iLayer, int iLadder, int iSensor, double theta) const;

    /** Return z pitch for a specific sensor.*/
    double getPitchZ(int iLayer, int iLadder, int iSensor) const;

    /** Return r-phi pitch for a specific sensor.*/
    double getPitchRPhi(int iLayer, int iLadder, int iSensor) const;


  protected:
    /** Spline interpolant for r-phi resolution in PXD layer 1.*/
    double PXD_ResFunc_rphi1(double x) const;
    /** Spline interpolant for r-phi resolution in PXD layer 2.*/
    double PXD_ResFunc_rphi2(double x) const;
    /** Spline interpolant for z resolution in PXD layer 1.*/
    double PXD_ResFunc_z1(double x) const;
    /** Spline interpolant for z resolution in PXD layer 2.*/
    double PXD_ResFunc_z2(double x) const;

  private:
    /** Pitch values. */
    double m_pitchRPhi1;        /**< r-phi pitch, PXD layer 1.*/
    double m_pitchRPhi2;        /**< r-phi pitch, PXD layer 2.*/
    double m_pitchZ1;           /**< z pitch, PXD layer 1.*/
    double m_pitchZ2;           /**< z pitch, PXD layer 2.*/

  }; // class PXDCheater

} // end namespace Belle2

#endif
