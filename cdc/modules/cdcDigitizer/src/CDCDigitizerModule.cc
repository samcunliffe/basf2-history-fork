/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Guofu Cao, Martin Heck                                   *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <cdc/modules/cdcDigitizer/CDCDigitizerModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationArray.h>
#include <framework/gearbox/Unit.h>
#include <framework/logging/Logger.h>

#include <generators/dataobjects/MCParticle.h>

#include <cdc/dataobjects/CDCHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <TRandom.h>

#include <utility>

using namespace std;
using namespace Belle2;
using namespace CDC;

// register module
REG_MODULE(CDCDigitizer)

CDCDigitizerModule::CDCDigitizerModule() : Module()
{
  // Set description
  setDescription("Creates CDCHits from CDCSimHits.");
  setPropertyFlags(c_ParallelProcessingCertified | c_InitializeInProcess);

  // Add parameters
  // I/O
  addParam("InputCDCSimHitsName",         m_inputCDCSimHitsName, "Name of input array. Should consist of CDCSimHits.", string(""));
  addParam("OutputCDCHitsName",           m_outputCDCHitsName,   "Name of output array. Will consist of CDCHits.",     string(""));

  //Relations
  addParam("MCParticlesToCDCSimHitsName",  m_MCParticlesToSimHitsName,
           "Name of relation between MCParticles and CDCSimHits used",     string(""));
  addParam("CDCSimHistToCDCHitsName",      m_SimHitsTOCDCHitsName,
           "Name of relation between the CDCSimHits and the CDCHits used", string(""));


  //Parameters for Digitization
  addParam("UseSimpleDigitization",       m_useSimpleDigitization,
           "If true, just float Gaussian smearing is used for the x-t function.", true);

  //float Gauss Parameters
  addParam("Fraction",                    m_fraction,    "Fraction of first Gaussian used to smear drift length in cm",    1.0);
  addParam("Mean1",                       m_mean1,       "Mean value of first Gaussian used to smear drift length in cm",  0.0000);
  //  addParam("Resolution1",                 m_resolution1, "Resolution of first Gaussian used to smear drift length in cm",  0.0010);
  addParam("Resolution1",                 m_resolution1, "Resolution of first Gaussian used to smear drift length in cm",  0.0130);
  addParam("Mean2",                       m_mean2,       "Mean value of second Gaussian used to smear drift length in cm", 0.0000);
  addParam("Resolution2",                 m_resolution2, "Resolution of second Gaussian used to smear drift length in cm", 0.0000);

  //Switches to control time information handling
  addParam("AddInWirePropagationDelay",   m_addInWirePropagationDelay,
           "A switch used to control adding propagation delay in the wire into the final drift time or not", false);
  addParam("AddTimeOfFlight",             m_addTimeOfFlight,
           "A switch used to control adding time of flight into the final drift time or not",                false);
  addParam("OutputNegativeDriftTime", m_outputNegativeDriftTime, "Output negative drift time", false);

  //TDC Threshold
  addParam("Threshold", m_tdcThreshold,
           "dEdx value for TDC Threshold in eV", 40.0);
  addParam("tMin", m_tMin, "Lower edge of time window in ns", -100.);
  addParam("tMaxOuter", m_tMaxOuter, "Upper edge of time window in ns for the outer layers", 500.);
  addParam("tMaxInner", m_tMaxInner, "Upper edge of time window in ns for the inner layers", 300.);
  // The following doesn't make any sense. The only reasonable steerable would be a switch to decide if the jitter shall be
  // activated. Then there has to be event by event jitter.
  /*  addParam("EventTime",                   m_eventTime,
             "It is a timing of event, which includes a time jitter due to the trigger system, set in ns",     float(0.0));*/

}

void CDCDigitizerModule::initialize()
{
  StoreArray<CDCSimHit>::required(m_inputCDCSimHitsName);

  // Register the arrays in the DataStore, that are to be added in this module.
  StoreArray<CDCHit>::registerPersistent(m_outputCDCHitsName);
  RelationArray::registerPersistent<CDCSimHit, CDCHit>(m_inputCDCSimHitsName, m_outputCDCHitsName);
  RelationArray::registerPersistent<MCParticle, CDCHit>("", m_outputCDCHitsName);
}

