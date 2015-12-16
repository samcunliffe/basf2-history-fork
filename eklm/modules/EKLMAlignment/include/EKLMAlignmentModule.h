/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EKLMALIGNMENTMODULE_H
#define EKLMALIGNMENTMODULE_H

/* Belle2 headers. */
#include <framework/core/Module.h>

namespace Belle2 {

  /**
   * Module EKLMAlignmentModule.
   * @details
   * Module for generation of transformation and alignment data.
   */
  class EKLMAlignmentModule : public Module {

  public:

    /**
     * Constructor.
     */
    EKLMAlignmentModule();

    /**
     * Destructor.
     */
    ~EKLMAlignmentModule();

    /**
     * Initializer.
     */
    void initialize();

    /**
     * Called when entering a new run.
     */
    void beginRun();

    /**
     * This method is called for each event.
     */
    void event();

    /**
     * This method is called if the current run ends.
     */
    void endRun();

    /**
     * This method is called at the end of the event processing.
     */
    void terminate();

  private:

    /** Mode. */
    std::string m_Mode;

    /** Name of output file. */
    std::string m_OutputFile;

    /**
     * Generation of zero displacements.
     */
    void generateZeroDisplacement();

    /**
     * Generation of random displacements.
     */
    void generateRandomDisplacement();

    /**
     * Generate random displacements and check if they are correct
     * (no overlaps).
     */
    void studyAlignmentLimits();

  };

}

#endif

