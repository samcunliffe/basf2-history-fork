#pragma once
/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2013 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Pulvermacher                                   *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#include <framework/core/Module.h>
#include <analysis/VariableManager/Manager.h>

#include <string>

namespace Belle2 {

  /** Module to calculate variable specified by the user and set return value accordingly
   */
  class VariableToReturnValueModule : public Module {
  public:
    /** Constructor. */
    VariableToReturnValueModule();
    /** Destructor. */
    ~VariableToReturnValueModule() {}

    /** Initialize */
    void initialize();
    /** Method called for each event. */
    void event();

  private:
    /** function pointer corresponding to given variable. */
    Variable::Manager::FunctionPtr m_function;
    std::string m_variable;

  };
} // end namespace Belle2
