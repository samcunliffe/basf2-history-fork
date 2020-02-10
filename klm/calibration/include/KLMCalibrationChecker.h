/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <string>

namespace Belle2 {

  /**
   * KLM calibration checker.
   */
  class KLMCalibrationChecker {

  public:

    /**
     * Constructor.
     */
    KLMCalibrationChecker();

    /**
     * Destructor.
     */
    ~KLMCalibrationChecker();

    /**
     * Set experiment and run numbers.
     */
    void setExperimentRun(int experiment, int run);

    /**
     * Set testing payload name.
     */
    void setTestingPayload(const std::string& testingPayloadName);

    /**
     * Set Global Tag name.
     */
    void setGlobalTag(const std::string& GlobalTagName);

    /**
     * Prepare the database.
     */
    void prepareDatabase();

    /**
     * Check strip efficiency.
     */
    void checkStripEfficiency();

  private:

    /**
     * Experiment number.
     */
    int m_experiment;

    /**
     * Run number.
     */
    int m_run;

    /**
     * Testing payload location.
     */
    std::string m_testingPayloadName = "";

    /**
     * Global Tag name.
     */
    std::string m_GlobalTagName = "";

  };

}

