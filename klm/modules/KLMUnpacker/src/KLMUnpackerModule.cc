/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Petr Katrenko                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

/* C++ headers. */
#include <cstdint>

/* Belle2 headers. */
#include <bklm/dataobjects/BKLMElementNumbers.h>
#include <bklm/dbobjects/BKLMElectronicMapping.h>
#include <klm/modules/KLMUnpacker/KLMUnpackerModule.h>
#include <klm/rawdata/RawData.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

using namespace std;
using namespace Belle2;

REG_MODULE(KLMUnpacker)

KLMUnpackerModule::KLMUnpackerModule() : Module(),
  m_triggerCTimeOfPreviousEvent(0)
{
  setDescription("KLM unpacker (creates BKLMDigits and EKLMDigits "
                 "from RawKLM).");
  setPropertyFlags(c_ParallelProcessingCertified);
  addParam("outputBKLMDigitsName", m_outputBKLMDigitsName,
           "Name of BKLMDigit store array.", string(""));
  addParam("outputEKLMDigitsName", m_outputEKLMDigitsName,
           "Name of EKLMDigit store array.", string(""));
  addParam("WriteWrongHits", m_WriteWrongHits,
           "Record wrong hits (e.g. for debugging).", false);
  addParam("IgnoreWrongHits", m_IgnoreWrongHits,
           "Ignore wrong hits (i.e. no B2ERROR).", false);
  addParam("IgnoreStrip0", m_IgnoreStrip0,
           "Ignore hits with strip = 0 (normally expected for certain firmware "
           "versions).", true);
  addParam("keepEvenPackages", m_keepEvenPackages,
           "Keep packages that have even length normally indicating that "
           "data was corrupted ", false);
  addParam("SciThreshold", m_scintThreshold,
           "Scintillator strip hits with charge lower this value will be "
           "marked as bad.", double(140.0));
  addParam("loadThresholdFromDB", m_loadThresholdFromDB,
           "Load threshold from database (true) or not (false)", true);
  m_ElementNumbers = &(EKLM::ElementNumbersSingleton::Instance());
}

KLMUnpackerModule::~KLMUnpackerModule()
{
}

void KLMUnpackerModule::initialize()
{
  /* Common. */
  m_RawKLMs.isRequired();
  /* BKLM. */
  m_bklmDigits.registerInDataStore(m_outputBKLMDigitsName);
  m_klmDigitRaws.registerInDataStore();
  m_bklmDigitOutOfRanges.registerInDataStore();
  m_DigitEventInfos.registerInDataStore();

  m_bklmDigits.registerRelationTo(m_klmDigitRaws);
  m_bklmDigitOutOfRanges.registerRelationTo(m_klmDigitRaws);
  m_DigitEventInfos.registerRelationTo(m_bklmDigits);
  m_DigitEventInfos.registerRelationTo(m_bklmDigitOutOfRanges);

  loadMapFromDB();
  /* EKLM. */
  m_eklmDigits.registerInDataStore(m_outputEKLMDigitsName);
  m_DigitEventInfos.registerInDataStore();
  m_eklmDigits.registerRelationTo(m_DigitEventInfos);
}

void KLMUnpackerModule::beginRun()
{
  if (!m_ElectronicsMap.isValid())
    B2FATAL("No EKLM electronics map.");
  if (!m_TimeConversion.isValid())
    B2FATAL("EKLM time conversion parameters are not available.");
  if (!m_Channels.isValid())
    B2FATAL("EKLM channel data are not available.");
  loadMapFromDB();
  if (m_loadThresholdFromDB)
    m_scintThreshold = m_ADCParams->getADCThreshold();
  m_triggerCTimeOfPreviousEvent = 0;
}

