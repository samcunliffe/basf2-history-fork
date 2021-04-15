/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2021 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Ludovico Massaccesi                                      *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#ifdef _BELLE2_EPICS
#include "cadef.h"
#endif

#include <dqm/analysis/modules/DQMHistAnalysis.h>
#include <TCanvas.h>
#include <TString.h>
#include <TPaveText.h>
#include <TH2.h>
#include <TH1.h>
#include <string>
#include <vector>

namespace Belle2 {
  /** The SVD dose-monitoring DQM analysis module.
   *
   * Takes the histograms from SVDDQMDoseModule and plots the
   * instantaneous occupancy distributions and the occupancy vs time
   * since last injection and time in beam revolution cycle.
   *
   * Sends the occupancies averaged over 5 minutes (TBD, can be
   * configured with the `epicsUpdateSeconds` parameter) to EPICS PVs
   * (the names of the PVs are like `pvPrefix + "L3" + pvSuffix` or
   * `pvPrefix + "L3:1" + pvSuffix`; the default names are like
   * `SVD:DQM:L3:OccPois:Avg`).
   *
   * Sends the occupancies averaged over the run to MiraBelle (via the
   * `svd` MonitoringObject).
   *
   * @sa https://agira.desy.de/browse/BII-7853
   */
  class DQMHistAnalysisSVDDoseModule : public DQMHistAnalysisModule {
  public:
    DQMHistAnalysisSVDDoseModule();
    virtual ~DQMHistAnalysisSVDDoseModule();

  private:
    /** A struct to define the sensors group we average over.
     * See Belle2::SVD::SVDDQMDoseModule::SensorGroup.
     */
    typedef struct SensorGroup {
      TString nameSuffix; ///< Suffix of the name of the histograms
      TString titleSuffix; ///< Suffix for the title of the canvases
      const char* pvMiddle; ///< Middle part of the PV name. See also m_pvPrefix and m_pvSuffix.
      int nStrips; ///< Total number of strips in the sensor group.
    } SensorGroup;

#ifdef _BELLE2_EPICS
    /// A struct to keep EPICS PV-related data.
    typedef struct MyPV {
      chid mychid; ///< Channel id
      double lastNHits = 0.0; ///< Hit count at last report. Needed for the delta.
      double lastNEvts = 0.0; ///< Events count at last report. Needed for the delta.
    } MyPV;
#endif

    void initialize() override final;
    void event() override final;
    void endRun() override final;

    void updateCanvases();

    /// Utility method
    template<typename T>
    inline T* findHistT(TString name) { return dynamic_cast<T*>(findHist(name.Data())); }

    /** Divide two histograms, ignoring errors on the second histogram.
     * The result is stored in the numerator histogram.
     * @param num The numerator histogram. Will be overwritten with the result.
     * @param den The denominator histogram. Must have the same bins as num.
     * @param scale Optional rescaling factor.
     *
     * In short, for each bin we have
     *  - `result_bin = scale * num_bin / den_bin`
     *  - `result_err = scale * num_err / den_bin`
     */
    static void divide(TH2F* num, TH2F* den, float scale = 1.0f);

    /// Carries the content of the overflow bin into the last bin
    static void carryOverflowOver(TH1F* h);

    // Steerable data members (parameters)
    std::string m_pvPrefix; ///< Prefix for EPICS PVs
    bool m_useEpics; ///< Whether to update EPICS PVs
    double m_epicsUpdateSeconds; ///< Minimum interval between successive PV updates
    std::string m_pvSuffix; ///< Suffix for EPICS PVs

    // Data members for outputs
    MonitoringObject* m_monObj = nullptr; ///< Monitoring object for MiraBelle
    std::vector<TCanvas*> m_c_instOccu; ///< Canvases for the instantaneous occupancy
    std::vector<TCanvas*> m_c_occuLER; ///< Canvases for the occu. vs time after LER inj.
    std::vector<TCanvas*> m_c_occuHER; ///< Canvases for the occu. vs time after HER inj.
    TPaveText* m_legend = nullptr; ///< Legend of the inst. occu. plots

#ifdef _BELLE2_EPICS
    std::vector<MyPV> m_myPVs; ///< EPICS stuff for each sensor group / PV
    double m_lastPVUpdate; ///< Time of the last PV update (seconds)
#endif

    /** List of sensors groups. Must match Belle2::SVD::SVDDQMDoseModule::c_sensorGroups.
     * Defined in DQMHistAnalysisSVDDose.cc.
     */
    static const std::vector<SensorGroup> c_sensorGroups;
  };
}