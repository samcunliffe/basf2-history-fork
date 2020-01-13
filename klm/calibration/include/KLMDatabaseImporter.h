/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Kirill Chilikin, Giacomo De Pietro                       *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

/* KLM headers. */
#include <klm/bklm/dbobjects/BKLMAlignment.h>
#include <klm/dbobjects/KLMChannelStatus.h>
#include <klm/dbobjects/KLMElectronicsMap.h>
#include <klm/dbobjects/KLMScintillatorDigitizationParameters.h>
#include <klm/dbobjects/KLMStripEfficiency.h>
#include <klm/dbobjects/KLMTimeConversion.h>
#include <klm/eklm/dbobjects/EKLMAlignment.h>
#include <klm/eklm/dbobjects/EKLMSegmentAlignment.h>

namespace Belle2 {

  /**
   * KLM database importer.
   */
  class KLMDatabaseImporter {

  public:

    /**
     * Constructor.
     */
    KLMDatabaseImporter();

    /**
     * Destructor.
     */
    ~KLMDatabaseImporter();

    /**
     * Set interval of validity.
     */
    void setIOV(int experimentLow, int runLow, int experimentHigh, int runHigh);

    /**
     * Import channel status.
     */
    void importChannelStatus(const KLMChannelStatus* channelStatus);

    /**
     * Import scintillator simulation parameters.
     */
    void importScintillatorDigitizationParameters(
      const KLMScintillatorDigitizationParameters* digitizationParameters);

    /**
     * Import time conversion parameters.
     * @param[in] timeConversion Time conversion parameters.
     */
    void importTimeConversion(const KLMTimeConversion* timeConversion);

    /**
     * Load strip efficiencies.
     *
     * @param[out] stripEfficiency
     * Strip efficiency.
     *
     * @param[in] fileName
     * Name of the ROOT file containing the efficiencies.
     */
    void loadStripEfficiency(KLMStripEfficiency* stripEfficiency,
                             std::string fileName);

    /**
     * Import strip efficiencies.
     * @param[in] stripEfficiency Strip efficiency.
     */
    void importStripEfficiency(const KLMStripEfficiency* stripEfficiency);

    /**
     * Import alignment.
     * @param[in] bklmAlignment        BKLM alignment.
     * @param[in] eklmAlignment        EKLM alignment.
     * @param[in] eklmSegmentAlignment EKLM segment alignment.
     * @param[in] displacement         Import displacement payload.
     */
    void importAlignment(const BKLMAlignment* bklmAlignment,
                         const EKLMAlignment* eklmAlignment,
                         const EKLMSegmentAlignment* eklmSegmentAlignment,
                         bool displacement = false);

    /**
     * Load BKLM electronics map.
     * @param[in] isExperiment10 Experiment 10 or earlier data.
     */
    void loadBKLMElectronicsMap(bool isExperiment10 = false);

    /**
     * Load EKLM electronics map.
     *
     * @param[in] version
     * Map version:
     * 1 = phase 2, experiment 3
     * 2 = phase 3, starting from experiment 4.
     *
     * @param[in] mc
     * MC or data (MC does not have occasional cable switches that exist
     * in the data).
     */
    void loadEKLMElectronicsMap(int version, bool mc);

    /**
     * Set non-default lane for all channels in a module.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector.
     * @param[in] layer       Layer.
     * @param[in] lane        Lane.
     */
    void setElectronicsMapLane(
      int subdetector, int section, int sector, int layer, int lane);

    /**
     * Set non-default lane for all channels in a plane.
     * @param[in] subdetector Subdetector.
     * @param[in] section     Section.
     * @param[in] sector      Sector.
     * @param[in] layer       Layer.
     * @param[in] plane       Plane.
     * @param[in] lane        Lane.
     */
    void setElectronicsMapLane(
      int subdetector, int section, int sector, int layer, int plane, int lane);

    /**
      * Import electronics map.
      */
    void importElectronicsMap();

  private:

    /**
     * Import BKLM alignment.
     * @param[in] bklmAlignment BKLM alignment.
     * @param[in] displacement  Import displacement payload.
     */
    void importBKLMAlignment(const BKLMAlignment* bklmAlignment,
                             bool displacement = false);

    /**
     * Import EKLM alignment.
     * @param[in] eklmAlignment EKLM alignment.
     * @param[in] displacement  Import displacement payload.
     */
    void importEKLMAlignment(const EKLMAlignment* eklmAlignment,
                             bool displacement = false);

    /**
     * Import EKLM segment alignment.
     * @param[in] eklmSegmentAlignment EKLM segment alignment.
     * @param[in] displacement         Import displacement payload.
     */
    void importEKLMSegmentAlignment(
      const EKLMSegmentAlignment* eklmSegmentAlignment,
      bool displacement = false);

    /**
     * Get EKLM firmware strip number by software strip number.
     * @param[in] stripFirmware Firmware strip number.
     */
    int getEKLMStripFirmwareBySoftware(int stripSoftware) const;

    /**
     * Get EKLM ASIC and channel numbers (both are 0-based)
     * by plane and strip numbers.
     * @param[in]  plane   Plane.
     * @param[in]  strip   Strip,
     * @param[out] asic    ASIC.
     * @param[out] channel Channel.
     */
    void getEKLMAsicChannel(int plane, int strip,
                            int* asic, int* channel) const;

    /**
     * Add EKLM electronics map lane.
     * @param[in] section Section.
     * @param[in] sector  Sector.
     * @param[in] layer   Layer.
     * @param[in] copper  Copper.
     * @param[in] slot    Slot.
     * @param[in] lane    Lane.
     */
    void addEKLMElectronicsMapLane(
      int section, int sector, int layer, int copper, int slot, int axis);

    /** Element numbers. */
    const KLMElementNumbers* m_ElementNumbers;

    /** Data for creation of the electronics map. */
    std::vector< std::pair<uint16_t, KLMElectronicsChannel> > m_ElectronicsChannels;

    /** Low experiment. */
    int m_ExperimentLow;

    /** Low run. */
    int m_RunLow;

    /** High experiment. */
    int m_ExperimentHigh;

    /** High run. */
    int m_RunHigh;

  };

}
