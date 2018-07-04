/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * right(C) 2018 - Belle II Collaboration                                 *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini, Jo-Frederik Krohn                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#pragma once


#include <analysis/dataobjects/Particle.h>
#include <vector>
#include <analysis/VertexFitting/TreeFitter/ErrCode.h>
#include <Eigen/Core>

namespace TreeFitter {
  class DecayChain;
  class FitParams;
  class ParticleBase;


  /** this class */
  class FitManager {
  public:

    /** status flag of the fit-itereation (the step in the newton method) */
    enum VertexStatus { Success = 0, NonConverged, BadInput, Failed, UnFitted };

    /** constructor  */
    FitManager() : m_particle(0), m_decaychain(0), m_fitparams(0), m_status(VertexStatus::UnFitted),
      m_chiSquare(-1), m_niter(-1), m_prec(0.01), m_updateDaugthers(false), m_ndf(0) {}

    /** constructor  */
    FitManager(Belle2::Particle* particle,
               double prec = 0.01,
               bool ipConstraint = false,
               bool customOrigin = false,
               bool updateDaughters = false,
               const std::vector<double> customOriginVertex = {0, 0, 0},
               const std::vector<double> customOriginCovariance = {0, 0, 0}
              );

    /** destructor does stuff */
    ~FitManager();

    /** main fit function that uses the kalman filter */
    bool fit();

    /** add extrainfo to particle */
    void setExtraInfo(Belle2::Particle* part, const std::string name, const double value);

    /** update particles parameters with the fit results */
    bool updateCand(Belle2::Particle& particle, const bool isTreeHead) const;

    /** locate particle base for a belle2 particle and update the particle with the values from particle base */
    void updateCand(const ParticleBase& pb, Belle2::Particle& cand, const bool isTreeHead) const;

    /** update the Belle2::Particles with the fit results  */
    void updateTree(Belle2::Particle& particle, const bool isTreeHead) const;

    /** extract cov from particle base */
    void getCovFromPB(const ParticleBase* pb, TMatrixFSym& returncov) const;

    /** get lifetime */
    std::tuple<double, double> getLifeTime(Belle2::Particle& cand) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(const ParticleBase* pb, const FitParams* fitparams) const;

    /**get decay length */
    std::tuple<double, double> getDecayLength(Belle2::Particle& cand) const;

    /** getter for the index of the vertex position in the state vector */
    int posIndex(Belle2::Particle* particle) const;

    /** getter for the index of the momentum in the state vector */
    int momIndex(Belle2::Particle* particle) const;

    /** getter for the index of tau, the lifetime in the statevector*/
    int tauIndex(Belle2::Particle* particle) const;

    /** getter for chi2 of the newton iteration */
    double chiSquare() const { return m_chiSquare ; }

    /**  getter for the decay chains chi2 */
    double globalChiSquare() const;

    /** getter for degrees of freedom of the fitparameters */
    int nDof() const;

    /** getter for the status of the newton iteration  */
    int status() const { return m_status; }

    /** getter for the current iteration number of the newton iteration  */
    int nIter() const { return m_niter; }

    /** getter for some errorcode flag  FIXME isn't this covered by the statusflag?*/
    const ErrCode& errCode() { return m_errCode; }


    ///** get the decay chain FIXME unused */
    //DecayChain* decaychain() { return m_decaychain; }

    /** get the entire statevector */
    FitParams* fitparams() { return m_fitparams; }

    /** const getter for the decay chain */
    const DecayChain* decaychain() const { return m_decaychain; }

    /** const getter for the statevector ???  */
    const FitParams* fitparams() const { return m_fitparams; }

    /**  getter for the head of the tree*/
    Belle2::Particle* particle() { return m_particle; }

  private:
    /** head of the tree  */
    Belle2::Particle* m_particle;

    /**  the decay tree */
    DecayChain* m_decaychain;

    /**  the statevector */
    FitParams* m_fitparams;

    /** status of the current iteration */
    int m_status;

    /** chi2 of the current iteration */
    double m_chiSquare;

    /** iteration index */
    int m_niter;

    /** precision that is needed for status:converged (delta chi2) */
    double m_prec;

    /** errorcode */
    ErrCode m_errCode;

    /** if this is set all daughters will be updated otherwise only the head of the tree */
    const bool m_updateDaugthers;

    /** number of degrees of freedom for this topology */
    int m_ndf;
  };
}
