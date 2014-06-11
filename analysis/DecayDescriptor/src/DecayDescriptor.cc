#include <framework/logging/Logger.h>
#include <analysis/DecayDescriptor/DecayDescriptor.h>
#include <analysis/DecayDescriptor/DecayStringParticle.h>
#include <analysis/DecayDescriptor/DecayStringDecay.h>
#include <analysis/DecayDescriptor/DecayStringGrammar.h>
#include <evtgen/EvtGenBase/EvtPDL.hh>
#include <evtgen/EvtGenBase/EvtId.hh>
#include <algorithm>
#include <boost/variant/get.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <utility>

using namespace Belle2;
using namespace std;

const DecayDescriptor& DecayDescriptor::s_NULL = DecayDescriptor();

DecayDescriptor::DecayDescriptor() :
  m_mother(),
  m_iDaughter_p(-1),
  m_daughters(),
  m_isIgnorePhotons(false),
  m_isIgnoreIntermediate(false),
  m_isInclusive(false),
  m_isNULL(false)
{
}

DecayDescriptor::DecayDescriptor(const DecayDescriptor& other) :
  m_mother(other.m_mother),
  m_iDaughter_p(other.m_iDaughter_p),
  m_daughters(other.m_daughters),
  m_isIgnorePhotons(other.m_isIgnorePhotons),
  m_isIgnoreIntermediate(other.m_isIgnoreIntermediate),
  m_isInclusive(other.m_isInclusive),
  m_isNULL(other.m_isNULL)
{
}

bool DecayDescriptor::init(const std::string str)
{
  // The decay string grammar
  DecayStringGrammar<std::string::const_iterator> g;
  DecayString s;
  std::string::const_iterator iter = str.begin();
  std::string::const_iterator end = str.end();
  bool r = phrase_parse(iter, end, g, boost::spirit::ascii::space, s);
  if (!r || iter != end) return false;
  return init(s);
}


bool DecayDescriptor::init(const DecayString& s)
{
  // The DecayString is a hybrid, it can be
  // a) DecayStringParticleList
  // b) DecayStringDecay

  bool isInitOK = false;

  if (const DecayStringParticle* p = boost::get< DecayStringParticle >(&s)) {
    isInitOK = m_mother.init(*p);
    if (!isInitOK) {
      B2WARNING("Could not initialise mother particle " << p->m_strName);
      return false;
    }
    return true;
  } else if (const DecayStringDecay* d = boost::get< DecayStringDecay > (&s)) {
    // Initialise list of mother particles
    isInitOK = m_mother.init(d->m_mother);
    if (!isInitOK) {
      B2WARNING("Could not initialise mother particle " << d->m_mother.m_strName);
      return false;
    }

    // Identify arrow type
    if (d->m_strArrow == "->") {
      m_isIgnorePhotons = false;
      m_isIgnoreIntermediate = false;
    } else if (d->m_strArrow == "=>") {
      m_isIgnorePhotons = true;
      m_isIgnoreIntermediate = false;
    } else if (d->m_strArrow == "-->") {
      m_isIgnorePhotons = false;
      m_isIgnoreIntermediate = true;
    } else if (d->m_strArrow == "==>") {
      m_isIgnorePhotons = true;
      m_isIgnoreIntermediate = true;
    } else {
      B2WARNING("Unknown arrow: " << d->m_strArrow);
      return false;
    }

    // Initialise list of daughters
    if (d->m_daughters.empty()) return false;
    int nDaughters = d->m_daughters.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      DecayDescriptor daughter;
      isInitOK = daughter.init(d->m_daughters[iDaughter]);
      if (!isInitOK) {
        B2WARNING("Could not initialise daughter!");
        return false;
      }
      m_daughters.push_back(daughter);
    }
    if (!d->m_strInclusive.empty()) m_isInclusive = true;
    return true;
  }
  return false;
}

