/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Luka Santelj                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef ARICHLIKELIHOOD_H
#define ARICHLIKELIHOOD_H

#include <framework/datastore/RelationsObject.h>

namespace Belle2 {



  /** This is a class to store ARICH likelihoods in the datastore.
   *
   *  It is the output of reconstruction module (ARICHReconstructor).
   */

  class ARICHLikelihood : public RelationsObject {

  public:

    /*! default constructor */

    ARICHLikelihood(): m_flag(0), m_logL_e(0), m_logL_mu(0), m_logL_pi(0), m_logL_K(0),
      m_logL_p(0), m_NphotD_e(0), m_NphotD_mu(0), m_NphotD_pi(0), m_NphotD_K(0), m_NphotD_p(0), m_Nphot_e(0), m_Nphot_mu(0), m_Nphot_pi(0),
      m_Nphot_K(0), m_Nphot_p(0) {
    }

    /*! full constructor
     * @param flag:     reconstruction flag
     * @param logL_e:   electron log likelihood
     * @param logL_mu:  muon log likelihood
     * @param logL_pi:  pion log likelihood
     * @param logL_K:   kaon log likelihood
     * @param logL_p:   proton log likelihood
     * @param NphotD_e: detected photons in electron expected ring
     * @param NphotD_e: detected photons in muon expected ring
     * @param NphotD_e: detected photons in pion expected ring
     * @param NphotD_e: detected photons in kaon expected ring
     * @param NphotD_e: detected photons in proton expected ring
     * @param Nphot_e:  expected photons for electron
     * @param Nphot_mu: expected photons for muon
     * @param Nphot_pi: expected photons for pion
     * @param Nphot_K:  expected photons for kaon
     * @param Nphot_p:  expected photons for proton
     */

    ARICHLikelihood(int flag,
                    double logL_e,
                    double logL_mu,
                    double logL_pi,
                    double logL_K,
                    double logL_p,
                    int NphotD_e,
                    int NphotD_mu,
                    int NphotD_pi,
                    int NphotD_K,
                    int NphotD_p,
                    double Nphot_e,
                    double Nphot_mu,
                    double Nphot_pi,
                    double Nphot_K,
                    double Nphot_p) {
      m_flag = flag;
      m_logL_e  = (float) logL_e;
      m_logL_mu = (float) logL_mu;
      m_logL_pi = (float) logL_pi;
      m_logL_K  = (float) logL_K;
      m_logL_p  = (float) logL_p;
      m_NphotD_e = NphotD_e;
      m_NphotD_mu = NphotD_mu;
      m_NphotD_pi = NphotD_pi;
      m_NphotD_K = NphotD_K;
      m_NphotD_p = NphotD_p;
      m_Nphot_e  = (float) Nphot_e;
      m_Nphot_mu = (float) Nphot_mu;
      m_Nphot_pi = (float) Nphot_pi;
      m_Nphot_K  = (float) Nphot_K;
      m_Nphot_p  = (float) Nphot_p;
    }

    /*! full constructor
     * @param flag:         reconstruction flag
     * @param logL:         log likelihoods in order e, mu, pi, K, p
     * @param NphotD:       detected photons in e,mu,pi,K,p ring
     * @param Nphot_expect: number of expected photons in order e, mu, pi, K, p
     */

    ARICHLikelihood(int flag, double logL[5], int NphotD[5], double Nphot_expect[5]) {
      m_flag = flag;
      m_logL_e  = (float) logL[0];
      m_logL_mu = (float) logL[1];
      m_logL_pi = (float) logL[2];
      m_logL_K  = (float) logL[3];
      m_logL_p  = (float) logL[4];
      m_NphotD_e = NphotD[0];
      m_NphotD_mu = NphotD[1];
      m_NphotD_pi = NphotD[2];
      m_NphotD_K = NphotD[3];
      m_NphotD_p = NphotD[4];
      m_Nphot_e  = (float) Nphot_expect[0];
      m_Nphot_mu = (float) Nphot_expect[1];
      m_Nphot_pi = (float) Nphot_expect[2];
      m_Nphot_K  = (float) Nphot_expect[3];
      m_Nphot_p  = (float) Nphot_expect[4];
    }

