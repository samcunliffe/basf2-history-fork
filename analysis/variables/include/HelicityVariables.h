/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Phillip Urquijo, Fernando Abudinen, Michael de Nuccio    *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/VariableManager/Manager.h>
#include <vector>
#include <string>

namespace Belle2 {
  class Particle;

  namespace Variable {

    /**
     * If the given particle has two daughters: cosine of the angle between the line defined by the momentum difference
     * of the two daughters in the frame of the given particle (mother) and the momentum of the given particle in the lab frame.
     * If the given particle has three daughters: cosine of the angle between the normal vector of the plane
     * defined by the momenta of the three daughters in the frame of the given particle (mother) and the momentum of the given particle in the lab frame.
     * Else: 0.
     */
    double cosHelicityAngle(const Particle* part);

    /**
     * To be used for the decay pi0 -> e+ e- gamma: cosine of the angle between the momentum of the gamma in the frame
     * of the given particle (mother) and the momentum of the given particle in the lab frame.
     * Else: 0.
     */
    double cosHelicityAnglePi0Dalitz(const Particle* part);

    /**
     * Cosine of the helicity angle of the i-th (where 'i' is the parameter passed to the function) daughter of the particle provided"
     * assuming that the mother of the provided particle correspond to the Centre of Mass System, whose parameters are
     * automatically loaded by the function, given the accelerators conditions.
     */
    Manager::FunctionPtr cosHelicityAngleIfCMSIsTheMother(const std::vector<std::string>& arguments);

    /**
     * Cosine of the helicity angle between the momentum of the provided particle and the momentum of the selected granddaughter
     * in the reference frame of the selected daughter (theta_1 and theta_2 in the PDG 2018, p. 722).
     * This variable needs two integer arguments: the first one is the index of the daughter and the second one is the index of the granddaughter.
     * For example, in the Decay B0 -> [J/psi -> mu+ mu-] [K*0 -> K+ pi-], if the provided particle is B0 and the selected indices are (0, 0), the variable will return the
     * angle between the momentum of the B0 and the momentum of the mu+, both momenta in the rest frame of the J/psi.
     * This variable is needed for angular analyses of B meson decays into two vector particles.             */
    Manager::FunctionPtr cosHelicityAngleIfRefFrameIsTheDaughter(const std::vector<std::string>& arguments);

    /**
     * Cosine of the acoplanarity angle (Phi in the PDG 2018, p. 722). Given a two-body decay, the acoplanarity angle is defined as
     * the angle between the two decay planes in the reference frame of the mother. We calculate the acoplanarity angle as the angle between the two
     * normal vectors of the decay planes. Each normal vector is the cross product of the momentum of one daughter (in the frame of the mother) and the momentum of
     * one of the granddaughters (in the reference frame of the daughter).
     * This variable requires two integer arguments: the first one is the index of the first grand daughter, and the second one the index of the second granddaughter.
     * For example, in the Decay B0 -> [J/psi -> mu+ mu-] [K*0 -> K+ pi-], if the provided particle is B0 and the selected indices are (0, 0), the variable will return the
     * acoplanarity using the mu+ and the K+ granddaughters.
     * */
    Manager::FunctionPtr cosAcoplanarityAngleIfRefFrameIsTheMother(const std::vector<std::string>& arguments);

  }
} // Belle2 namespace
