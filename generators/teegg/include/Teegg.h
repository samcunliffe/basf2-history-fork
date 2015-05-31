/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef TEEGG_H
#define TEEGG_H

#include <mdst/dataobjects/MCParticleGraph.h>
#include <TLorentzRotation.h>
#include <utility>

namespace Belle2 {

  /**
   * C++ Interface for the Fortran generator TEEGG.
   *
   */

  class Teegg {
  public:

    /** Constructor.
     * Sets the default settings.
     */
    Teegg();

    /** Destrucotr. */
    ~Teegg();

    /** Sets the default settings for the TEEGG Fortran generator. */
    void setDefaultSettings();

    /** Sets the Lorentz boost vector which should be applied to the generated particles.
     * @param boostVector The Lorentz boost vector which is applied to the generated particles.
     */
    void setBoost(TLorentzRotation boostVector) { m_boostVector = boostVector; }

    /** Enables the boost of the generated particles.
     * @param applyBoost Set to true to enabled the boost. Also make sure you have set the boost vector using setBoost().
     */
    void enableBoost(bool applyBoost = true) { m_applyBoost = applyBoost; }

    /** specifies which vacuum polarization code (NOT IMPLEMENTED YET)
    * @param vacpol vacuum polarization code (NOT IMPLEMENTED YET)
    */
    void setVacPol(std::string vacpol = "OFF") {m_vacPol = vacpol;}

    /** set maximum theta of e+ in final state (in radians)
     * @param teveto maximum theta of e+ in final state (in radians)
     */
    void setTEVETO(double teveto = 666.66) { m_TEVETO = teveto; }

    /** set minimum angle between the e- and -z axis (egamma conf. only)
     * @param temin minimum angle between the e- and -z axis (egamma conf. only)
     */
    void setTEMIN(double temin = 666.66) { m_TEMIN = temin; }

    /** set minimum angle between the gamma and -z axis
     * @param tgmin minimum angle between the gamma and -z axis
     */
    void setTGMIN(double tgmin = 666.66) { m_TGMIN = tgmin; }

    /** set maximum angle between the gamma and -z axis(etron conf. only)
     * @param tgveto maximum angle between the gamma and -z axis(etron conf. only)
     */
    void setTGVETO(double tgveto = 666.66) { m_TGVETO = tgveto; }

    /** set minimum energy of the e- (egamma & etron configurations)
     * @param eemin minimum energy of the e- (egamma & etron configurations)
     */
    void setEEMIN(double eemin = 666.66) { m_EEMIN = eemin; }

    /** set minimum energy of the gamma (egamma & gamma configurations)
     * @param egmin minimum energy of the gamma (egamma & gamma configurations)
     */
    void setEGMIN(double egmin = 666.66) { m_EGMIN = egmin; }

    /** set minimum phi sep of e-gamma (egamma config with hard rad corr)
     * @param pegmin minimum phi sep of e-gamma (egamma config with hard rad corr)
     */
    void setPEGMIN(double pegmin = 666.66) { m_PEGMIN = pegmin; }

    /** set minimum energy to veto(gamma  config with hard rad corr)
     * @param eeveto minimum energy to veto(gamma  config with hard rad corr)
     */
    void setEEVETO(double eeveto = 666.66) { m_EEVETO = eeveto; }

    /** set minimum energy to veto(etron/gamma config with hard rad corr)
     * @param egveto minimum energy to veto(etron/gamma config with hard rad corr)
     */
    void setEGVETO(double egveto = 666.66) { m_EGVETO = egveto; }

    /** set minimum phi sep to veto(etron/gamma config with hard rad corr)
     * @param phveto minimum phi sep to veto(etron/gamma config with hard rad corr)
     */
    void setPHVETO(double phveto = 666.66) { m_PHVETO = phveto; }

    /** set cutoff energy for radiative corrections (in CM frame)
     * @param cutoff cutoff energy for radiative corrections (in CM frame)
     */
    void setCUTOFF(double cutoff = 666.66) { m_CUTOFF = cutoff; }

