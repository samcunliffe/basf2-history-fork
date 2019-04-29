/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2018 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Chunhua Li, Thomas Hauth, Nils Braun                     *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
#include <hlt/softwaretrigger/modules/dqm/SoftwareTriggerHLTDQMModule.h>

#include <TDirectory.h>

#include <hlt/softwaretrigger/core/SoftwareTriggerDBHandler.h>
#include <hlt/softwaretrigger/core/FinalTriggerDecisionCalculator.h>

#include <framework/core/ModuleParam.templateDetails.h>

#include <algorithm>

using namespace Belle2;
using namespace SoftwareTrigger;

REG_MODULE(SoftwareTriggerHLTDQM)


SoftwareTriggerHLTDQMModule::SoftwareTriggerHLTDQMModule() : HistoModule()
{
  setDescription("Monitor Physics Trigger");
  setPropertyFlags(c_ParallelProcessingCertified);

  // Fill in the default values of the module parameters
  m_param_variableIdentifiers = {};

  m_param_cutResultIdentifiers["filter"] = {"total_result"};

  addParam("cutResultIdentifiers", m_param_cutResultIdentifiers,
           "Which cuts should be reported? Please remember to include the total_result also, if wanted.",
           m_param_cutResultIdentifiers);

  addParam("variableIdentifiers", m_param_variableIdentifiers,
           "Which variables should be reported?",
           m_param_variableIdentifiers);

  addParam("histogramDirectoryName", m_param_histogramDirectoryName,
           "SoftwareTrigger DQM histograms will be put into this directory", m_param_histogramDirectoryName);
}

void SoftwareTriggerHLTDQMModule::defineHisto()
{
  TDirectory* oldDirectory = nullptr;

  if (!m_param_histogramDirectoryName.empty()) {
    oldDirectory = gDirectory;
    TDirectory* histDir = oldDirectory->mkdir(m_param_histogramDirectoryName.c_str());
    histDir->cd();
  }

  for (auto const& variable : m_param_variableIdentifiers) {
    // todo: make correct range
    const unsigned int numberOfBins = 50;
    const double lowerX = 0;
    const double upperX = 50;
    m_triggerVariablesHistograms.emplace(variable, new TH1F(variable.c_str(), variable.c_str(), numberOfBins, lowerX, upperX));
    m_triggerVariablesHistograms[variable]->SetXTitle(("SoftwareTriggerVariable " + variable).c_str());
  }

  for (const auto& cutIdentifier : m_param_cutResultIdentifiers) {
    const std::string& baseIdentifier = cutIdentifier.first;
    const auto& cuts = cutIdentifier.second;

    const unsigned int numberOfBins = cuts.size();
    const double lowerX = 0;
    const double upperX = numberOfBins;
    m_cutResultHistograms.emplace(baseIdentifier,
                                  new TH1F(baseIdentifier.c_str(), baseIdentifier.c_str(), numberOfBins, lowerX, upperX));
    m_cutResultHistograms[baseIdentifier]->SetXTitle(("Prescaled Cut Result for " + baseIdentifier).c_str());
    m_cutResultHistograms[baseIdentifier]->SetOption("bar");
    m_cutResultHistograms[baseIdentifier]->SetFillStyle(0);
    m_cutResultHistograms[baseIdentifier]->SetStats(false);
  }

  // We add one for the total result
  const unsigned int numberOfBins = m_param_cutResultIdentifiers.size() + 1;
  const double lowerX = 0;
  const double upperX = numberOfBins;
  m_cutResultHistograms.emplace("total_result",
                                new TH1F("total_result", "total_result", numberOfBins, lowerX, upperX));
  m_cutResultHistograms["total_result"]->SetXTitle("Total Cut Result");
  m_cutResultHistograms["total_result"]->SetOption("bar");
  m_cutResultHistograms["total_result"]->SetFillStyle(0);
  m_cutResultHistograms["total_result"]->SetStats(false);

  if (oldDirectory) {
    oldDirectory->cd();
  }
}


void SoftwareTriggerHLTDQMModule::initialize()
{
  // Register histograms (calls back defineHisto)
  REG_HISTOGRAM
}

void SoftwareTriggerHLTDQMModule::event()
{
  // this might be pre-scaled for performance reasons in the final configuration, therefore this structure
  // might not be filled in every event
  if (m_variables.isValid()) {
    for (auto& variableNameAndTH1F : m_triggerVariablesHistograms) {
      const std::string& variable = variableNameAndTH1F.first;
      TH1F* histogram = variableNameAndTH1F.second;

      // try to load this variable from the computed trigger variables
      if (!m_variables->has(variable)) {
        B2FATAL("Variable " << variable << " configured for SoftwareTriggerDQM plotting is not available");
      } else {
        const double value = m_variables->getVariable(variable);
        histogram->Fill(value);
      }
    }
  }

  if (m_triggerResult.isValid()) {
    for (auto const& cutIdentifier : m_param_cutResultIdentifiers) {
      const std::string& baseIdentifier = cutIdentifier.first;
      const auto& cuts = cutIdentifier.second;

      for (const std::string& cutTitle : cuts) {
        const std::string& cutName = cutTitle.substr(0, cutTitle.find("\\"));
        const std::string& fullCutIdentifier = SoftwareTriggerDBHandler::makeFullCutName(baseIdentifier, cutName);

        // check if the cutResult is in the list, be graceful when not available
        auto const cutEntry = m_triggerResult->getResults().find(fullCutIdentifier);

        if (cutEntry != m_triggerResult->getResults().end()) {
          const int cutResult = cutEntry->second;
          m_cutResultHistograms[baseIdentifier]->Fill(cutTitle.c_str(), cutResult > 0);
        }
      }

      const std::string& totalCutIdentifier = SoftwareTriggerDBHandler::makeTotalResultName(baseIdentifier);
      const int cutResult = static_cast<int>(m_triggerResult->getResult(totalCutIdentifier));

      m_cutResultHistograms["total_result"]->Fill(totalCutIdentifier.c_str(), cutResult > 0);
    }

    const bool totalResult = FinalTriggerDecisionCalculator::getFinalTriggerDecision(*m_triggerResult);
    m_cutResultHistograms["total_result"]->Fill("total_result", totalResult > 0);
  }
}

void SoftwareTriggerHLTDQMModule::beginRun()
{
  std::for_each(m_cutResultHistograms.begin(), m_cutResultHistograms.end(),
  [](auto & it) {it.second->Reset();});
  std::for_each(m_triggerVariablesHistograms.begin(), m_triggerVariablesHistograms.end(),
  [](auto & it) {it.second->Reset();});
}

