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
#include <background/modules/BeamBkgHitRateMonitor/CDCHitRateCounter.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

#include <tracking/trackFindingCDC/rootification/StoreWrappedObjPtr.h>
#include <tracking/trackFindingCDC/eventdata/hits/CDCWireHit.h>

#include <cdc/geometry/CDCGeometryPar.h>
#include <cdc/dataobjects/WireID.h>

using namespace std;

namespace Belle2 {

  using TrackFindingCDC::StoreWrappedObjPtr;
  using TrackFindingCDC::CDCWireHit;

  namespace Background {

    void CDCHitRateCounter::initialize(TTree* tree)
    {
      // register collection(s) as optional, your detector might be excluded in DAQ
      m_digits.isOptional();

      // set branch address
      //tree->Branch("cdc", &m_rates, "averageRate/F:numEvents/I:valid/O");
      stringstream leafList;
      leafList
          << "layerHitRate[" << f_nLayer << "]/F:"
          << "superLayerHitRate[" << f_nSuperLayer << "]/F:"
          << "averageRate/F:"
          << "numEvents/I:"
          << "valid/O";
      tree->Branch("cdc", &m_rates, leafList.str().c_str());

      countActiveWires();
    }

    void CDCHitRateCounter::clear()
    {
      m_buffer.clear();
    }

    void CDCHitRateCounter::accumulate(unsigned timeStamp)
    {
      // check if data are available
      if (not m_digits.isValid()) return;

      // get buffer element
      auto& rates = m_buffer[timeStamp];

      // increment event counter
      rates.numEvents++;

      ///// get background flag as a map ( ASIC crosstalk, etc..? )
      std::map<const CDCHit*, bool> CDCHitToBackgroundFlag;
      {
        StoreWrappedObjPtr<std::vector<CDCWireHit>> storeVector("CDCWireHitVector");
        if (not storeVector) {
          B2FATAL("CDCWireHitVector is unaccessible in DataStore."
                  "Need TFCDC_WireHitParameter module before.");
        }
        const std::vector<CDCWireHit>& cdcWireHitVector = *storeVector;
        for (const auto& cdcWireHit : cdcWireHitVector) {
          const CDCHit* cdcHit = cdcWireHit.getHit();
          CDCHitToBackgroundFlag[cdcHit] = cdcWireHit->hasBackgroundFlag(); ///// cdcWireHit.getAutomatonCell().hasBackgroundFlag()
        }
      }

      CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      ///// getter functions of CDCHit:
      /////   unsigned short getID() /* encoded wire ID*/
      /////   unsigned short CDCHit::getICLayer() /* 0-55 */
      /////   unsigned short CDCHit::getISuperLayer() /* 0-8 */
      /////   short          CDCHit::getTDCCount()
      /////   unsigned short CDCHit::getADCCount() /* integrated charge over the cell */
      /////   unsigned short CDCHit::getTOT() /* time over threshold */
      for (CDCHit& hit : m_digits) {
        const WireID wireID(hit.getID());
        if (geometryPar.isBadWire(wireID))
          continue;

        const int iLayer         = hit.getICLayer();
        const int iSuperLayer    = hit.getISuperLayer();
        //const unsigned short adc = hit.getADCCount();

        //if (adc < 20) continue;
        if (CDCHitToBackgroundFlag[&hit]) {
          unsigned short newStatus = (hit.getStatus() | 0x100);
          hit.setStatus(newStatus);
          continue;
        }
        rates.layerHitRate[iLayer] += 1;
        rates.superLayerHitRate[iSuperLayer] += 1;
        rates.averageRate += 1;
      }

      // set flag to true to indicate the rates are valid
      rates.valid = true;
    }

