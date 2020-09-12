/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * This module performs the correction for EM shower (mainly longitudinal *
 * leakage): corr = (Reconstructed / Truth).                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Torben Ferber (torben.ferber@desy.de) (TF)               *
 *               Alon Hershenhorn (hersehn@physics.ubc.ca)                *
 *               Suman Koirala (suman@ntu.edu.tw)                         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

// STL
#include <vector>

//ROOT
#include <TGraph2D.h>

// FRAMEWORK
#include <framework/core/Module.h>
#include <framework/database/DBObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

namespace Belle2 {
  class EventLevelClusteringInfo;
  class ECLShower;
  class ECLShowerCorrectorLeakageCorrection;
  class ECLShowerEnergyCorrectionTemporary;

  /** Class to perform the shower correction */
  class ECLShowerCorrectorModule : public Module {

  public:
    /** Constructor. */
    ECLShowerCorrectorModule();

    /** Destructor. */
    ~ECLShowerCorrectorModule();

    /** Initialize. */
    virtual void initialize() override;

    /** Begin run. */
    virtual void beginRun() override;

    /** Event. */
    virtual void event() override;

    /** End run. */
    virtual void endRun() override;

    /** Terminate. */
    virtual void terminate() override;

    /** Prepare correction */
    void prepareLeakageCorrections();

    /** Get correction for BGx0*/
    double getLeakageCorrection(const double theta, const double phi, const double energy, const double background) const;

    /** Get correction for BGx1 (temporary)*/
    double getLeakageCorrectionTemporary(const double theta, const double phi, const double energy, const double background);

  private:
    DBObjPtr<ECLShowerCorrectorLeakageCorrection> m_leakageCorrectionPtr_bgx0;  /**< Leakage corrections from DB for BGx0 */
    // Elisa's and Claudia's corrections
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_thetaGeo_phase2bgx1;  /**< Leakage corrections from DB for Phase2 BG15x1.0, geometry correction as a function of theta*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_thetaGeo_phase3bgx1;  /**< Leakage corrections from DB for Phase3 BG15x1.0, geometry correction as a function of theta*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phiGeo_phase2bgx1;  /**< Leakage corrections from DB for Phase2 BG15x1.0, geometry correction as a function of phi*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phiGeo_phase3bgx1;  /**< Leakage corrections from DB for Phase3 BG15x1.0, geometry correction as a function of phi*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_thetaEn_phase2bgx1;  /**< Leakage corrections from DB for Phase2 BG15x1.0, energy correction as a function of theta*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_thetaEn_phase3bgx1;  /**< Leakage corrections from DB for Phase3 BG15x1.0, energy correction as a function of theta*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phiEn_phase2bgx1;  /**< Leakage corrections from DB for Phase2 BG15x1.0, energy correction as a function of phi*/
    DBObjPtr<ECLShowerEnergyCorrectionTemporary>
    m_leakageCorrectionPtr_phiEn_phase3bgx1;  /**< Leakage corrections from DB for Phase3 BG15x1.0, energy correction as a function of phi*/

    // TO BE UPDATED
    const double m_fullBkgdCount = 183.0; /**< Nominal Background at BGx1.0 (MC12) */

    // For temporary BGx1
    /** the leakage in BGx1 */
    TGraph2D m_leakage_bgx1[8]; //0: theta_geo ph2, 1: theta_geo ph3, 2: phi_geo ph2, 3: phi_geo ph3, 4: theta_en ph2, 5: theta_en ph3, 6: phi_en ph2, 7 : phi_en ph3,
    /** limits for the leakage in BGx1 */
    std::vector<double> m_leakage_bgx1_limits[8];

    // For BGx0 (complicated theta and phi dependency)
    // Vectors with one entry each:
    int m_numOfBfBins = 0; /**< number of background fraction bins; currently only two */
    int m_numOfEnergyBins = 0; /**< number of energy bins */
    int m_numOfPhiBins = 0; /**< number of phi bins */
    int m_numOfReg1ThetaBins = 0; /**< number of region 1 theta bins */
    int m_numOfReg2ThetaBins = 0; /**< number of region 2 theta bins */
    int m_numOfReg3ThetaBins = 0; /**< number of region 3 theta bins */
    int m_phiPeriodicity = 0; /**< repeating pattern in phi direction, for barrel it is 72 */
    float m_lReg1Theta = 0; /**< lower boundary of the region 1 theta */
    float m_hReg1Theta = 0; /**< upper boundary of the region 1 theta */
    float m_lReg2Theta = 0; /**< lower boundary of the region 2 theta */
    float m_hReg2Theta = 0; /**< upper boundary of the region 2 theta */
    float m_lReg3Theta = 0; /**< lower boundary of the region 3 theta */
    float m_hReg3Theta = 0; /**< upper boundary of the region 3 theta */

    // Vector with (right now) 15 entries
    std::vector<float> m_avgRecEn; /**< averages of the energy bins */

    // Vectors with all corrections
    std::vector<int> m_bgFractionBinNum; /**< BG fraction bin */
    std::vector<int> m_regNum; /**< region bin */
    std::vector<int> m_phiBinNum; /**< phi bin */
    std::vector<int> m_thetaBinNum; /**< theta bin*/
    std::vector<int> m_energyBinNum; /**< energu bin */
    std::vector<float> m_correctionFactor; /**< correction value*/

    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg1CorrFactorArrays; /**< region 1 corrections */
    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg2CorrFactorArrays; /**< region 2 corrections */
    std::vector < std::vector < std::vector < std::vector < float > > > > m_reg3CorrFactorArrays; /**< region 3 corrections */

    /** Store array: ECLShower. */
    StoreArray<ECLShower> m_eclShowers;

    /** Store object pointer: EventLevelClusteringInfo. */
    StoreObjPtr<EventLevelClusteringInfo> m_eventLevelClusteringInfo;

  public:
    /** We need names for the data objects to differentiate between PureCsI and default*/

    /** Default name ECLShowers */
    virtual const char* eclShowerArrayName() const
    { return "ECLShowers" ; }

    /** Name to be used for default option: EventLevelClusteringInfo.*/
    virtual const char* eventLevelClusteringInfoName() const
    { return "EventLevelClusteringInfo" ; }

  };

  /** The very same module but for PureCsI */
  class ECLShowerCorrectorPureCsIModule : public ECLShowerCorrectorModule {
  public:

    /** PureCsI name ECLShowersPureCsI */
    virtual const char* eclShowerArrayName() const override
    { return "ECLShowersPureCsI" ; }

    /** Name to be used for PureCsI option: EventLevelClusteringInfoPureCsI.*/
    virtual const char* eventLevelClusteringInfoName() const override
    { return "EventLevelClusteringInfoPureCsI" ; }

  };

} // end of Belle2 namespace
