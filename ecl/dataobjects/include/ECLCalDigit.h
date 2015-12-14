/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ECLCALDIGIT_H
#define ECLCALDIGIT_H

#include <framework/datastore/RelationsObject.h>
namespace Belle2 {

  /*! Class to store calibrated ECLDigits: ECLCalDigits
   */

  class ECLCalDigit : public RelationsObject {
  public:

    // status enumerator
    enum StatusBit {
      c_IsEnergyCalibrated = 1 << 0,
      c_IsTimeCalibrated = 1 << 1,
      c_IsCalibrated = c_IsEnergyCalibrated | c_IsTimeCalibrated,
    };

    /** default constructor for ROOT */
    ECLCalDigit()
    {
      m_CellId = 0; /**< Cell ID */
      m_Time   = 0; /**< Calibrated Time */
      m_Energy = 0; /**< Calibrated Energy */
      m_Status = 0; /**< Calibration Status */
    }

    /*! Set  Cell ID
     */
    void setCellId(int CellId) { m_CellId = CellId; }

    /*! Set Calibrated Energy
     */
    void setEnergy(double Energy) { m_Energy = Energy; }

    /*! Set Calibrated Time
     */
    void setTime(double Time) { m_Time = Time; }

    /*! Set Calibration Status (overwrites previously set bits)
     */
    void setStatus(unsigned short int status) { m_Status = status; }

    /*! Add Calibration Status
     */
    void addStatus(unsigned short int bitmask) { m_Status |= bitmask; }

    /*! Remove Calibration Status
     */
    void removeStatus(unsigned short int bitmask) { m_Status &= (~bitmask); }

    /*! Get Cell ID
     * @return cell ID
     */
    int getCellId() const { return m_CellId; }

    /*! Get Calibrated Energy
     * @return Calibrated Energy
     */
    double getEnergy() const { return m_Energy; }

    /*! Get Calibrated Time
     * @return Calibrated Time
     */
    double getTime() const { return m_Time; }

    /*! Get Calibration Status
     * @return Calibration Status
     */
    bool hasStatus(unsigned short int bitmask) const { return (m_Status & bitmask) == bitmask; }

    /*! Get Boolean Calibration Status
    * @return Calibration Status
    */
    bool isCalibrated() const;

    /*! Get Boolean Energy Calibration Status
    * @return Energy Calibration Status
    */
    bool isEnergyCalibrated() const;

    /*! Get Boolean Time Calibration Status
     * @return Time Calibration Status
     */
    bool isTimeCalibrated() const;

  private:

    int m_CellId;   /**< Cell ID */
    double m_Time;   /**< Calibrated Time */
    double m_Energy; /**< Calibrated Energy */
    unsigned short int m_Status;   /**< Calibration Status */

    ClassDef(ECLCalDigit, 1); /**< ClassDef */

  };

  // inline
  inline bool ECLCalDigit::isCalibrated() const
  {
    return hasStatus(c_IsCalibrated);
  }

  // inline
  inline bool ECLCalDigit::isEnergyCalibrated() const
  {
    return hasStatus(c_IsEnergyCalibrated);
  }

  // inline
  inline bool ECLCalDigit::isTimeCalibrated() const
  {
    return hasStatus(c_IsTimeCalibrated);
  }

} // end namespace Belle2

#endif