void KLMUnpackerModule::unpackEKLMDigit(
  const int* rawData, EKLMDataConcentratorLane* lane,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  int endcap, layer, sector, strip = 0;
  KLM::RawData raw;
  KLM::unpackRawData(rawData, &raw, nullptr, nullptr, false);
  if ((raw.triggerBits & 0x10) != 0)
    return;
  /**
   * The possible values of the strip number in the raw data are
   * from 0 to 127, while the actual range of strip numbers is from
   * 1 to 75. A check is required. The unpacker continues to work
   * with B2ERROR because otherwise debugging is not possible.
   */
  bool correctHit = m_ElementNumbers->checkStrip(raw.channel, false);
  if (!correctHit) {
    if (!(m_IgnoreWrongHits ||
          (raw.channel == 0 && m_IgnoreStrip0))) {
      B2ERROR("Incorrect strip number in raw data."
              << LogVar("Strip number", raw.channel));
    }
    if (!m_WriteWrongHits)
      return;
    strip = raw.channel;
  } else {
    strip = m_ElementNumbers->getStripSoftwareByFirmware(
              raw.channel);
  }
  uint16_t plane = raw.axis + 1;
  /*
   * The possible values of the plane number in the raw data are from
   * 1 to 2. The range is the same as in the detector geometry.
   * Consequently, a check of the plane number is useless: it is
   * always correct.
   */
  lane->setLane(raw.lane);
  const int* sectorGlobal = m_ElectronicsMap->getSectorByLane(lane);
  if (sectorGlobal == nullptr) {
    if (!m_IgnoreWrongHits) {
      B2ERROR("Lane does not exist in the EKLM electronics map."
              << LogVar("Copper", lane->getCopper())
              << LogVar("Data concentrator", lane->getDataConcentrator())
              << LogVar("Lane", lane->getLane()));
    }
    if (!m_WriteWrongHits)
      return;
    endcap = 0;
    layer = 0;
    sector = 0;
    correctHit = false;
  } else {
    m_ElementNumbers->sectorNumberToElementNumbers(
      *sectorGlobal, &endcap, &layer, &sector);
  }
  EKLMDigit* eklmDigit = m_eklmDigits.appendNew();
  eklmDigit->addRelationTo(klmDigitEventInfo);
  eklmDigit->setCTime(raw.ctime);
  eklmDigit->setTDC(raw.tdc);
  eklmDigit->setTime(
    m_TimeConversion->getTime(raw.ctime, raw.tdc,
                              klmDigitEventInfo->getTriggerCTime(), true));
  eklmDigit->setEndcap(endcap);
  eklmDigit->setLayer(layer);
  eklmDigit->setSector(sector);
  eklmDigit->setPlane(plane);
  eklmDigit->setStrip(strip);
  eklmDigit->setCharge(raw.charge);
  if (correctHit) {
    int stripGlobal = m_ElementNumbers->stripNumber(
                        endcap, layer, sector, plane, strip);
    const EKLMChannelData* channelData =
      m_Channels->getChannelData(stripGlobal);
    if (channelData == nullptr)
      B2FATAL("Incomplete EKLM channel data.");
    if (raw.charge < channelData->getThreshold())
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareSuccessfulFit);
    else
      eklmDigit->setFitStatus(KLM::c_ScintillatorFirmwareNoSignal);
  }
}

