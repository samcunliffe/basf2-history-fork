/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

namespace Belle2 {
  /** Internal module used by Path.add_skim_path(). Don't use it directly. */
  class SwitchDsModule : public Module {

  public:

    /**
     * Constructor.
     * Sets the description, the properties and the parameters of the module.
     */
    SwitchDsModule();

    ~SwitchDsModule();

    virtual void initialize();
    virtual void beginRun();
    virtual void endRun();
    virtual void event();
    virtual void terminate();

  private:
    std::string m_from; /**< active DataStore ID before this module. */
    std::string m_to; /**< active DataStore ID after this module. */
    bool m_doCopy; /**< should data be copied to m_to? */
  };
}
