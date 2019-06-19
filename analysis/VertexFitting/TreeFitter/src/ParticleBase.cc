/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

//Creates a particle. Base class for all other particle classes.

#include <TDatabasePDG.h>

#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/InternalParticle.h>
#include <analysis/VertexFitting/TreeFitter/RecoComposite.h>
#include <analysis/VertexFitting/TreeFitter/RecoResonance.h>
#include <analysis/VertexFitting/TreeFitter/RecoTrack.h>

#include <analysis/VertexFitting/TreeFitter/RecoPhoton.h>
#include <analysis/VertexFitting/TreeFitter/RecoKlong.h>
#include <analysis/VertexFitting/TreeFitter/Resonance.h>
#include <analysis/VertexFitting/TreeFitter/Origin.h>
#include <analysis/VertexFitting/TreeFitter/FitParams.h>

#include <analysis/VertexFitting/TreeFitter/ConstraintConfig.h>

namespace TreeFitter {

  bool massConstraintType;
  std::vector<int> massConstraintListPDG;
  std::vector<int> fixedToMotherVertexListPDG;
  std::vector<int> geoConstraintListPDG;
  std::vector<std::string> removeConstraintList;

  ParticleBase::ParticleBase(Belle2::Particle* particle, const ParticleBase* mother) :
    m_particle(particle),
    m_mother(mother),
    m_index(0),
    m_pdgMass(particle->getPDGMass()),
    m_pdgWidth(0),
    m_pdgLifeTime(TDatabasePDG::Instance()->GetParticle(particle->getPDGCode())->Lifetime() * 1e9),
    m_charge(0),
    m_name("Unknown")
  {
    if (particle) {
      const int pdgcode = particle->getPDGCode();
      if (pdgcode) { // PDG code != 0

        double fltcharge = particle->getCharge();

        //  round to nearest integer
        m_charge = fltcharge < 0 ? int(fltcharge - 0.5) : int(fltcharge + 0.5);
        m_name = particle->getName();
      } else {// PDG code = 0
        m_charge = particle->getCharge() > 0 ? 1 : (particle->getCharge() < 0 ? -1 : 0);
      }
    }
  }


  ParticleBase::ParticleBase(const std::string& name) :
    m_particle(nullptr),
    m_mother(nullptr),
    m_index(0),
    m_pdgMass(0),
    m_pdgWidth(0),
    m_pdgLifeTime(0),
    m_charge(0),
    m_name(name)
  {}


  ParticleBase::~ParticleBase()
  {
    for (auto daughter : m_daughters) {
      delete daughter;
    }
    m_daughters.clear();
  }

  ParticleBase* ParticleBase::addDaughter(Belle2::Particle* cand, bool forceFitAll)
  {
    auto newDaughter = ParticleBase::createParticle(cand, this, forceFitAll);
    m_daughters.push_back(newDaughter);
    return m_daughters.back();
  }


  void ParticleBase::removeDaughter(const ParticleBase* pb)
  {
    auto iter = std::find(m_daughters.begin(), m_daughters.end(), pb);
    if (iter != m_daughters.end()) {
      delete *iter;
      m_daughters.erase(iter);
    } else {
      B2ERROR("Cannot remove particle, because not found ...");
    }
  }

  void ParticleBase::updateIndex(int& offset)
  {
    for (auto* daughter : m_daughters) {
      daughter->updateIndex(offset);
    }
    m_index = offset;
    offset += dim();
  }

  ParticleBase* ParticleBase::createOrigin(
    Belle2::Particle* daughter,
    bool forceFitAll,
    const std::vector<double>& customOriginVertex,
    const std::vector<double>& customOriginCovariance,
    const bool isBeamSpot
  )
  {
    return new Origin(daughter, forceFitAll, customOriginVertex, customOriginCovariance, isBeamSpot);
  }

