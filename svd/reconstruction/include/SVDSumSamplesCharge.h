/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <svd/reconstruction/RawCluster.h>
#include <svd/reconstruction/SVDClusterCharge.h>

#include <svd/calibration/SVDPulseShapeCalibrations.h>

#include <vector>

namespace Belle2 {

  namespace SVD {

    /**
     * Derived Class representing the SVD cluster charge
     * computed summing the max sample of each strip
     */
    class SVDSumSamplesCharge : public SVDClusterCharge {

    public:

      /**
       * @return the strip charge
       */
      double getStripCharge(int indexInRawCluster);

      /**
       * @return the strip charge error
       */
      double getStripChargeError(int indexInRawCluster);

      /**
       * @return the cluster charge
       */
      double getClusterCharge() override;

      /**
       * @return the cluster charge error
       */
      double getClusterChargeError() override;

      /**
       * @return the cluster seed charge
       */
      double getClusterSeedCharge() override;

      /**
       * virtual destructor
       */
      //      virtual ~SVDMaxSampleQCharge();

    private:

      SVDPulseShapeCalibrations m_PulseShapeCal; /**<SVDPulseShaper calibration wrapper*/

    };

  }

}

