/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christian Pulvermacher                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/modules/BestCandidateSelection/BestCandidateSelectionModule.h>

#include <analysis/dataobjects/Particle.h>

#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;


REG_MODULE(BestCandidateSelection)


BestCandidateSelectionModule::BestCandidateSelectionModule():
  m_variable(nullptr)
{
  setDescription("Selects Particles with the highest values of 'variable' in the input list and removes all other particles from the list. The remaining list is sorted from best to worst candidate.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("particleList", m_inputListName, "Name of the ParticleList to reduce to the best candidates");
  addParam("variable", m_variableName, "Variable which defines the candidate ranking (see selectLowest for ordering)");
  addParam("selectLowest", m_selectLowest, "Candidate with lower value are better (default: higher is better))", false);
  addParam("numBest", m_numBest, "Keep this many of the best candidates (0: keep all)", 1);

}

BestCandidateSelectionModule::~BestCandidateSelectionModule()
{
}

void BestCandidateSelectionModule::initialize()
{
  StoreArray<Particle>::required();
  m_inputList.isRequired(m_inputListName);

  m_variable = Variable::Manager::Instance().getVariable(m_variableName);
  if (!m_variable) {
    B2ERROR("Variable '" << m_variableName << "' is not available in Variable::Manager!");
  }
  if (m_numBest < 0) {
    B2ERROR("value of numBest must be >= 0!");
  }
}

void BestCandidateSelectionModule::event()
{
  StoreArray<Particle> particles;

  if (!m_inputList) {
    B2WARNING("Input list " << m_inputList.getName() << " was not created?");
    return;
  }
  std::function<bool(double, double)> betterThan = [](double a, double b) -> bool { return a > b; };
  if (m_selectLowest) {
    betterThan = [](double a, double b) -> bool { return a < b; };
  }

  //create list of particle index and the corresponding value of variable
  std::map<double, unsigned int, decltype(betterThan)> valueToIndex(betterThan);
  unsigned int numParticles = m_inputList->getListSize();
  for (unsigned int i = 0; i < numParticles; i++) {
    const Particle* p = m_inputList->getParticle(i);
    double value = m_variable->function(p);
    valueToIndex[value] = p->getArrayIndex();
  }

  auto cutoff = valueToIndex.cbegin();
  int iCandidate = 0;
  for (; cutoff != valueToIndex.cend() && iCandidate < m_numBest; ++cutoff) {
    iCandidate++;
  }
  if (m_numBest == 0)
    cutoff = valueToIndex.cend();

  //remove everything but best candidates
  //everything in range [begin, cutoff) is to be kept
  m_inputList->clear();
  for (auto it = valueToIndex.cbegin(); it != cutoff; ++it) {
    m_inputList->addParticle(particles[it->second]);
  }
}
