/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2018 - Belle II Collaboration                             *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#pragma once

#include <analysis/NtupleTools/NtupleFlatTool.h>

#include <analysis/dataobjects/Particle.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>

#include <TTree.h>
#include <string>
#include <utility>
#include <vector>
namespace Belle2 {

  /**
   * NtupleTool to write the kinematics of the recoil system that recoils against given particle.
   */
  class NtupleiTOPCalibTool : public NtupleFlatTool {

  private:

    std::vector<float> m_nDigits; /**< number of iTOP Digits in the module traversed by the given Particle */
    std::vector<float> m_nReflected; /**< number of reflected iTOP Digits */
    std::vector<float> m_maxGap; /**< index of the digit with the largest time difference to the previous hit of all hits */
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();

  public:
    /** Constuctor */
    NtupleiTOPCalibTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}

    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

