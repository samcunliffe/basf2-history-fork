/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2019 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <background/modules/BeamBkgHitRateMonitor/HitRateBase.h>
#include <framework/datastore/StoreArray.h>
#include <cdc/dataobjects/CDCHit.h>
#include <TTree.h>
#include <map>


namespace Belle2 {
  namespace Background {

    /**
     * Class for monitoring beam background hit rates of CDC
     */
    class CDCHitRateCounter: public HitRateBase {

    private:
      static const int f_nLayer = 56; /**< the number of layers*/
      static const int f_nSuperLayer = 9; /**< the number of super layers*/

    public:

      /**
       * tree structure
       */
      struct TreeStruct {
        float layerHitRate[f_nLayer] = {0}; /**< Layer average hit rate in kHz*/
        float superLayerHitRate[f_nSuperLayer] = {0}; /**< SuperLayer average hit rate in kHz */
        float averageRate = 0; /**< total detector average hit rate in KHz */
        int   timeWindowForSmallCell  = -1;/**< time window for the small cells in ns */
        int   timeWindowForNormalCell = -1;/**< time window for the normal cells in ns */

        int   nActiveWireInLayer[f_nLayer] = {0}; /**<  number of wires used in this analysis in each layer */
        int   nActiveWireInSuperLayer[f_nSuperLayer] = {0}; /**<  number of wires used in this analysis in each super layer*/
        int   nActiveWireInTotal = 0; /**<  number of wires used in this analysis in the whole CDC*/
        int   numEvents = 0; /**< number of events accumulated */
        bool  valid = false;  /**< status: true = rates valid */

        /**
         * normalize accumulated hits to hit rate in kHz
         */
        void normalize()
        {
          if (numEvents == 0) return;
          averageRate /= (numEvents * timeWindowForNormalCell * 1e-6);

          for (int iSL = 0 ; iSL < f_nSuperLayer ; ++iSL) {
            if (iSL == 0)
              superLayerHitRate[iSL] /= (numEvents * timeWindowForSmallCell * 1e-6);
            else
              superLayerHitRate[iSL] /= (numEvents * timeWindowForNormalCell * 1e-6);
          }

          for (int iL = 0 ; iL < f_nLayer ; ++iL) {
            if (0 <= iL && iL <= 7) ///// SL0
              layerHitRate[iL] /= (numEvents * timeWindowForSmallCell * 1e-6);
            else
              layerHitRate[iL] /= (numEvents * timeWindowForNormalCell * 1e-6);
          }
        }
      };

      /**
       * Constructor
       * @param timeWindowLowerEdge_smallCell lower edge of the timing window for the layers with small cells (SL0)
       * @param timeWindowUpperEdge_smallCell upper edge of the timing window for the layers with small cells (SL0)
       * @param timeWindowLowerEdge_normalCell lower edge of the timing window for the layers with normal cells (SL1-8)
       * @param timeWindowUpperEdge_normalCell upper edge of the timing window for the layers with normal cells (SL1-8)
       * @param enableBadWireTreatment flag to enable the bad wire treatment. default: true
       * @param enableBackgroundHitFilter flag to enable the CDC background hit (crosstakl, noise) filter. default: true
       * @param enableMarkBackgroundHit flag to enable to mark background flag on CDCHit (set 0x100 bit for CDCHit::m_status). default: false
       */
      CDCHitRateCounter(const int  timeWindowLowerEdge_smallCell,
                        const int  timeWindowUpperEdge_smallCell,
                        const int  timeWindowLowerEdge_normalCell,
                        const int  timeWindowUpperEdge_normalCell,
                        const bool enableBadWireTreatment,
                        const bool enableBackgroundHitFilter,
                        const bool enableMarkBackgroundHit):
        m_timeWindowLowerEdge_smallCell(timeWindowLowerEdge_smallCell),
        m_timeWindowUpperEdge_smallCell(timeWindowUpperEdge_smallCell),
        m_timeWindowLowerEdge_normalCell(timeWindowLowerEdge_normalCell),
        m_timeWindowUpperEdge_normalCell(timeWindowUpperEdge_normalCell),
        m_enableBadWireTreatment(enableBadWireTreatment),
        m_enableBackgroundHitFilter(enableBackgroundHitFilter),
        m_enableMarkBackgroundHit(enableMarkBackgroundHit)
      {
        if (m_timeWindowUpperEdge_smallCell  - m_timeWindowLowerEdge_smallCell  <= 0 ||
            m_timeWindowUpperEdge_normalCell - m_timeWindowLowerEdge_normalCell <= 0) {
          B2FATAL("invalid seting of CDC time window");
        }
      }
      /**
       * Class initializer: set branch addresses and other staf
       * @param tree a valid TTree pointer
       */
      virtual void initialize(TTree* tree) override;

