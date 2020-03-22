/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/dataobjects/eklm/EKLMSimHit.h>
#include <klm/dataobjects/bklm/BKLMElementNumbers.h>
#include <klm/dataobjects/bklm/BKLMSimHit.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMScintillatorFirmwareFitResult.h>

/* Belle 2 headers. */
#include <framework/dataobjects/DigitBase.h>

namespace Belle2 {

  /**
   * KLM digit (class representing a digitized hit in RPCs or scintillators).
   */
  class KLMDigit : public DigitBase {

  public:

    /**
     * Constructor.
     */
    KLMDigit();

    /**
     * Constructor for EKLM simulation.
     * @param[in] simHit EKLM simulation hit.
     */
    KLMDigit(const EKLMSimHit* simHit);

    /**
     * Constructor for BKLM RPC simulation.
     * @param[in] simHit BKLM simulation hit.
     * @param[in] strip  Strip.
     */
    KLMDigit(const BKLMSimHit* simHit, int strip);

    /**
     * Constructor for BKLM RPC simulation.
     * @param[in] simHit BKLM simulation hit.
     */
    KLMDigit(const BKLMSimHit* simHit);

    /**
     * Destructor.
     */
    virtual ~KLMDigit()
    {
    }

    /**
     * Get unique channel identifier.
     */
    unsigned int getUniqueChannelID() const override;

    /**
     * The pile-up method.
     */
    EAppendStatus addBGDigit(const DigitBase* bg) override;

    /**
     * Get subdetector number.
     * @return Subdetector number.
     */
    int getSubdetector() const
    {
      return m_Subdetector;
    }

    /**
     * Set subdetector number.
     * @param[in] subdetector Subdetector number.
     */
    void setSubdetector(int subdetector)
    {
      m_Subdetector = subdetector;
    }

    /**
     * Get section number.
     * @return Section number.
     */
    int getSection() const
    {
      return m_Section;
    }

    /**
     * Set section number.
     * @param[in] section Section number.
     */
    void setSection(int section)
    {
      m_Section = section;
    }

    /**
     * Get sector number.
     * @return Sector number.
     */
    int getSector() const
    {
      return m_Sector;
    }

    /**
     * Set sector number.
     * @param[in] sector Sector number.
     */
    void setSector(int sector)
    {
      m_Sector = sector;
    }

    /**
     * Get layer number.
     * @return Layer number.
     */
    int getLayer() const
    {
      return m_Layer;
    }

    /**
     * Set layer number.
     * @param[in] layer Layer number.
     */
    void setLayer(int layer)
    {
      m_Layer = layer;
    }

    /**
     * Get plane number.
     * @return Plane number.
     */
    int getPlane() const
    {
      return m_Plane;
    }

    /**
     * Set plane number.
     * @param[in] plane Plane number.
     */
    void setPlane(int plane)
    {
      m_Plane = plane;
    }

    /**
     * Get strip number.
     * @return Strip number.
     */
    int getStrip() const
    {
      return m_Strip;
    }

    /**
     * Set strip number.
     * @param[in] strip Strip number.
     */
    void setStrip(int strip)
    {
      m_Strip = strip;
    }

    /**
     * Determine whether the hit is in RPC or scintillator.
     * @return Whether hit is in RPC (true) or scintillator (false).
     */
    bool inRPC() const
    {
      return (m_Subdetector == KLMElementNumbers::c_BKLM &&
              m_Layer >= BKLMElementNumbers::c_FirstRPCLayer);
    }

    /**
     * Whether BKLM digit is in phi or z plane.
     * @return True if the digit is in phi plane, false otherwise.
     */
    bool isPhiReadout() const;

    /**
     * Get charge.
     * @return Charge.
     */
    uint16_t getCharge() const
    {
      return m_Charge;
    }

    /**
     * Set charge.
     * @param[in] charge Charge.
     */
    void setCharge(uint16_t charge)
    {
      m_Charge = charge;
    }

    /**
     * Get CTIME.
     * @return CTIME.
     */
    uint16_t getCTime() const
    {
      return m_CTime;
    }

