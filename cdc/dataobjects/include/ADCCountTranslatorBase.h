/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#pragma once

#include <cdc/dataobjects/WireID.h>

#include <TMath.h>

namespace Belle2 {
  namespace CDC {
    /** Abstract Base class for the ADC count translator. */
    class ADCCountTranslatorBase {
    public:
      /** Constructor. */
      ADCCountTranslatorBase() {}

      /** Destructor is virtual, as this is supposed to be a base class. */
      virtual ~ADCCountTranslatorBase() {};

      /** Function, for which this actually was meant.
       *
       *  @param adcCount  ADC count as measured from the hardware. This is the sum of several flash-ADC measurements.
       *  @param wireID    WireID, that uniquely identifies the wire for which the current translation is to be done.
       *  @param ambiguityDiscriminator  Information to resolve left/right ambiguity.
       *  @param z         z-position of the point, where the charge hit the wire.
       *  @param theta     Angle under which the track moves in the drift cell in r-phi direction.
       */
      virtual float getCharge(unsigned short adcCount = 0,
                              const WireID&  wireID = WireID(),
                              bool ambiguityDiscriminator = false,
                              float z = 0, float theta = static_cast<float>(TMath::Pi() / 2.)) = 0;
    };
  }
}