    /*! Get reconstruction flag
     * @return reconstruction flag: 1=OK, 0=out of acceptance, -1=inside gap btw. bars
     */
    int getFlag() const {return m_flag;}

    /*! Get electron log likelihood
     * @return electron log likelihood
     */
    double getLogL_e() const {return m_logL_e;}

    /*! Get muon log likelihood
     * @return muon log likelihood
     */
    double getLogL_mu() const {return m_logL_mu;}

    /*! Get pion log likelihood
     * @return pion log likelihood
     */
    double getLogL_pi() const {return m_logL_pi;}

    /*! Get kaon log likelihood
     * @return kaon log likelihood
     */
    double getLogL_K() const {return m_logL_K;}

    /*! Get proton log likelihood
     * @return proton log likelihood
     */
    double getLogL_p() const {return m_logL_p;}

    /*! Get number of detected photons in electron expected ring
     * @return number of detected photons  in electron expected ring
     */
    int getNphotD_e() const {return m_NphotD_e;}

    /*! Get number of detected photons in muon expected ring
     * @return number of detected photons in muon expected ring
     */
    int getNphotD_mu() const {return m_NphotD_mu;}

    /*! Get number of detected photons in pion expected ring
     * @return number of detected photons in pion expected ring
     */
    int getNphotD_pi() const {return m_NphotD_pi;}

    /*! Get number of detected photons in kaon expected ring
     * @return number of detected photons in kaon expected ring
     */
    int getNphotD_K() const {return m_NphotD_K;}

    /*! Get number of detected photons in proton expected ring
     * @return number of detected photons in proton expected ring
     */
    int getNphotD_p() const {return m_NphotD_p;}

    /*! Get number of expected photons for electron
     * @return number of expected photons for electron
     */
    double getNphot_e() const {return m_Nphot_e;}

    /*! Get number of expected photons for muon
     * @return number of expected photons for muon
     */
    double getNphot_mu() const {return m_Nphot_mu;}

    /*! Get number of expected photons for pion
     * @return number of expected photons for pion
     */
    double getNphot_pi() const {return m_Nphot_pi;}

    /*! Get number of expected photons for kaon
     * @return number of expected photons for kaon
     */
    double getNphot_K() const {return m_Nphot_K;}

    /*! Get number of expected photons for proton
     * @return number of expected photons for proton
     */
    double getNphot_p() const {return m_Nphot_p;}

  private:
    int m_flag;          /**< reconstruction flag */
    float m_logL_e;     /**< log likelihood for electron hypothesis */
    float m_logL_mu;    /**< log likelihood for muon hypothesis */
    float m_logL_pi;    /**< log likelihood for pion hypothesis */
    float m_logL_K;     /**< log likelihood for kaon hypothesis */
    float m_logL_p;     /**< log likelihood for proton hypothesis */
    int m_NphotD_e;     /**< number of detected photons in electron expected ring */
    int m_NphotD_mu;    /**< number of detected photons in muon expected ring */
    int m_NphotD_pi;    /**< number of detected photons in pion expected ring */
    int m_NphotD_K;     /**< number of detected photons in kaon expected ring */
    int m_NphotD_p;     /**< number of detected photons in proton expected ring */
    float m_Nphot_e;    /**< number of expected photons for electron hypothesis */
    float m_Nphot_mu;   /**< number of expected photons for muon hypothesis */
    float m_Nphot_pi;   /**< number of expected photons for pion hypothesis */
    float m_Nphot_K;    /**< number of expected photons for kaon hypothesis */
    float m_Nphot_p;    /**< number of expected photons for proton hypothesis */

    ClassDef(ARICHLikelihood, 1); /**< the class title */

  };

} // end namespace Belle2

#endif

