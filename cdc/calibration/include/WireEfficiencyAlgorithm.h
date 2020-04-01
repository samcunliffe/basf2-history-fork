/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:  Henrikas Svidras                                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <TEfficiency.h>
#include <calibration/CalibrationAlgorithm.h>
#include <framework/database/DBObjPtr.h>
#include <cdc/dbobjects/CDCGeometry.h>

#include "string"

namespace Belle2 {
  namespace CDC {
    /**
     * Class for Wire Efficiency estimation.
     */
    class WireEfficiencyAlgorithm: public CalibrationAlgorithm {
    public:
      /// Constructor.
      WireEfficiencyAlgorithm();
      /// Destructor
      ~WireEfficiencyAlgorithm() {}

    protected:
      /// Run algo on data
      EResult calibrate() override;
      ///create histo for each channel
      void buildEfficiencies();

    private:
      TEfficiency* m_efficiencyInLayer[56];  /**< 2D efficiency objects for each layer */
      std::string m_outputFileName = "wire_efficiencies.root"; /**< name of the output file */
      DBObjPtr<CDCGeometry> m_cdcGeo; /**< Geometry of CDC */
    };


  }
}
