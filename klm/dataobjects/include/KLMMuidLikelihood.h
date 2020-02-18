/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Leo Piilonen                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/dataobjects/BKLMElementNumbers.h>
#include <klm/eklm/dataobjects/EKLMElementNumbers.h>

/* Belle 2 headers. */
#include <framework/datastore/RelationsObject.h>

namespace Belle2 {

  //! Store the muon-identification information for an extrapolated track
  class KLMMuidLikelihood : public RelationsObject {

  public:

    //! Empty constructor for ROOT IO (needed to make the class storable)
    KLMMuidLikelihood();

    //! Constructor with initial value
    //! @param pdgCode PDG code of the hypothesis used for this extrapolation
    explicit KLMMuidLikelihood(int pdgCode);

    //! Destructor
    virtual ~KLMMuidLikelihood() {}

    //! @return PDG code of the hypothesis used for this extrapolation
    int getPDGCode() const { return m_PDGCode; }

    //! @return muon PDF value for this extrapolation (normalized with all others)
    double getMuonPDFValue() const { return m_MuonPDFValue; }

    //! @return pion PDF value for this extrapolation (normalized with all others)
    double getPionPDFValue() const { return m_PionPDFValue; }

    //! @return kaon PDF value for this extrapolation (normalized with all others)
    double getKaonPDFValue() const { return m_KaonPDFValue; }

    //! @return proton PDF value for this extrapolation (normalized with all others)
    double getProtonPDFValue() const { return m_ProtonPDFValue; }

    //! @return deuteron PDF value for this extrapolation (normalized with all others)
    double getDeuteronPDFValue() const { return m_DeuteronPDFValue; }

    //! @return electron PDF value for this extrapolation (normalized with all others)
    double getElectronPDFValue() const { return m_ElectronPDFValue; }

    //! @return junk PDF value for this extrapolation (1 if Muon+Pion+Kaon+Proton+Electron ~ 0)
    double getJunkPDFValue() const { return m_JunkPDFValue; }

    //! @return muon log-likelihood for this extrapolation (not normalized)
    double getLogL_mu() const {return m_LogL_mu;}

    //! @return pion log-likelihood for this extrapolation (not normalized)
    double getLogL_pi() const {return m_LogL_pi;}

    //! @return kaon log-likelihood for this extrapolation (not normalized)
    double getLogL_K() const {return m_LogL_K;}

    //! @return proton log-likelihood for this extrapolation (not normalized)
    double getLogL_p() const {return m_LogL_p;}

    //! @return deuteron log-likelihood for this extrapolation (not normalized)
    double getLogL_d() const {return m_LogL_d;}

    //! @return electron log-likelihood for this extrapolation (not normalized)
    double getLogL_e() const {return m_LogL_e;}

    //! @return status word (bit pattern) for this extrapolation
    unsigned int getStatus() const { return m_Status; }

    //! @return chi-squared for this extrapolation
    double getChiSquared() const { return m_ChiSquared; }

    //! @return number of degrees of freedom in chi-squared calculation
    int getDegreesOfFreedom() const { return m_DegreesOfFreedom; }

    //! @return outcome of this extrapolation.
    //! All the possible outcome values are defined in MuidElementNumbers
    //! (see enum Outcome and calculateExtrapolationOutcome).
    unsigned int getOutcome() const { return m_Outcome; }

    //! @return if this extrapolation was in forward or backward
    bool getIsForward() const { return m_IsForward; }

    //! @return outermost BKLM layer crossed by track during extrapolation
    int getBarrelExtLayer() const { return m_BarrelExtLayer; }

    //! @return outermost EKLM layer crossed by track during extrapolation
    int getEndcapExtLayer() const { return m_EndcapExtLayer; }

    //! @return outermost BKLM layer with a matching hit
    int getBarrelHitLayer() const { return m_BarrelHitLayer; }

    //! @return outermost EKLM layer with a matching hit
    int getEndcapHitLayer() const { return m_EndcapHitLayer; }

    //! @return outermost BKLM or EKLM layer crossed by track during extrapolation
    int getExtLayer() const { return m_ExtLayer; }

    //! @return outermost BKLM or EKLM layer with a matching hit
    int getHitLayer() const { return m_HitLayer; }

    //! @return layer-crossing bit pattern during extrapolation
    unsigned int getExtLayerPattern() const { return m_ExtLayerPattern; }

    //! @return matching-hit bit pattern
    unsigned int getHitLayerPattern() const { return m_HitLayerPattern; }

    //! @return total number of matching BKLM hits
    unsigned int getTotalBarrelHits() const;

    //! @return total number of matching EKLM hits
    unsigned int getTotalEndcapHits() const;