  ParticleBase* ParticleBase::createParticle(Belle2::Particle* particle, const ParticleBase* mother, bool forceFitAll)
  {
    ParticleBase* rc = nullptr;

    if (!mother) { // 'head of tree' particles
      if (!particle->getMdstArrayIndex()) { //0 means it's a composite
        rc = new InternalParticle(particle, nullptr, forceFitAll);

      } else {

        rc = new InternalParticle(particle, nullptr, forceFitAll); //FIXME obsolete not touching it now god knows where this might be needed

      }

    } else if (particle->getMdstArrayIndex() ||
               particle->getTrack() ||
               particle->getECLCluster() ||
               particle->getKLMCluster()) { // external particles and final states

      if (particle->getTrack()) {
        rc = new RecoTrack(particle, mother);

      } else if (particle->getECLCluster()) {
        rc = new RecoPhoton(particle, mother);

      } else if (particle->getKLMCluster()) {
        rc = new RecoKlong(particle, mother);

      } else if (isAResonance(particle)) {
        rc = new RecoResonance(particle, mother);

      }  else {
        rc = new RecoComposite(particle, mother);

      }

    } else { // 'internal' particles

      if (false) {   // fitted composites //JFK::eventually implement prefitting mechanic to prefit composites with other fitters
        if (isAResonance(particle)) {

          rc = new RecoResonance(particle, mother);

        } else {
          rc = new RecoComposite(particle, mother);
        }

      } else {         // unfitted composites

        if (isAResonance(particle)) {
          rc = new Resonance(particle, mother, forceFitAll);

        } else {
          rc = new InternalParticle(particle, mother, forceFitAll);
        }
      }
    }
    return rc;
  }


  double ParticleBase::pdgLifeTime(Belle2::Particle* particle)
  {
    int pdgcode = particle->getPDGCode();
    double lifetime = 0;

    if (pdgcode) {
      lifetime = TDatabasePDG::Instance()->GetParticle(pdgcode)->Lifetime() * 1e9;
    }

    return lifetime ;
  }


  bool ParticleBase::isAResonance(Belle2::Particle* particle)
  {
    bool rc = false ;
    const int pdgcode = std::abs(particle->getPDGCode());

    if (pdgcode && !(particle->getMdstArrayIndex())) {
      switch (pdgcode) {
        case 22:  //photon conversion
          rc = false;
          break ;

        case -11: //bremsstrahlung
        case 11:
          rc = true ;
          break ;
        case 413:
        case 423:
          rc = false;
          break;
        default: //everything with boosted flight length less than 1 micrometer
          rc = (pdgcode && pdgLifeTime(particle) < 1e-5);
      }
    }
    return rc ;
  }

  void ParticleBase::collectVertexDaughters(std::vector<ParticleBase*>& particles, int posindex)
  {
    if (mother() && mother()->posIndex() == posindex) {
      particles.push_back(this);
    }

    for (auto* daughter : m_daughters) {
      daughter->collectVertexDaughters(particles, posindex);
    }
  }

  ErrCode ParticleBase::initCovariance(FitParams& fitparams) const
  {
    ErrCode status;

    const int posindex = posIndex();
    if (posindex >= 0) {
      for (int i = 0; i < 3; ++i) {
        fitparams.getCovariance()(posindex + i, posindex + i) = 50;
      }
    }

    // momentum
    const int momindex = momIndex();
    if (momindex >= 0) {
      const double initVal = 0.5;
      const int maxrow = hasEnergy() ? 4 : 3;

      for (int i = 0; i < maxrow; ++i) {
        fitparams.getCovariance()(momindex + i, momindex + i) = initVal;
      }
    }

    const int tauindex = tauIndex();

    if (tauindex >= 0) {

      /** cant multiply by momentum here because unknown but average mom is 3 Gev */
      /** if pdgMass = 0 then tauindex is always = -1, so this is safe */

      const double maxDecayLengthSigma = 1000;
      double tau = pdgTime() * Belle2::Const::speedOfLight / pdgMass() * 3;
      double sigtau = tau > 0 ? std::min(20 * tau, maxDecayLengthSigma)  : maxDecayLengthSigma;

      fitparams.getCovariance()(tauindex, tauindex) = 1000;
    }
    return status;
  }

  std::string ParticleBase::parname(int thisindex) const
  {
    std::string rc = name();
    switch (thisindex) {
      case 0: rc += "_x  "; break;
      case 1: rc += "_y  "; break;
      case 2: rc += "_z  "; break;
      case 3: rc += "_tau"; break;
      case 4: rc += "_px "; break;
      case 5: rc += "_py "; break;
      case 6: rc += "_pz "; break;
      case 7: rc += "_E  "; break;
      default:;
    }
    return rc;
  }

  const ParticleBase* ParticleBase::locate(Belle2::Particle* particle) const
  {
    const ParticleBase* rc = (m_particle == particle) ? this : nullptr;
    if (!rc) {
      for (auto* daughter : m_daughters) {
        rc = daughter->locate(particle);
        if (rc) {break;}
      }
    }
    return rc;
  }

