/**************************************************************************
 * BASF2 (Belle Analysis Framework 2) *
 * Copyright(C) 2012 Belle II Collaboration *
 * *
 * Author: The Belle II Collaboration *
 * Contributors:
 * *
 * This software is provided "as is" without any warranty. *
 **************************************************************************/

#ifndef EvtB0toKsKK_H
#define EvtB0toKsKK_H

#include "EvtGenBase/EvtDecayAmp.hh"
#include "EvtGenBase/EvtParticle.hh"

class EvtB0toKsKK : public  EvtDecayAmp {

public:

  /* Constructor. */

  EvtB0toKsKK() {}

  /* Destructor. */

  virtual ~EvtB0toKsKK();

  std::string getName();  /**< Get function Name  */

  EvtDecayBase* clone();  /**< Clone the decay of B0toKsKK */

  void init();  /**< Initialize standard stream objects  */

  void initProbMax();  /**< Initialize standard stream objects for probability function  */

  void decay(EvtParticle* p); /**< Member of particle in EvtGen */

  EvtVector4R umu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);  /**< Function 4Vector umu */
  EvtVector4R Smu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);  /**< Function 4Vector Smu */
  EvtVector4R Lmu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                  const EvtVector4R& p4c);  /**< Function 4Vector Lmu */
  EvtTensor4C gmunu_tilde(const EvtVector4R& p4a,
                          const EvtVector4R& p4b,
                          const EvtVector4R& p4c); /**< Function Tensor gmunu  */
  EvtTensor4C Tmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);     /**< Function Tensor Tmunu  */
  EvtTensor4C Multiply(const EvtTensor4C& t1,
                       const EvtTensor4C& t2);   /**< Function Tensor Multiply  */
  EvtTensor4C RaiseIndices(const EvtTensor4C& t);  /**< Function RaiseIndices   */
  void RaiseIndex(EvtVector4R& vector);            /**< Member */
  EvtTensor4C Mmunu(const EvtVector4R& p4a, const EvtVector4R& p4b,
                    const EvtVector4R& p4c);
  double BWBF(const double& q, const unsigned int& L);
  double BWBF(const double& q, const double& q0,
              const unsigned int& L);
  EvtComplex BreitWigner(const double& m, const double& m0,
                         const double& Gamma0,
                         const double& q, const double& q0,
                         const unsigned int& L);   /**< BreitWigner Shape */
  EvtVector4R Boost(const EvtVector4R& p4,
                    const EvtVector4R& boost);     /**< Parameter for boost frame  */
  double p(const double& mab, const double& M, const double& mc);  /**< Constants p  */
  double q(const double& mab, const double& ma, const double& mb); /**< Constants q */
  EvtComplex Flatte_k(const double& s, const double& m_h); /**< Constant Flatte_k */
  EvtComplex Flatte(const double& m, const double& m0);    /**< Constant Flatte */

  EvtComplex A_f0ks(const EvtVector4R& p4ks,   /**< Variable A_f0ks */
                    const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_phiks(const EvtVector4R& p4ks,  /**< Variable A_phiks */
                     const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_fxks(const EvtVector4R& p4ks,   /**< Variable A_fxks */
                    const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_chic0ks(const EvtVector4R& p4ks, /**< Variable A_chic0ks */
                       const EvtVector4R& p4kp, const EvtVector4R& p4km);
  EvtComplex A_kknr(const EvtVector4R& p4k1, const EvtVector4R& p4k2,
                    const double& alpha_kk);    /**< Variable  A_kknr */

private:

  /**<Variable names*/

  EvtComplex a_f0ks_;      /**< Variable member  a_f0ks_   */
  EvtComplex a_phiks_;     /**< Variable member  a_phiks_  */
  EvtComplex a_fxks_;      /**< Variable member  a_fxks_   */
  EvtComplex a_chic0ks_;   /**< Variable member  a_chic0ks_*/
  EvtComplex a_kpkmnr_;    /**< Variable member  a_kpkmnr_ */
  EvtComplex a_kskpnr_;    /**< Variable member  a_kskpnr_ */
  EvtComplex a_kskmnr_;    /**< Variable member  a_kskmnr_ */

  EvtComplex abar_f0ks_;    /**< Variable member abar_f0ks_  */
  EvtComplex abar_phiks_;   /**< Variable member abar_phiks_ */
  EvtComplex abar_fxks_;    /**< Variable member abar_fxks_  */
  EvtComplex abar_chic0ks_; /**< Variable member abar_chic0ks_ */
  EvtComplex abar_kpkmnr_;  /**< Variable member abar_kpkmnr_  */
  EvtComplex abar_kskpnr_;  /**< Variable member abar_kskpnr_  */
  EvtComplex abar_kskmnr_;  /**< Variable member abar_kskmnr_  */

  double alpha_kpkmnr;      /**< Variable member alpha_kpkmnr */
  double alpha_kskpnr;      /**< Variable member alpha_kskpnr */
  double alpha_kskmnr;      /**< Variable member alpha_kskmnr */

  std::ofstream debugfile_;
};

#endif