    //! @return BKLM efficiency value
    float getExtBKLMEfficiencyValue(int index) const { return m_ExtBKLMEfficiencyValue[index]; }

    //! @return EKLM efficiency vector
    float getExtEKLMEfficiencyValue(int index) const { return m_ExtEKLMEfficiencyValue[index]; }

    //! assign muon PDF value for this extrapolation
    //! @param pdfValue muon PDF value (normalized) for this extrapolation
    void setMuonPDFValue(double pdfValue) { m_MuonPDFValue = pdfValue; }

    //! assign pion PDF value for this extrapolation
    //! @param pdfValue pion PDF value (normalized) for this extrapolation
    void setPionPDFValue(double pdfValue) { m_PionPDFValue = pdfValue; }

    //! assign kaon PDF value for this extrapolation
    //! @param pdfValue kaon PDF value (normalized) for this extrapolation
    void setKaonPDFValue(double pdfValue) { m_KaonPDFValue = pdfValue; }

    //! assign proton PDF value for this extrapolation
    //! @param pdfValue proton PDF value (normalized) for this extrapolation
    void setProtonPDFValue(double pdfValue) { m_ProtonPDFValue = pdfValue; }

    //! assign deuteron PDF value for this extrapolation
    //! @param pdfValue deuteron PDF value (normalized) for this extrapolation
    void setDeuteronPDFValue(double pdfValue) { m_DeuteronPDFValue = pdfValue; }

    //! assign electron PDF value for this extrapolation
    //! @param pdfValue electron PDF value (normalized) for this extrapolation
    void setElectronPDFValue(double pdfValue) { m_ElectronPDFValue = pdfValue; }

    //! assign junk flag for this extrapolation
    //! @param pdfValue junk flag for this extrapolation (0 if not junk, 1 if junk)
    void setJunkPDFValue(double pdfValue) { m_JunkPDFValue = pdfValue; }

    //! assign muon log-likelihood for this extrapolation
    //! @param value logarithm of muon PDF value (unnormalized) for this extrapolation
    void setLogL_mu(double value) { m_LogL_mu = value; }

    //! assign pion log-likelihood for this extrapolation
    //! @param value logarithm of pion PDF value (unnormalized) for this extrapolation
    void setLogL_pi(double value) { m_LogL_pi = value; }

    //! assign kaon log-likelihood for this extrapolation
    //! @param value logarithm of kaon PDF value (unnormalized) for this extrapolation
    void setLogL_K(double value) { m_LogL_K = value; }

    //! assign proton log-likelihood for this extrapolation
    //! @param value logarithm of proton PDF value (unnormalized) for this extrapolation
    void setLogL_p(double value) { m_LogL_p = value; }

    //! assign deuteron log-likelihood for this extrapolation
    //! @param value logarithm of deuteron PDF value (unnormalized) for this extrapolation
    void setLogL_d(double value) { m_LogL_d = value; }

    //! assign electron log-likelihood for this extrapolation
    //! @param value logarithm of electron PDF value (unnormalized) for this extrapolation
    void setLogL_e(double value) { m_LogL_e = value; }

    //! assign status word (bit pattern) for this extrapolation
    //! @param status final state of this extrapolation
    void setStatus(unsigned int status) { m_Status = status; }

    //! assign chi-squared for this extrapolation
    //! @param chiSquared total chi-squared for this extrapolation
    void setChiSquared(double chiSquared) { m_ChiSquared = chiSquared; }

    //! assign number of degrees of freedom used in chi-squared calculation
    //! @param dof number of degrees of freedom for this extrapolation
    void setDegreesOfFreedom(int dof) { m_DegreesOfFreedom = dof; }

    //! assign outcome of this extrapolation
    //! @param outcome final outcome of this extrapolation
    void setOutcome(unsigned int outcome) { m_Outcome = outcome; }

    //! assign forward or backward for this extrapolation
    //! @param isForward fordward or backward for this extrapolation
    void setIsForward(bool isForward) { m_IsForward = isForward; }

    //! assign outermost BKLM layer crossed by track during extrapolation
    //! @param layer outermost BKLM layer crossed by track during extrapolation
    void setBarrelExtLayer(int layer) { m_BarrelExtLayer = layer; }

    //! assign outermost EKLM layer crossed by track during extrapolation
    //! @param layer outermost EKLM layer crossed by track during extrapolation
    void setEndcapExtLayer(int layer) { m_EndcapExtLayer = layer; }

    //! assign outermost BKLM layer with a matching hit
    //! @param layer outermost BKLM layer with a matching hit
    void setBarrelHitLayer(int layer) { m_BarrelHitLayer = layer; }

