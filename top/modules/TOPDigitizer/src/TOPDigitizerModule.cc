/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/
// Own include
#include <top/modules/TOPDigitizer/TOPDigitizerModule.h>
#include <top/geometry/TOPGeometryPar.h>
#include <top/modules/TOPDigitizer/TimeDigitizer.h>

// Hit classes
#include <top/dataobjects/TOPSimHit.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPRecBunch.h>
#include <mdst/dataobjects/MCParticle.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

// ROOT
#include <TRandom.h>

#include <map>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register the Module
  //-----------------------------------------------------------------

  REG_MODULE(TOPDigitizer)


  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPDigitizerModule::TOPDigitizerModule() : Module()
  {
    // Set description()
    setDescription("Digitize TOPSimHits");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("timeZeroJitter", m_timeZeroJitter,
             "r.m.s of T0 jitter [ns]", 25e-3);
    addParam("darkNoise", m_darkNoise,
             "uniformly distributed dark noise (hits per module)", 0.0);
    addParam("trigT0Sigma", m_trigT0Sigma,
             "trigger T0 resolution [ns], if >0 trigger T0 will be simulated", 0.0);

  }

  TOPDigitizerModule::~TOPDigitizerModule()
  {
  }

  void TOPDigitizerModule::initialize()
  {
    // input
    StoreArray<TOPSimHit> simHits;
    simHits.isRequired();
    StoreArray<MCParticle> mcParticles;
    mcParticles.isOptional();

    // output
    StoreArray<TOPRawDigit> rawDigits;
    rawDigits.registerInDataStore();
    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();
    digits.registerRelationTo(simHits);
    digits.registerRelationTo(mcParticles);
    digits.registerRelationTo(rawDigits);
    StoreObjPtr<TOPRecBunch> recBunch;
    recBunch.registerInDataStore();

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // set pile-up and double hit resolution times (needed for BG overlay)
    TOPDigit::setDoubleHitResolution(geo->getNominalTDC().getDoubleHitResolution());
    TOPDigit::setPileupTime(geo->getNominalTDC().getPileupTime());

    // bunch time separation
    if (m_trigT0Sigma > 0) {
      m_bunchTimeSep = geo->getNominalTDC().getBunchSeparationTime();
    }
  }

  void TOPDigitizerModule::beginRun()
  {

  }

  void TOPDigitizerModule::event()
  {

    // input: simulated hits
    StoreArray<TOPSimHit> simHits;

    // output: digitized hits
    StoreArray<TOPDigit> digits;
    StoreArray<TOPRawDigit> rawDigits;

    // output: simulated bunch values
    StoreObjPtr<TOPRecBunch> recBunch;
    if (!recBunch.isValid()) recBunch.create();

    const auto* geo = TOPGeometryPar::Instance()->getGeometry();

    // simulate trigger T0 accuracy in finding the right bunch crossing
    double trigT0 = 0;
    if (m_trigT0Sigma > 0) {
      trigT0 = gRandom->Gaus(0., m_trigT0Sigma);
      int relBunchNo = round(trigT0 / m_bunchTimeSep);
      trigT0 = relBunchNo * m_bunchTimeSep;
      recBunch->setSimulated(relBunchNo, trigT0);
    }

    // simulate start time (bunch time given by trigger smeared according to T0 jitter)
    double startTime = gRandom->Gaus(trigT0, m_timeZeroJitter);

    // pixels with time digitizers
    std::map<unsigned, TimeDigitizer> pixels;
    typedef std::map<unsigned, TimeDigitizer>::iterator Iterator;

    // add simulated hits
    double electronicEfficiency = geo->getNominalTDC().getEfficiency();
    const auto& tts = geo->getNominalTTS();
    for (const auto& simHit : simHits) {
      // simulate electronic efficiency
      if (gRandom->Rndm() > electronicEfficiency) continue;

      // Do spatial digitization
      double x = simHit.getX();
      double y = simHit.getY();
      int pmtID = simHit.getPmtID();
      int pixelID = geo->getPMTArray().getPixelID(x, y, pmtID);
      if (pixelID == 0) continue;

      // add TTS to photon time and make it relative to start time
      double time = simHit.getTime() + tts.generateTTS() - startTime;

      // add time to digitizer of a given pixel
      TimeDigitizer digitizer(simHit.getModuleID(), pixelID);
      unsigned id = digitizer.getUniqueID();
      Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
      it->second.addTimeOfHit(time, &simHit);
    }

    // add randomly distributed dark noise
    if (m_darkNoise > 0) {
      int numModules = geo->getNumModules();
      int numPixels = geo->getPMTArray().getNumPixels();
      double timeMin = geo->getNominalTDC().getTimeMin();
      double timeMax = geo->getNominalTDC().getTimeMax();
      for (int moduleID = 1; moduleID <= numModules; moduleID++) {
        int numHits = gRandom->Poisson(m_darkNoise);
        for (int i = 0; i < numHits; i++) {
          int pixelID = int(gRandom->Rndm() * numPixels) + 1;
          double time = (timeMax - timeMin) * gRandom->Rndm() + timeMin;
          TimeDigitizer digitizer(moduleID, pixelID);
          unsigned id = digitizer.getUniqueID();
          Iterator it = pixels.insert(pair<unsigned, TimeDigitizer>(id, digitizer)).first;
          it->second.addTimeOfHit(time);
        }
      }
    }

    // digitize in time
    double electronicJitter = geo->getNominalTDC().getTimeJitter();
    for (auto& pixel : pixels) {
      pixel.second.digitize(digits, electronicJitter);
    }

    // convert to raw digits (gaussian pulse shape assumed!)
    // to be moved to TimeDigitizer!!!
    const auto& tdc = geo->getNominalTDC();
    const auto& frontEndMapper = TOPGeometryPar::Instance()->getFrontEndMapper();
    const auto& channelMapper = TOPGeometryPar::Instance()->getChannelMapper();

    for (auto& digit : digits) {
      double time = digit.getTime();
      double width = digit.getPulseWidth();
      double height = digit.getADC();
      double mean = time + width / 2;
      double sigma = width / 2.35482;
      digit.setHitQuality(TOPDigit::c_Junk);

      int sampleRise = tdc.getSample(time);
      if (!tdc.isSampleValid(sampleRise)) continue;
      short vRise0 = height * gauss(tdc.getSampleTime(sampleRise), mean, sigma);
      short vRise1 = height * gauss(tdc.getSampleTime(sampleRise + 1), mean, sigma);

      int sampleFall = tdc.getSample(time + width);
      if (!tdc.isSampleValid(sampleFall + 1)) continue;
      short vFall0 = height * gauss(tdc.getSampleTime(sampleFall), mean, sigma);
      short vFall1 = height * gauss(tdc.getSampleTime(sampleFall + 1), mean, sigma);

      int samplePeak = tdc.getSample(mean);
      short vPeak = height * gauss(tdc.getSampleTime(samplePeak), mean, sigma);
      short vPeak1 = height * gauss(tdc.getSampleTime(samplePeak + 1), mean, sigma);
      if (vPeak1 > vPeak) {
        vPeak = vPeak1;
        samplePeak++;
      }

      auto moduleID = digit.getModuleID();
      auto channel = digit.getChannel();
      unsigned bs = 0;
      unsigned carrier = 0;
      unsigned asic = 0;
      unsigned chan = 0;
      channelMapper.splitChannelNumber(channel, bs, carrier, asic, chan);
      const auto* map = frontEndMapper.getMap(moduleID, bs);
      if (!map) continue;
      auto scrodID = map->getScrodID();
      auto* rawDigit = rawDigits.appendNew(scrodID);
      rawDigit->setCarrierNumber(carrier);
      rawDigit->setASICNumber(asic);
      rawDigit->setASICChannel(chan);
      rawDigit->setASICWindow(digit.getFirstWindow());
      rawDigit->setTFine(0);
      rawDigit->setSampleRise(sampleRise);
      rawDigit->setDeltaSamplePeak(samplePeak - sampleRise);
      rawDigit->setDeltaSampleFall(sampleFall - sampleRise);
      rawDigit->setValueRise0(vRise0);
      rawDigit->setValueRise1(vRise1);
      rawDigit->setValueFall0(vFall0);
      rawDigit->setValueFall1(vFall1);
      rawDigit->setValuePeak(vPeak);
      rawDigit->setIntegral(digit.getIntegral());

      double cfdTime = rawDigit->getCFDLeadingTime() * tdc.getSampleWidth()
                       - tdc.getOffset();
      double cfdWidth = rawDigit->getFWHM() * tdc.getSampleWidth();
      int TDCcount = tdc.getTDCcount(cfdTime);

      /*
      cout<< digit.getTime() << " " << cfdTime << " ";
      cout<< digit.getPulseWidth() << " "<< cfdWidth << " ";
      cout<< digit.getTDC() << " " << TDCcount << " ";
      cout<< digit.getADC() << " " << vPeak << endl;
      */

      digit.setTime(cfdTime);
      digit.setPulseWidth(cfdWidth);
      digit.setTDC(TDCcount);
      digit.setADC(vPeak);
      digit.setHitQuality(TOPDigit::c_Good);
    }

  }


  void TOPDigitizerModule::endRun()
  {

  }

  void TOPDigitizerModule::terminate()
  {

  }


} // end Belle2 namespace