    void CDCHitRateCounter::normalize(unsigned timeStamp)
    {
      // copy buffer element
      m_rates = m_buffer[timeStamp];

      if (not m_rates.valid) return;

      // normalize : nhit / nEvent in a second(time stamp) => # of hit in a event
      m_rates.normalize();

      // optionally: convert to MHz, correct for the masked-out channels etc.
      ///// (# of hit in a event) / nActiveWire => occupancy in an event
      ///// Total occupancy
      if (m_nActiveWireInTotal == 0) {
        m_rates.averageRate = 0;
      } else {
        m_rates.averageRate /= m_nActiveWireInTotal;
      }
      ///// SuperLayer occupancy
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL)
        if (m_nActiveWireInSuperLayer[iSL] == 0) {
          m_rates.superLayerHitRate[iSL] = 0;
        } else {
          m_rates.superLayerHitRate[iSL] /= m_nActiveWireInSuperLayer[iSL];
        }
      ///// Layer occupancy
      for (int iLayer = 0 ; iLayer < f_nLayer ; ++iLayer)
        if (m_nActiveWireInLayer[iLayer] == 0) {
          m_rates.layerHitRate[iLayer] = 0;
        } else {
          m_rates.layerHitRate[iLayer] /= m_nActiveWireInLayer[iLayer];
        }
    }


    void CDCHitRateCounter::countActiveWires_countAll()
    {
      const int nlayer_in_SL[f_nSuperLayer] = { 8, 6, 6,
                                                6, 6, 6,
                                                6, 6, 6
                                              };
      const int nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                  224, 256, 288,
                                                  320, 352, 384
                                                };

      m_nActiveWireInTotal = 0; ///// initialize
      int contLayerID = 0;//// continuous layer numbering
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        m_nActiveWireInSuperLayer[iSL] = 0; //// initialize
        for (int i = 0 ; i < nlayer_in_SL[iSL] ; ++i) {
          m_nActiveWireInLayer[contLayerID] = nwire_in_layer[iSL];
          m_nActiveWireInSuperLayer[iSL]   += nwire_in_layer[iSL];
          m_nActiveWireInTotal             += nwire_in_layer[iSL];
          ++contLayerID;
        }
      }
    }



    void CDCHitRateCounter::countActiveWires()
    {
      const int nlayer_in_SL[f_nSuperLayer] = { 8, 6, 6,
                                                6, 6, 6,
                                                6, 6, 6
                                              };
      const int nwire_in_layer[f_nSuperLayer] = { 160, 160, 192,
                                                  224, 256, 288,
                                                  320, 352, 384
                                                };

      CDC::CDCGeometryPar& geometryPar = CDC::CDCGeometryPar::Instance();

      m_nActiveWireInTotal = 0; ///// initialize
      int contLayerID = 0;//// continuous layer numbering

      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        m_nActiveWireInSuperLayer[iSL] = 0; //// initialize
        for (int iL = 0 ; iL < nlayer_in_SL[iSL] ; ++iL) { //// iL: layerID in SL
          m_nActiveWireInLayer[contLayerID] = 0;//// initialize
          for (int i = 0 ; i < nwire_in_layer[iSL] ; ++i) {
            WireID wireID(iSL, iL, i);
            if (not geometryPar.isBadWire(wireID))
              ++m_nActiveWireInLayer[contLayerID];
          }/// end i loop
          m_nActiveWireInSuperLayer[iSL] += m_nActiveWireInLayer[contLayerID];
          ++contLayerID;
        }/// end iL loop
        m_nActiveWireInTotal += m_nActiveWireInSuperLayer[iSL];
      }/// end iSL loop


      //B2INFO("CDC, # of Active wires / # of total wires");
      std::cout << "CDC, # of Active wires / # of total wires" << std::endl;
      int contLayerID_2 = 0;
      for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
        for (int iL = 0 ; iL < nlayer_in_SL[iSL] ; ++iL) {
          //B2INFO("Layer "<< contLayerID_2 << ": "
          std::cout << "Layer " << contLayerID_2 << ": "
                    << m_nActiveWireInLayer[contLayerID_2] << " / "
                    //<< nwire_in_layer[iSL] );
                    << nwire_in_layer[iSL] << std::endl;
          ++contLayerID_2;
        }
      }

    }


  } // Background namespace
} // Belle2 namespace