    //! assign outermost EKLM layer with a matching hit
    //! @param layer outermost EKLM layer with a matching hit
    void setEndcapHitLayer(int layer) { m_EndcapHitLayer = layer; }

    //! assign outermost BKLM or EKLM layer crossed by track during extrapolation
    //! @param layer outermost BKLM or EKLM crossed by track during extrapolation
    void setExtLayer(int layer) { m_ExtLayer = layer; }

    //! assign Outermost BKLM or EKLM layer with a matching hit
    //! @param layer outermost BKLM or EKLM layer with a matching hit
    void setHitLayer(int layer) { m_HitLayer = layer; }

    //! assign Layer-crossing bit pattern during extrapolation
    //! @param pattern bit pattern of crossed layers during extrapolation
    void setExtLayerPattern(unsigned int pattern) { m_ExtLayerPattern = pattern; }

    //! assign Matching-hit bit pattern
    //! @param pattern bit pattern of matching-hit layers during extrapolation
    void setHitLayerPattern(unsigned int pattern) { m_HitLayerPattern = pattern; }

    //! assign BKLMefficiency value
    void setExtBKLMEfficiencyValue(int index, float efficiencyValue) { m_ExtBKLMEfficiencyValue[index] = efficiencyValue; }

    //! assign EKLM efficiency value
    void setExtEKLMEfficiencyValue(int index, float efficiencyValue) { m_ExtEKLMEfficiencyValue[index] = efficiencyValue; }

  private:

    //! PDG particleID hypothesis used for this extrapolation (typically muon)
    int m_PDGCode;

    //! Muon PDF value for this extrapolation (normalized with all others)
    float m_MuonPDFValue;

    //! Pion PDF value for this extrapolation (normalized with all others)
    float m_PionPDFValue;

    //! Kaon PDF value for this extrapolation (normalized with all others)
    float m_KaonPDFValue;

    //! Proton PDF value for this extrapolation (normalized with all others)
    float m_ProtonPDFValue;

    //! Deuteron PDF value for this extrapolation (normalized with all others)
    float m_DeuteronPDFValue;

    //! Electron PDF value for this extrapolation (normalized with all others)
    float m_ElectronPDFValue;

    //! Junk flag for this extrapolation (0 if not junk, 1 if junk)
    float m_JunkPDFValue;

    //! Muon log-likelihood for this extrapolation (not normalized)
    float m_LogL_mu;

    //! Pion log-likelihood for this extrapolation (not normalized)
    float m_LogL_pi;

    //! Kaon log-likelihood for this extrapolation (not normalized)
    float m_LogL_K;

    //! Proton log-likelihood for this extrapolation (not normalized)
    float m_LogL_p;

    //! Deuteron log-likelihood for this extrapolation (not normalized)
    float m_LogL_d;

    //! Electron log-likelihood for this extrapolation (not normalized)
    float m_LogL_e;

    //! Status word (bit pattern) for this extrapolation
    unsigned int m_Status;

    //! Chi-squared for this extrapolation
    float m_ChiSquared;

    //! number of degrees of freedom used in chi-squared calculation
    int m_DegreesOfFreedom;

    //! outcome of this extrapolation.
    unsigned int m_Outcome;

    //! is the extrapolation in forward or backward?
    bool m_IsForward;

    //! outermost BKLM layer crossed by track during extrapolation
    int m_BarrelExtLayer;

    //! outermost EKLM layer crossed by track during extrapolation
    int m_EndcapExtLayer;

    //! outermost BKLM layer with a matching hit
    int m_BarrelHitLayer;

    //! outermost EKLM layer with a matching hit
    int m_EndcapHitLayer;

    //! outermost BKLM or EKLM layer crossed by track during extrapolation
    int m_ExtLayer;

    //! outermost BKLM or EKLM layer with a matching hit
    int m_HitLayer;

    //! Layer-crossing bit pattern during extrapolation
    //! (bits 0..14 = barrel layers 1..15,
    //! bits 15..28 = endcap layers 1..14)
    unsigned int m_ExtLayerPattern;

    //! Matching-hit bit pattern
    //! (bits 0..14 = barrel layers 1..15,
    //! bits 15..28 = endcap layers 1..14)
    unsigned int m_HitLayerPattern;

    //! Vector of BKLM layer efficiencies.
    float m_ExtBKLMEfficiencyValue[BKLMElementNumbers::getMaximalLayerNumber()];

    //! Vector of EKLM layer efficiencies
    float m_ExtEKLMEfficiencyValue[EKLMElementNumbers::getMaximalLayerNumber()];

    //! Needed to make the ROOT object storable
    ClassDef(KLMMuidLikelihood, 2)

  };
}
