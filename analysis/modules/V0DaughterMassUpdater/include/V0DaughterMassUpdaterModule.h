/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Yefan Tao    ustctao@ufl.edu                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once
#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>


namespace Belle2 {

  /**This module replaces the mass of two daughters of the selected V0 particles inside the given particleLists with masses of given pdgCode.*/

  class V0DaughterMassUpdaterModule : public Module {
  private:
    int m_pdg_pos_dau; /**< PDG code for V0's positive daughter */
    int m_pdg_neg_dau; /**< PDG code for V0's negative daughter */

    /** Name of the lists */
    std::vector<std::string> m_strParticleLists;

  public:
    /** Constructor. */
    V0DaughterMassUpdaterModule();

    /** Method called for each event. */
    virtual void event() override;
  };
} // end namespace Belle2