  void ParticleBase::retrieveIndexMap(indexmap& map) const
  {
    map.push_back(std::pair<const ParticleBase*, int>(this, index()));
    for (auto* daughter : m_daughters) {
      daughter->retrieveIndexMap(map);
    }
  }

  double ParticleBase::chiSquare(const FitParams& fitparams) const
  {
    double rc = 0;
    for (auto* daughter : m_daughters) {
      rc += daughter->chiSquare(fitparams);
    }
    return rc;
  }

  int ParticleBase::nFinalChargedCandidates() const
  {
    int rc = 0;
    for (auto* daughter : m_daughters) {
      rc += daughter->nFinalChargedCandidates();
    }
    return rc;
  }

  ErrCode ParticleBase::projectGeoConstraint(const FitParams& fitparams, Projection& p) const
  {
    const int posindexmother = mother()->posIndex();
    const int posindex = posIndex();
    const int tauindex = tauIndex();
    const int momindex = momIndex();

    const Eigen::Matrix<double, 1, 3> p_vec = fitparams.getStateVector().segment(momindex, 3);
    const double mom = p_vec.norm();
    const double mom3 = mom * mom * mom;
    const Eigen::Matrix<double, 1, 3> x_vec = fitparams.getStateVector().segment(posindex, 3);
    const Eigen::Matrix<double, 1, 3> x_m = fitparams.getStateVector().segment(posindexmother, 3);

    double tau = fitparams.getStateVector()(tauindex);
    // linear approximation is fine
    for (int row = 0; row < 3; ++row) {
      double posxmother = x_m(row);
      double posx       = x_vec(row);
      double momx       = p_vec(row);

      /** the direction of the momentum is very well known from the kinematic constraints
       *  that is why we do not extract the distance as a vector here
       * */
      p.getResiduals()(row) = posxmother + tau * momx / mom - posx ;

      p.getH()(row, posindexmother + row) = 1;
      p.getH()(row, posindex + row) = -1;
      p.getH()(row, tauindex) = momx / mom;
    }
    //diagonal momentum
    p.getH()(0, momindex)     = tau * (p_vec(1) * p_vec(1) + p_vec(2) * p_vec(2)) / mom3 ;
    p.getH()(1, momindex + 1) = tau * (p_vec(0) * p_vec(0) + p_vec(2) * p_vec(2)) / mom3 ;
    p.getH()(2, momindex + 2) = tau * (p_vec(1) * p_vec(1) + p_vec(0) * p_vec(0)) / mom3 ;

    //offdiagonal momentum
    p.getH()(0, momindex + 1) = - tau * p_vec(0) * p_vec(1) / mom3 ;
    p.getH()(0, momindex + 2) = - tau * p_vec(0) * p_vec(2) / mom3 ;

    p.getH()(1, momindex + 0) = - tau * p_vec(1) * p_vec(0) / mom3 ;
    p.getH()(1, momindex + 2) = - tau * p_vec(1) * p_vec(2) / mom3 ;

    p.getH()(2, momindex + 0) = - tau * p_vec(2) * p_vec(0) / mom3 ;
    p.getH()(2, momindex + 1) = - tau * p_vec(2) * p_vec(1) / mom3 ;
    return ErrCode(ErrCode::Status::success);
  }

  void inline setExtraInfo(Belle2::Particle* part, const std::string& name, const double value)
  {
    if (part) {
      if (part->hasExtraInfo(name)) {
        part->setExtraInfo(name, value);
      } else {
        part->addExtraInfo(name, value);
      }
    }
  }

