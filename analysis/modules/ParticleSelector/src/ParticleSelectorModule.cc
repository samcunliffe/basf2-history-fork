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
#include <analysis/modules/ParticleSelector/ParticleSelectorModule.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// dataobjects
#include <analysis/dataobjects/Particle.h>
#include <analysis/dataobjects/ParticleList.h>

// utilities
#include <analysis/utility/EvtPDLUtil.h>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleSelector)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleSelectorModule::ParticleSelectorModule() : Module()
  {
    setDescription("Selects reconstructed Particles from StoreArray<Particles> according to given decay and selection criteria and fills them into a ParticleList.");
    setPropertyFlags(c_ParallelProcessingCertified);

    addParam("decayString", m_decayString, "Input DecayDescriptor string (see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/DecayString).");

    Variable::Cut::Parameter emptyCut;
    addParam("cut", m_cutParameter, "Selection criteria to be applied, see https://belle2.cc.kek.jp/~twiki/bin/view/Physics/ParticleSelectorFunctions", emptyCut);

    addParam("persistent", m_persistent,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    // initializing the rest of private memebers
    m_pdgCode = 0;
    m_isSelfConjugatedParticle = 0;
  }

  void ParticleSelectorModule::initialize()
  {
    // obtain the input and output particle lists from the decay string
    bool valid = m_decaydescriptor.init(m_decayString);
    if (!valid)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString: " << m_decayString);

    int nProducts = m_decaydescriptor.getNDaughters();
    if (nProducts > 0)
      B2ERROR("ParticleSelectorModule::initialize Invalid input DecayString " << m_decayString
              << ". DecayString should not contain any daughters, only the mother particle.");

    // Mother particle
    const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

    m_pdgCode  = mother->getPDGCode();
    m_listName = mother->getFullName();

    m_isSelfConjugatedParticle = !(Belle2::EvtPDLUtil::hasAntiParticle(m_pdgCode));
    m_antiListName             = Belle2::EvtPDLUtil::antiParticleListName(m_pdgCode, mother->getLabel());


    StoreObjPtr<ParticleList> particleList(m_listName);
    if (!particleList.isOptional()) {
      //if it doesn't exist:

      DataStore::EStoreFlags flags = m_persistent ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
      particleList.registerInDataStore(flags);
      if (!m_isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiParticleList(m_antiListName);
        antiParticleList.registerInDataStore(flags);
      }
    }

    m_cut.init(m_cutParameter);

    B2INFO("ParticleSelector: " << m_listName << " (" << m_antiListName << ") ");
  }

  void ParticleSelectorModule::event()
  {
    StoreObjPtr<ParticleList> plist(m_listName);
    bool existingList = plist.isValid();

    if (!existingList) { // new particle list: fill selected
      // TODO: this is a dirty hack to prevent addition of duplicated Particles to ParticleList
      // keep track of mdst indices of Particles added to the list; another Particle
      // with the same mdst index will not be added
      std::vector<int> mdstIndices;

      plist.create();
      plist->initialize(m_pdgCode, m_listName);

      if (!m_isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(m_antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * m_pdgCode, m_antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }

      StoreArray<Particle> Particles;
      for (int i = 0; i < Particles.getEntries(); i++) {
        const Particle* part = Particles[i];
        if (abs(part->getPDGCode()) != abs(m_pdgCode)) continue;
        if (m_cut.check(part)) {

          // TODO: part of the dirty hack
          if (std::find(mdstIndices.begin(), mdstIndices.end(), part->getMdstArrayIndex()) == mdstIndices.end()
              || part->getParticleType() == Particle::EParticleType::c_Undefined
              || part->getParticleType() == Particle::EParticleType::c_Composite) {
            plist->addParticle(part);
            mdstIndices.push_back(part->getMdstArrayIndex());
          }
        }
      }
    } else { // existing particle list: apply selections and remove unselected
      // loop over list only if cuts should be applied
      std::vector<unsigned int> toRemove;
      unsigned int n = plist->getListSize();
      for (unsigned i = 0; i < n; i++) {
        const Particle* part = plist->getParticle(i);
        if (!m_cut.check(part)) toRemove.push_back(part->getArrayIndex());
      }

      plist->removeParticles(toRemove);
    }
  }
} // end Belle2 namespace

