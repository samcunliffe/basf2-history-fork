/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marcel Hohmann                                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <analysis/modules/CurlTagger/Selector.h>

#include <analysis/dataobjects/Particle.h>
#include <vector>

//Root Includes
#include "TLorentzVector.h"
#include "TVector3.h"
#include "TMath.h"


namespace Belle2 {
  namespace CurlTagger {

    class SelectorCut : public Selector {
      /** Simple cut based selector for curl tracks taken from Belle note 1079 */

    public:
      /** Constructor */
      SelectorCut(bool belleFlag);

      /** Destructor */
      ~SelectorCut();

      /** Probability that this pair of particles come from the same mc/actual particle */
      virtual float getProbability(Particle* iPart, Particle* jPart);

      /** returns vector of variables used by this selector. Not really useful here but included to be consistent with eventual MVA method. */
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart);

    private:
      /** cut for momentum magnitude difference */
      double m_magDiffPCut;

      /** cut for angle between momenta of the two particles when they have the same charge */
      double m_sameChargePhiCut;

      /** cut for angle between momenta of the two particles when they have the opposite charge */
      double m_oppositeChargePhiCut;

    }; //class

  } // curlTagger Module namespace
}// Belle 2 namespace