  ErrCode ParticleBase::projectMassConstraintDaughters(const FitParams& fitparams,
                                                       Projection& p) const
  {
    const double mass = pdgMass();
    const double mass2 = mass * mass;
    double px = 0;
    double py = 0;
    double pz = 0;
    double E  = 0;

    // the parameters of the daughters must be used otherwise the mass constraint does not have an effect on the extracted daughter momenta
    for (const auto* daughter : m_daughters) {
      const int momindex = daughter->momIndex();
      // in most cases the daughters will be final states so we cache the value to use it in the energy column
      const double px_daughter = fitparams.getStateVector()(momindex);
      const double py_daughter = fitparams.getStateVector()(momindex + 1);
      const double pz_daughter = fitparams.getStateVector()(momindex + 2);

      px += px_daughter;
      py += py_daughter;
      pz += pz_daughter;
      if (daughter->hasEnergy()) {
        E += fitparams.getStateVector()(momindex + 3);
      } else {
        // final states dont have an energy index
        const double m = daughter->pdgMass();
        E += std::sqrt(m * m + px_daughter * px_daughter + py_daughter * py_daughter + pz_daughter * pz_daughter);
      }
    }

    /** be aware that the signs here are important
     * E-|p|-m extracts a negative mass and messes with the momentum !
     * */
    p.getResiduals()(0) = mass2 - E * E + px * px + py * py + pz * pz;

    for (const auto* daughter : m_daughters) {
      //dr/dx = d/dx m2-{E1+E2+...}^2+{p1+p2+...}^2 = 2*x (x= E or p)
      const int momindex = daughter->momIndex();
      p.getH()(0, momindex)     = 2.0 * px;
      p.getH()(0, momindex + 1) = 2.0 * py;
      p.getH()(0, momindex + 2) = 2.0 * pz;

      if (daughter->hasEnergy()) {
        p.getH()(0, momindex + 3) = -2.0 * E;
      } else {
        const double px_daughter = fitparams.getStateVector()(momindex);
        const double py_daughter = fitparams.getStateVector()(momindex + 1);
        const double pz_daughter = fitparams.getStateVector()(momindex + 2);
        const double m = daughter->pdgMass();

        const double E_daughter = std::sqrt(m * m + px_daughter * px_daughter + py_daughter * py_daughter + pz_daughter * pz_daughter);
        const double E_by_E_daughter = E / E_daughter;
        p.getH()(0, momindex)     -= 2.0 * E_by_E_daughter * px_daughter;
        p.getH()(0, momindex + 1) -= 2.0 * E_by_E_daughter * py_daughter;
        p.getH()(0, momindex + 2) -= 2.0 * E_by_E_daughter * pz_daughter;
      }

    }
    return ErrCode(ErrCode::Status::success);
  }

  ErrCode ParticleBase::projectMassConstraintParticle(const FitParams& fitparams,
                                                      Projection& p) const
  {
    const double mass = pdgMass();
    const double mass2 = mass * mass;
    const int momindex = momIndex();
    const double px = fitparams.getStateVector()(momindex);
    const double py = fitparams.getStateVector()(momindex + 1);
    const double pz = fitparams.getStateVector()(momindex + 2);
    const double E  = fitparams.getStateVector()(momindex + 3);

    /** be aware that the signs here are important
     * E-|p|-m extracts a negative mass and messes with the momentum !
     * */
    p.getResiduals()(0) = -mass2 + E * E - px * px - py * py - pz * pz;

    p.getH()(0, momindex)     = -2.0 * px;
    p.getH()(0, momindex + 1) = -2.0 * py;
    p.getH()(0, momindex + 2) = -2.0 * pz;
    p.getH()(0, momindex + 3) = 2.0 * E;

    return ErrCode(ErrCode::Status::success);
  }

  ErrCode ParticleBase::projectMassConstraint(const FitParams& fitparams,
                                              Projection& p) const
  {
    if (TreeFitter::massConstraintType == 0) {
      return projectMassConstraintParticle(fitparams, p);
    } else {
      return projectMassConstraintDaughters(fitparams, p);
    }
  }

  ErrCode ParticleBase::projectConstraint(Constraint::Type type, const FitParams& fitparams, Projection& p) const
  {
    ErrCode status;
    if (type == Constraint::mass) {
      return projectMassConstraint(fitparams, p);
    } else {
      B2FATAL("Trying to project constraint of ParticleBase type. This is undefined.");
    }
    return ErrCode(ErrCode::Status::badsetup);
  }

  double ParticleBase::bFieldOverC()
  {
    return (Belle2::BFieldManager::getField(TVector3(0, 0, 0)).Z() * Belle2::Const::speedOfLight);
  }

  ErrCode ParticleBase::initTau(FitParams& fitparams) const
  {
    const int tauindex = tauIndex();
    if (tauindex >= 0 && hasPosition()) {

      assert(mother());

      /**
       * In principle we have to divide by a momentum here but since the initial momentum is unknown
       * and is maximally a factor of 5 we don't need to do that here
       * */
      const double value = pdgTime() * Belle2::Const::speedOfLight / pdgMass();

      fitparams.getStateVector()(tauindex) = value * 100;
    }

    return ErrCode(ErrCode::Status::success);
  }
} //end namespace TreeFitter

