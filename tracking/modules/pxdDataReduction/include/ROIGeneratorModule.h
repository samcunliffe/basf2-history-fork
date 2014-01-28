/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa, Eugenio Paoloni                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ROI_GENERATOR_H_
#define ROI_GENERATOR_H_

#include <framework/core/Module.h>

namespace Belle2 {

  /** The ROI generator Module
   *
   * This module is used to generate a certain number of ROIs
   *
   */

  class ROIGeneratorModule : public Module {

  public:

    /**
     * Constructor of the module.
     */
    ROIGeneratorModule();

    /**
     * Destructor of the module.
     */
    virtual ~ROIGeneratorModule();

    /**
     *Initializes the Module.
     */
    virtual void initialize();

    virtual void beginRun();

    virtual void event();

    virtual void endRun();

    /**
     * Termination action.
     */
    virtual void terminate();

  protected:

    std::string m_ROIListName;
    int m_divider;
    int m_layer  ;
    int m_ladder ;
    int m_sensor ;

    int m_minU   ;
    int m_maxU   ;

    int m_minV   ;
    int m_maxV   ;


    int m_nROIs;

  };
}
#endif
