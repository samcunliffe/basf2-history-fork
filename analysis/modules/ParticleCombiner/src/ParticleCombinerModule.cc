/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleCombiner/ParticleCombinerModule.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <mdst/dataobjects/MCParticle.h>
#include <analysis/dataobjects/Particle.h>
#include <analysis/utility/VariableManager.h>

#include <algorithm>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleCombiner)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleCombinerModule::ParticleCombinerModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Makes particle combinations");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("PDG", m_pdg, "PDG code", 0);
    addParam("ListName", m_listName, "name of the output particle list", string(""));
    vector<string> defaultList;
    addParam("InputListNames", m_inputListNames, "list of input particle list names",
             defaultList);
    addParam("MassCut", m_massCut, "[GeV] tuple of lower and upper mass cut", std::make_tuple(0.0, 100.0));
    std::map<std::string, std::tuple<double, double>> defaultMap;
    addParam("cutsOnProduct", m_productCuts, "Map of Variable and Cut Values. Cuts are performed on the product of the variable value of all daughter particles.", defaultMap);
    addParam("cutsOnSum", m_sumCuts, "Map of Variable and Cut Values. Cuts are performed on the sum of the variable value of all daughter particles.", defaultMap);
    addParam("persistent", m_persistent,
             "toggle output particle list btw. transient/persistent", false);

  }

  ParticleCombinerModule::~ParticleCombinerModule()
  {
  }

  void ParticleCombinerModule::initialize()
  {
    if (m_persistent) {
      StoreObjPtr<ParticleList>::registerPersistent(m_listName);
    } else {
      StoreObjPtr<ParticleList>::registerTransient(m_listName);
    }
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList>::required(m_inputListNames[i]);
    }
  }

  void ParticleCombinerModule::beginRun()
  {
  }

  void ParticleCombinerModule::event()
  {
    StoreObjPtr<ParticleList> outputList(m_listName);
    outputList.create();
    outputList->setPDG(m_pdg);

    vector<StoreObjPtr<ParticleList> > plists;
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      StoreObjPtr<ParticleList> list(m_inputListNames[i]);
      if (!list) {
        B2ERROR("ParticleList " << m_inputListNames[i] << " not found");
        return;
      }
      if (list->getParticleCollectionName() != std::string("Particles")) {
        B2ERROR("ParticleList " << m_inputListNames[i] <<
                " does not refer to the default Particle collection");
        return;
      }
      plists.push_back(list);
    }

    if (outputList->getFlavorType() == 1) {
      vector<int> decay, decaybar;
      for (unsigned i = 0; i < plists.size(); i++) {
        decay.push_back(plists[i]->getPDG());
        decaybar.push_back(plists[i]->getPDGbar());
      }
      std::sort(decay.begin(), decay.end());
      std::sort(decaybar.begin(), decaybar.end());
      if (decay == decaybar) outputList->swapFlavorType();
    }

    combination(outputList, plists, 0);
    if (outputList->getFlavorType() == 1) combination(outputList, plists, 1);

    // printout with B2INFO
    std::string decay;
    for (unsigned i = 0; i < m_inputListNames.size(); i++) {
      decay = decay + " " + m_inputListNames[i];
    }
    switch (outputList->getFlavorType()) {
      case 0:
        B2INFO("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
               << " size=" << outputList->getList(0).size());
        break;
      case 1:
        B2INFO("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
               << " size="
               << outputList->getList(0).size() << "+" << outputList->getList(1).size());
        break;
      default:
        B2ERROR("ParticleCombiner: " << m_pdg << " " << m_listName << "->" << decay
                << " ***invalid FlavorType: "
                << outputList->getFlavorType());
    }

  }


  void ParticleCombinerModule::endRun()
  {
  }

  void ParticleCombinerModule::terminate()
  {
  }

  void ParticleCombinerModule::combination(StoreObjPtr<ParticleList>& outputList,
                                           vector<StoreObjPtr<ParticleList> >& plists,
                                           unsigned chargeState)
  {
    StoreArray<Particle> Particles;

    // arrays needed to construct N nested loops

    int N = plists.size(); // number of particle lists to combine
    int n[N]; // array of list sizes
    int k[N]; // array of list element indices

    // initialize arrays and list indices

    int nLoop = 1;
    for (int i = 0; i < N; i++) {
      n[i] = plists[i]->getList(chargeState).size();
      if (n[i] == 0) return;
      k[i] = 0;
      nLoop *= n[i];
    }

    // stack needed to check for unique combination

    unordered_set<boost::dynamic_bitset<> > indexStack;

    double massCutLow = 0.0;
    double massCutHigh = 0.0;
    std::tie(massCutLow, massCutHigh) = m_massCut;

    // N nested loops

    for (int loop = 0; loop < nLoop; loop++) {

      // increment indices first
      for (int i = 0; i < N; i++) {
        k[i]++;
        if (k[i] < n[i]) break;
        k[i] = 0;
      }

      // check if particle sources are all different
      vector<Particle*> particleStack;
      vector<int> indices;
      for (int i = 0; i < N; i++) {
        int ii = plists[i]->getList(chargeState)[k[i]];
        particleStack.push_back(Particles[ii]);
        indices.push_back(ii);
      }
      if (!differentSources(particleStack, Particles)) continue;

      // Check invariant mass requirements
      TLorentzVector vec(0., 0., 0., 0.);
      for (unsigned i = 0; i < particleStack.size(); i++) {
        vec = vec + particleStack[i]->get4Vector();
      }
      double mass = vec.M();

      if (mass < massCutLow || mass > massCutHigh) continue;

      // Check if all (product and sum) cut requirements are fulfilled
      if (!checkCuts(particleStack)) continue;

      // check if the combination is unique (e.g. it's permutation is not on the stack)
      if (!uniqueCombination(indexStack, indices)) continue;

      // store combination
      new(Particles.nextFreeAddress()) Particle(vec,
                                                outputList->getPDG(chargeState),
                                                outputList->getFlavorType(),
                                                indices);
      int iparticle = Particles.getEntries() - 1;
      outputList->addParticle(iparticle, chargeState);

    }

  }

  bool ParticleCombinerModule::checkCuts(const std::vector<Particle*>& particleStack)
  {

    VariableManager& manager = VariableManager::Instance();

    for (auto & cut : m_productCuts) {
      // Now calculate the value by multiplying all decay product values,
      double value = 1.0;
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO("ParticleCombiner: VariableManager doesn't have variable" <<  cut.first)
        return false;
      }
      for (unsigned i = 0; i < particleStack.size(); i++) {
        value *= var->function(particleStack[i]);
      }
      if (value < std::get<0>(cut.second) || value > std::get<1>(cut.second)) return false;
    }

    for (auto & cut : m_sumCuts) {
      // Now calculate the value by adding all decay product values,
      double value = 0.0;
      auto var = manager.getVariable(cut.first);
      if (var == nullptr) {
        B2INFO("ParticleCombiner: VariableManager doesn't have variable" <<  cut.first)
        return false;
      }
      for (unsigned i = 0; i < particleStack.size(); i++) {
        value += var->function(particleStack[i]);
      }
      if (value < std::get<0>(cut.second) || value > std::get<1>(cut.second)) return false;
    }

    return true;

  }

  bool ParticleCombinerModule::differentSources(std::vector<Particle*> stack,
                                                const StoreArray<Particle>& Particles)
  {
    vector<int> sources; // stack for particle sources

    while (!stack.empty()) {
      Particle* p = stack.back();
      stack.pop_back();
      const vector<int>& daughters = p->getDaughterIndices();
      if (daughters.empty()) {
        int source = p->getMdstSource();
        for (unsigned i = 0; i < sources.size(); i++) {
          if (source == sources[i]) return false;
        }
        sources.push_back(source);
      } else {
        for (unsigned i = 0; i < daughters.size(); i++) stack.push_back(Particles[daughters[i]]);
      }
    }
    return true;
  }


  bool ParticleCombinerModule::uniqueCombination(
    std::unordered_set<boost::dynamic_bitset<> >& indexStack,
    const std::vector<int>& indices)
  {
    int max_element = *std::max_element(indices.begin(), indices.end());
    boost::dynamic_bitset<> indicesBits(max_element + 1); //all zeroes
    for (int idx : indices) {
      indicesBits[idx] = 1;
    }

    if (indexStack.find(indicesBits) != indexStack.end())
      return false;

    indexStack.insert(indicesBits);
    return true;
  }


} // end Belle2 namespace

