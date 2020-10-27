/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2019 - Belle II Collaboration                        *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric, Anze Zupanc                                *
 *               Sam Cunliffe, Torben Ferber                              *
 *               Frank Meier                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <analysis/modules/ParticleLoader/ParticleLoaderModule.h>

// framework aux
#include <framework/logging/Logger.h>
#include <framework/core/ModuleParam.templateDetails.h>

// utilities
#include <analysis/DecayDescriptor/ParticleListName.h>
#include <analysis/utility/PCmsLabTransform.h>

#include <utility>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(ParticleLoader)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  ParticleLoaderModule::ParticleLoaderModule() : Module()

  {
    setDescription("Loads MDST dataobjects as Particle objects to the StoreArray<Particle> and collects them in specified ParticleList.");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    std::vector<std::tuple<std::string, std::string>> emptyDecayStringsAndCuts;

    addParam("decayStringsWithCuts", m_decayStringsWithCuts,
             "List of (decayString, Variable::Cut) tuples that specify all output ParticleLists to be created by the module. Only Particles that pass specified selection criteria are added to the ParticleList (see :ref:`DecayString` and `cut_strings_selections`).",
             emptyDecayStringsAndCuts);

    addParam("useMCParticles", m_useMCParticles,
             "Use MCParticles instead of reconstructed MDST dataobjects (tracks, ECL, KLM, clusters, V0s, ...)", false);

    addParam("useROEs", m_useROEs,
             "Use ROE instead of reconstructed MDST dataobjects (tracks, ECL, KLM, clusters, V0s, ...)", false);

    addParam("roeMaskName", m_roeMaskName,
             "ROE mask name to load", std::string(""));

    addParam("sourceParticleListName", m_sourceParticleListName,
             "Particle list name from which we need to get ROEs", std::string(""));

    addParam("useMissing", m_useMissing,
             "If true, the Particle List will be filled with missing momentum from the ROE and signal particle.", false);

    addParam("writeOut", m_writeOut,
             "If true, the output ParticleList will be saved by RootOutput. If false, it will be ignored when writing the file.", false);

    addParam("addDaughters", m_addDaughters,
             "If true, the particles from the bottom part of the selected particle's decay chain will also be created in the datastore and mother-daughter relations are recursively set",
             false);

    addParam("skipNonPrimaryDaughters", m_skipNonPrimaryDaughters,
             "If true, the secondary MC daughters will be skipped, default is false",
             false);

    addParam("trackHypothesis", m_trackHypothesis,
             "Track hypothesis to use when loading the particle. By default, use the particle's own hypothesis.",
             0);

    addParam("enforceFitHypothesis", m_enforceFitHypothesis,
             "If true, a Particle is only created if a track fit with the particle hypothesis passed to the ParticleLoader is available.",
             m_enforceFitHypothesis);
  }

  void ParticleLoaderModule::initialize()
  {
    B2INFO("ParticleLoader's Summary of Actions:");

    m_particles.registerInDataStore();
    m_particleExtraInfoMap.registerInDataStore();
    m_eventExtraInfo.registerInDataStore();
    //register relations if these things exists
    if (m_mcparticles.isOptional()) {
      m_particles.registerRelationTo(m_mcparticles);
    }
    if (m_pidlikelihoods.isOptional()) {
      m_particles.registerRelationTo(m_pidlikelihoods);
    }
    if (m_trackfitresults.isOptional()) {
      m_particles.registerRelationTo(m_trackfitresults);
    }

    if (m_useMCParticles) {
      m_mcparticles.isRequired();
    }

    if (m_decayStringsWithCuts.empty()) {
      B2WARNING("Obsolete usage of the ParticleLoader module (load all MDST objects as all possible Particle object types). Specify the particle type via decayStringsWithCuts module parameter instead.");
    } else {
      for (auto decayStringCutTuple : m_decayStringsWithCuts) {

        // parsing of the input tuple (DecayString, Cut)
        string decayString = get<0>(decayStringCutTuple);
        std::string cutParameter = get<1>(decayStringCutTuple);

        // obtain the output particle lists from the decay string
        bool valid = m_decaydescriptor.init(decayString);
        if (!valid)
          B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString: " << decayString);

        // Mother particle
        const DecayDescriptorParticle* mother = m_decaydescriptor.getMother();

        int pdgCode  = mother->getPDGCode();
        string listName = mother->getFullName();

        // special case. if the list is called "all" it may not have a
        // corresponding cut string this can introduce very dangerous bugs
        string listLabel = mother->getLabel();
        if (listLabel == "all")
          if (cutParameter != "")
            B2FATAL("You have tried to create a list " << listName << " with cuts! This is *very* error prone, so it is now forbidden.");

        if (not isValidPDGCode(pdgCode) and (m_useMCParticles == false and m_useROEs == false))
          B2ERROR("Invalid particle type requested to be loaded. Set a valid decayString module parameter.");

        // if we're not loading MCParticles and we are loading K0S, Lambdas, or photons --> ee then this decaystring is a V0
        bool mdstSourceIsV0 = false;
        if (!m_useMCParticles &&
            (abs(pdgCode) == abs(Const::Kshort.getPDGCode()) || abs(pdgCode) == abs(Const::Lambda.getPDGCode())
             || (abs(pdgCode) == abs(Const::photon.getPDGCode()) && m_addDaughters == true)))
          mdstSourceIsV0 = true;

        int nProducts = m_decaydescriptor.getNDaughters();
        if (mdstSourceIsV0 == false) {
          if (nProducts > 0) {
            if (!m_useROEs) {
              B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString " << decayString
                      << ". DecayString should not contain any daughters, only the mother particle.");
            } else {
              B2INFO("ParticleLoaderModule: Replacing the source particle list name by " <<
                     m_decaydescriptor.getDaughter(0)->getMother()->getFullName()
                     << " all other daughters will be ignored.");
              m_sourceParticleListName = m_decaydescriptor.getDaughter(0)->getMother()->getFullName();
            }
          }
        } else {
          if (nProducts != 2)
            B2ERROR("ParticleLoaderModule::initialize Invalid input DecayString " << decayString
                    << ". MDST source of the particle list is V0, DecayString should contain exactly two daughters, as well as the mother particle.");
          else {
            if (m_decaydescriptor.getDaughter(0)->getMother()->getPDGCode() * m_decaydescriptor.getDaughter(1)->getMother()->getPDGCode() > 0)
              B2ERROR("MDST source of the particle list is V0, the two daughters should have opposite charge");
          }
        }

        string antiListName = ParticleListName::antiParticleListName(listName);
        bool isSelfConjugatedParticle = (listName == antiListName);

        StoreObjPtr<ParticleList> particleList(listName);
        if (!particleList.isOptional()) {
          //if it doesn't exist:

          DataStore::EStoreFlags flags = m_writeOut ? DataStore::c_WriteOut : DataStore::c_DontWriteOut;
          particleList.registerInDataStore(flags);
          if (!isSelfConjugatedParticle) {
            StoreObjPtr<ParticleList> antiParticleList(antiListName);
            antiParticleList.registerInDataStore(flags);
          }
        } else {
          // TODO: test that this actually works
          B2WARNING("The ParticleList with name " << listName << " already exists in the DataStore. Nothing to do.");
          continue;
        }

        std::shared_ptr<Variable::Cut> cut = std::shared_ptr<Variable::Cut>(Variable::Cut::compile(cutParameter));

        // add PList to corresponding collection of Lists
        B2INFO(" o) creating (anti-)ParticleList with name: " << listName << " (" << antiListName << ")");
        if (m_useROEs) {
          B2INFO("   -> MDST source: RestOfEvents");
          m_ROE2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
        } else if (m_useMCParticles) {
          B2INFO("   -> MDST source: MCParticles");
          m_MCParticles2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
        } else {
          bool chargedFSP = Const::chargedStableSet.contains(Const::ParticleType(abs(pdgCode)));
          if (chargedFSP) {
            B2INFO("   -> MDST source: Tracks");
            m_Tracks2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
          }

          if (abs(pdgCode) == abs(Const::photon.getPDGCode())) {
            if (m_addDaughters == false) {
              B2INFO("   -> MDST source: ECLClusters");
              m_ECLClusters2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
            } else {
              B2INFO("   -> MDST source: V0");
              m_V02Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
            }
          }

          if (abs(pdgCode) == abs(Const::Kshort.getPDGCode())) {
            B2INFO("   -> MDST source: V0");
            m_V02Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
          }

          if (abs(pdgCode) == abs(Const::Klong.getPDGCode()) || abs(pdgCode) == abs(Const::neutron.getPDGCode())) {
            B2INFO("   -> MDST source: exclusively KLMClusters or exclusively ECLClusters (matching between those not used)");
            m_KLMClusters2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
            m_ECLClusters2Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
          }

          if (abs(pdgCode) == abs(Const::Lambda.getPDGCode())) {
            B2INFO("   -> MDST source: V0");
            m_V02Plists.emplace_back(pdgCode, listName, antiListName, isSelfConjugatedParticle, cut);
          }
        }
        B2INFO("   -> With cuts  : " << cutParameter);
      }
    }


    m_chargeZeroTrackCounts = std::vector<int>(m_Tracks2Plists.size(), 0);
    m_sameChargeDaughtersV0Counts = std::vector<int>(m_V02Plists.size(), 0);
  }

  void ParticleLoaderModule::event()
  {
    if (not m_particleExtraInfoMap) {
      m_particleExtraInfoMap.create();
    }

    if (m_useROEs)
      roeToParticles();
    else if (m_useMCParticles)
      mcParticlesToParticles();
    else {
      tracksToParticles();
      eclClustersToParticles();
      klmClustersToParticles();
      v0sToParticles();
    }
  }

  void ParticleLoaderModule::terminate()
  {
    // report track errors integrated
    for (size_t i = 0; i < m_Tracks2Plists.size(); i++)
      if (m_chargeZeroTrackCounts[i] > 0) {
        auto track2Plist = m_Tracks2Plists[i];
        B2WARNING("There were " << m_chargeZeroTrackCounts[i]
                  << " tracks skipped because of zero charge for "
                  << get<c_PListName>(track2Plist));
      }
    // report V0 errors integrated
    for (size_t i = 0; i < m_V02Plists.size(); i++)
      if (m_sameChargeDaughtersV0Counts[i] > 0) {
        auto v02Plist = m_V02Plists[i];
        B2WARNING("There were " << m_sameChargeDaughtersV0Counts[i]
                  << " v0s skipped because of same charge daughters for "
                  << get<c_PListName>(v02Plist));
      }
  }

  void ParticleLoaderModule::roeToParticles()
  {
    if (m_ROE2Plists.empty()) // nothing to do
      return;
    // Multiple particle lists are not supported
    auto roe2Plist = m_ROE2Plists[0];
    string listName = get<c_PListName>(roe2Plist);
    string antiListName = get<c_AntiPListName>(roe2Plist);
    int pdgCode = get<c_PListPDGCode>(roe2Plist);
    bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(roe2Plist);

    StoreObjPtr<ParticleList> plist(listName);
    plist.create();
    plist->initialize(pdgCode, listName);

    if (!isSelfConjugatedParticle) {
      StoreObjPtr<ParticleList> antiPlist(antiListName);
      antiPlist.create();
      antiPlist->initialize(-1 * pdgCode, antiListName);
      antiPlist->bindAntiParticleList(*(plist));
    }
    if (m_sourceParticleListName != "") {
      // Take related ROEs from a particle list
      StoreObjPtr<ParticleList> pList(m_sourceParticleListName);
      if (!pList.isValid())
        B2FATAL("ParticleList " << m_sourceParticleListName << " could not be found or is not valid!");
      for (unsigned int i = 0; i < pList->getListSize(); i++) {
        RestOfEvent* roe = pList->getParticle(i)->getRelatedTo<RestOfEvent>("ALL");
        if (!roe) {
          B2ERROR("ParticleList " << m_sourceParticleListName << " has no associated ROEs!");
        } else {
          addROEToParticleList(roe, i, pdgCode, isSelfConjugatedParticle);
        }
      }

    } else {
      // Take all ROE if no particle list provided
      for (int i = 0; i < m_roes.getEntries(); i++) {
        addROEToParticleList(m_roes[i], i);
      }
    }
  }

  void ParticleLoaderModule::addROEToParticleList(RestOfEvent* roe, int mdstIndex, int pdgCode, bool isSelfConjugatedParticle)
  {

    Particle* newPart = nullptr;
    if (!m_useMissing) {
      // Convert ROE to particle
      newPart = roe->convertToParticle(m_roeMaskName, pdgCode, isSelfConjugatedParticle);
    } else {
      // Create a particle from missing momentum
      auto* signalSideParticle = roe->getRelatedFrom<Particle>();
      PCmsLabTransform T;
      TLorentzVector boost4Vector = T.getBeamFourMomentum();

      TLorentzVector signal4Vector = signalSideParticle->get4Vector();
      TLorentzVector roe4Vector = roe->get4Vector(m_roeMaskName);
      TLorentzVector missing4Vector;
      missing4Vector.SetVect(boost4Vector.Vect() - (signal4Vector.Vect() + roe4Vector.Vect()));
      missing4Vector.SetE(missing4Vector.Vect().Mag());
      auto isFlavored = (isSelfConjugatedParticle) ? Particle::EFlavorType::c_Unflavored : Particle::EFlavorType::c_Flavored;
      newPart = m_particles.appendNew(missing4Vector, pdgCode, isFlavored, Particle::EParticleSourceObject::c_Undefined, mdstIndex);

    }
    for (auto roe2Plist : m_ROE2Plists) {
      string listName = get<c_PListName>(roe2Plist);
      auto& cut = get<c_CutPointer>(roe2Plist);
      StoreObjPtr<ParticleList> plist(listName);
      if (cut->check(newPart))
        plist->addParticle(newPart);
    }

  }


  void ParticleLoaderModule::v0sToParticles()
  {
    if (m_V02Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto v02Plist : m_V02Plists) {
      string listName = get<c_PListName>(v02Plist);
      string antiListName = get<c_AntiPListName>(v02Plist);
      int pdgCode = get<c_PListPDGCode>(v02Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(v02Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    // check if the order of the daughters in the decay string (decided by the user) is the same of the v0 daughers order (fixed)
    bool matchingDaughtersOrder = true;
    if (m_decaydescriptor.getDaughter(0)->getMother()->getPDGCode() < 0
        && m_decaydescriptor.getDaughter(1)->getMother()->getPDGCode() > 0)
      matchingDaughtersOrder = false;

    // load reconstructed V0s as Kshorts (pi-pi+ combination), Lambdas (p+pi- combinations), and converted photons (e-e+ combinations)
    for (int i = 0; i < m_v0s.getEntries(); i++) {
      const V0* v0 = m_v0s[i];
      Const::ParticleType v0Type = v0->getV0Hypothesis();

      // inner loop over the ParticleLists
      for (size_t ilist = 0; ilist < m_V02Plists.size(); ilist++) {
        auto v02Plist = m_V02Plists[ilist];
        int listPDGCode = get<c_PListPDGCode>(v02Plist);

        if (abs(listPDGCode) != abs(v0Type.getPDGCode()))
          continue;

        // check if the charge of the 2 V0's daughters is opposite
        if (v0->getTrackFitResults().first->getChargeSign() == v0->getTrackFitResults().second->getChargeSign()) {
          B2DEBUG(19, "V0 with same charge daughters skipped!");
          m_sameChargeDaughtersV0Counts[ilist]++;
          continue;
        }

        Const::ChargedStable pTypeP(Const::pion);
        Const::ChargedStable pTypeM(Const::pion);
        Particle::EFlavorType v0FlavorType = Particle::c_Unflavored;

        if (v0Type.getPDGCode() == Const::Kshort.getPDGCode()) { // K0s -> pi+ pi-
          pTypeP = Const::pion;
          pTypeM = Const::pion;
        } else if (v0Type.getPDGCode() == Const::Lambda.getPDGCode()) { // Lambda -> p+ pi-
          pTypeP = Const::proton;
          pTypeM = Const::pion;
          v0FlavorType = Particle::c_Flavored; // K0s are not flavoured, lambdas are
        } else if (v0Type.getPDGCode() == Const::antiLambda.getPDGCode()) { // anti-Lambda -> pi+ anti-p-
          pTypeP = Const::pion;
          pTypeM = Const::proton;
          v0FlavorType = Particle::c_Flavored;
        } else if (v0Type.getPDGCode() == Const::photon.getPDGCode()) { // gamma -> e+ e-
          pTypeP = Const::electron;
          pTypeM = Const::electron;
        } else {
          B2WARNING("Unknown V0 hypothesis!");
        }

        // check if, given the initial user's decay descriptor, the current v0 is a particle or an anti-particle.
        // in the V0 the order of the daughters is fixed, first the positive and then the negative; to be coherent with the decay desctiptor, when creating
        // one particle list and one anti-particle, the v0 daughters' order has to be switched only in one case
        bool correctOrder = matchingDaughtersOrder;
        if (abs(v0Type.getPDGCode()) == abs(m_decaydescriptor.getMother()->getPDGCode())
            && v0Type.getPDGCode() != m_decaydescriptor.getMother()->getPDGCode())
          correctOrder = !correctOrder;

        std::pair<Track*, Track*> v0Tracks = v0->getTracks();
        std::pair<TrackFitResult*, TrackFitResult*> v0TrackFitResults = v0->getTrackFitResults();

        Particle daugP((v0Tracks.first)->getArrayIndex(), v0TrackFitResults.first, pTypeP);
        Particle daugM((v0Tracks.second)->getArrayIndex(), v0TrackFitResults.second, pTypeM);

        const PIDLikelihood* pidP = (v0Tracks.first)->getRelated<PIDLikelihood>();
        const PIDLikelihood* pidM = (v0Tracks.second)->getRelated<PIDLikelihood>();

        const auto& mcParticlePWithWeight = (v0Tracks.first)->getRelatedToWithWeight<MCParticle>();
        const auto& mcParticleMWithWeight = (v0Tracks.second)->getRelatedToWithWeight<MCParticle>();

        // add V0 daughters to the Particle StoreArray
        Particle* newDaugP;
        Particle* newDaugM;

        if (correctOrder) {
          newDaugP = m_particles.appendNew(daugP);
          newDaugM = m_particles.appendNew(daugM);
        } else {
          newDaugM = m_particles.appendNew(daugM);
          newDaugP = m_particles.appendNew(daugP);
        }

        // if there are PIDLikelihoods and MCParticles then also add relations to the particles
        if (pidP)
          newDaugP->addRelationTo(pidP);
        if (mcParticlePWithWeight.first)
          newDaugP->addRelationTo(mcParticlePWithWeight.first, mcParticlePWithWeight.second);
        newDaugP->addRelationTo(v0TrackFitResults.first);

        if (pidM)
          newDaugM->addRelationTo(pidM);
        if (mcParticleMWithWeight.first)
          newDaugM->addRelationTo(mcParticleMWithWeight.first, mcParticleMWithWeight.second);
        newDaugM->addRelationTo(v0TrackFitResults.second);

        // sum the 4-momenta of the daughters and construct a particle object
        TLorentzVector v0Momentum = newDaugP->get4Vector() + newDaugM->get4Vector();
        Particle v0P(v0Momentum, v0Type.getPDGCode(), v0FlavorType,
                     Particle::EParticleSourceObject::c_V0, v0->getArrayIndex());

        // add the daughters of the V0 (in the correct order) and don't update
        // the type to c_Composite (i.e. maintain c_V0)
        if (correctOrder) {
          v0P.appendDaughter(newDaugP, false);
          v0P.appendDaughter(newDaugM, false);
        } else {
          v0P.appendDaughter(newDaugM, false);
          v0P.appendDaughter(newDaugP, false);
        }

        // append the particle to the Particle StoreArray and check that we pass
        // any cuts before adding the new particle to the ParticleList
        Particle* newPart = m_particles.appendNew(v0P);
        string listName = get<c_PListName>(v02Plist);
        auto& cut = get<c_CutPointer>(v02Plist);
        StoreObjPtr<ParticleList> plist(listName);

        if (cut->check(newPart))
          plist->addParticle(newPart);
      }
    }
  }

  void ParticleLoaderModule::tracksToParticles()
  {
    if (m_Tracks2Plists.empty()) // nothing to do
      return;

    // create and initialize the particle lists
    for (auto track2Plist : m_Tracks2Plists) {
      string listName = get<c_PListName>(track2Plist);
      string antiListName = get<c_AntiPListName>(track2Plist);
      int pdgCode = get<c_PListPDGCode>(track2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(track2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      // if cc exists then also create and bind that list
      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    // the outer loop over all tracks from which Particles
    // are created, and get sorted in the particle lists
    for (int i = 0; i < m_tracks.getEntries(); i++) {
      const Track* track = m_tracks[i];
      const PIDLikelihood* pid = track->getRelated<PIDLikelihood>();
      const auto& mcParticleWithWeight = track->getRelatedToWithWeight<MCParticle>();

      // inner loop over the ParticleLists
      for (size_t ilist = 0; ilist < m_Tracks2Plists.size(); ilist++) {
        auto track2Plist = m_Tracks2Plists[ilist];
        string listName = get<c_PListName>(track2Plist);
        auto& cut = get<c_CutPointer>(track2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        // if no track hypothesis is requested, use the particle's own
        int pdgCode;
        if (m_trackHypothesis == 0)
          pdgCode = get<c_PListPDGCode>(track2Plist);
        else pdgCode = m_trackHypothesis;
        Const::ChargedStable type(abs(pdgCode));

        // load the TrackFitResult for the requested particle or if not available use
        // the one with the closest mass
        const TrackFitResult* trackFit = track->getTrackFitResultWithClosestMass(type);

        if (!trackFit) { // should never happen with the "closest mass" getter - leave as a sanity check
          B2WARNING("Track returned null TrackFitResult pointer for ChargedStable::getPDGCode()  = " << type.getPDGCode());
          continue;
        }

        if (m_enforceFitHypothesis && (trackFit->getParticleType().getPDGCode() != type.getPDGCode())) {
          // the required hypothesis does not exist for this track, skip it
          continue;
        }

        // charge zero tracks can appear, filter them and
        // count number of tracks filtered out
        int charge = trackFit->getChargeSign();
        if (charge == 0) {
          B2DEBUG(19, "Track with charge = 0 skipped!");
          m_chargeZeroTrackCounts[ilist]++;
          continue;
        }

        // create particle and add it to the Particle list.
        Particle particle(track->getArrayIndex(), trackFit, type);

        if (particle.getParticleSource() == Particle::c_Track) { // should always hold but...

          Particle* newPart = m_particles.appendNew(particle);
          if (pid)
            newPart->addRelationTo(pid);
          if (mcParticleWithWeight.first)
            newPart->addRelationTo(mcParticleWithWeight.first, mcParticleWithWeight.second);
          newPart->addRelationTo(trackFit);

          if (cut->check(newPart))
            plist->addParticle(newPart);

        } // sanity check correct particle type
      } // particle lists
    } // loop over tracks
  }

  void ParticleLoaderModule::eclClustersToParticles()
  {
    if (m_ECLClusters2Plists.empty()) // nothing to do
      return;

    // create and register all ParticleLists
    for (auto eclCluster2Plist : m_ECLClusters2Plists) {
      string listName = get<c_PListName>(eclCluster2Plist);
      string antiListName = get<c_AntiPListName>(eclCluster2Plist);
      int pdgCode = get<c_PListPDGCode>(eclCluster2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(eclCluster2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      if (!plist) { // create the list only if the klmClustersToParticles function has not already created it
        plist.create();
        plist->initialize(pdgCode, listName);

        // create anti-particle list if necessary
        if (!isSelfConjugatedParticle) {
          StoreObjPtr<ParticleList> antiPlist(antiListName);
          antiPlist.create();
          antiPlist->initialize(-1 * pdgCode, antiListName);

          antiPlist->bindAntiParticleList(*(plist));
        }
      }
    }

    // outer loop over all ECLClusters
    for (int i = 0; i < m_eclclusters.getEntries(); i++) {
      const ECLCluster* cluster      = m_eclclusters[i];

      // ECLClusters can be reconstructed under different hypotheses, for
      // example photons or neutral hadrons, we only load particles from these
      // for now
      if (!cluster->isNeutral()) continue;
      if (not cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons)
          and not cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))
        continue;

      // ECLCluster can be matched to multiple MCParticles
      // order the relations by weights and set Particle -> multiple MCParticle relation
      // preserve the weight
      RelationVector<MCParticle> mcRelations = cluster->getRelationsTo<MCParticle>();
      // order relations by weights
      std::vector<std::pair<int, double>> weightsAndIndices;
      for (unsigned int iMCParticle = 0; iMCParticle < mcRelations.size(); iMCParticle++) {
        const MCParticle* relMCParticle = mcRelations[iMCParticle];
        double weight = mcRelations.weight(iMCParticle);
        if (relMCParticle)
          weightsAndIndices.emplace_back(relMCParticle->getArrayIndex(), weight);
      }
      // sort descending by weight
      std::sort(weightsAndIndices.begin(), weightsAndIndices.end(),
      [](const std::pair<int, double>& left, const std::pair<int, double>& right) {
        return left.second > right.second;
      });

      // inner loop over ParticleLists: fill each relevant list with Particles
      // created from ECLClusters
      for (auto eclCluster2Plist : m_ECLClusters2Plists) {
        string listName = get<c_PListName>(eclCluster2Plist);
        int listPdgCode = get<c_PListPDGCode>(eclCluster2Plist);
        Const::ParticleType thisType(listPdgCode);

        // don't fill photon list with clusters that don't have
        // the nPhotons hypothesis (ECL people call this N1)
        if (listPdgCode == Const::photon.getPDGCode()
            and not cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_nPhotons))
          continue;

        // don't fill a KLong list with clusters that don't have the neutral
        // hadron hypothesis set (ECL people call this N2)
        if (listPdgCode == Const::Klong.getPDGCode()
            and not cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))
          continue;

        // don't fill a neutron list with clusters that don't have the neutral
        // hadron hypothesis set (ECL people call this N2)
        if (abs(listPdgCode) == Const::neutron.getPDGCode()
            and not cluster->hasHypothesis(ECLCluster::EHypothesisBit::c_neutralHadron))
          continue;

        // create particle and check it before adding to list
        Particle particle(cluster, thisType);
        if (particle.getParticleSource() != Particle::c_ECLCluster) {
          B2FATAL("Particle created from ECLCluster does not have ECLCluster type.");
          continue;
        }
        Particle* newPart = m_particles.appendNew(particle);

        // set relations to mcparticles
        for (auto& weightsAndIndex : weightsAndIndices) {
          const MCParticle* relMCParticle = m_mcparticles[weightsAndIndex.first];
          double weight = weightsAndIndex.second;

          // TODO: study this further and avoid hard-coded values
          // set the relation only if the MCParticle(reconstructed Particle)'s
          // energy contribution to this cluster amounts to at least 30(20)%
          if (relMCParticle)
            if (weight / newPart->getECLClusterEnergy() > 0.20
                && weight / relMCParticle->getEnergy() > 0.30)
              newPart->addRelationTo(relMCParticle, weight);
        }


        // add particle to list if it passes the selection criteria
        auto& cut = get<c_CutPointer>(eclCluster2Plist);
        StoreObjPtr<ParticleList> plist(listName);
        if (cut->check(newPart))
          plist->addParticle(newPart);

      } // loop over particle lists to be filled by ECLClusters
    } // loop over cluster
  }

  void ParticleLoaderModule::klmClustersToParticles()
  {
    if (m_KLMClusters2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto klmCluster2Plist : m_KLMClusters2Plists) {
      string listName = get<c_PListName>(klmCluster2Plist);
      string antiListName = get<c_AntiPListName>(klmCluster2Plist);
      int pdgCode = get<c_PListPDGCode>(klmCluster2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(klmCluster2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      if (!plist) { // create the list only if the eclClustersToParticle has not already created it
        plist.create();
        plist->initialize(pdgCode, listName);

        if (!isSelfConjugatedParticle) {
          StoreObjPtr<ParticleList> antiPlist(antiListName);
          antiPlist.create();
          antiPlist->initialize(-1 * pdgCode, antiListName);

          antiPlist->bindAntiParticleList(*(plist));
        }
      }
    }

    // load reconstructed neutral KLM cluster's as Klongs or neutrons
    for (int i = 0; i < m_klmclusters.getEntries(); i++) {
      const KLMCluster* cluster      = m_klmclusters[i];

      if (std::isnan(cluster->getMomentumMag())) {
        B2WARNING("Skipping KLMCluster because of nan momentum.");
        continue;
      }

      const MCParticle* mcParticle = cluster->getRelated<MCParticle>();

      for (auto klmCluster2Plist : m_KLMClusters2Plists) {
        string listName = get<c_PListName>(klmCluster2Plist);
        int pdgCode = get<c_PListPDGCode>(klmCluster2Plist);

        // create particle and check its type before adding it to list
        Particle particle(cluster, pdgCode);
        if (particle.getParticleSource() != Particle::c_KLMCluster) {
          B2FATAL("Particle created from KLMCluster does not have KLMCluster type.");
        }
        Particle* newPart = m_particles.appendNew(particle);

        if (mcParticle)
          newPart->addRelationTo(mcParticle);

        // add particle to list if it passes the selection criteria
        auto&  cut = get<c_CutPointer>(klmCluster2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        if (cut->check(newPart))
          plist->addParticle(newPart);
      }
    }
  }

  void ParticleLoaderModule::mcParticlesToParticles()
  {
    if (m_MCParticles2Plists.empty()) // nothing to do
      return;

    // create all lists
    for (auto mcParticle2Plist : m_MCParticles2Plists) {
      string listName = get<c_PListName>(mcParticle2Plist);
      string antiListName = get<c_AntiPListName>(mcParticle2Plist);
      int pdgCode = get<c_PListPDGCode>(mcParticle2Plist);
      bool isSelfConjugatedParticle = get<c_IsPListSelfConjugated>(mcParticle2Plist);

      StoreObjPtr<ParticleList> plist(listName);
      plist.create();
      plist->initialize(pdgCode, listName);

      if (!isSelfConjugatedParticle) {
        StoreObjPtr<ParticleList> antiPlist(antiListName);
        antiPlist.create();
        antiPlist->initialize(-1 * pdgCode, antiListName);

        antiPlist->bindAntiParticleList(*(plist));
      }
    }

    for (int i = 0; i < m_mcparticles.getEntries(); i++) {
      const MCParticle* mcParticle = m_mcparticles[i];

      for (auto mcParticle2Plist : m_MCParticles2Plists) {
        int pdgCode = get<c_PListPDGCode>(mcParticle2Plist);

        if (abs(pdgCode) != abs(mcParticle->getPDG()))
          continue;

        Particle particle(mcParticle);
        Particle* newPart = m_particles.appendNew(particle);
        newPart->addRelationTo(mcParticle);

        //append the whole bottom part of the decay tree to this particle
        if (m_addDaughters) appendDaughtersRecursive(newPart);

        string listName = get<c_PListName>(mcParticle2Plist);
        auto&  cut = get<c_CutPointer>(mcParticle2Plist);
        StoreObjPtr<ParticleList> plist(listName);

        if (cut->check(newPart))
          plist->addParticle(newPart);
      }
    }
  }

  bool ParticleLoaderModule::isValidPDGCode(const int pdgCode)
  {
    bool result = false;

    // is particle type = charged final state particle?
    if (Const::chargedStableSet.find(abs(pdgCode)) != Const::invalidParticle)
      return true;

    if (abs(pdgCode) == abs(Const::photon.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Kshort.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Klong.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::Lambda.getPDGCode()))
      return true;

    if (abs(pdgCode) == abs(Const::neutron.getPDGCode()))
      return true;

    return result;
  }

  void ParticleLoaderModule::appendDaughtersRecursive(Particle* mother)
  {
    auto* mcmother = mother->getRelated<MCParticle>();

    if (!mcmother)
      return;

    vector<MCParticle*> mcdaughters = mcmother->getDaughters();

    for (auto& mcdaughter : mcdaughters) {
      if (!mcdaughter->hasStatus(MCParticle::c_PrimaryParticle) and m_skipNonPrimaryDaughters) continue;
      Particle particle(mcdaughter);
      Particle* daughter = m_particles.appendNew(particle);
      daughter->addRelationTo(mcdaughter);
      mother->appendDaughter(daughter, false);

      if (mcdaughter->getNDaughters() > 0)
        appendDaughtersRecursive(daughter);
    }
  }


} // end Belle2 namespace
