/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEFLATTOOL_H
#define NTUPLEFLATTOOL_H
#include <analysis/dataobjects/Particle.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;

namespace Belle2 {
  /** Interface for tools to create flat n-tuples. */
  class NtupleFlatTool {
  protected:
    /** Pointer to the TTree (owned by NtupleOutModule) */
    TTree* m_tree;
    /** DecayDescriptor for this tree. */
    DecayDescriptor m_decaydescriptor;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    virtual void setupTree() = 0;
  public:
    /** Constructor. */
    NtupleFlatTool(TTree* tree, DecayDescriptor& decaydescriptor);
    /** Destructor. */
    virtual ~NtupleFlatTool() {}
    /** Calculate branch variables from provided Particle. */
    virtual void eval(const Particle* p) = 0;
  };

} // namepspace Belle2

#endif // NTUPLEFLATTOOL_H

