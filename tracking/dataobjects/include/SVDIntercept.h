/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2013 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Giulia Casarosa                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <tracking/dataobjects/VXDIntercept.h>

namespace Belle2 {

  /** SVDIntercept stores the U,V coordinates and uncertainties of the intersection of a track
   *   with an SVD sensor.
   */
  class SVDIntercept : public VXDIntercept {

    //! Needed to make the ROOT object storable
    ClassDef(SVDIntercept, 1);
  };
}
