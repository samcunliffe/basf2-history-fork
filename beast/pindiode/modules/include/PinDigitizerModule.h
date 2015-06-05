/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Igal Jaegle                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef PINDIGITIZERMODULE_H
#define PINDIGITIZERMODULE_H

#include <framework/core/Module.h>
#include <string>
#include <vector>

//ROOT
#include <TRandom3.h>
#include <TF1.h>
#include <TVector3.h>

/** size of hit */
const Int_t MAXSIZE         = 10000;
/** size of pixel hit */
const Int_t MAXtSIZE        = 1000;

namespace Belle2 {
  namespace pindiode {
    /**
     * Pin tube digitizer
     *
     * Creates PinHits from PinSimHits
     *
     */
    class PinDigitizerModule : public Module {

    public:

      /**
       * Constructor: Sets the description, the properties and the parameters of the module.
       */
      PinDigitizerModule();

      /**  */
      virtual ~PinDigitizerModule();

      /**  */
      virtual void initialize();

      /**  */
      virtual void beginRun();

      /**  */
      virtual void event();

      /**  */
      virtual void endRun();
      /**  */
      virtual void terminate();


    private:

      /** reads data from PINDIODE.xml: tube location, drift data filename, sigma of impulse response function */
      virtual void getXMLData();

      /** number of detectors. Read from PINDIODE.xml*/
      int nPIN = 0;
      /** PIN coordinate */
      std::vector<TVector3> PINCenter;
      /** Cremat gain */
      double m_CrematGain;
      /** Event counter */
      int Event = 0;
    };

  }
}

#endif /* PINDIGITIZERMODULE_H */
