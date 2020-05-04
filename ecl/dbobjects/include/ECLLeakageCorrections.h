/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * DB object to store leakage corrections, including dependence on nCrys     *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Christopher Hearty (hearty@physics.ubc.ca)               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once
#include <TObject.h>
#include <framework/logging/Logger.h>
#include <TH2F.h>

namespace Belle2 {

  /** DB object to store leakage corrections, including nCrys dependence  */
  class ECLLeakageCorrections: public TObject {


//---------------------------------------------------
  public:

    /** Constructor. */
    ECLLeakageCorrections() :
      m_logEnergiesFwd(),
      m_logEnergiesBrl(),
      m_logEnergiesBwd(),
      m_thetaBoundaries(),
      m_thetaCorrections(),
      m_phiCorrections(),
      m_nCrystalCorrections()
    { };

    //---------------------------------------------------
    //..Getters for the various calibration components

    /** Get vector of log(energies) used to evaluate the leakage corrections in the forward endcap*/
    const std::vector<float>& getlogEnergiesFwd() const {return m_logEnergiesFwd;}

    /** Get vector of log(energies) used in the barrel. Same number of test energies as forward, but different values*/
    const std::vector<float>& getlogEnergiesBrl() const {return m_logEnergiesBrl;}

    /** Get vector of log(energies) used in the backward endcap. Same number of test energies as forward, but different values*/
    const std::vector<float>& getlogEnergiesBwd() const {return m_logEnergiesBwd;}

    /** Get vector containing theta (deg) separating theta rings */
    const std::vector<float>& getThetaBoundaries() const {return m_thetaBoundaries;}

    /** Get 2D histogram of theta-dependent constants  (one column per thetaID & energy) */
    const TH2F getThetaCorrections() const {return m_thetaCorrections;}

    /** Get 2D histogram of phi-dependent constants  (one column per thetaID & energy in barrel, 2 in endcaps) */
    const TH2F getPhiCorrections() const {return m_phiCorrections;}

    /** Get 2D histogram of nCrys dependent constants (one column per thetaID & energy) */
    const TH2F getnCrystalCorrections() const {return m_nCrystalCorrections;}


    //---------------------------------------------------
    //..Setters for the various calibration components

    /** Set the vector of energies used to evaluate the leakage corrections in the forward endcap*/
    void setlogEnergiesFwd(const std::vector<float>& logEnergiesFwd) {m_logEnergiesFwd = logEnergiesFwd;}

    /** Set the vector of energies used to evaluate the leakage corrections in the barrel*/
    void setlogEnergiesBrl(const std::vector<float>& logEnergiesBrl) {m_logEnergiesBrl = logEnergiesBrl;}

    /** Set the vector of energies used to evaluate the leakage corrections in the backward endcap*/
    void setlogEnergiesBwd(const std::vector<float>& logEnergiesBwd) {m_logEnergiesBwd = logEnergiesBwd;}

    /** Set the 70-element vector of theta lab values bounding the 69 theta rings */
    void setThetaBoundaries(const std::vector<float>& thetaBoundaries)
    {
      if (thetaBoundaries.size() != 70) {B2FATAL("ECLLeakageCorrections: wrong size vector uploaded for thetaBoundaries, " << thetaBoundaries.size() << " instead of 70");}        m_thetaBoundaries
        = thetaBoundaries;
    }

    /** Set the 2D histogram containing the theta corrections for each thetaID and energy*/
    void setThetaCorrections(const TH2F& thetaCorrections) {m_thetaCorrections = thetaCorrections;}

    /** Set the 2D histogram containing the phi corrections for each thetaID and energy*/
    void setPhiCorrections(const TH2F& phiCorrections) {m_phiCorrections = phiCorrections;}

    /** Set the 2D histogram containing the nCrys corrections for each thetaID and energy*/
    void setnCrystalCorrections(const TH2F& nCrystalCorrections) {m_nCrystalCorrections = nCrystalCorrections;}


//---------------------------------------------------
  private:
    std::vector<float> m_logEnergiesFwd; /**< log(energies) used to find constants in the forward endcap (GeV) */
    std::vector<float> m_logEnergiesBrl; /**< log(energies) used to find constants in the barrel (GeV) */
    std::vector<float> m_logEnergiesBwd; /**< log(energies) used to find constants in the backward endcap (GeV) */
    std::vector<float> m_thetaBoundaries; /**< 70 values of theta that bound the 69 theta rings (rad) */
    TH2F m_thetaCorrections; /**< 2D histogram containing theta-dependent correction for each thetaID & energy */
    TH2F m_phiCorrections; /**< 2D histogram containing phi-dependent correction for each thetaID & energy */
    TH2F m_nCrystalCorrections; /**< 2D histogram containing nCrys-dependent correction for each thetaID & energy */

    ClassDef(ECLLeakageCorrections, 1); /**< ClassDef */
  };
}
