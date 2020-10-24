/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Anze Zupanc                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleListManipulator/ParticleListManipulatorModule.h>


// framework - DataStore
#include <framework/datastore/DataStore.h>

// framework aux
#include <framework/logging/Logger.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/ValueIndexPairSorting.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleListManipulator)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleListManipulatorModule::ParticleListManipulatorModule():
    m_variable(nullptr)

  {
    setDescription("Manipulates ParticleLists: copies/merges/performs particle selection");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("outputListName", m_outputListName, "Output ParticleList name");

    vector<string> defaultList;
    addParam("inputListNames", m_inputListNames,
             "list of input ParticleList names", defaultList);

    addParam("cut", m_cutParameter, "Selection criteria to be applied", std::string(""));

    addParam("variable", m_variableName, "Variable which defines the best duplicate (see ``selectLowest`` for ordering)",
             std::string("mdstIndex"));
    addParam("preferLowest", m_preferLowest,
             "If true, duplicate with lowest value of ``variable`` is accepted, otherwise higher one.", true);

    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private members
    m_pdgCode   = 0;
    m_isSelfConjugatedParticle = false;
  }

  void ParticleListManipulatorModule::initialize()
  {
    m_particles.isRequired();

    m_pdgCode  = 0;

    // check the validity of output ParticleList name
    bool valid = m_decaydescriptor.init(m_outputListName);
    if (!valid)
      B2ERROR("ParticleListManipulatorModule::initialize Invalid output ParticleList name: " << m_outputListName);

    // Output particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

    m_pdgCode  = mother->getPDGCode();

    m_outputAntiListName = ParticleListName::antiParticleListName(m_outputListName);
    m_isSelfConjugatedParticle = (m_outputListName == m_outputAntiListName);

    // Input lists
    for (const std::string& listName : m_inputListNames) {
      if (listName == m_outputListName) {
        B2ERROR("ParticleListManipulatorModule: cannot copy Particles from " << listName <<
                " to itself! Use applyCuts() (ParticleSelector module) instead.");
      } else if (!m_decaydescriptor.init(listName)) {
        B2ERROR("Invalid input ParticleList name: " << listName);
      } else {
        StoreObjPtr<ParticleList>().isRequired(listName);
      }
    }

    DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
    m_particleList.registerInDataStore(m_outputListName, flags);
    if (!m_isSelfConjugatedParticle) {
      m_antiParticleList.registerInDataStore(m_outputAntiListName, flags);
    }

    m_variable = Variable::Manager::Instance().getVariable(m_variableName);
    if (!m_variable) {
      B2ERROR("Variable '" << m_variableName << "' is not available in Variable::Manager!");
    }
    m_cut = Variable::Cut::compile(m_cutParameter);
  }

  void ParticleListManipulatorModule::event()
  {
    // clear the list
    m_particlesInTheList.clear();

    bool existingList = m_particleList.isValid();

    if (!existingList) {
      // new particle list: create it
      m_particleList.create();
      m_particleList->initialize(m_pdgCode, m_outputListName);

      if (!m_isSelfConjugatedParticle) {
        m_antiParticleList.create();
        m_antiParticleList->initialize(-1 * m_pdgCode, m_outputAntiListName);

        m_antiParticleList->bindAntiParticleList(*(m_particleList));
      }
    } else {
      // output list already contains Particles
      // fill m_particlesInTheList with unique
      // identifiers of particles already in
      for (unsigned i = 0; i < m_particleList->getListSize(); i++) {
        const Particle* particle = m_particleList->getParticle(i);

        std::vector<int> idSeq;
        fillUniqueIdentifier(particle, idSeq);
        m_particlesInTheList.push_back(idSeq);
      }
    }

    // create list of candidate indices and corresponding sorting values
    typedef std::pair<double, unsigned int> ValueIndexPair;
    std::vector<ValueIndexPair> valueToIndex;

    // fill all particles from input lists that pass selection criteria into comparison list
    for (const auto& inputListName : m_inputListNames) {
      const StoreObjPtr<ParticleList> inPList(inputListName);

      std::vector<int> fsParticles = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, false);
      const std::vector<int>& scParticles     = inPList->getList(ParticleList::EParticleType::c_SelfConjugatedParticle, false);
      const std::vector<int>& fsAntiParticles = inPList->getList(ParticleList::EParticleType::c_FlavorSpecificParticle, true);

      fsParticles.insert(fsParticles.end(), scParticles.begin(), scParticles.end());
      fsParticles.insert(fsParticles.end(), fsAntiParticles.begin(), fsAntiParticles.end());

      for (int fsParticle : fsParticles) {
        const Particle* part = m_particles[fsParticle];

        if (m_cut->check(part)) {
          valueToIndex.emplace_back(m_variable->function(part), part->getArrayIndex());
        }
      }
    }

    // use stable sort to make sure we keep the relative order of elements with
    // same value as it was before
    if (m_preferLowest) {
      std::stable_sort(valueToIndex.begin(), valueToIndex.end(), ValueIndexPairSorting::lowerPair<ValueIndexPair>);
    } else {
      std::stable_sort(valueToIndex.begin(), valueToIndex.end(), ValueIndexPairSorting::higherPair<ValueIndexPair>);
    }

    // starting from the best candidate add all particles to output list that are not already in it
    for (const auto& candidate : valueToIndex) {
      const Particle* part = m_particles[candidate.second];

      std::vector<int> idSeq;
      fillUniqueIdentifier(part, idSeq);
      bool uniqueSeq = isUnique(idSeq);

      if (uniqueSeq) {
        m_particleList->addParticle(part);
        m_particlesInTheList.push_back(idSeq);
      }
    }
  }

  void ParticleListManipulatorModule::fillUniqueIdentifier(const Particle* p, std::vector<int>& idSequence)
  {
    idSequence.push_back(p->getPDGCode());

    if (p->getNDaughters() == 0) {
      idSequence.push_back(p->getMdstArrayIndex());
    } else {
      // decorate number of daughters with flavor sign of particle
      idSequence.push_back(p->getNDaughters());
      // this is not FSP (go one level down)
      auto daughters = p->getDaughters();
      sort(daughters.begin(), daughters.end(), [](const auto a, const auto b) {
        return a->getPDGCode() > b->getPDGCode();
      });
      for (const auto& daughter : daughters)
        fillUniqueIdentifier(daughter, idSequence);
    }
  }

  bool ParticleListManipulatorModule::isUnique(const std::vector<int>& idSeqOUT)
  {
    for (const auto& idSeqIN : m_particlesInTheList) {
      bool sameSeq = (idSeqIN == idSeqOUT);
      if (sameSeq)
        return false;
    }

    return true;
  }
} // end Belle2 namespace