void CDCDigitizerModule::event()
{
  // Get SimHit array, MCParticle array, and relation between the two.
  StoreArray<CDCSimHit> simHits(m_inputCDCSimHitsName);

  StoreArray<MCParticle> mcParticles;                //needed to use the relations with MCParticles
  RelationArray mcParticlesToCDCSimHits(mcParticles, simHits);  //RelationArray created by CDC SensitiveDetector


  //--- Start Primitive Digitization --------------------------------------------------------------------------------------------
  if (m_useSimpleDigitization) {
    // Merge the hits in the same cell and save them into CDC signal map.

    // Define signal map
    map<WireID, SignalInfo> signalMap;
    map<WireID, SignalInfo>::iterator iterSignalMap;

    // Get instance of cdc geometry parameters
    CDCGeometryPar& cdcp = CDCGeometryPar::Instance();

    // Loop over all hits
    B2DEBUG(250, "Number of CDCSimHits in the current event: " << simHits.getEntries());
    int nHits = simHits.getEntries();
    for (int iHits = 0; iHits < nHits; ++iHits) {
      // Get a hit
      CDCSimHit* aCDCSimHit = simHits[iHits];

      // Hit geom. info
      WireID wireID        =   aCDCSimHit->getWireID();
      B2DEBUG(250, "Encoded wire number of current CDCSimHit: " << wireID);

      TVector3 hitPosWire  =   aCDCSimHit->getPosWire();
      TVector3 backWirePos =   cdcp.wireBackwardPosition(wireID);

      // Hit phys. info
      float hitdEdx        = aCDCSimHit->getEnergyDep()   * Unit::GeV;
      float hitDriftLength = aCDCSimHit->getDriftLength() * Unit::cm;
      float hitTOF         = aCDCSimHit->getFlightTime()  * Unit::ns;
      float dedxThreshold = m_tdcThreshold * Unit::eV;

      // If hitdEdx < dedxThreshold (default 40 eV), the hit is ignored
      // M. Uchida 2012.08.31
      //
      if (hitdEdx < dedxThreshold) {
        B2DEBUG(250, "Below Ethreshold: " << hitdEdx << " " << dedxThreshold);
        continue;
      }

      B2DEBUG(250, "Energy deposition: " << hitdEdx << ", DriftLength: " << hitDriftLength << ", TOF: " << hitTOF);

      // calculate measurement time.
      // calculate signal propagation length in the wire
      float propLength     = (hitPosWire - backWirePos).Mag();
      B2DEBUG(250, "Propagation in wire length: " << propLength);

      // smear drift length
      hitDriftLength        = smearDriftLength(hitDriftLength);
      float hitDriftTime   = getDriftTime(hitDriftLength, hitTOF, propLength);

      //add randamized event time for a beam bg. hit
      if (aCDCSimHit->getBackgroundTag() != 0) {
        hitDriftTime +=
          aCDCSimHit->getGlobalTime() - aCDCSimHit->getFlightTime();
      }

      //apply time window cut
      double tMax = m_tMaxOuter;
      if (wireID.getICLayer() < 8) tMax = m_tMaxInner;
      if (hitDriftTime < m_tMin || hitDriftTime > tMax) continue;

      //remove negative drift time upon request
      //      if (!m_outputNegativeDriftTime && hitDriftTime < 0.) continue;

      iterSignalMap = signalMap.find(wireID);

      if (iterSignalMap == signalMap.end()) {
        // new entry
        signalMap.insert(make_pair(wireID, SignalInfo(iHits, wireID, hitDriftTime, hitdEdx)));
        B2DEBUG(150, "Creating new Signal with encoded wire number: " << wireID);

      } else {
        // ... smallest drift time has to be checked, ...

        if (hitDriftTime < iterSignalMap->second.m_driftTime) {
          iterSignalMap->second.m_driftTime   = hitDriftTime;
          iterSignalMap->second.m_simHitIndex = iHits;
          B2DEBUG(250, "hitDriftTime of current Signal: " << hitDriftTime << ",  hitDriftLength: " << hitDriftLength);
        }
        // ... total charge has to be updated.
        iterSignalMap->second.m_charge += hitdEdx;
      }
    } // end loop over SimHits.

    //--- Now Store the results into CDCHits and
    // create corresponding relations between SimHits and CDCHits.

    unsigned int iCDCHits = 0;

    StoreArray<CDCHit> cdcHits(m_outputCDCHitsName);
    cdcHits.create();

    RelationArray cdcSimHitsToCDCHits(simHits, cdcHits); //SimHit<->CDCHit
    RelationArray mcParticlesToCDCHits(mcParticles, cdcHits); //MCParticle<->CDCHit

    for (iterSignalMap = signalMap.begin(); iterSignalMap != signalMap.end(); ++iterSignalMap) {

      //remove negative drift time (TDC) upon request

      if (!m_outputNegativeDriftTime &&
          iterSignalMap->second.m_driftTime < -0.5) {
        continue;
      }

      new(cdcHits.nextFreeAddress()) CDCHit(static_cast<unsigned short>((iterSignalMap->second.m_driftTime) + 0.5), getADCCount(iterSignalMap->second.m_charge),
                                            iterSignalMap->second.m_wireID);

      //add entry : CDCSimHit <-> CDCHit
      cdcSimHitsToCDCHits.add(iterSignalMap->second.m_simHitIndex, iCDCHits);

      const CDCHit* cdcHit = cdcHits[cdcHits.getEntries() - 1];
      RelationVector<MCParticle> rels = simHits[iterSignalMap->second.m_simHitIndex]->getRelationsFrom<MCParticle>();

      if (rels.size() != 0) {
        //assumption: only one MCParticle
        const MCParticle* mcparticle = rels[0];
        double weight = rels.weight(0);

        mcparticle->addRelationTo(cdcHit, weight);
      }

      iCDCHits++;
    }
  } else {
    B2FATAL("Proper digitization is not available.");
  }
}

