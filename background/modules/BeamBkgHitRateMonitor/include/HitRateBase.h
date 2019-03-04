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

#include <TTree.h>

namespace Belle2 {
  namespace Background {

    /**
     * Abstract base class for monitoring beam background hit rates
     * All the monitoring classes must inherit from this one
     */
    class HitRateBase {

    public:

      /**
       * virtual destructor
       */
      virtual ~HitRateBase()
      {}

      /**
       * Class initializer: set branch address
       * @param tree TTree pointer
       */
      virtual void initialize(TTree* tree) = 0;

      /**
       * Clear the tree structure and other relevant variables to prepare for 'accumulate'
       */
      virtual void clear() = 0;

      /**
       * Accumulate hits
       */
      virtual void accumulate() = 0;

      /**
       * Normalize accumulated hits (e.g. transform to rates)
       */
      virtual void normalize() = 0;

    };

  } // Background namespace
} // Belle2 namespace
