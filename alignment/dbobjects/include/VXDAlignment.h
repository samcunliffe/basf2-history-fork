/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Tadeas Bilka                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/DBObjCalibrationConstMapBase.h>
#include <vxd/dataobjects/VxdID.h>

namespace Belle2 {
  /// VXD alignment (and maybe some calibration) parameters
  class VXDAlignment: public DBObjCalibrationConstMapBase {
  public:
    // Alignment parameters unique identifiers:
    /// Sensor/layer/ladder alignment in local u
    static const baseType dU = 1;
    /// Sensor/layer/ladder alignment in local v
    static const baseType dV = 2;
    /// Sensor/layer/ladder alignment in local w
    static const baseType dW = 3;
    /// Sensor/layer/ladder alignment in local alpha
    static const baseType dAlpha = 4;
    /// Sensor/layer/ladder alignment in local beta
    static const baseType dBeta = 5;
    /// Sensor/layer/ladder alignment in local gamma
    static const baseType dGamma = 6;
    // Calibrations (tentative):
    /// Correction of common Lorentz shift w.r.t. nominal
    static const baseType dLorentzAngle = 11;
    /// Correction of Lorentz shift w.r.t. common per layer
    static const baseType dLayerLorentzAngle = 12;

    static unsigned short getGlobalUniqueID() {return 1;}

    /// Constructor
    VXDAlignment() : DBObjCalibrationConstMapBase() {}
    /// Destructor
    ~VXDAlignment() {}
  private:
    ClassDef(VXDAlignment, 2); /**< VXD allignment and calibration constants */
  };
} // end namespace Belle2

