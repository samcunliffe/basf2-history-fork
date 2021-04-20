/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018-2020 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de)                    *
 *               Sam Cunliffe  (sam.cunliffe@desy.de)                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <vector>
#include <string>
#include <analysis/VariableManager/Manager.h>


namespace Belle2 {

  class Particle;

  namespace Variable {

    /**
     * returns true if any L1 PSNM bit is true
     */
    double L1Trigger(const Particle*);

    /**
     * returns L1 trigger time type
     */
    double L1TimeType(const Particle*);

    /**
     * returns L1 trigger time quality
     */
    double L1TimeQuality(const Particle*);

    /**
     * returns 1 if poisson random trigger is within injection veto window
     */
    double isPoissonInInjectionVeto(const Particle*);

    /**
     * returns 1 if the event passes a given software trigger identifier, NAN if
     * the trigger identifier is not found
     */
    Manager::FunctionPtr softwareTriggerResult(const std::vector<std::string>& args);

    /**
     * returns 1 if the event would have passed a given software trigger identifier,
     * if this trigger would not be prescaled. Please note, this is not the
     * final HLT decision! NAN if the trigger identifier is not found.
     */
    Manager::FunctionPtr softwareTriggerResultNonPrescaled(const std::vector<std::string>& args);

    /**
     * returns the prescaling for the specific software trigger identifier.
     * Please note, this prescaling is taken from the currently setup database. It only corresponds
     * to the correct HLT prescale if you are using the online database!
     * Returns NAN if the trigger identifier is not found.
     */
    Manager::FunctionPtr softwareTriggerPrescaling(const std::vector<std::string>& args);

    /**
     * returns 1 if the event passes the HLT
     */
    double passesAnyHighLevelTrigger(const Particle*);
  }
}
