/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <background/modules/BeamBkgHitRateMonitor/BeamBkgHitRateMonitorModule.h>
#include <background/modules/BeamBkgHitRateMonitor/PXDHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/SVDHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/TOPHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/ARICHHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/ECLHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/BKLMHitRateCounter.h>
#include <background/modules/BeamBkgHitRateMonitor/EKLMHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(BeamBkgHitRateMonitor)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  BeamBkgHitRateMonitorModule::BeamBkgHitRateMonitorModule() : Module()

  {
    // set module description
    setDescription("A module for off-line monitoring of beam background hit rates.");

    /* No multiprocessing allowed!
     * setPropertyFlags(c_ParallelProcessingCertified);
     */

    // Add parameters
    addParam("outputFileName", m_outputFileName, "output file name",
             string("beamBkgHitRates.root"));
    addParam("treeName", m_treeName, "output tree name",
             string("tree"));
    m_trgTypes.push_back(TRGSummary::TTYP_DPHY);
    m_trgTypes.push_back(TRGSummary::TTYP_RAND);
    addParam("trgTypes", m_trgTypes,
             "trigger types for event selection (see TRGSummary.h for definitions). "
             "Empty list means all trigger types.",
             m_trgTypes);
    addParam("writeEmptyTimeStamps", m_writeEmptyTimeStamps,
             "if true, write to ntuple also empty time stamps", false);
    addParam("topTimeOffset", m_topTimeOffset,
             "TOP: time offset of hits (to be subtracted) [ns]", 350.0);
    addParam("topTimeWindow", m_topTimeWindow,
             "TOP: time window in which to count hits [ns]", 100.0);

  }

  BeamBkgHitRateMonitorModule::~BeamBkgHitRateMonitorModule()
  {
    for (auto& monitor : m_monitors) {
      if (monitor) delete monitor;
    }
  }

  void BeamBkgHitRateMonitorModule::initialize()
  {
    // collections registration
    m_eventMetaData.isRequired();
    if (m_trgTypes.empty()) {
      m_trgSummary.isOptional(); // enables to run the module when TRGSummary is absent
    } else {
      m_trgSummary.isRequired();
    }

    // create, set and append hit rate monitoring classes
    auto* pxd = new Background::PXDHitRateCounter();
    m_monitors.push_back(pxd);
    auto* svd = new Background::SVDHitRateCounter();
    m_monitors.push_back(svd);
    auto* cdc = new Background::CDCHitRateCounter();
    m_monitors.push_back(cdc);
    auto* top = new Background::TOPHitRateCounter(m_topTimeOffset, m_topTimeWindow);
    m_monitors.push_back(top);
    auto* arich = new Background::ARICHHitRateCounter();
    m_monitors.push_back(arich);
    auto* ecl = new Background::ECLHitRateCounter();
    m_monitors.push_back(ecl);
    auto* bklm = new Background::BKLMHitRateCounter();
    m_monitors.push_back(bklm);
    auto* eklm = new Background::EKLMHitRateCounter();
    m_monitors.push_back(eklm);

    // open output root file
    m_file = TFile::Open(m_outputFileName.c_str(), "RECREATE");
    if (not m_file) {
      B2FATAL("Cannot open output file '" << m_outputFileName << "' for writing");
    }

    // create tree
    m_tree = new TTree(m_treeName.c_str(), "hit rates of selected events");

    // set tree branches
    m_tree->Branch("run", &m_run, "run/I");
    m_tree->Branch("numEvents", &m_numEvents, "numEvents/I");
    m_tree->Branch("timeStamp", &m_timeStamp, "timeStamp/i");
    m_tree->Branch("time", &m_time, "time/I");
    for (auto& monitor : m_monitors) {
      monitor->initialize(m_tree);
    }

  }

  void BeamBkgHitRateMonitorModule::beginRun()
  {
    // clear buffers
    for (auto& monitor : m_monitors) {
      monitor->clear();
    }
    m_eventCounts.clear();

    // clear counters
    m_numEventsSelected = 0;
    m_trgTypesCount.clear();

    // set run number
    m_run = m_eventMetaData->getRun();

    // set unix time of the first event in the run
    unsigned utime = m_eventMetaData->getTime() / 1000000000;
    m_utimeFirst = utime;
    m_utimeMin = utime;
    m_utimeMax = utime + 1;

  }

  void BeamBkgHitRateMonitorModule::event()
  {
    // get unix time of the event
    unsigned utime = m_eventMetaData->getTime() / 1000000000;
    m_utimeMin = std::min(m_utimeMin, utime);
    m_utimeMax = std::max(m_utimeMax, utime + 1);

    // event selection
    if (not isEventSelected()) return;
    m_numEventsSelected++;

    // accumulate
    for (auto& monitor : m_monitors) {
      monitor->accumulate(utime);
    }
    m_eventCounts[utime] += 1;

  }

  void BeamBkgHitRateMonitorModule::endRun()
  {
    // fill ntuple
    for (unsigned utime = m_utimeMin; utime < m_utimeMax; utime++) {
      if (not m_writeEmptyTimeStamps) {
        if (m_eventCounts.find(utime) == m_eventCounts.end()) continue;
      }
      m_numEvents = m_eventCounts[utime];
      m_timeStamp = utime;
      m_time = utime - m_utimeMin;
      for (auto& monitor : m_monitors) {
        monitor->normalize(utime);
      }
      m_tree->Fill();
    }

    // print a summary for this run
    std::string trigs;
    for (const auto& trgType : m_trgTypesCount) {
      trigs += "        trigger type " + std::to_string(trgType.first) + ": " +
               std::to_string(trgType.second) + " events\n";
    }
    B2INFO("Run " << m_run << ": " << m_numEventsSelected
           << " events selected for beam background hit rate monitoring.\n"
           << trigs
           << LogVar("first event utime ", m_utimeMin)
           << LogVar("start utime       ", m_utimeMin)
           << LogVar("stop utime        ", m_utimeMax)
           << LogVar("duration [seconds]", m_utimeMax - m_utimeMin)
          );
  }

  void BeamBkgHitRateMonitorModule::terminate()
  {

    // write to file and close
    m_file->cd();
    m_file->Write();
    m_file->Close();

    B2INFO("Output file: " << m_outputFileName);
  }

  bool BeamBkgHitRateMonitorModule::isEventSelected()
  {
    auto trgType = TRGSummary::TTYP_NONE;
    if (m_trgSummary.isValid()) trgType = m_trgSummary->getTimType();

    if (m_trgTypes.empty()) {
      m_trgTypesCount[trgType] += 1;
      return true;
    }
    for (auto type : m_trgTypes) {
      if (trgType == type) {
        m_trgTypesCount[trgType] += 1;
        return true;
      }
    }
    return false;
  }



} // end Belle2 namespace

