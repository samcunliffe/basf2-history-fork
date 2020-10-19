/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

//abstract class:
#include <svd/reconstruction/SVDClusterPosition.h>
//derived classes:
#include <svd/reconstruction/SVDCoGOnlyPosition.h>
#include <svd/reconstruction/SVDOldDefaultPosition.h>

namespace Belle2 {

  namespace SVD {

    /**
     * Cluster Position Factory Class
     */
    class SVDRecoPositionFactory {

    public:

      /**
       * static function that returns the
       * class to compute the cluster position
       */
      static SVDClusterPosition* NewPosition(const std::string& description)
      {
        if (description == "CoGOnly")
          return new SVDCoGOnlyPosition;
        if (description == "oldDefault")
          return new SVDOldDefaultPosition;

        B2WARNING("the SVD cluster position algorithm is not recognized, using oldDefault!");

        return new SVDOldDefaultPosition;
      }
    };

  }

}