    /** set param. epsilon_s (smaller val. increases sampling of k_s^pbc)
     * @param eps param. epsilon_s (smaller val. increases sampling of k_s^pbc)
     */
    void setEPS(double eps = 666.66) { m_EPS = eps; }

    /** set fraction of time phi_ks is generated with peak(hard rad corr)
     * @param fraphi parfraction of time phi_ks is generated with peak(hard rad corr)
     */
    void setFRAPHI(double fraphi = 666.66) { m_FRAPHI = fraphi; }

    /** set param. epsilon_phi ('cutoff' of the phi_ks peak)
     * @param epsphi param. epsilon_phi ('cutoff' of the phi_ks peak)
     */
    void setEPSPHI(double epsphi = 666.66) { m_EPSPHI = epsphi; }

    /** set maximum weight for generation of QP0, cos(theta QP)
     * @param wght1m maximum weight for generation of QP0, cos(theta QP)
     */
    void setWGHT1M(double wght1m = 666.66) { m_WGHT1M = wght1m; }

    /** set maximum weight for the trial events
     * @param wghtmx maximum weight for the trial events
     */
    void setWGHTMX(double wghtmx = 666.66) { m_WGHTMX = wghtmx; }

    /** specifies radiative correction (NONE SOFT or HARD)
    * @param radcor radiative correction (NONE SOFT or HARD)
    */
    void setRADCOR(std::string radcor = "NONE") {m_sRADCOR = radcor;}

    /** specifies the event configuration (EGAMMA GAMMA or ETRON)
    * @param config event configuration (EGAMMA GAMMA or ETRON)
    */
    void setCONFIG(std::string config = "NONE") {m_sCONFIG = config;}

    /** specifies which eeg matrix element (BK BKM2 TCHAN or EPA)
    * @param matrix eeg matrix element (BK BKM2 TCHAN or EPA)
    */
    void setMATRIX(std::string matrix = "NONE") {m_sMATRIX = matrix;}

    /** specifies which eegg matrix element (EPADC BEEGG or MEEGG)
    * @param mtrxgg eegg matrix element (EPADC BEEGG or MEEGG)
    */
    void setMTRXGG(std::string mtrxgg = "NONE") {m_sMTRXGG = mtrxgg;}

    /** set logical variable. If true then generate unweighted events
      * @param wghtmx logical variable. If true then generate unweighted events
      */
    void setUNWGHT(int unwght = 1) { m_UNWGHT = unwght; }



    /** Sets the CMS energy.
     * @param cmsEnergy The CMS energy in [GeV].
     */
    void setCMSEnergy(double cmsEnergy) { m_cmsEnergy = cmsEnergy; }


    void init();

    /** Generates one single event.
     * @param mcGraph Reference to the MonteCarlo graph into which the generated particles will be stored.
     */
    void generateEvent(MCParticleGraph& mcGraph);

//     /** Returns weight
//      * @param SDif Returns weight
//      */
    double getT() {return m_t;};
    double getW2() {return m_w2;};
    double getWeight() {return m_weight;};
    double getReANSK() {return m_reansk;};
    double getReANSK2() {return m_reansk2;};
    double getFullANSK() {return m_fullansk;};
    double getFullANSK2() {return m_fullansk2;};

    /**
     * Terminates the generator.
     * Closes the internal Fortran generator.
     */
    void term();

  protected:
    bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
    double m_pi;                    /**< pi=3.1415.... */
    double m_conversionFactor;      /**< Conversion factor for hbarc to nb. */
    double m_alphaQED0;             /**< QED coupling constant at Q=0. */
    double m_massElectron;          /**< muon mass. */

    std::string m_vacPol;           /**< vacuum polarization: off, hadr5 (Jegerlehner) or hmnt (Teubner). */
    double m_cmsEnergy;             /**< CMS Energy = 2*Ebeam [GeV]. */

