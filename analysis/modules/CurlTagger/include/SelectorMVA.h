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
#include <string>

#include <mva/methods/FastBDT.h>
#include <mva/interface/Interface.h>

//Root Includes
#include "TFile.h"
#include "TTree.h"

namespace Belle2 {
  namespace CurlTagger {

    /** MVA based selector for tagging curl tracks in Belle and Belle II */
    class SelectorMVA : public Selector {

    public:
      /** Constructor */
      SelectorMVA(bool belleFlag, bool trainFlag);

      /** Destructor */
      ~SelectorMVA();

      /** Probability that this pair of particles come from the same mc/actual particle */
      virtual float getResponse(Particle* iPart, Particle* jPart);

      /** returns vector of variables used by this selector. */
      virtual std::vector<float> getVariables(Particle* iPart, Particle* jPart);

      /** initialize whatever needs to be initalized (root file etc)  */
      virtual void initialize();

      /** collect training data and save to a root file*/
      virtual void collectTrainingInfo(Particle* iPart, Particle* jPart);

      /** finalize whatever needs to be finalized (train the MVA) */
      virtual void finalize();

    private:

      /** updates the value of the MVA variable */
      void updateVariables(Particle* iPart, Particle* jPart);

      /** applying mva or training it */
      bool m_TrainFlag;

      /** name of output file for training data */
      std::string m_TFileName;

      /**output file for training data */
      TFile* m_TFile;

      /**training data tree */
      TTree* m_TTree;

      /** mva general options (for the expert)*/
      MVA::GeneralOptions m_generalOptions;

      /** mva expert */
      MVA::FastBDTExpert m_expert;


      // General Options data - just use same names
      /** mva identifier */
      std::string m_identifier;

      /** name of datafile */
      std::vector<std::string> m_datafiles;

      /** names of variables used by mva */
      std::vector<std::string> m_variables;

      /** name of target variable (isCurl) */
      std::string m_target_variable;


      // MVA Variables
      /** angle between particle momentum vectors */
      Float_t m_PPhi;

      /** charge(p1) * charge(p2) */
      Float_t m_ChargeProduct;

      /** error weighted particle Pt difference */
      Float_t m_PtDiffEW;

      /** error weighted particle Pz difference */
      Float_t m_PzDiffEW;

      /** error weighted track D0 difference */
      Float_t m_TrackD0DiffEW;

      /** error weighted track Z0 difference */
      Float_t m_TrackZ0DiffEW;

      /** error weighted track tan lambda diff difference */
      Float_t m_TrackTanLambdaDiffEW;

      /** error weighted track Phi0 difference */
      Float_t m_TrackPhi0DiffEW;

      /** error weighted track Omega difference */
      Float_t m_TrackOmegaDiffEW;

      /** isCurl Truth */
      Float_t m_IsCurl;
    }; //selectorMVA class

  } // curlTagger Module namespace
}// Belle 2 namespace

