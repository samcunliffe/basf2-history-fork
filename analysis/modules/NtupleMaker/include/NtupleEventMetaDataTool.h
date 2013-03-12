/**************************************************************************
* BASF2 (Belle Analysis Framework 2)                                     *
* Copyright(C) 2010 - Belle II Collaboration                             *
*                                                                        *
* Author: The Belle II Collaboration                                     *
* Contributors: Christian Oswald                                         *
*                                                                        *
* This software is provided "as is" without any warranty.                *
**************************************************************************/

#ifndef NTUPLEEVENTMETADATATOOL_H
#define NTUPLEEVENTMETADATATOOL_H
#include <boost/function.hpp>
#include <analysis/dataobjects/Particle.h>
#include <framework/dataobjects/EventMetaData.h>
#include <analysis/modules/NtupleMaker/NtupleFlatTool.h>
#include <analysis/modules/NtupleMaker/DecayDescriptor.h>

using namespace std;
using namespace Belle2;

namespace Belle2 {
  /** Tool for NtupleMaker to write exp_no, run_no, evt_no to flat ntuple. */
  class NtupleEventMetaDataTool : public NtupleFlatTool {
  private:
    /** Experiment number. */
    int m_iExperiment;
    /** Run number. */
    int m_iRun;
    /** Event number. */
    int m_iEvent;
    /** Create branches in m_tree - this function should be called by the constructor only. */
    void setupTree();
  public:
    /** Constructor. */
    NtupleEventMetaDataTool(TTree* tree, DecayDescriptor& decaydescriptor) : NtupleFlatTool(tree, decaydescriptor) {setupTree();}
    /** Set branch variables to properties of the provided Particle. */
    void eval(const Particle* p);
  };
} // namepspace Belle2

#endif // NTUPLEEVENTMETADATATOOL_H
