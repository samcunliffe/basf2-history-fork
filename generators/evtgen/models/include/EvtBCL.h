/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#pragma once

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtSemiLeptonicFF.hh"
#include "EvtGenBase/EvtSemiLeptonicAmp.hh"

class EvtParticle;

namespace Belle2 {

  /** The class provides the form factors for orbitally excited semileptonic decays
   */
  class EvtBCL : public EvtDecayAmp {

  public:

    /** Default constructor */
    EvtBCL();

    /** virtual destructor */
    virtual ~EvtBCL();

    /** Returns name of module */
    std::string getName();

    /** Clones module */
    EvtDecayBase* clone();

    /** Creates a decay */
    void decay(EvtParticle* p);

    /** Sets maximal probab. */
    void initProbMax();

    /** Initializes module */
    void init();

  private:

    /** Pointers needed for FFs */
    EvtSemiLeptonicFF* bclmodel;

    /** Pointers needed to calculate amplitude */
    EvtSemiLeptonicAmp* calcamp;

  };

}