template <class T>
int DecayDescriptor::match(const T* p, int iDaughter_p)
{
  // this DecayDescriptor was not matched or
  // it is not the daughter of another DecayDescriptor
  m_iDaughter_p = -1;

  if (!p) {
    B2WARNING("NULL pointer provided instead of particle.")
    return 0;
  }

  int iPDGCode_p = 0;
  if (const Particle* test = dynamic_cast<const Particle*>(p)) iPDGCode_p = test->getPDGCode();
  else if (const MCParticle* test = dynamic_cast<const MCParticle*>(p)) iPDGCode_p = test->getPDG();
  else {
    B2WARNING("Template type not supported!");
    return 0;
  }
  EvtId evtId_p = EvtPDL::evtIdFromStdHep(iPDGCode_p);
  int iPDGCodeCC_p = EvtPDL::getStdHep(EvtPDL::chargeConj(evtId_p));
  int iPDGCode_d = m_mother.getPDGCode();
  if (abs(iPDGCode_d) != abs(iPDGCode_p)) return 0;
  int iCC = 0;
  if (iPDGCode_p == iPDGCodeCC_p) iCC = 3;
  else if (iPDGCode_d == iPDGCode_p) iCC = 1;
  else if (iPDGCode_d == iPDGCodeCC_p) iCC = 2;

  const std::vector<T*> daughterList = p->getDaughters();
  int nDaughters_p = daughterList.size();

  // 1st case: the descriptor has no daughters => nothing to check
  if (getNDaughters() == 0) {
    m_iDaughter_p = iDaughter_p;
    return iCC;
  }

  // 2nd case: the descriptor has daughters, but not the particle
  // => that is not allowed!
  if (nDaughters_p == 0) return 0;

  // 3rd case: the descriptor and the particle have daughters
  // There are two cases that can happen when matching the
  // DecayDescriptor daughters to the particle daughters:
  // 1. The match is unambigous -> no problem
  // 2. Multiple particle daughters match the same DecayDescriptor daughter
  // -> in the latter case the ambiguity is resolved later

  // 1. DecayDescriptor -> Particle relation for the cases where only one particle matches
  vector< pair< int, int > > singlematch;
  // 2. DecayDescriptor -> Particle relation for the cases where multiple particles match
  vector< pair< int, set<int> > > multimatch;
  // Are there ambiguities in the match?
  bool isAmbiguities = false;
  // The particle daughters that have been matched
  set<int> matches_global;

  // check if the daughters match
  for (int iDaughter_d = 0; iDaughter_d < getNDaughters(); iDaughter_d++) {
    set<int> matches;
    for (int iDaughter_p = 0; iDaughter_p < nDaughters_p; iDaughter_p++) {
      const T* daughter = daughterList[iDaughter_p];
      int iPDGCode_daughter_p = 0;
      if (const Particle* test = dynamic_cast<const Particle*>(daughter)) iPDGCode_daughter_p = test->getPDGCode();
      else if (const MCParticle* test = dynamic_cast<const MCParticle*>(daughter)) iPDGCode_daughter_p = test->getPDG();
      if (iDaughter_d == 0 && m_isIgnorePhotons && iPDGCode_daughter_p == 22) matches_global.insert(iDaughter_p);
      int iMatchResult = m_daughters[iDaughter_d].match(daughter, iDaughter_p);
      if (iMatchResult < 0) isAmbiguities = true;
      if (abs(iMatchResult) == 2 && iCC == 1) continue;
      if (abs(iMatchResult) == 1 && iCC == 2) continue;
      if (abs(iMatchResult) == 2 && iCC == 3) continue;
      matches.insert(iDaughter_p);
      matches_global.insert(iDaughter_p);
    }
    if (matches.empty()) return 0;
    if (matches.size() == 1) {
      int iDaughter_p = *(matches.begin());
      singlematch.push_back(make_pair(iDaughter_d, iDaughter_p));
    } else multimatch.push_back(make_pair(iDaughter_d, matches));
  }

  // Now, all daughters of the particles should be matched to at least one DecayDescriptor daughter
  if (!m_isInclusive && int(matches_global.size()) != nDaughters_p) return 0;

  // In case that there are DecayDescriptor daughters with multiple matches, try to solve the problem
  // by removing the daughter candidates which are already used in other unambigous relations.
  // This is done iteratively. We limit the maximum number of attempts to 20 to avoid an infinit loop.
  bool isModified = true;
  for (int iTry = 0; iTry < 20; iTry++) {
    if (int(singlematch.size()) == getNDaughters()) break;
    if (!isModified) break;
    isModified = false;
    for (vector< pair< int, set<int> > >::iterator itMulti = multimatch.begin(); itMulti != multimatch.end(); ++itMulti) {
      for (vector< pair< int, int > >::iterator itSingle = singlematch.begin(); itSingle != singlematch.end(); ++itSingle) {
        // try to remove particle from the multimatch list
        if (itMulti->second.erase(itSingle->second)) {
          // if multimatch list contains only one particle candidate, move the entry to the singlematch list
          if (itMulti->second.size() == 1) {
            int iDaughter_d = itMulti->first;
            int iDaughter_p = *(itMulti->second.begin());
            singlematch.push_back(make_pair(iDaughter_d, iDaughter_p));
            multimatch.erase(itMulti);
            // call match function again to set the correct daughter
            if (!isAmbiguities) {
              const T* daughter = daughterList[iDaughter_p];
              if (!daughter) continue;
              m_daughters[iDaughter_d].match(daughter, iDaughter_p);
            }
            itMulti--;
            isModified = true;
            break;
          }
        }
      }
    }
  }

  if (!multimatch.empty()) isAmbiguities = true;
  if (isAmbiguities) return -iCC;
  else {
    m_iDaughter_p = iDaughter_p;
    return iCC;
  }
  return 0;
}