void KLMUnpackerModule::unpackBKLMDigit(
  const int* rawData, int copper, int hslb,
  KLMDigitEventInfo* klmDigitEventInfo)
{
  KLM::RawData raw;
  KLMDigitRaw* klmDigitRaw;
  KLM::unpackRawData(rawData, &raw, &m_klmDigitRaws, &klmDigitRaw, true);
  int electId = electCooToInt(copper - BKLM_ID, hslb,
                              raw.lane, raw.axis, raw.channel);
  int moduleId = 0;
  std::map<int, int>::iterator it;
  it = m_electIdToModuleId.find(electId);
  if (it == m_electIdToModuleId.end()) {
    B2DEBUG(20, "KLMUnpackerModule:: could not find in mapping"
            << LogVar("Copper", copper)
            << LogVar("Finesse", hslb + 1)
            << LogVar("Lane", raw.lane)
            << LogVar("Axis", raw.axis));
    if (!m_WriteWrongHits)
      return;
    /* Try to find element with the same module ID. */
    for (it = m_electIdToModuleId.begin(); it != m_electIdToModuleId.end();
         ++it) {
      /* Copper, finesse, and lane are 11 least-significant bits. */
      if ((it->first & 0x3FF) == electId) {
        // increase by 1 the event-counter of outOfRange-flagged hits
        klmDigitEventInfo->increaseOutOfRangeHits();

        // store the digit in the appropriate dataobject
        BKLMDigitOutOfRange* bklmDigitOutOfRange =
          m_bklmDigitOutOfRanges.appendNew(
            moduleId, raw.ctime, raw.tdc, raw.charge);
        bklmDigitOutOfRange->addRelationTo(klmDigitRaw);
        klmDigitEventInfo->addRelationTo(bklmDigitOutOfRange);

        std::string message = "channel number is out of range";
        m_rejected[message] += 1;
        m_rejectedCount++;
        B2DEBUG(21, "KLMUnpackerModule:: raw channel number is out of range"
                << LogVar("Channel", raw.channel));

        break;
      }
    }
    return;
  }
  moduleId = it->second;

  // moduleId counts are zero based
  int layer = (moduleId & BKLM_LAYER_MASK) >> BKLM_LAYER_BIT;
  if ((layer < 2) && ((raw.triggerBits & 0x10) != 0))
    return;
  // int sector = (moduleId & BKLM_SECTOR_MASK) >> BKLM_SECTOR_BIT;
  // int isForward = (moduleId & BKLM_END_MASK) >> BKLM_END_BIT;
  // int plane = (moduleId & BKLM_PLANE_MASK) >> BKLM_PLANE_BIT;
  int channel = (moduleId & BKLM_STRIP_MASK) >> BKLM_STRIP_BIT;

  if (layer > 14) {
    B2DEBUG(20, "KLMUnpackerModule:: strange that the layer number is larger than 14 "
            << LogVar("Layer", layer));
    return;
  }

  // still have to add channel and axis to moduleId
  if (layer > 1) {
    moduleId |= BKLM_INRPC_MASK;
    klmDigitEventInfo->increaseRPCHits();
  } else
    klmDigitEventInfo->increaseSciHits();
  // moduleId |= (((channel - 1) & BKLM_STRIP_MASK) << BKLM_STRIP_BIT) | (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);
  moduleId |= (((channel - 1) & BKLM_MAXSTRIP_MASK) << BKLM_MAXSTRIP_BIT);

  BKLMDigit* bklmDigit =
    m_bklmDigits.appendNew(moduleId, raw.ctime, raw.tdc, raw.charge);
  bklmDigit->setTime(
    m_TimeConversion->getTime(raw.ctime, raw.tdc,
                              klmDigitEventInfo->getTriggerCTime(),
                              layer <= 1));
  if (layer < 2 && (raw.charge < m_scintThreshold))
    bklmDigit->isAboveThreshold(true);

  bklmDigit->addRelationTo(klmDigitRaw);
  klmDigitEventInfo->addRelationTo(bklmDigit);
}