float CDCDigitizerModule::smearDriftLength(float driftLength)
{
  float mean, resolution;
  if (gRandom->Uniform() <= m_fraction) {
    mean = m_mean1;
    resolution = m_resolution1;
  } else {
    mean = m_mean2;
    resolution = m_resolution2;
  }

  // Smear drift length
  float newDL = gRandom->Gaus(driftLength + mean , resolution);
  while (newDL <= 0.) newDL = gRandom->Gaus(driftLength + mean, resolution);
  return newDL;
}


float CDCDigitizerModule::getDriftTime(float driftLength, float tof, float propLength)
{
  //---------------------------------------------------------------------------------
  // Method returning electron drift time (parameters: position in cm)
  // T(drift) = TOF + T(true drift time) + T(propagation delay in wire) - T(event),
  // T(event) is a timing of event, which includes a time jitter due to
  // the trigger system.
  //---------------------------------------------------------------------------------
  float propagationDelayFactor = 1.0;
  float tofFactor = 1.0;

  if (!m_addInWirePropagationDelay)  propagationDelayFactor = 0;
  if (!m_addTimeOfFlight)            tofFactor = 0;

  //drift speed: 4.0cm/us, propagation speed: 27.25cm/ns, provided by iwasaki-san and hard-coded here.
  return (1000  * (driftLength / Unit::cm) / 4.0 + tofFactor * tof / Unit::ns + propagationDelayFactor * (propLength / Unit::cm) / 27.25 /*+ m_eventTime*/);
}

unsigned short CDCDigitizerModule::getADCCount(const float charge)
{
  // The current value is taken from E. Nakano from some test beam results. This should be a somewhat realistic value, but doesn't need to be exact.
  // Similar as geometry parameters are for the ideal geometry, not the real one.
  float conversionChargeToADC = (100.0 / 3.2) * 1e6;
  return static_cast<unsigned short>(conversionChargeToADC * charge);
}
