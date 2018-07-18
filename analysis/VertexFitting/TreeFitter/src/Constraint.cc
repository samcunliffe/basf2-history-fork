/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2017 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Francesco Tenchini,Jo-Frederik Krohn                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <analysis/VertexFitting/TreeFitter/FitParams.h>
#include <analysis/VertexFitting/TreeFitter/ParticleBase.h>
#include <analysis/VertexFitting/TreeFitter/Constraint.h>
#include <analysis/VertexFitting/TreeFitter/KalmanCalculator.h>
#include <iostream>
#include <framework/logging/Logger.h>

namespace TreeFitter {

  bool Constraint::operator<(const Constraint& rhs) const
  {
    return m_depth < rhs.m_depth  ||
           (m_depth == rhs.m_depth && m_type < rhs.m_type);
    //return m_type < rhs.m_type  ||
    //       (m_type == rhs.m_type && m_depth < rhs.m_depth);
  }

  ErrCode Constraint::project(const FitParams& fitpar, Projection& p) const
  {
    return m_node->projectConstraint(m_type, fitpar, p);
  }

  ErrCode Constraint::filter(FitParams& fitpar)
  {
    ErrCode status;
    Projection p(fitpar.getDimensionOfState(), m_dim);
    KalmanCalculator kalman(m_dim, fitpar.getDimensionOfState());

    B2DEBUG(11, "Filtering: " << this->name() << " dim state " << fitpar.getDimensionOfState()
            << " dim contr " << m_dim << "\n");

    double chisq(0);
    int iter(0);
    bool finished(false) ;

    bool deleteFitpars = false;

    FitParams* unfilteredState = nullptr;
    if (m_maxNIter > 1) {
      unfilteredState = new FitParams(fitpar);
      deleteFitpars = true;
    }
    while (!finished && !status.failure()) {

      p.resetProjection();
      status |= project(fitpar, p);

      if (!status.failure()) {

        status |= kalman.calculateGainMatrix(
                    p.getResiduals(),
                    p.getH(),
                    fitpar,
                    &p.getV(),
                    1
                  );

        if (!status.failure()) {
          if (iter > 0) {
            kalman.updateState(fitpar, *unfilteredState);
          } else {
            kalman.updateState(fitpar);
          }

          double newchisq = kalman.getChiSquare();

          double dchisqconverged = 0.001 ;

          double dchisq = newchisq - chisq;
          bool diverging = iter > 0 && dchisq > 0;
          bool converged = std::abs(dchisq) < dchisqconverged;
          finished  = ++iter >= m_maxNIter || diverging || converged;
          chisq = newchisq;
        }
      }
    }

    const unsigned int NDF = kalman.getConstraintDim();
    fitpar.addChiSquare(kalman.getChiSquare(), NDF);

    //std::cout << this->name() << " " << kalman.getChiSquare()  << std::endl;

    if (deleteFitpars) { delete unfilteredState; }
    kalman.updateCovariance(fitpar);
    m_chi2 = kalman.getChiSquare();
    return status;
  }

  ErrCode Constraint::filterWithReference(FitParams& fitpar, const FitParams& oldState)
  {
    ErrCode status;
    Projection p(fitpar.getDimensionOfState(), m_dim);
    KalmanCalculator kalman(m_dim, fitpar.getDimensionOfState());
    B2DEBUG(11, "Filtering: " << this->name() << " dim state " << fitpar.getDimensionOfState()
            << " dim contr " << m_dim << "\n");
    double chisq(0);
    int iter(0);
    bool finished(false) ;

    while (!finished && !status.failure()) {

      p.resetProjection();

      /** here we project the old state and use only the change with respect to the new state
       * instead of the new state in the update . the advantage is smaller steps */
      status |= project(oldState, p);

      p.getResiduals() += p.getH() * (fitpar.getStateVector() - oldState.getStateVector());

      if (!status.failure()) {
        status |= kalman.calculateGainMatrix(
                    p.getResiduals(),
                    p.getH(),
                    fitpar,
                    &p.getV(),
                    1
                  );

        if (!status.failure()) {
          kalman.updateState(fitpar);
          double newchisq = kalman.getChiSquare();

          double dchisqconverged = 0.001 ;

          double dchisq = newchisq - chisq;
          bool diverging = iter > 0 && dchisq > 0;
          bool converged = std::abs(dchisq) < dchisqconverged;
          finished  = ++iter >= m_maxNIter || diverging || converged;
          chisq = newchisq;
        }
      }
    }

    const unsigned int NDF = kalman.getConstraintDim();
    fitpar.addChiSquare(kalman.getChiSquare(), NDF);

    //std::cout << this->name() << " " << kalman.getChiSquare()  << std::endl;

    kalman.updateCovariance(fitpar);
    m_chi2 = kalman.getChiSquare();
    return status;
  }

  std::string Constraint::name() const
  {
    std::string rc = "unknown constraint!";
    switch (m_type) {
      case beamspot:     rc = "beamspot";   break;
      case beamenergy:   rc = "beamenergy"; break;
      case origin:       rc = "origin"; break;
      case composite:    rc = "composite";  break;
      case resonance:    rc = "resonance";  break;
      case track:        rc = "track";      break;
      case photon:       rc = "photon";     break;
      case klong:        rc = "klong";     break;
      case kinematic:    rc = "kinematic";  break;
      case geometric:    rc = "geometric";  break;
      case mass:         rc = "mass";       break;
      case massEnergy:   rc = "massEnergy"; break;
      case lifetime:     rc = "lifetime";   break;
      case merged:       rc = "merged";     break;
      case conversion:   rc = "conversion"; break;
      case ntypes:
      case unknown:
        break;
    }
    return rc;
  }
}
