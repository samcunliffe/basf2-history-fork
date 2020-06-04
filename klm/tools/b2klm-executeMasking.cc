/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2020  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giacomo De Pietro                                        *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* KLM headers. */
#include <klm/dataobjects/KLMChannelArrayIndex.h>
#include <klm/dataobjects/KLMChannelIndex.h>
#include <klm/dataobjects/KLMElementNumbers.h>
#include <klm/dataobjects/KLMSectorArrayIndex.h>

/* Belle 2 headers. */
#include <framework/logging/Logger.h>

/* ROOT headers. */
#include <TFile.h>
#include <TH1.h>
#include <TSystem.h>

/* C++ headers. */
#include <cstdlib>
#include <string>

using namespace Belle2;

int main(int argc, char* argv[])
{
  int nChannels = argc - 2;
  if (nChannels == 0) {
    B2ERROR("There are no channels to mask!");
    return 0;
  }
  std::string inputFileName(argv[1]);
  if (inputFileName.find(".root") == std::string::npos) {
    B2ERROR("The input file is not a .root file!");
    return 0;
  }
  if (gSystem->AccessPathName(inputFileName.c_str())) {
    B2ERROR("The input file does not exist!");
    return 0;
  }
  TFile* inputFile = new TFile(inputFileName.c_str(), "UPDATE");
  if (!inputFile or inputFile->IsZombie()) {
    B2ERROR("The input file is not working!");
    return 0;
  }
  inputFile->cd();
  const KLMElementNumbers* elementNumbers = &(KLMElementNumbers::Instance());
  const KLMChannelArrayIndex* channelArrayIndex = &(KLMChannelArrayIndex::Instance());
  const KLMSectorArrayIndex* sectorArrayIndex = &(KLMSectorArrayIndex::Instance());
  TH1* histoSummary = (TH1*)inputFile->Get("KLM/masked_channels");
  if (!histoSummary) {
    B2ERROR("The histogram KLM/masked_channels is not found!");
    return 0;
  }
  for (int i = 2; i <= nChannels + 1; ++i) {
    uint16_t channelNumber = std::atoi(argv[i]);
    int subdetector, section, sector, layer, plane, strip;
    elementNumbers->channelNumberToElementNumbers(
      channelNumber, &subdetector, &section, &sector, &layer, &plane, &strip);
    /* First: mask the channel in occupancy plot. */
    uint16_t channelIndex = channelArrayIndex->getIndex(channelNumber);
    int nHistoOccupancy;
    if (subdetector == KLMElementNumbers::c_BKLM)
      nHistoOccupancy = 2;
    else
      nHistoOccupancy = 3;
    for (int j = 0; j < nHistoOccupancy; ++j) {
      std::string histoOccupancyName = "KLM/strip_hits_subdetector_" + std::to_string(subdetector) +
                                       "_section_" + std::to_string(section) +
                                       "_sector_" + std::to_string(sector) +
                                       "_" + std::to_string(j);
      TH1* histoOccupancy = (TH1*)inputFile->Get(histoOccupancyName.c_str());
      if (!histoOccupancy) {
        B2ERROR("The histogram " << histoOccupancyName << " is not found!");
        return 0;
      }
      TAxis* xAxis = histoOccupancy->GetXaxis();
      double xMin = xAxis->GetXmin();
      double xMax = xAxis->GetXmax();
      if ((channelIndex >= xMin) and (channelIndex < xMax)) {
        int bin = xAxis->FindBin(channelIndex);
        histoOccupancy->SetBinContent(bin, 0);
        inputFile->Write("", TObject::kOverwrite);
      }
    }
    /* Second: add the masked channel to the summary plot. */
    uint16_t sectorNumber;
    if (subdetector == KLMElementNumbers::c_BKLM)
      sectorNumber = elementNumbers->sectorNumberBKLM(section, sector);
    else
      sectorNumber = elementNumbers->sectorNumberEKLM(section, sector);
    uint16_t sectorIndex = sectorArrayIndex->getIndex(sectorNumber);
    histoSummary->Fill(sectorIndex);
  }
  inputFile->Write("", TObject::kOverwrite);
  inputFile->Close();
  delete inputFile;
  return 0;
}
