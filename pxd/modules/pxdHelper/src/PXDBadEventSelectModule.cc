/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Bjoern Spruck                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <pxd/modules/pxdHelper/PXDBadEventSelectModule.h>
// #include <framework/datastore/DataStore.h>


using namespace std;
using namespace Belle2;
using namespace Belle2::PXD;
using namespace Belle2::PXD::PXDError;


REG_MODULE(PXDBadEventSelect)


PXDBadEventSelectModule::PXDBadEventSelectModule() :
  Module()
{
  //Set module properties
  setDescription("Mark bad-data PXD modules");
  setPropertyFlags(c_ParallelProcessingCertified);
}

void PXDBadEventSelectModule::initialize()
{
  //Register output collections
  m_storeRawHits.isRequired(m_PXDRawHitsName);
  m_storeDAQEvtStats.isRequired(m_PXDDAQEvtStatsName);

  m_cut = {
    {VxdID("1.1.1"), 100},
    {VxdID("1.1.2"), 100},
    {VxdID("1.2.1"), 100},
    {VxdID("1.2.2"), 100},
    {VxdID("1.3.1"), 100},

    {VxdID("1.3.2"), 100},
    {VxdID("1.4.1"), 100},
    {VxdID("1.4.2"), 200},
    {VxdID("1.5.1"), 100},
    {VxdID("1.5.2"), 140},

    {VxdID("1.6.1"), 100},
    {VxdID("1.6.2"), 100},
    {VxdID("1.7.1"), 100},
    {VxdID("1.7.2"), 220},
    {VxdID("1.8.1"), 100},

    {VxdID("1.8.2"), 100},
    {VxdID("2.4.1"), 100},
    {VxdID("2.4.2"), 250},
    {VxdID("2.5.1"), 200},
    {VxdID("2.5.2"), 200}
  };
}

void PXDBadEventSelectModule::event()
{
  std::map <VxdID, int> freq;
  for (auto& p : m_storeRawHits) {
    // if c.getSensorID() == a and c.getCharge()>15 and c.getCharge()<250]
    freq[p.getSensorID()]++;
  }

  for (auto& p : *m_storeDAQEvtStats) {
    for (auto& c : p) {
      for (auto& e : c) {
        if (freq[e.getSensorID()] > m_cut[e.getSensorID()]) {
          e.markUnusable();
        }
      }
    }
  }
}