    double m_TEVETO; /**< maximum theta of e+ in final state (in radians)*/
    double m_TEMIN;  /**< minimum angle between the e- and -z axis (egamma conf. only)*/
    double m_TGMIN;  /**< minimum angle between the gamma and -z axis*/
    double m_TGVETO; /**< maximum angle between the gamma and -z axis(etron conf. only)*/
    double m_EEMIN;  /**< minimum energy of the e- (egamma & etron configurations)*/
    double m_EGMIN;  /**< minimum energy of the gamma (egamma & gamma configurations)*/
    double m_PEGMIN; /**< minimum phi sep of e-gamma (egamma config with hard rad corr)*/
    double m_EEVETO; /**< minimum energy to veto(gamma  config with hard rad corr)*/
    double m_EGVETO; /**< minimum energy to veto(etron/gamma config with hard rad corr)*/
    double m_PHVETO; /**< minimum phi sep to veto(etron/gamma config with hard rad corr*/
    double m_CUTOFF; /**< cutoff energy for radiative corrections (in CM frame)*/
    double m_EPS; /**< param. epsilon_s (smaller val. increases sampling of k_s^pbc)*/
    double m_FRAPHI; /**< fraction of time phi_ks is generated with peak(hard rad corr)*/
    double m_EPSPHI; /**< param. epsilon_phi ('cutoff' of the phi_ks peak)*/
    double m_WGHT1M; /**< maximum weight for generation of QP0, cos(theta QP)*/
    double m_WGHTMX; /**< maximum weight for the trial events*/

    int m_RADCOR; /**< specifies radiative correction (NONE SOFT or HARD)*/
    int m_CONFIG; /**< specifies the event configuration (EGAMMA GAMMA or ETRON)*/
    int m_MATRIX; /**< specifies which eeg matrix element (BK BKM2 TCHAN or EPA)*/
    int m_MTRXGG; /**< specifies which eegg matrix element (EPADC BEEGG or MEEGG)*/
    std::string m_sRADCOR; /**< specifies radiative correction (NONE SOFT or HARD)*/
    std::string m_sCONFIG; /**< specifies the event configuration (EGAMMA GAMMA or ETRON)*/
    std::string m_sMATRIX; /**< specifies which eeg matrix element (BK BKM2 TCHAN or EPA)*/
    std::string m_sMTRXGG; /**< specifies which eegg matrix element (EPADC BEEGG or MEEGG)*/
    int m_UNWGHT; /**< logical variable. If true then generate unweighted events.*/

    double m_t;  /**< T=-Q2 */
    double m_w2;  /**< W2 */
    double m_weight;  /** weight per event */
    double m_reansk;  /** vacuum polarization contribution from NSK routine, real part only */
    double m_reansk2;  /** vacuum polarization contribution from NSK routine squared, real part only */
    double m_fullansk;  /** vacuum polarization contribution from NSK routine */
    double m_fullansk2;  /** vacuum polarization contribution from NSK routine squared */

//     bool m_applyBoost;              /**< Apply a boost to the MCParticles. */
    TLorentzRotation m_boostVector; /**< The Lorentz boost vector for the transformation CMS to LAB frame. */

    /** Apply the settings to the internal Fortran generator. */
    void applySettings();

    /** Store a single generated particle into the MonteCarlo graph.
     * @param mcGraph Reference to the MonteCarlo graph into which the particle should be stored.
     * @param mom The 3-momentum of the particle in [GeV].
     * @param pdg The PDG code of the particle.
     * @param isVirtual If the particle is a virtual particle, such as the incoming particles, set this to true.
     * @param isInitial If the particle is a initial particle for ISR, set this to true.
     */
    void storeParticle(MCParticleGraph& mcGraph, const double* mom, int pdg, bool isVirtual = false, bool isInitial = false);

  private:

    int    m_npar[100];  /**< Integer parameters for Teegg. */
    double m_xpar[100];  /**< Double parameters for Teegg. */
  };
}


#endif /* TEEG_H */