void KLMUnpackerModule::event()
{
  /*
   * Length of one hit in 4-byte words. This is needed to find the hits in the
   * detector buffer.
   */
  const int hitLength = 2;
  EKLMDataConcentratorLane lane;
  for (int i = 0; i < m_RawKLMs.getEntries(); i++) {
    if (m_RawKLMs[i]->GetNumEvents() != 1) {
      B2ERROR("RawKLM a wrong number of entries (should be 1)."
              << LogVar("RawKLM index", i)
              << LogVar("Number of entries", m_RawKLMs[i]->GetNumEvents()));
      continue;
    }
    /*
     * getNumEntries is defined in RawDataBlock.h and gives the
     * numberOfNodes*numberOfEvents. Number of nodes is num copper boards.
     */
    for (int j = 0; j < m_RawKLMs[i]->GetNumEntries(); j++) {
      unsigned int copperId = m_RawKLMs[i]->GetNodeID(j);
      bool eklmHit = false;
      if ((copperId >= EKLM_ID) && (copperId <= EKLM_ID + 4))
        eklmHit = true;
      else if (!((copperId >= BKLM_ID) && (copperId <= BKLM_ID + 4)))
        continue;
      uint16_t copperN = copperId - EKLM_ID;
      lane.setCopper(copperN);
      m_RawKLMs[i]->GetBuffer(j);
      for (int finesse_num = 0; finesse_num < 4; finesse_num++) {
        KLMDigitEventInfo* klmDigitEventInfo =
          m_DigitEventInfos.appendNew(m_RawKLMs[i], j);
        klmDigitEventInfo->setPreviousEventTriggerCTime(
          m_triggerCTimeOfPreviousEvent);
        m_triggerCTimeOfPreviousEvent = klmDigitEventInfo->getTriggerCTime();
        int numDetNwords = m_RawKLMs[i]->GetDetectorNwords(j, finesse_num);
        int* buf_slot    = m_RawKLMs[i]->GetDetectorBuffer(j, finesse_num);
        int numHits = numDetNwords / hitLength;
        lane.setDataConcentrator(finesse_num);
        if (numDetNwords % hitLength != 1 && numDetNwords != 0) {
          B2ERROR("Incorrect number of data words."
                  << LogVar("Number of data words", numDetNwords));
          if (!m_keepEvenPackages)
            continue;
        }
        if (numDetNwords > 0) {
          /*
           * In the last word there is the user word
           * (from data concentrators).
           */
          unsigned int userWord = (buf_slot[numDetNwords - 1] >> 16) & 0xFFFF;
          klmDigitEventInfo->setUserWord(userWord);
        } else {
          klmDigitEventInfo->setUserWord(0);
        }
        for (int iHit = 0; iHit < numHits; iHit++) {
          if (eklmHit) {
            unpackEKLMDigit(&buf_slot[iHit * hitLength], &lane,
                            klmDigitEventInfo);
          } else {
            unpackBKLMDigit(&buf_slot[iHit * hitLength], copperId, finesse_num,
                            klmDigitEventInfo);
          }
        }
      }
    }
  }
}

void KLMUnpackerModule::endRun()
{
}

void KLMUnpackerModule::terminate()
{
  for (const auto& message : m_rejected) {
    B2DEBUG(20, "KLMUnpackerModule:: " << message.first << " (occured " << message.second << " times)");
  }
}

void KLMUnpackerModule::loadMapFromDB()
{
  B2DEBUG(29, "KLMUnpackerModule:: reading from database...");

  DBArray<BKLMElectronicMapping> elements;
  for (const auto& element : elements) {
    B2DEBUG(29, "KLMUnpackerModule:: version = " << element.getBKLMElectronictMappingVersion() << ", copperId = " <<
            element.getCopperId() <<
            ", slotId = " << element.getSlotId() << ", axisId = " << element.getAxisId() << ", laneId = " << element.getLaneId() <<
            ", isForward = " << element.getIsForward() << " sector = " << element.getSector() << ", layer = " << element.getLayer() <<
            " plane(z/phi) = " << element.getPlane());

    int copperId = element.getCopperId();
    int slotId = element.getSlotId();
    int laneId = element.getLaneId();
    int axisId = element.getAxisId();
    int channelId = element.getChannelId();
    int sector = element.getSector();
    int isForward = element.getIsForward();
    int layer = element.getLayer();
    int plane =  element.getPlane();
    int stripId = element.getStripId();
    int elecId = electCooToInt(copperId - BKLM_ID, slotId - 1 , laneId, axisId, channelId);
    int moduleId = 0;

    moduleId = BKLMElementNumbers::channelNumber(isForward, sector, layer,
                                                 plane, stripId);
    m_electIdToModuleId[elecId] = moduleId;

    B2DEBUG(29, "KLMUnpackerModule:: electId: " << elecId << " moduleId: " << moduleId);
  }
}

int KLMUnpackerModule::electCooToInt(int copper, int finesse, int lane, int axis, int channel)
{
  // there are at most 16 coppers --> 4 bit
  // 4 finesse --> 2 bit
  // < 20 lanes --> 5 bit
  // axis --> 1 bit
  // channel --> 6 bit
  int ret = 0;
  copper = copper & 0xF;
  ret |= copper;
  finesse = finesse & 3;
  ret |= (finesse << 4);
  lane = lane & 0x1F;
  ret |= (lane << 6);
  axis = axis & 0x1;
  ret |= (axis << 11);
  channel = channel & 0x3F;
  ret |= (channel << 12);

  return ret;
}
