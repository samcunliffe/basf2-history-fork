/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Maximilian Welsch, Pascal Schmolz                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/VariableManager/Utility.h>

#include <TLorentzVector.h>

#include <string>
#include <memory>


namespace Belle2 {
  /**
   * Inclusive D* reconstruction module.
   *
   * This module creates a D* particle list by estimating the D* four momenta
   * from slow pions, specified by a given cut. The D* energy is approximated
   * as  E(D*) = m(D*)/(m(D*) - m(D)) * E(pi). The absolute value of the D*
   * momentum is calculated using the D* PDG mass and the direction is collinear
   * to the slow pion direction.
   *
   * The decay must be specified by a DecayString of the following forms
   * - [D*+ -> pi+ ...]
   * - [D*+ -> pi0 ...]
   * - [D*0 -> pi0 ...]
   * where the ellipsis is essential for truth-matching.
   *
   * Note that the inclusive reconstruction of a D* by a pi0 is ambigious as
   * the flavor of the D* cannot be determined. Therefore there are two D*
   * candidates for every pi0 with both flavors.
   */
  class InclusiveDstarReconstructionModule : public Module {

  public:

    /**
     * Constructor.
     */
    InclusiveDstarReconstructionModule();

    /**
     * Destructor.
     */
    virtual ~InclusiveDstarReconstructionModule();

    /**
     * Initialize the Module.
     * This method is called at the beginning of data processing.
     */
    virtual void initialize() override;

    /**
     * Event Processor.
     */
    virtual void event() override;


  private:

    /**
     * Estimates the D* four momentum given a slow pion
     *
     * @param pion - a pion particle
     * @returns estimated D* four momentum
     * */
    TLorentzVector estimateDstarFourMomentum(const Particle* pion);

    /**
     * Checks if the given pion is list if compatible with the charge
     * of the D* particle
     *
     * @param pion_pdg_code - PDG code of the processed pion particle
     * @returns true (false) if pion pdg code is compatible (not compatible) with the current D* list
     * */
    bool pionCompatibleWithDstar(int pion_pdg_code);

    DecayDescriptor m_decaydescriptor; /**< Decay descriptor for parsing the user specifed DecayString */

    std::unique_ptr<Variable::Cut> m_cut; /**< cut object which performs the cuts */

    std::string m_pionListName;  /**< Name of the input pion particle list */
    std::string m_decayString;  /**< Input DecayDescriptor string */
    std::string m_outputListName;  /**< Name of the output D* particle list */
    std::string m_outputAntiListName;  /**< Name of the output anti-D* particle list */
    std::string m_slowPionCut;  /**< Cut used to identify slow pions */

    int m_properties; /**< Decay property: ignore massive daughter particle */
    int m_dstar_pdg_code; /**< PDG code of the given D* particle list */

    float m_dstar_pdg_mass; /**< PDG mass of the mother-D* */
    float m_d_pdg_mass; /**< PDG mass of the daughter-D  */
  };

}
