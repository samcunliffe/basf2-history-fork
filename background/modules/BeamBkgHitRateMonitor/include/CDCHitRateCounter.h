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

    public:

      /**
       * tree structure
       */
      struct TreeStruct {

        float averageRate = 0; /**< total detector average hit rate */
        int numEvents = 0; /**< number of events accumulated */
        bool valid = false;  /**< status: true = rates valid */

        /**
         * normalize accumulated hits to single event
         */
        void normalize()
        {
          if (numEvents == 0) return;
          averageRate /= numEvents;
        }

      };

      /**
       * Constructor
       */
      CDCHitRateCounter()
      {}

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

      // DB payloads

      // other

    };

  } // Background namespace
} // Belle2 namespace