      /**
       * Clear time-stamp buffer to prepare for 'accumulate'
       */
      virtual void clear() override;

      /**
       * Accumulate hits
       * @param timeStamp time stamp
       */
      virtual void accumulate(unsigned timeStamp) override;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       * @param timeStamp time stamp
       */
      virtual void normalize(unsigned timeStamp) override;

    private:

      // class parameters: to be set via constructor or setters

      // tree structure
      TreeStruct m_rates; /**< tree variables */

      // buffer
      std::map<unsigned, TreeStruct> m_buffer; /**< average rates in time stamps */

      // collections
      StoreArray<CDCHit> m_digits;  /**< collection of digits */

      ///// time window
      const int m_timeWindowLowerEdge_smallCell;  /**< lower edge of the time window for small cells [tdc count = ns] */
      const int m_timeWindowUpperEdge_smallCell;  /**< upper edge of the time window for small cells [tdc count = ns] */
      const int m_timeWindowLowerEdge_normalCell; /**< lower edge of the time window for normal cells [tdc count = ns] */
      const int m_timeWindowUpperEdge_normalCell; /**< upper edge of the time window for normal cells [tdc count = ns] */

      /**
       * return true if the hit is in the given time window
       * @param SL super layer ID which the wire of the hit belongs to
       * @param tdc TDC value of the hit
       */
      bool isInTimeWindow(const int SL, const short tdc)
      {
        if (SL == 0) {
          return (m_timeWindowLowerEdge_smallCell < tdc && tdc <= m_timeWindowUpperEdge_smallCell);
        } else {
          return (m_timeWindowLowerEdge_normalCell < tdc && tdc <= m_timeWindowUpperEdge_normalCell);
        }
      }

      // DB payloads

      // other
      const bool m_enableBadWireTreatment;    /**< flag to enable the bad wire treatment. default: true */
      const bool m_enableBackgroundHitFilter; /**< flag to enable the CDC background hit (crosstakl, noise) filter. default: true */
      const bool
      m_enableMarkBackgroundHit;   /**< flag to enable to mark background flag on CDCHit (set 0x100 bit for CDCHit::m_status). default: false */

      int m_nActiveWireInTotal = 0;                       /**< the number of wires used in this hit-rate calculation in the whole CDC */
      int m_nActiveWireInSuperLayer[f_nSuperLayer] = {0}; /**< the number of wires used in this hit-rate calculation in each suler layer */
      int m_nActiveWireInLayer[f_nLayer] = {0};           /**< the number of wires used in this hit-rate calculation in each layer */

      /**
       * set m_nActiveWireInTotal, m_nActiveWireInLayer[] and m_nActiveWireInSuperLayer[].
       * called in initialize function.
       * count the number of all the wires including dead wires (bad channels).
       */
      void countActiveWires_countAll();

      /**
       * set m_nActiveWireInTotal, m_nActiveWireInLayer[] and m_nActiveWireInSuperLayer[].
       * called in initialize function.
       * count the number of wires excluding dead wires (bad channels).
       */
      void countActiveWires();


    };

  } // Background namespace
} // Belle2 namespace
