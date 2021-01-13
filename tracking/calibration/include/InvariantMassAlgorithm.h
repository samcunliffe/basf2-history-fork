/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Radek Zlebcik                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <calibration/CalibrationAlgorithm.h>

namespace Belle2 {
  /**
   * Class implementing InvariantMass calibration algorithm
   */
  class InvariantMassAlgorithm : public CalibrationAlgorithm {
  public:

    /// Constructor set the prefix to BoostVectorCollector
    InvariantMassAlgorithm();

    /// Destructor
    virtual ~InvariantMassAlgorithm() {}

    void setInnerLoss(TString loss) { m_lossFunctionInner = loss; }
    void setOuterLoss(TString loss) { m_lossFunctionOuter = loss; }

  protected:

    /// Run algo on data
    virtual EResult calibrate() override;

  private:
    TString m_lossFunctionOuter = "pow(rawTime - 2.0, 2) + 10 * pow(maxGap, 2)";
    TString m_lossFunctionInner = "pow(rawTime - 0.5, 2) + 10 * pow(maxGap, 2)";
  };
} // namespace Belle2


