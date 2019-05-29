/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* C++ headers. */
#include <cstdint>

namespace Belle2 {

  /**
   * BKLM element numbers.
   */
  class BKLMElementNumbers {

  public:

    /**
     * Constructor.
     */
    BKLMElementNumbers();

    /**
     * Destructor.
     */
    ~BKLMElementNumbers();

    /**
     * Get channel number.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    static uint16_t channelNumber(int forward, int sector, int layer, int plane,
                                  int strip);

    /**
     * Get module number.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     */
    static uint16_t moduleNumber(int forward, int sector, int layer);

    /**
     * Get layer global number.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     */
    static int layerGlobalNumber(int forward, int sector, int layer);

    /**
     * Get number of strips.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     */
    static int getNStrips(int forward, int sector, int layer, int plane);

    /**
     * Check channel number.
     * @param[in] forward Forward (1) or backward (0) BKLM.
     * @param[in] sector  Sector (1-based).
     * @param[in] layer   Layer (1-based).
     * @param[in] plane   Plane (0-based).
     * @param[in] strip   Strip (1-based).
     */
    static bool checkChannelNumber(
      int forward, int sector, int layer, int plane, int strip);

    /**
     * Get maximal forward number (0-based).
     */
    static constexpr int getMaximalForwardNumber()
    {
      return m_MaximalForwardNumber;
    }

    /**
     * Get maximal sector number (1-based).
     */
    static constexpr int getMaximalSectorNumber()
    {
      return m_MaximalSectorNumber;
    }

    /**
     * Get maximal layer number (1-based).
     */
    static constexpr int getMaximalLayerNumber()
    {
      return m_MaximalLayerNumber;
    }

    /**
     * Get maximal plane number (0-based).
     */
    static constexpr int getMaximalPlaneNumber()
    {
      return m_MaximalPlaneNumber;
    }

    /**
     * Get maximal sector global number.
     */
    static constexpr int getMaximalSectorGlobalNumber()
    {
      return (m_MaximalForwardNumber + 1) * m_MaximalSectorNumber;
    }

    /**
     * Get maximal layer global number.
     */
    static constexpr int getMaximalLayerGlobalNumber()
    {
      return (m_MaximalForwardNumber + 1) * m_MaximalSectorNumber * m_MaximalLayerNumber;
    }

    /**
     * Get element numbers by layer global number.
     * @param[in]  layerGlobal  Layer global number.
     * @param[out] forward      Forward (1) or backward (0) BKLM.
     * @param[out] sector       Sector (1-based).
     * @param[out] layer        Layer (1-based).
     */
    static void layerGlobalNumberToElementNumbers(int layerGlobal, int* forward, int* sector, int* layer);

  protected:

    /** Maximal forward number (0-based). */
    static constexpr int m_MaximalForwardNumber = 1;

    /** Maximal sector number (1-based). */
    static constexpr int m_MaximalSectorNumber = 8;

    /** Maximal layer number (1-based). */
    static constexpr int m_MaximalLayerNumber = 15;

    /** Maximal plane number (0-based). */
    static constexpr int m_MaximalPlaneNumber = 1;

  };

}