void DecayDescriptor::resetMatch()
{
  m_iDaughter_p = -1;
  int nDaughters = m_daughters.size();
  for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) m_daughters[iDaughter].resetMatch();
}

vector<const Particle*> DecayDescriptor::getSelectionParticles(const Particle* particle)
{
  // Create vector for output
  vector<const Particle*> selparticles;
  if (m_mother.isSelected()) selparticles.push_back(particle);
  int nDaughters_d = getNDaughters();
  for (int iDaughter_d = 0; iDaughter_d < nDaughters_d; ++iDaughter_d) {
    // retrieve the particle daughter ID from this DecayDescriptor daughter
    int iDaughter_p = m_daughters[iDaughter_d].getMatchedDaughter();
    // If the particle daughter ID is below one, the match function was not called before
    // or the match was ambigous. In this case try to use the daughter ID of the DecayDescriptor.
    // This corresponds to using the particle order in the decay string.
    if (iDaughter_p < 0) iDaughter_p = iDaughter_d;
    const Particle* daughter = particle->getDaughter(iDaughter_p);
    if (!daughter) {
      B2WARNING("Could not find daughter!");
      continue;
    }
    // check if the daughter has the correct PDG code
    if (abs(daughter->getPDGCode()) != abs(m_daughters[iDaughter_d].getMother()->getPDGCode())) {
      B2ERROR("The PDG code of the particle daughter does not match the PDG code of the DecayDescriptor daughter");
      break;
    }
    vector<const Particle*> seldaughters = m_daughters[iDaughter_d].getSelectionParticles(daughter);
    selparticles.insert(selparticles.end(), seldaughters.begin(), seldaughters.end());
  }
  return selparticles;
}

bool DecayDescriptor::isSelfConjugated() const
{

  std::vector<int> decay, decaybar;
  for (int i = 0; i < getNDaughters(); ++i) {
    const DecayDescriptorParticle* daughter = getDaughter(i)->getMother();
    int pdg = daughter->getPDGCode();
    decay.push_back(pdg);
    decaybar.push_back(Belle2::EvtPDLUtil::hasAntiParticle(pdg) ? -pdg : pdg);
  }

  std::sort(decay.begin(), decay.end());
  std::sort(decaybar.begin(), decaybar.end());

  return (not Belle2::EvtPDLUtil::hasAntiParticle(getMother()->getPDGCode())) || (decay == decaybar);

}

vector<string> DecayDescriptor::getSelectionNames()
{
  vector<string> strNames;
  if (m_mother.isSelected()) strNames.push_back(m_mother.getNameSimple());
  for (vector<DecayDescriptor>::iterator i = m_daughters.begin(); i != m_daughters.end(); ++i) {
    vector<string> strDaughterNames = i->getSelectionNames();
    int nDaughters = strDaughterNames.size();
    for (int iDaughter = 0; iDaughter < nDaughters; iDaughter++) {
      strDaughterNames[iDaughter] = m_mother.getNameSimple() + "_" + strDaughterNames[iDaughter];
    }
    strNames.insert(strNames.end(), strDaughterNames.begin(), strDaughterNames.end());
  }

  // search for multiple occurrence of the same name and then distinguish by attaching a number
  for (vector<string>::iterator itName = strNames.begin(); itName != strNames.end(); ++itName) {
    if (count(itName, strNames.end(), *itName) == 1) continue;
    // multiple occurrence found!
    string strNameOld = *itName;
    vector<string>::iterator itOccurrence = strNames.begin();
    int iOccurrence = 0;
    while (iOccurrence <= 10) {
      // find next occurence of the identical particle name defined in DecayDescriptor
      itOccurrence = find(itOccurrence, strNames.end(), strNameOld);
      // stop, if nothing found
      if (itOccurrence == strNames.end()) break;
      // create new particle name by attaching a number
      string strNameNew = strNameOld + boost::lexical_cast<string>(iOccurrence);
      // ceck if the new particle name exists already, if not, then it is OK to use it
      if (count(strNames.begin(), strNames.end(), strNameNew) == 0) {
        *itOccurrence = strNameNew;
        ++itOccurrence;
      }
      iOccurrence++;
    }
    if (iOccurrence == 10) {
      printf("DecayDescriptor::getSelectionNames - Something is wrong! More than 10x the same name!\n");
      break;
    }
  }
  return strNames;
}