    /**
     * Set CTIME.
     * @param[in] ctime CTime
     */
    void setCTime(uint16_t ctime)
    {
      m_CTime = ctime;
    }

    /**
     * Get TDC.
     * @return TDC.
     */
    uint16_t getTDC() const
    {
      return m_TDC;
    }

    /**
     * Set TDC.
     * @param[in] tdc TDC.
     */
    void setTDC(uint16_t tdc)
    {
      m_TDC = tdc;
    }

    /**
      * Get hit time.
      * @return Hit time.
      */
    float getTime() const
    {
      return m_Time;
    }

    /**
     * Set hit time.
     * @param[in] time hit time.
     */
    void setTime(float time)
    {
      m_Time = time;
    }

    /**
     * Get energy deposit.
     * @return Energy deposit.
     */
    float getEnergyDeposit() const
    {
      return m_EnergyDeposit;
    }

    /**
     * Set EnergyDeposit.
     * @param[in] eDep Energy deposit.
     */
    void setEnergyDeposit(float eDep)
    {
      m_EnergyDeposit = eDep;
    }

    /**
     * Get number of photoelectrons.
     * @return Number of photoelectrons.
     */
    float getNPE() const
    {
      return m_NPE;
    }

    /**
     * Set number of photoelectrons.
     * @param[in] npe Number of photoelectrons.
     */
    void setNPE(float npe)
    {
      m_NPE = npe;
    }

    /**
     * Get generated number of photoelectrons.
     * @return Number of photoelectrons.
     */
    int getGeneratedNPE() const
    {
      return m_GeneratedNPE;
    }

    /**
     * Set generated number of photoelectrons.
     * @param[in] npe Number of photoelectrons.
     */
    void setGeneratedNPE(int npe)
    {
      m_GeneratedNPE = npe;
    }

    /**
     * Whether hit could be used late (if it passed discriminator threshold)
     * @return True if could be used.
     */
    bool isGood() const
    {
      return m_FitStatus == KLM::c_ScintillatorFirmwareSuccessfulFit;
    }

    /**
     * Get fit status.
     * @return Fit status.
     */
    int getFitStatus() const
    {
      return m_FitStatus;
    }

    /**
     * Set fit status.
     * @param[in] s Fit status.
     */
    void setFitStatus(int s)
    {
      m_FitStatus = s;
    }

    /**
     * Get MC time.
     * @return MC time.
     */
    float getMCTime() const
    {
      return m_MCTime;
    }

    /**
     * Set MC time.
     * @param[in] time MC time.
     */
    void setMCTime(float time)
    {
      m_MCTime = time;
    }

    /**
     * Get SiPM MC time.
     * @return SiPM MC yime.
     */
    float getSiPMMCTime() const
    {
      return m_SiPMMCTime;
    }

    /**
     * Set SiPM MC time.
     * @param[in] time Time.
     */
    void setSiPMMCTime(float t)
    {
      m_SiPMMCTime = t;
    }

  protected:

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers; //! ROOT streamer

    /** Number of subdetector. */
    int m_Subdetector;

    /** Number of section. */
    int m_Section;

    /** Number of sector. */
    int m_Sector;

    /** Number of layer. */
    int m_Layer;

    /** Number of plane. */
    int m_Plane;

    /** Number of strip. */
    int m_Strip;

    /** Charge (integral of ADC signal). */
    uint16_t m_Charge;

    /** CTIME (time provided by B2TT). */
    uint16_t m_CTime;

    /** TDC (time provided by ASIC). */
    uint16_t m_TDC;

    /** Time of the hit. */
    float m_Time;

    /** Energy deposition. */
    float m_EnergyDeposit;

    /** Number of photoelectrons. */
    float m_NPE;

    /** Generated number of photoelectrons (MC only). */
    int m_GeneratedNPE;

    /** Fit status. */
    int m_FitStatus;

    /** MC time. */
    float m_MCTime;

    /** MC time at SiPM. */
    float m_SiPMMCTime;

    /** Class version. */
    ClassDefOverride(Belle2::KLMDigit, 1);

  };

}
