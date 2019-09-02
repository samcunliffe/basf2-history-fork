/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2015  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* External headers. */
#include <TObject.h>

namespace Belle2 {

  /**
   * EKLM Alignment data.
   */
  class KLMAlignmentData : public TObject {

  public:

    /**
     * Constructor.
     */
    KLMAlignmentData() :
      m_DeltaU(0),
      m_DeltaV(0),
      m_DeltaW(0),
      m_DeltaAlpha(0),
      m_DeltaBeta(0),
      m_DeltaGamma(0)
    {
    }

    /**
     * Constructor.
     * @param[in] deltaU     Shift in U (EKLM: local X).
     * @param[in] deltaV     Shift in V (EKLM: local Y).
     * @param[in] deltaW     Shift in W.
     * @param[in] deltaAlpha Rotation in alpha.
     * @param[in] deltaBeta  Rotation in beta.
     * @param[in] deltaGamma Rotation in gamma (EKLM: rotation in local plane).
     */
    KLMAlignmentData(float deltaU, float deltaV, float deltaW,
                     float deltaAlpha, float deltaBeta, float deltaGamma) :
      m_DeltaU(deltaU),
      m_DeltaV(deltaV),
      m_DeltaW(deltaW),
      m_DeltaAlpha(deltaAlpha),
      m_DeltaBeta(deltaBeta),
      m_DeltaGamma(deltaGamma)
    {
    }

    /**
     * Destructor.
     */
    ~KLMAlignmentData()
    {
    }

    /**
     * Get shift in U.
     */
    float getDeltaU() const
    {
      return m_DeltaU;
    }

    /**
     * Set shift in U.
     * @param[in] deltaU Shift in U.
     */
    void setDeltaU(float deltaU)
    {
      m_DeltaU = deltaU;
    }

    /**
     * Get shift in V.
     */
    float getDeltaV() const
    {
      return m_DeltaV;
    }

    /**
     * Set shift in V.
     * @param[in] deltaV Shift in V.
     */
    void setDeltaV(float deltaV)
    {
      m_DeltaV = deltaV;
    }

    /**
     * Get shift in W.
     */
    float getDeltaW() const
    {
      return m_DeltaW;
    }

    /**
     * Set shift in W.
     * @param[in] deltaW Shift in W.
     */
    void setDeltaW(float deltaW)
    {
      m_DeltaW = deltaW;
    }

    /**
     * Get rotation in alpha.
     */
    float getDeltaAlpha() const
    {
      return m_DeltaAlpha;
    }

    /**
     * Set rotation in alpha.
     * @param[in] deltaAlpha Rotation in alpha.
     */
    void setDeltaAlpha(float deltaAlpha)
    {
      m_DeltaAlpha = deltaAlpha;
    }

    /**
     * Get rotation in alpha.
     */
    float getDeltaBeta() const
    {
      return m_DeltaBeta;
    }

    /**
     * Set rotation in alpha.
     * @param[in] deltaBeta Rotation in alpha.
     */
    void setDeltaBeta(float deltaBeta)
    {
      m_DeltaBeta = deltaBeta;
    }

    /**
     * Get rotation in alpha.
     */
    float getDeltaGamma() const
    {
      return m_DeltaGamma;
    }

    /**
     * Set rotation in alpha.
     * @param[in] deltaGamma Rotation in alpha.
     */
    void setDeltaGamma(float deltaGamma)
    {
      m_DeltaGamma = deltaGamma;
    }

  private:

    /** Shift in U (EKLM: local X). */
    float m_DeltaU;

    /** Shift in V (EKLM: local Y). */
    float m_DeltaV;

    /** Shift in W. */
    float m_DeltaW;

    /** Rotation in alpha. */
    float m_DeltaAlpha;

    /** Rotation in beta. */
    float m_DeltaBeta;

    /** Rotation in gamma (EKLM: rotation in local plane). */
    float m_DeltaGamma;

    /** Class version. */
    ClassDef(Belle2::KLMAlignmentData, 1);

  };

}
